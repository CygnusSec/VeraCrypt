#ifndef VERACRYPT_EXTERNAL_CIPHER_ADAPTER_H
#define VERACRYPT_EXTERNAL_CIPHER_ADAPTER_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VC_EXTERNAL_CIPHER_ADAPTER_MAGIC 0x56434131u
#define VC_EXTERNAL_CIPHER_ADAPTER_API_V1 1u

typedef struct VeraCryptExternalCipherAdapterV1
{
  uint32_t struct_size;
  uint32_t magic;
  uint32_t adapter_version;
  const char *module_id;
  const char *display_name;
  uint32_t block_size;
  uint32_t key_size;
  uint32_t context_size;
  int (*self_test)(void);
  int (*set_key)(void *context, const uint8_t *key, size_t key_size);
  int (*encrypt_block)(const void *context, const uint8_t *input, uint8_t *output);
  int (*decrypt_block)(const void *context, const uint8_t *input, uint8_t *output);
} VeraCryptExternalCipherAdapterV1;

extern const VeraCryptExternalCipherAdapterV1 VeraCryptExternalCipherAdapter;

#ifdef __cplusplus
}
#endif

#endif
