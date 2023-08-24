/**
 * @file      mcps_crypto.c
 *
 * @brief     Implementation of crypto functionalities
 *
 * @author    Decawave Applications
 *
 * @attention Copyright (c) 2021 - 2022, Qorvo US, Inc.
 * All rights reserved
 * Redistribution and use in source and binary forms, with or without modification,
 *  are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this
 *  list of conditions, and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 * 3. You may only use this software, with or without any modification, with an
 *  integrated circuit developed by Qorvo US, Inc. or any of its affiliates
 *  (collectively, "Qorvo"), or any module that contains such integrated circuit.
 * 4. You may not reverse engineer, disassemble, decompile, decode, adapt, or
 *  otherwise attempt to derive or gain access to the source code to any software
 *  distributed under this license in binary or object code form, in whole or in
 *  part.
 * 5. You may not use any Qorvo name, trademarks, service marks, trade dress,
 *  logos, trade names, or other symbols or insignia identifying the source of
 *  Qorvo's products or services, or the names of any of Qorvo's developers to
 *  endorse or promote products derived from this software without specific prior
 *  written permission from Qorvo US, Inc. You must not call products derived from
 *  this software "Qorvo", you must not have "Qorvo" appear in their name, without
 *  the prior permission from Qorvo US, Inc.
 * 6. Qorvo may publish revised or new version of this license from time to time.
 *  No one other than Qorvo US, Inc. has the right to modify the terms applicable
 *  to the software provided under this license.
 * THIS SOFTWARE IS PROVIDED BY QORVO US, INC. "AS IS" AND ANY EXPRESS OR IMPLIED
 *  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. NEITHER
 *  QORVO, NOR ANY PERSON ASSOCIATED WITH QORVO MAKES ANY WARRANTY OR
 *  REPRESENTATION WITH RESPECT TO THE COMPLETENESS, SECURITY, RELIABILITY, OR
 *  ACCURACY OF THE SOFTWARE, THAT IT IS ERROR FREE OR THAT ANY DEFECTS WILL BE
 *  CORRECTED, OR THAT THE SOFTWARE WILL OTHERWISE MEET YOUR NEEDS OR EXPECTATIONS.
 * IN NO EVENT SHALL QORVO OR ANYBODY ASSOCIATED WITH QORVO BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 *
 */

#include "mcps_crypto.h"
#include "nrf_crypto_aes.h"
#include "nrf_crypto_aead.h"
#include "uwbmac_error.h"
#include "uwbmac.h"

static int nrf_crypto_error_to_std_error(ret_code_t rc)
{
    switch (rc)
    {
    case NRF_SUCCESS:
        return UWBMAC_SUCCESS;
    case NRF_ERROR_CRYPTO_ALLOC_FAILED:
    case NRF_ERROR_CRYPTO_STACK_OVERFLOW:
        return UWBMAC_ENOMEM;
    case NRF_ERROR_CRYPTO_AEAD_INVALID_MAC:
        return UWBMAC_EBADMSG;
    case NRF_ERROR_CRYPTO_BUSY:
        return UWBMAC_EBUSY;
    case NRF_ERROR_CRYPTO_FEATURE_UNAVAILABLE:
        return UWBMAC_ENOTSUP;
    default:
        return UWBMAC_EINVAL;
    }
}

uwbmac_error mcps_crypto_cmac_aes_128_digest(const uint8_t *key, const uint8_t *data,
                                             unsigned int data_len, uint8_t *out)
{
    /* Init cbc_mac context */
    nrf_crypto_aes_context_t cmac_ctx;
    size_t out_len = 128;
    ret_code_t rc = nrf_crypto_aes_init(&cmac_ctx, &g_nrf_crypto_aes_cmac_128_info,
                                        NRF_CRYPTO_MAC_CALCULATE);
    if (rc != NRF_SUCCESS)
    {
        return nrf_crypto_error_to_std_error(rc);
    }
    /* Set encryption key */
    rc = nrf_crypto_aes_key_set(&cmac_ctx, (uint8_t *)key);
    if (rc != NRF_SUCCESS)
    {
        return nrf_crypto_error_to_std_error(rc);
    }
    /* Calculate MAC */
    rc = nrf_crypto_aes_finalize(&cmac_ctx, (uint8_t *)data, data_len,
                                 out, &out_len);
    if (rc != NRF_SUCCESS)
    {
        return nrf_crypto_error_to_std_error(rc);
    }
    /* Free memory */
    rc = nrf_crypto_aes_uninit(&cmac_ctx);
    if (rc != NRF_SUCCESS)
    {
        return nrf_crypto_error_to_std_error(rc);
    }
    return UWBMAC_SUCCESS;
}

