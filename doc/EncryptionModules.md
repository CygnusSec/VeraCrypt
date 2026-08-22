# External encryption modules

VeraCrypt can discover private block ciphers at runtime through the versioned
C ABI declared in `src/Volume/EncryptionModuleApi.h`. Cipher source code does
not need to be compiled into or distributed with the VeraCrypt source tree.

## ABI v1 requirements

An ABI v1 module is a dynamic library that exports
`VeraCryptGetEncryptionModuleV1` and returns one immutable
`VeraCryptEncryptionModuleV1` descriptor.

The first ABI version intentionally accepts only:

- 16-byte blocks
- 32-byte keys
- an XTS encryption algorithm with one cipher

The module supplies its context size, a mandatory known-answer self-test, key
setup, and single-block encryption/decryption callbacks. VeraCrypt allocates
the context in a `SecureBuffer`; allocation never crosses the ABI boundary.
Use a globally unique, permanent `module_id`. Changing the implementation in
a way that changes ciphertext requires a new ID and display name.

## Discovery

Production modules are loaded from:

- macOS: `/Library/Application Support/VeraCrypt/EncryptionModules`
- Linux: `/usr/local/lib/veracrypt/encryption-modules` and
  `/usr/lib/veracrypt/encryption-modules`

For development only, set a colon-separated search path:

```sh
export VERACRYPT_ENCRYPTION_MODULE_PATH=/path/to/module/build
./src/Main/VeraCrypt
```

Use the `.dylib` extension on macOS and `.so` on Linux. A library is
ignored if its ABI, parameters, callbacks, self-test, or module ID is invalid.
Duplicate IDs are ignored after the first valid module.

## Building and embedding modules from source

Each private module is a source tree with a `src` directory. VeraCrypt compiles
all `.c`, `.cc`, and `.cpp` files directly inside it. The ABI adapter must be
named `src/veracrypt_cipher_adapter.c`; headers may use any name. The private module
does not need a build script or checked-in binary artifacts.

On macOS, pass the source-tree root with `-m`:

```sh
./src/Build/build_veracrypt_macosx.sh -b -f -p -m /path/to/algorithm
```

Repeat `-m` to build and embed multiple algorithms:

```sh
./src/Build/build_veracrypt_macosx.sh -b -f -p \
  -m /path/to/algorithm -m /path/to/another-cipher
```

VeraCrypt builds each module in a temporary directory and embeds it at
`VeraCrypt.app/Contents/Resources/EncryptionModules`. The modules are then
included in the PKG and DMG.

On Linux, the regular and no-SSE2 build scripts accept the same repeatable `-m` flag and embed the generated `.so` in the installer and AppImage:

```sh
sudo ./src/Build/build_veracrypt_linux.sh -m /path/to/algorithm
sudo ./src/Build/build_veracrypt_linux_no_sse2.sh -m /path/to/algorithm
```

For a module-only development build, use:

```sh
./src/Build/build_external_encryption_modules.sh /tmp/modules /path/to/algorithm
```

Linux packages and AppImages place modules in `usr/lib/veracrypt/encryption-modules`; FreeBSD packages use `usr/local/lib/veracrypt/encryption-modules`.

### Windows static-link build

Windows volume I/O runs inside the signed `veracrypt.sys` kernel driver, so
the Unix dynamic-library loader is not used. Instead, one private external
cipher source tree is compiled into Mount, Format, ExpandVolume, and Driver in
the same solution build:

```powershell
.\src\Build\build_veracrypt_windows.ps1 `
  -ExternalModule C:\private\mkc4 `
  -ExternalCipherName MKC4B `
  -Configuration Release `
  -Platform x64
```

The equivalent direct MSBuild properties are:

```powershell
msbuild .\src\VeraCrypt.sln /m /t:Build `
  /p:Configuration=Release /p:Platform=x64 `
  /p:VCExternalModuleSourceDir=C:\private\mkc4 `
  /p:VCExternalCipherName=MKC4B
```

The Windows v1 integration supports exactly one static external cipher per
build. Its adapter must be `src\veracrypt_cipher_adapter.c`, its context must
not exceed 4096 bytes, and its ABI self-test must pass. It is available only
for non-system volumes; boot/system encryption remains unchanged. The
resulting `.exe` and `veracrypt.sys` files must be signed and distributed
together because all consumers must use the same cipher registry.

## Operational requirements

Every process that creates, mounts, changes the password of, or repairs a
volume must have the same module installed. VeraCrypt volume headers do not
store an algorithm ID; header decryption tries all currently available
algorithms. If a required module is missing, the volume cannot be opened.

Loading a module executes native code inside VeraCrypt and its core service.
Only install modules from a trusted publisher. Distribute and sign private
modules independently from the public VeraCrypt build.
