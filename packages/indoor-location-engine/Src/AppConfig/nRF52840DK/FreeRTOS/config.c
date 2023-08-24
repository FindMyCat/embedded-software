/*
 * @file      config.c
 *
 * @brief     supports NVM and bss configuration sections:
 *            defaultFConfig : section in RAM, where default parameters are saved and is not re-writabele.
 *             FCONFIG_ADDR  : section in NVM, where current parameters are saved and this is re-writabele.
 *                bssConfig  : section in RAM, which is representing config data exist in FCONFIG_ADDR.
 *
 *            application on startup shall load_bssConfig() : this will copy data from FCONFIG_ADDR -> tmpConfig
 *            Accessing to variables is by pointer get_pbssConfig();
 *
 *            if application wants to re-write data in FCONFIG_ADDR, use save_bssConfig(*newRamParametersBlock);
 *
 *            NOTE: The code is very MCU dependent and save will work with nRF52840 only
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
 */

#include <stdint.h>
#include <string.h>

#include "deca_device_api.h"
#include "rtls_version.h"

#include "appConfig.h"
#include "nrf_nvmc.h"
#include "crc16.h"

//------------------------------------------------------------------------------
extern uint8_t __config_entry_start[];
extern uint8_t __config_entry_end[];

extern uint8_t __fconfig_start[];

extern uint8_t __rconfig_start[];
extern uint8_t __rconfig_end[];
extern uint8_t __rconfig_crc_end[];

__attribute__((section(".rconfig_crc"))) static uint16_t config_crc = 0;
__attribute__((section(".fconfig"))) const uint32_t DummyConfig[1]  = {0xDEADBEEF};

/* @fn      load_bssConfig
 * @brief   copy parameters from NVM to RAM structure.
 *
 *          assumes that memory model in the MCU of .text and .bss are the same
 * */
void load_bssConfig(void)
{
    uint16_t rconfig_len = (uint16_t)((uint8_t *)&__rconfig_end - (uint8_t *)&__rconfig_start);
    uint16_t rconfig_len_with_crc = (uint16_t)((uint8_t *)&__rconfig_crc_end - (uint8_t *)&__rconfig_start);

    memcpy((uint8_t *)&__rconfig_start, (uint8_t *)&__fconfig_start, rconfig_len_with_crc);
    uint16_t crc = calc_crc16((uint8_t *)&__rconfig_start, rconfig_len);
    if (crc != config_crc)
    {
        restore_bssConfig();
    }
}

/* @fn      restore_bssConfig
 * @brief   copy parameters from default RAM section to RAM structure.
 *
 *          assumes that memory model in the MCU of .text and .bss are the same
 * */
void restore_bssConfig(void)
{
    // Copy default to RAM
    for (uint32_t *ptr = (uint32_t *)&__config_entry_start; ptr < (uint32_t *)&__config_entry_end; ptr++)
    {
        void (*restore)(void) = (void (*)(void))(*ptr);
        restore();
    }
    save_bssConfig();
}

/* @brief    save pNewRamParametersBlock to FCONFIG_ADDR
 * @return  _NO_ERR for success and error_e code otherwise
 * */
error_e save_bssConfig(void)
{
    uint16_t rconfig_len = (uint16_t)((uint8_t *)&__rconfig_end - (uint8_t *)&__rconfig_start);
    uint16_t rconfig_len_with_crc = (uint16_t)((uint8_t *)&__rconfig_crc_end - (uint8_t *)&__rconfig_start);

    config_crc = calc_crc16((uint8_t *)&__rconfig_start, rconfig_len);
    nrf_nvmc_page_erase((uint32_t)&__fconfig_start);
    nrf_nvmc_write_bytes((uint32_t)&__fconfig_start, (const uint8_t *)__rconfig_start, rconfig_len_with_crc);

    return (_NO_ERR);
}