void *mcps_crypto_aead_aes_ccm_star_128_create(const uint8_t *key)
{
    nrf_crypto_aead_context_t *ccm_ctx;
    ccm_ctx = malloc(sizeof(nrf_crypto_aead_context_t));
    ret_code_t rc = nrf_crypto_aead_init(ccm_ctx,
                                         &g_nrf_crypto_aes_ccm_128_info,
                                         (uint8_t *)key);
    if (rc != NRF_SUCCESS)
    {
        return NULL;
    }
    return ccm_ctx;
}

void mcps_crypto_aead_aes_ccm_star_128_destroy(void *ctx)
{
    nrf_crypto_aead_context_t *ccm_ctx = ctx;
    ret_code_t rc = nrf_crypto_aead_uninit(ccm_ctx);
    if (rc != NRF_SUCCESS)
    {
        return;
    }
    free(ccm_ctx);
}

uwbmac_error mcps_crypto_aead_aes_ccm_star_128_encrypt(
    void *ctx, const uint8_t *nonce, const uint8_t *header, unsigned int header_len,
    uint8_t *data, unsigned int data_len, uint8_t *mac, unsigned int mac_len)
{
    nrf_crypto_aead_context_t *ccm_ctx = ctx;
    ret_code_t rc = nrf_crypto_aead_crypt(ccm_ctx, NRF_CRYPTO_ENCRYPT,
                                          (uint8_t *)nonce, MCPS_CRYPTO_AES_CCM_STAR_NONCE_LEN,
                                          (uint8_t *)header, header_len,
                                          data, data_len, data, mac, mac_len);
    if (rc != NRF_SUCCESS)
    {
        return nrf_crypto_error_to_std_error(rc);
    }
    return UWBMAC_SUCCESS;
}

uwbmac_error mcps_crypto_aead_aes_ccm_star_128_decrypt(
    void *ctx, const uint8_t *nonce, const uint8_t *header, unsigned int header_len,
    uint8_t *data, unsigned int data_len, uint8_t *mac, unsigned int mac_len)
{
    nrf_crypto_aead_context_t *ccm_ctx = ctx;
    ret_code_t rc = nrf_crypto_aead_crypt(ccm_ctx, NRF_CRYPTO_DECRYPT,
                                          (uint8_t *)nonce, MCPS_CRYPTO_AES_CCM_STAR_NONCE_LEN,
                                          (uint8_t *)header, header_len,
                                          data, data_len, data, mac, mac_len);
    if (rc != NRF_SUCCESS)
    {
        return nrf_crypto_error_to_std_error(rc);
    }
    return UWBMAC_SUCCESS;
}

struct nrf_ecb_128_context
{
    nrf_crypto_aes_context_t nrf_ctx;
    uint8_t key[NRF_CRYPTO_KEY_SIZE_128 / 8];
};

void *mcps_crypto_aes_ecb_128_create(const uint8_t *key)
{
    struct nrf_ecb_128_context *ecb_ctx;
    ecb_ctx = malloc(sizeof(*ecb_ctx));

    memcpy(ecb_ctx->key, key, sizeof(ecb_ctx->key));

    return ecb_ctx;
}

void mcps_crypto_aes_ecb_128_destroy(void *ctx)
{
    struct nrf_ecb_128_context *ecb_ctx = ctx;

    free(ecb_ctx);
}

uwbmac_error mcps_crypto_aes_ecb_128_encrypt(void *ctx, const uint8_t *data,
                                             unsigned int data_len, uint8_t *out)
{
    struct nrf_ecb_128_context *ecb_ctx = ctx;
    size_t out_len = data_len;
    ret_code_t rc;

    rc = nrf_crypto_aes_crypt(&ecb_ctx->nrf_ctx, &g_nrf_crypto_aes_ecb_128_info,
                              NRF_CRYPTO_ENCRYPT,
                              ecb_ctx->key, NULL,
                              (uint8_t *)data, data_len,
                              out, &out_len);
    if (rc != NRF_SUCCESS)
    {
        return nrf_crypto_error_to_std_error(rc);
    }
    return UWBMAC_SUCCESS;
}
