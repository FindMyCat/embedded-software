/*
 * Copyright (c) 2020–2021 Qorvo, Inc
 *
 * All rights reserved.
 *
 * NOTICE: All information contained herein is, and remains the property
 * of Qorvo, Inc. and its suppliers, if any. The intellectual and technical
 * concepts herein are proprietary to Qorvo, Inc. and its suppliers, and
 * may be covered by patents, patent applications, and are protected by
 * trade secret and/or copyright law. Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written
 * permission is obtained from Qorvo, Inc.
 *
 */

#ifndef MCPS_CRYPTO_H
#define MCPS_CRYPTO_H

#include <stdint.h>
#include <uwbmac/uwbmac_error.h>

#define MCPS_CRYPTO_AES_CCM_STAR_NONCE_LEN 13

/**
 * mcps_crypto_cmac_aes_128_digest() - Compute a cmac AES 128.
 * @key: AES key.
 * @data: Input data.
 * @data_len: Input data length in bytes.
 * @out: Output hash, with length AES_BLOCK_SIZE.
 *
 * NOTE: This API should be implemented by platform.
 *
 * Return:
 * 	UWBMAC_SUCCESS on success.
 * 	UWBMAC_ENOTSUP if feature not available.
 * 	UWBMAC_EINVAL if bad input data.
 * 	UWBMAC_ENOMEM if memory allocation.
 *	UWBMAC_EBADMSG if other error.
 */
uwbmac_error mcps_crypto_cmac_aes_128_digest(const uint8_t *key,
					     const uint8_t *data,
					     unsigned int data_len,
					     uint8_t *out);

/**
 * mcps_crypto_aead_aes_ccm_star_128_create() - Create a context using
 * Authenticated Encryption Associated Data with AES CCM* 128.
 * @key: AES key.
 *
 * NOTE: This API should be implemented by platform.
 *
 * Return: The pointer to the context that will be used to encrypt & decrypt.
 */
void *mcps_crypto_aead_aes_ccm_star_128_create(const uint8_t *key);

/**
 * mcps_crypto_aead_aes_ccm_star_128_destroy() - Destroy the Authenticated
 * Encryption Associated Data with AES CCM* 128 context.
 * @ctx: Context.
 *
 * NOTE: This API should be implemented by platform.
 */
void mcps_crypto_aead_aes_ccm_star_128_destroy(void *ctx);

/**
 * mcps_crypto_aead_aes_ccm_star_128_encrypt() - Encrypt using Authenticated
 * Encryption Associated Data with AES CCM* 128.
 * @ctx: Context.
 * @nonce: Nonce, with length MCPS_CRYPTO_AES_CCM_STAR_NONCE_LEN.
 * @header: Header data.
 * @header_len: Header length in bytes.
 * @data: Data to encrypt, will be replaced with encrypted data.
 * @data_len: Data length in bytes.
 * @mac: AES CCM* MAC.
 * @mac_len: AES CCM* MAC size in bytes.
 *
 * NOTE: This API should be implemented by platform.
 *
 * Return:
 * 	UWBMAC_SUCCESS on success.
 * 	UWBMAC_ENOTSUP if feature not available.
 * 	UWBMAC_EINVAL if bad input data.
 * 	UWBMAC_ENOMEM if memory allocation.
 *	UWBMAC_EBADMSG if other error.
 */
uwbmac_error mcps_crypto_aead_aes_ccm_star_128_encrypt(
	void *ctx, const uint8_t *nonce, const uint8_t *header,
	unsigned int header_len, uint8_t *data, unsigned int data_len,
	uint8_t *mac, unsigned int mac_len);

/**
 * mcps_crypto_aead_aes_ccm_star_128_decrypt() - Decrypt using Authenticated
 * Encryption Associated Data with AES CCM* 128.
 * @ctx: Context.
 * @nonce: Nonce, with length MCPS_CRYPTO_AES_CCM_STAR_NONCE_LEN.
 * @header: Header data.
 * @header_len: Header length in bytes.
 * @data: Data to decrypt, will be replaced with decrypted data.
 * @data_len: Data length in bytes.
 * @mac: AES CCM* MAC.
 * @mac_len: AES CCM* MAC size in bytes.
 *
 * NOTE: This API should be implemented by platform. In case of mismatch
 * between the MAC and calculated MAC, this function should return UWBMAC_EBADMSG.
 *
 * Return:
 * 	UWBMAC_SUCCESS on success.
 * 	UWBMAC_ENOTSUP if feature not available.
 * 	UWBMAC_EINVAL if bad input data.
 * 	UWBMAC_ENOMEM if memory allocation.
 *	UWBMAC_EBADMSG if other error.
 */
uwbmac_error mcps_crypto_aead_aes_ccm_star_128_decrypt(
	void *ctx, const uint8_t *nonce, const uint8_t *header,
	unsigned int header_len, uint8_t *data, unsigned int data_len,
	uint8_t *mac, unsigned int mac_len);

/**
 * mcps_crypto_aes_ecb_128_create() - Create a context using AES ECB 128.
 * @key: AES key.
 *
 * NOTE: This API should be implemented by platform.
 *
 * Return: The pointer to the context that will be used to encrypt & decrypt.
 */
void *mcps_crypto_aes_ecb_128_create(const uint8_t *key);

/**
 * mcps_crypto_aes_ecb_128_destroy() - Destroy the AES ECB 128 context.
 * @ctx: Context.
 *
 * NOTE: This API should be implemented by platform.
 */
void mcps_crypto_aes_ecb_128_destroy(void *ctx);

/**
 * mcps_crypto_aes_ecb_128_encrypt() - Encrypt using AES ECB 128.
 * @ctx: Context.
 * @data: Data to encrypt.
 * @data_len: Data length in bytes, should be a multiple of AES_BLOCK_SIZE.
 * @out: Ciphered data with same length as data.
 *
 * NOTE: This API should be implemented by platform.
 *
 * Return:
 * 	UWBMAC_SUCCESS on success.
 * 	UWBMAC_ENOTSUP if feature not available.
 * 	UWBMAC_EINVAL if bad input data.
 * 	UWBMAC_ENOMEM if memory allocation.
 *	UWBMAC_EBADMSG if other error.
 */
uwbmac_error mcps_crypto_aes_ecb_128_encrypt(void *ctx, const uint8_t *data,
					     unsigned int data_len,
					     uint8_t *out);

#endif /* MCPS_CRYPTO_H */
