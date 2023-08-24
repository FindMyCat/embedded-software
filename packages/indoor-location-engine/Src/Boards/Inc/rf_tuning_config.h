/**
 * @file      rf_tuning_config.h
 *
 * @brief     RF tuning data for NVM initialization
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

#ifndef RF_TUNING_CONFIG_H
#define RF_TUNING_CONFIG_H
#include <stdint.h>
#include "deca_device_api.h"

typedef enum
{
    ANT_TYPE_NONE,
    ANT_TYPE_MAN5,
    ANT_TYPE_CPWING5,
    ANT_TYPE_CPWING9,
    ANT_TYPE_MONALISA5,
    ANT_TYPE_MONALISA9,
    ANT_TYPE_JOLIE5,
    ANT_TYPE_JOLIE9,
    ANT_TYPE_CUSTOM,
} antenna_type_e;

/* Antenna structure,  */
struct antenna_s
{
    antenna_type_e port1;
    antenna_type_e port2;
    antenna_type_e port3;
    antenna_type_e port4;
};
typedef struct antenna_s antenna_t;

struct rf_tuning_s
{
    dwt_txconfig_t  txConfig;
    int16_t       pdoaOffset_deg; /**< Calibration: the Phase Differences offset */
    int16_t       rngOffset_mm;   /**< Calibration: the Ranging offset */
    uint16_t      antRx_a;        /**< antenna delay values for the left port */
    uint16_t      antTx_a;        /**< antenna delay values for the left port */
    uint16_t      antRx_b;        /**< antenna delay values for the right port  */
    antenna_t     antenna;
    uint8_t       xtalTrim;
    uint8_t       paverage;
    //QM35725 specific
    uint8_t     tx_ant;
    uint8_t     rxa_ant;
    uint8_t     rxb_ant;
    uint8_t     lna1;
    uint8_t     lna2;
    uint8_t     pa;
};

typedef struct rf_tuning_s rf_tuning_t;

#if CONFIG_PEG_UWB != 1
#define DEFAULT_XTAL_TRIM 0x2E
#endif

rf_tuning_t *get_rf_tuning_config(void);
dwt_txconfig_t *get_dwt_txconfig(void);
void rf_tuning_set_tx_power_pg_delay(uint32_t);


#endif