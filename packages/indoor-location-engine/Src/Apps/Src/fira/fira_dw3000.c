/**
 * @file      fira_dw3000.c
 *
 * @brief     FiRa for DW3000
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

#include "dw3000_mcps_mcu.h"
#include "common_fira.h"
#include "rf_tuning_config.h"
#include "debug_config.h"

static struct dwchip_s *dw = NULL;

static void driver_configure_for_fira(struct dwchip_s *dw, fira_param_t *fira_param)
{
    static rxtx_configure_t rxtx_config_fira_app;
    static dwt_mcps_config_t dw_conf;
    static dwt_config_t dwt_config;

    static dwt_sts_cp_key_t dummy_stsKey = {};
    static dwt_sts_cp_iv_t dummy_stsIv = {};

    rf_tuning_t *rf_tuning = get_rf_tuning_config();
    debug_config_t *debug_config = get_debug_config();

    rxtx_config_fira_app.txAntDelay = rf_tuning->antTx_a;
    rxtx_config_fira_app.rxAntDelay = rf_tuning->antRx_a;
    rxtx_config_fira_app.txConfig = get_dwt_txconfig();

    rxtx_config_fira_app.frameFilter = DWT_FF_ENABLE_802_15_4;
    rxtx_config_fira_app.frameFilterMode = (DWT_FF_BEACON_EN | DWT_FF_DATA_EN | DWT_FF_ACK_EN | DWT_FF_COORD_EN);

    /*FiRa variable PHY cfg*/
    dwt_config.chan = fira_param->session.channel_number;
    dwt_config.sfdType = (fira_param->session.sfd_id == 0) ? DWT_SFD_IEEE_4A : DWT_SFD_IEEE_4Z;
    dwt_config.txCode = fira_param->session.preamble_code_index;
    dwt_config.rxCode = fira_param->session.preamble_code_index;
    /*FiRa fixed PHY cfg*/
    dwt_config.txPreambLength = DWT_PLEN_64;
    dwt_config.rxPAC = DWT_PAC8;
    dwt_config.dataRate = DWT_BR_6M8;
    dwt_config.phrMode = DWT_PHRMODE_STD;
    dwt_config.phrRate = DWT_PHRRATE_STD;
    dwt_config.sfdTO = (64 + 1 + 8 - 8);
    dwt_config.stsMode = DWT_STS_MODE_OFF;
    dwt_config.stsLength = DWT_STS_LEN_64;
    dwt_config.pdoaMode = DWT_PDOA_M1;

    rxtx_config_fira_app.pdwCfg = &dwt_config;


    /* setting mcps config */
    dw_conf.rxtx_config = &rxtx_config_fira_app;
    dw_conf.mode = DWT_DW_INIT;
    dw_conf.do_reset = 1;
    dw_conf.led_mode = DWT_LEDS_ENABLE | DWT_LEDS_INIT_BLINK;
    dw_conf.lnapamode = DWT_TXRX_EN;
    dw_conf.bitmask_lo = DWT_INT_SPIRDY_BIT_MASK | DWT_INT_TXFRS_BIT_MASK | DWT_INT_RXFCG_BIT_MASK | DWT_INT_ARFE_BIT_MASK
                         | DWT_INT_RXFSL_BIT_MASK | DWT_INT_RXSTO_BIT_MASK | DWT_INT_RXPHE_BIT_MASK | DWT_INT_RXFCE_BIT_MASK
                         | DWT_INT_RXFTO_BIT_MASK | DWT_INT_RXFR_BIT_MASK | DWT_INT_RXPTO_BIT_MASK;
    dw_conf.bitmask_hi = 0;
    dw_conf.int_options = DWT_ENABLE_INT_ONLY;
    dw_conf.sleep_config.mode = DWT_CONFIG | DWT_PGFCAL;
    dw_conf.sleep_config.wake = DWT_PRES_SLEEP | DWT_WAKE_CSN | DWT_SLP_EN;
    dw_conf.xtalTrim = rf_tuning->xtalTrim;

    dw_conf.stsKey = &dummy_stsKey;
    dw_conf.stsIv = &dummy_stsIv;

    dw_conf.loadIv = 0;
    dw_conf.event_counter = 1;
    dw_conf.cia_enable_mask = DW_CIA_DIAG_LOG_ALL;

    /* Setting of the driver config is done */
    dw->config = &dw_conf;

    dw->mcps_runtime->diag.enable = debug_config->diagEn;
    dw->mcps_runtime->pdoa_offset = rf_tuning->pdoaOffset_deg;

    dw->llhw->current_preamble_code = fira_param->session.preamble_code_index;
    dw->llhw->hw->phy->current_channel = fira_param->session.channel_number;
}

void fira_uwb_mcps_init(fira_param_t *fira_param)
{
    dw = dw3000_mcps_alloc();
    driver_configure_for_fira(dw, fira_param);

    int r = dw3000_mcps_register(dw);
    assert(!r);
}

void fira_uwb_mcps_deinit(void)
{
    dw3000_mcps_unregister(dw);
    dw3000_mcps_free(dw);
}

int32_t fira_uwb_mcps_get_cfo_ppm(void)
{
    return dw->mcps_runtime->diag.cfo_ppm;
}

bool fira_uwb_is_diag_enabled(void)
{
    return dw->mcps_runtime->diag.enable;
}

int fira_uwb_add_diag(char *str, int len, int max_len)
{
    if (dw->mcps_runtime->diag.rssi < 0.0)
    {
        len += snprintf(&str[len], max_len - len, ",\"RSSI_dBm\":\"%.1f\"", dw->mcps_runtime->diag.rssi);
    }
    else
    {
        len += snprintf(&str[len], max_len - len, ",\"RSSI_dBm\":\"Invalid\"");
    }
    len += snprintf(&str[len], max_len - len, ",\"NLOS_%%\":%d", (int)dw->mcps_runtime->diag.non_line_of_sight);
    return len;
}
