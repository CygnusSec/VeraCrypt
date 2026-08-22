#include "EncryptionModuleApi.h"
#include "ExternalCipherAdapter.h"

VC_ENCRYPTION_MODULE_EXPORT const VeraCryptEncryptionModuleV1 *
VeraCryptGetEncryptionModuleV1(void)
{
  const VeraCryptExternalCipherAdapterV1 *adapter = &VeraCryptExternalCipherAdapter;
  static VeraCryptEncryptionModuleV1 module;
  static int initialized;

  if (adapter->struct_size < sizeof(VeraCryptExternalCipherAdapterV1)
      || adapter->magic != VC_EXTERNAL_CIPHER_ADAPTER_MAGIC
      || adapter->adapter_version != VC_EXTERNAL_CIPHER_ADAPTER_API_V1)
    return 0;

  if (!initialized)
  {
    module.struct_size = sizeof(module);
    module.abi_version = VC_ENCRYPTION_MODULE_ABI_V1;
    module.module_id = adapter->module_id;
    module.display_name = adapter->display_name;
    module.block_size = adapter->block_size;
    module.key_size = adapter->key_size;
    module.context_size = adapter->context_size;
    module.self_test = adapter->self_test;
    module.set_key = adapter->set_key;
    module.encrypt_block = adapter->encrypt_block;
    module.decrypt_block = adapter->decrypt_block;
    initialized = 1;
  }

  return &module;
}
