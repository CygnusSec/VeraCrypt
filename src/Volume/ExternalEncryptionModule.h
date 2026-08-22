#ifndef TC_HEADER_ExternalEncryptionModule
#define TC_HEADER_ExternalEncryptionModule

#include "Platform/Platform.h"
#include "EncryptionModuleApi.h"

namespace VeraCrypt
{
	class ExternalEncryptionModule
	{
	public:
		~ExternalEncryptionModule ();

		const VeraCryptEncryptionModuleV1 &GetDescriptor () const { return *Descriptor; }
		wstring GetName () const;
		static const vector < shared_ptr <ExternalEncryptionModule> > &GetAvailableModules ();

	private:
		ExternalEncryptionModule (void *libraryHandle, const VeraCryptEncryptionModuleV1 *descriptor);
		static void LoadDirectory (const string &path, vector < shared_ptr <ExternalEncryptionModule> > &modules);

		void *LibraryHandle;
		const VeraCryptEncryptionModuleV1 *Descriptor;
	};
}

#endif
