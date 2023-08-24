/**
 * @file      rf_tuning_config.h
 *
 * @brief     Driver config file for NVM initialization
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
#include <string.h>

#include "rf_tuning_config.h"

#include "deca_device_api.h"
#include "default_config.h"


#define DEFAULT_ANTD    (513.484f * 1e-9 / DWT_TIME_UNITS) /*Total antenna delay*/
#define DEFAULT_PDOAOFF 0                                  /**< Phase Differences offset */
#define DEFAULT_RNGOFF  0                                  /**< Ranging offset */

#define DEFAULT_ANTENNA_TYPE  ANT_TYPE_CUSTOM

#define DEFAULT_TX_POWER_REGULAR_PULSE_AOA       0xa6a6a6a6UL
#define DEFAULT_TX_POWER_REGULAR_PULSE_NON_AOA   0x8a8a8a8aUL
#define DEFAULT_TX_POWER_ALTERNATE_PULSE_AOA     0x93939393UL
#define DEFAULT_TX_POWER_ALTERNATE_PULSE_NON_AOA 0x63636363UL

static const rf_tuning_t rf_tuning_config_flash_default =
{
    .txConfig.PGdly   = 0x27,
    .txConfig.power   = DEFAULT_TX_POWER_REGULAR_PULSE_NON_AOA,
    .txConfig.PGcount = 0,
    .pdoaOffset_deg   = DEFAULT_PDOAOFF,
    .rngOffset_mm     = DEFAULT_RNGOFF,
    .antRx_a          = (uint16_t)(0.5 * DEFAULT_ANTD),
    .antTx_a          = (uint16_t)(0.5 * DEFAULT_ANTD),
    .antRx_b          = (uint16_t)(0.5 * DEFAULT_ANTD),
    .antenna          = { DEFAULT_ANTENNA_TYPE, DEFAULT_ANTENNA_TYPE, 
                          ANT_TYPE_NONE,        ANT_TYPE_NONE },
    .xtalTrim         = (DEFAULT_XTAL_TRIM),
    .paverage         = 10,
    //QM35725 specific
    .tx_ant           = 1,  //select ANT1 as TX
    .rxa_ant          = 3,  //select ANT3 as RX, PATH A
    .rxb_ant          = 2,  //select ANT2 as, RX PATH B
    .lna1             = 0,  //BYPASS
    .lna2             = 0,  //BYPASS
    .pa               = 0   //LOW PA
};

static rf_tuning_t rf_tuning_config_ram __attribute__((section(".rconfig"))) = {0};

rf_tuning_t *get_rf_tuning_config(void) {
    return &rf_tuning_config_ram;
}

dwt_txconfig_t *get_dwt_txconfig(void) {
    return &rf_tuning_config_ram.txConfig;
}

void rf_tuning_set_tx_power_pg_delay(uint32_t dev_id)
{
    switch (dev_id)
    {
    case DWT_DW3000_PDOA_DEV_ID:
        rf_tuning_config_ram.txConfig.power = DEFAULT_TX_POWER_REGULAR_PULSE_AOA;
        rf_tuning_config_ram.txConfig.PGdly = 0x34;
        break;
    case DWT_DW3000_DEV_ID:
        rf_tuning_config_ram.txConfig.power = DEFAULT_TX_POWER_REGULAR_PULSE_NON_AOA;
        rf_tuning_config_ram.txConfig.PGdly = 0x34;
        break;
    case DWT_QM33120_PDOA_DEV_ID:
        rf_tuning_config_ram.txConfig.power = DEFAULT_TX_POWER_ALTERNATE_PULSE_AOA;
        rf_tuning_config_ram.txConfig.PGdly = 0x27;
        break;
    case DWT_QM33110_DEV_ID:
        rf_tuning_config_ram.txConfig.power = DEFAULT_TX_POWER_ALTERNATE_PULSE_NON_AOA;
        rf_tuning_config_ram.txConfig.PGdly = 0x27;
        break;
    default:
        break;
    }   
}

static void restore_rf_tuning_default_config(void) {
    memcpy(&rf_tuning_config_ram, &rf_tuning_config_flash_default, sizeof(rf_tuning_config_ram));
}

__attribute__((section(".config_entry"))) const void (*p_restore_rf_tuning_default_config)(void) = (const void *)&restore_rf_tuning_default_config;