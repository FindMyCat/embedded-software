/**
 * @file      default_config.h
 *
 * @brief     Default config file for NVM initialization
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

#ifndef __DEFAULT_CONFIG_H__H__
#define __DEFAULT_CONFIG_H__H__ 1

#ifdef __cplusplus
 extern "C" {
#endif

#include "net/fira_region_params.h"
#include "deca_device_api.h"
#include "fira_helper.h"
#include "app.h"

extern const app_definition_t idle_app[];

/* UWB config */
#define DEFAULT_CHANNEL             (9)
#define DEFAULT_TXPREAMBLENGTH      (DWT_PLEN_64)
#define DEFAULT_RXPAC               (DWT_PAC8)
#define DEFAULT_PCODE               (11)
#define DEFAULT_NSSFD               (DWT_SFD_IEEE_4Z)
#define DEFAULT_DATARATE            (DWT_BR_6M8)
#define DEFAULT_PHRMODE             (DWT_PHRMODE_STD)
#define DEFAULT_PHRRATE             (DWT_PHRRATE_STD)
#define DEFAULT_SFDTO               (64 + 1 + 8 - 8)
#define DEFAULT_STS_MODE            (DWT_STS_MODE_OFF)
#define DEFAULT_STS_LENGTH          (DWT_STS_LEN_64)
#define DEFAULT_PDOA_MODE           (DWT_PDOA_M0)

/* run-time config */
#define DEFAULT_ANTD                (513.484f * 1e-9 / DWT_TIME_UNITS) /* Mean Total Antenna Delay for DWM3001C module */
#define DEFAULT_RNGOFF_MM           (0) /* Can be used to offset Range */
#define DEFAULT_REPORT_LEVEL        (1)

#define FCONFIG_SIZE                (0x200)

#ifdef __cplusplus
}
#endif

#endif /* __DEFAULT_CONFIG__H__ */

