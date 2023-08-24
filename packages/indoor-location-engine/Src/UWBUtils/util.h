/**
 * @file    util.h
 *
 * @brief   Decawave Application Layer utility functions & Macros
 *
 * @author Decawave Applications
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
 */

#ifndef __UTIL__H__
#define __UTIL__H__ 1

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include "deca_device_api.h"

#define AR2U32(x) (((uint32_t)x[3]) << 24 | ((uint32_t)x[2]) << 16 | ((uint32_t)x[1]) << 8 | ((uint32_t)x[0]))

#define AR2U16(x) ((x[1] << 8) | x[0])


#define TS2U64_MEMCPY(x, y)                                                                                                      \
    do                                                                                                                           \
    {                                                                                                                            \
        x = (uint64_t)(((uint64_t)y[4] << 32) | ((uint64_t)y[3] << 24) | ((uint64_t)y[2] << 16) | ((uint64_t)y[1] << 8) | y[0]); \
    } while (0)

#define TS2TS_MEMCPY(x, y)                    \
    do                                        \
    {                                         \
        for (int i = 0; i < TS_40B_SIZE; i++) \
        {                                     \
            x[i] = y[i];                      \
        }                                     \
    } while (0)

#define TS2TS_UWB_MEMCPY(x, y)                \
    do                                        \
    {                                         \
        for (int i = 0; i < TS_UWB_SIZE; i++) \
        {                                     \
            x[i] = y[i];                      \
        }                                     \
    } while (0)

#define U642TS_MEMCPY(x, y)                          \
    do                                               \
    {                                                \
        for (int i = 0; i < TS_40B_SIZE; i++)        \
        {                                            \
            x[i] = (uint8_t)((y >> (i * 8) & 0xFF)); \
        }                                            \
    } while (0)

#define U642TS_UWB_MEMCPY(x, y)                      \
    do                                               \
    {                                                \
        for (int i = 0; i < TS_UWB_SIZE; i++)        \
        {                                            \
            x[i] = (uint8_t)((y >> (i * 8) & 0xFF)); \
        }                                            \
    } while (0)

#define U32TOAR_MEMCPY(x, y)                         \
    do                                               \
    {                                                \
        for (int i = 0; i < 4; i++)                  \
        {                                            \
            x[i] = (uint8_t)((y >> (i * 8) & 0xFF)); \
        }                                            \
    } while (0)

uint64_t util_us_to_dev_time(double us);
double util_dev_time_to_sec(uint64_t dt);
uint64_t util_sec_to_dev_time(double sec);
double util_us_to_sy(double us);

int16_t calc_sfd_to(void *pCfg);
uint16_t calc_rx_to_sy(void *p, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* __UTIL__H__ */
