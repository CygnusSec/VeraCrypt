#include "ExternalEncryptionModule.h"

#ifndef TC_WINDOWS
#include <dirent.h>
#include <dlfcn.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef TC_MACOSX
#include <mach-o/dyld.h>
#endif
#endif

namespace VeraCrypt
{
#ifndef TC_WINDOWS
	static string GetExecutableDirectory ()
	{
		char path[PATH_MAX];
#ifdef TC_MACOSX
		uint32_t pathSize = sizeof (path);
		if (_NSGetExecutablePath (path, &pathSize) != 0)
			return string();
#else
		ssize_t pathSize = readlink ("/proc/self/exe", path, sizeof (path) - 1);
		if (pathSize <= 0)
			return string();
		path[pathSize] = 0;
#endif
		string executablePath (path);
		size_t separator = executablePath.find_last_of ('/');
		return separator == string::npos ? string() : executablePath.substr (0, separator);
	}
#endif

	ExternalEncryptionModule::ExternalEncryptionModule (
		void *libraryHandle, const VeraCryptEncryptionModuleV1 *descriptor)
		: LibraryHandle (libraryHandle), Descriptor (descriptor)
	{
	}

	ExternalEncryptionModule::~ExternalEncryptionModule ()
	{
#ifndef TC_WINDOWS
		if (LibraryHandle)
			dlclose (LibraryHandle);
#endif
	}

	wstring ExternalEncryptionModule::GetName () const
	{
		return StringConverter::ToWide (Descriptor->display_name);
	}

	void ExternalEncryptionModule::LoadDirectory (
		const string &path, vector < shared_ptr <ExternalEncryptionModule> > &modules)
	{
#ifndef TC_WINDOWS
		DIR *directory = opendir (path.c_str());
		if (!directory)
			return;

		struct dirent *entry;
		while ((entry = readdir (directory)) != nullptr)
		{
			string fileName (entry->d_name);
#ifdef TC_MACOSX
			const string extension = ".dylib";
#else
			const string extension = ".so";
#endif
			if (fileName.size() <= extension.size()
				|| fileName.compare (fileName.size() - extension.size(), extension.size(), extension) != 0)
				continue;

			string modulePath = path + "/" + fileName;
			void *handle = dlopen (modulePath.c_str(), RTLD_NOW | RTLD_LOCAL);
			if (!handle)
				continue;

			VeraCryptGetEncryptionModuleV1Fn getModule =
				reinterpret_cast<VeraCryptGetEncryptionModuleV1Fn> (dlsym (handle, VC_ENCRYPTION_MODULE_ENTRY_POINT));
			const VeraCryptEncryptionModuleV1 *descriptor = getModule ? getModule() : nullptr;
			bool valid = descriptor
				&& descriptor->struct_size >= sizeof (VeraCryptEncryptionModuleV1)
				&& descriptor->abi_version == VC_ENCRYPTION_MODULE_ABI_V1
				&& descriptor->module_id && descriptor->module_id[0]
				&& descriptor->display_name && descriptor->display_name[0]
				&& descriptor->block_size == 16
				&& descriptor->key_size == 32
				&& descriptor->context_size > 0
				&& descriptor->self_test && descriptor->set_key
				&& descriptor->encrypt_block && descriptor->decrypt_block
				&& descriptor->self_test() == 1;

			for (size_t i = 0; valid && i < modules.size(); ++i)
				if (strcmp (modules[i]->GetDescriptor().module_id, descriptor->module_id) == 0)
					valid = false;

			if (valid)
				modules.push_back (shared_ptr <ExternalEncryptionModule> (
					new ExternalEncryptionModule (handle, descriptor)));
			else
				dlclose (handle);
		}
		closedir (directory);
#endif
	}

	const vector < shared_ptr <ExternalEncryptionModule> > &ExternalEncryptionModule::GetAvailableModules ()
	{
		static vector < shared_ptr <ExternalEncryptionModule> > modules;
		static bool loaded = false;
		if (loaded)
			return modules;
		loaded = true;

#ifndef TC_WINDOWS
		const char *developmentPath = getenv ("VERACRYPT_ENCRYPTION_MODULE_PATH");
		if (developmentPath && developmentPath[0])
		{
			string paths (developmentPath);
			size_t start = 0;
			while (start <= paths.size())
			{
				size_t separator = paths.find (':', start);
				string path = paths.substr (start, separator - start);
				if (!path.empty())
					LoadDirectory (path, modules);
				if (separator == string::npos)
					break;
				start = separator + 1;
			}
		}

#ifdef TC_MACOSX
		string executableDirectory = GetExecutableDirectory ();
		if (!executableDirectory.empty())
			LoadDirectory (executableDirectory + "/../Resources/EncryptionModules", modules);
		LoadDirectory ("/Library/Application Support/VeraCrypt/EncryptionModules", modules);
#else
		string executableDirectory = GetExecutableDirectory ();
		if (!executableDirectory.empty())
			LoadDirectory (executableDirectory + "/../lib/veracrypt/encryption-modules", modules);
		LoadDirectory ("/usr/local/lib/veracrypt/encryption-modules", modules);
		LoadDirectory ("/usr/lib/veracrypt/encryption-modules", modules);
#endif
#endif
		return modules;
	}
}
