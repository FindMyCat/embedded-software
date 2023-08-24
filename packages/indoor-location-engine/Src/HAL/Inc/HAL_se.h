/**
 * @file      HAL_se.h
 *
 * @brief     Header for HAL for SE access
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

#ifndef HAL_SE_H
#define HAL_SE_H

#include <stdint.h>

#ifdef __ZEPHYR__
#include <sys/byteorder.h>
#define be16_to_cpu sys_be16_to_cpu
#define cpu_to_be32 sys_cpu_to_be32
#endif

#define AES_KEYSIZE_128             16
#define AES_KEYSIZE_256             32

#define APDU_STATUS_WORD_SIZE       2
#define APDU_MAX_RESPONSE_SIZE      256

#define CLA_COMMAND_ISO             0x00
#define CLA_COMMAND_GPC             0x80
#define CLA_NO_SM                   0x00
#define CLA_GPC_SM                  0x04
#define CLA_ISO_SM                  0x08
#define CLA_ISO_SM_CMAC             0x0C
#define CLA_LOGICAL_CHANNEL(ch)     ((ch) & 0x03)

typedef enum
{
    SE_ERROR_STATE_UNCHANGED                = 0x6200,
    SE_ERROR_NO_SPECIFIC_DIAGNOSIS          = 0x6400,
    SE_ERROR_WRONG_LENGTH                   = 0x6700,
    SE_ERROR_INVALID_LOGICAL_CHANNEL        = 0x6881,
    SE_ERROR_SECURITY_STATUS_NOT_SATISFIED  = 0x6982,
    SE_ERROR_CONDITION_OF_USE_NOT_SATISFIED = 0x6985,
    SE_ERROR_INCORRECT_P1_P2                = 0x6A86,
    SE_ERROR_SESSION_ID_NOT_FOUND           = 0x6A82,
    SE_ERROR_REFERENCE_DATA_NOT_FOUND       = 0x6A88,
    SE_ERROR_INVALID_INSTRUCTION            = 0x6D00,
    SE_ERROR_INVALID_CLASS                  = 0x6E00,
    SE_NO_ERROR                             = 0x9000,
} se_status_t;

typedef enum
{
    TAG_APPLET_BUILD_INFO        = 0x80, // Context-specific class / Primitive data object
    TAG_URSK                     = 0xC0, // Private class / Primitive data object: UWB Ranging Session Key
    TAG_RESPONDER_SUBSESSION_KEY = 0xC1,
    TAG_PROXIMITY_DISTANCE       = 0xC2,
    TAG_ANGLE_OF_ARRIVAL         = 0xC3,
    TAG_CLIENT_SPECIFIC_DATA     = 0xC4,
    TAG_KEY_AGREEMENT_KID        = 0xC6,
    TAG_SERVICE_APPLET_AID       = 0xCE,
    TAG_UWB_SESSION_ID           = 0xCF,
    TAG_DATA_PROTECTION_KEY      = 0xD0,
    TAG_PRIVACY_KEY              = 0xD1,
} se_tag_t;

typedef enum
{
    COMMAND_MANAGE_TEST_RDS = 0x0B,
    COMMAND_GET_RDS         = 0x40,
    COMMAND_ERASE_RDS       = 0x41,
    COMMAND_MANAGE_CHANNEL  = 0x70,
    COMMAND_SELECT          = 0xA4,
    COMMAND_LOAD            = 0xA8,
    COMMAND_GET_DATA        = 0xCA,
    COMMAND_GET_DATA_ODD    = 0xCB,
    COMMAND_PUT_KEY         = 0xD8,
    COMMAND_STORE_DATA      = 0xE2,
    COMMAND_DELETE          = 0xE4,
    COMMAND_INSTALL         = 0xE6,
    COMMAND_SET_STATUS      = 0xF0,
    COMMAND_GET_STATUS      = 0xF2,
} se_command_t;

/**
 * @fn void se_init(void)
 *
 * @brief   HAL SE initialisation
 *
 * Init the SPI link to SE and init the t1 library
 */
void se_init(void);


/**
 * @fn int se_gto_apdu_transmit(const void *apdu, int n, void *resp, int r)
 *
 * @brief   send and receive apdu frame to SE
 *
 * check parameters and send/receive through T1 protocol.
 *
 * @param apdu pointer on the apdu frame to send
 *
 * @param n lenght of apdu frame
 *
 * @param resp pointer on the response buffer
 *
 * @param r length of the response
 *
 * @return length of the response (r) or -1 in case of error ( response too short or protocol fail)
 */
int se_gto_apdu_transmit(const void *apdu, int n, void *resp, int r);

#endif /* HAL_SE_H */