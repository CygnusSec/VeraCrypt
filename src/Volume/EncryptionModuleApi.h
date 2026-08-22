#ifndef VERACRYPT_ENCRYPTION_MODULE_API_H
#define VERACRYPT_ENCRYPTION_MODULE_API_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VC_ENCRYPTION_MODULE_ABI_V1 1u
#define VC_ENCRYPTION_MODULE_ENTRY_POINT "VeraCryptGetEncryptionModuleV1"

typedef struct VeraCryptEncryptionModuleV1
{
	uint32_t struct_size;
	uint32_t abi_version;
	const char *module_id;
	const char *display_name;
	uint32_t block_size;
	uint32_t key_size;
	uint32_t context_size;
	int (*self_test)(void);
	int (*set_key)(void *context, const uint8_t *key, size_t key_size);
	int (*encrypt_block)(const void *context, const uint8_t *input, uint8_t *output);
	int (*decrypt_block)(const void *context, const uint8_t *input, uint8_t *output);
} VeraCryptEncryptionModuleV1;

typedef const VeraCryptEncryptionModuleV1 *(*VeraCryptGetEncryptionModuleV1Fn)(void);

#if defined(_WIN32)
#define VC_ENCRYPTION_MODULE_EXPORT __declspec(dllexport)
#else
#define VC_ENCRYPTION_MODULE_EXPORT __attribute__((visibility("default")))
#endif

VC_ENCRYPTION_MODULE_EXPORT const VeraCryptEncryptionModuleV1 *
VeraCryptGetEncryptionModuleV1(void);

#ifdef __cplusplus
}
#endif

#endif
