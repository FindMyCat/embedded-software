/**
 * @file   dw3000_mcps_mcu.c
 *
 * @brief  DW3000 MAC Common Part Sublayer (MCPS)
 *
 * @author Decawave Application
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

#include <stddef.h> //for offsetof
#include "_types.h" // for u8
#include <linux/types.h>

#include "dw3000.h"
#include "dw3000_calib_mcu.h"
#include "deca_interface.h"
// clang-format off
/**
 * dw3000_calib_keys - calibration parameters keys table
 */

const char *const dw3000_calib_keys[] = {
	// ant0.ch*
	"ant0.ch5.prf16.antenna_delay",
	"ant0.ch5.prf16.tx_power",
	"ant0.ch5.prf16.pg_count",
	"ant0.ch5.prf16.pg_delay",
	"ant0.ch5.prf64.antenna_delay",
	"ant0.ch5.prf64.tx_power",
	"ant0.ch5.prf64.pg_count",
	"ant0.ch5.prf64.pg_delay",
	"ant0.ch9.prf16.antenna_delay",
	"ant0.ch9.prf16.tx_power",
	"ant0.ch9.prf16.pg_count",
	"ant0.ch9.prf16.pg_delay",
	"ant0.ch9.prf64.antenna_delay",
	"ant0.ch9.prf64.tx_power",
	"ant0.ch9.prf64.pg_count",
	"ant0.ch9.prf64.pg_delay",
	// ant0.*
	"ant0.port",
	"ant0.selector_gpio",
	"ant0.selector_gpio_value",
	// ant1.ch*
	"ant1.ch5.prf16.antenna_delay",
	"ant1.ch5.prf16.tx_power",
	"ant1.ch5.prf16.pg_count",
	"ant1.ch5.prf16.pg_delay",
	"ant1.ch5.prf64.antenna_delay",
	"ant1.ch5.prf64.tx_power",
	"ant1.ch5.prf64.pg_count",
	"ant1.ch5.prf64.pg_delay",
	"ant1.ch9.prf16.antenna_delay",
	"ant1.ch9.prf16.tx_power",
	"ant1.ch9.prf16.pg_count",
	"ant1.ch9.prf16.pg_delay",
	"ant1.ch9.prf64.antenna_delay",
	"ant1.ch9.prf64.tx_power",
	"ant1.ch9.prf64.pg_count",
	"ant1.ch9.prf64.pg_delay",
	// ant1.*
	"ant1.port",
	"ant1.selector_gpio",
	"ant1.selector_gpio_value",
	// ant0.ant1.*
	"ant0.ant1.ch5.pdoa_offset",
	"ant0.ant1.ch5.pdoa_lut",
	"ant0.ant1.ch9.pdoa_offset",
	"ant0.ant1.ch9.pdoa_lut",
	// chY.*
	"ch5.pll_locking_code",
	"ch9.pll_locking_code",
#if 0
	// other
	"xtal_trim",
	"temperature_reference",
	// NULL terminated array for caller of dw3000_calib_list_keys().
#endif
	NULL
};

#define sizeof_field(t, f) (sizeof(((t*)0)->f))

#define CAL_OFFSET(m) offsetof(struct dw3000_calibration_data, m)
#define CAL_SIZE(m) sizeof_field(struct dw3000_calibration_data, m)
#define CAL_INFO(m) { .offset = CAL_OFFSET(m), .length = CAL_SIZE(m) }

#if 0
#define OTP_OFFSET(m) offsetof(struct dw3000_otp_data, m)
#define OTP_SIZE(m) sizeof_field(struct dw3000_otp_data, m)
#define OTP_INFO(m) { .offset = OTP_OFFSET(m), .length = OTP_SIZE(m) }
#endif

const struct {
    unsigned int offset;
    unsigned int length;
} dw3000_calib_keys_info[] = {
    // ant0.*
    CAL_INFO(ant[0].ch[0].prf[0].ant_delay),
    CAL_INFO(ant[0].ch[0].prf[0].tx_power),
    CAL_INFO(ant[0].ch[0].prf[0].pg_count),
    CAL_INFO(ant[0].ch[0].prf[0].pg_delay),
    CAL_INFO(ant[0].ch[0].prf[1].ant_delay),
    CAL_INFO(ant[0].ch[0].prf[1].tx_power),
    CAL_INFO(ant[0].ch[0].prf[1].pg_count),
    CAL_INFO(ant[0].ch[0].prf[1].pg_delay),
    CAL_INFO(ant[0].ch[1].prf[0].ant_delay),
    CAL_INFO(ant[0].ch[1].prf[0].tx_power),
    CAL_INFO(ant[0].ch[1].prf[0].pg_count),
    CAL_INFO(ant[0].ch[1].prf[0].pg_delay),
    CAL_INFO(ant[0].ch[1].prf[1].ant_delay),
    CAL_INFO(ant[0].ch[1].prf[1].tx_power),
    CAL_INFO(ant[0].ch[1].prf[1].pg_count),
    CAL_INFO(ant[0].ch[1].prf[1].pg_delay),
    CAL_INFO(ant[0].port),
    CAL_INFO(ant[0].selector_gpio),
    CAL_INFO(ant[0].selector_gpio_value),
    // ant1.*
    CAL_INFO(ant[1].ch[0].prf[0].ant_delay),
    CAL_INFO(ant[1].ch[0].prf[0].tx_power),
    CAL_INFO(ant[1].ch[0].prf[0].pg_count),
    CAL_INFO(ant[1].ch[0].prf[0].pg_delay),
    CAL_INFO(ant[1].ch[0].prf[1].ant_delay),
    CAL_INFO(ant[1].ch[0].prf[1].tx_power),
    CAL_INFO(ant[1].ch[0].prf[1].pg_count),
    CAL_INFO(ant[1].ch[0].prf[1].pg_delay),
    CAL_INFO(ant[1].ch[1].prf[0].ant_delay),
    CAL_INFO(ant[1].ch[1].prf[0].tx_power),
    CAL_INFO(ant[1].ch[1].prf[0].pg_count),
    CAL_INFO(ant[1].ch[1].prf[0].pg_delay),
    CAL_INFO(ant[1].ch[1].prf[1].ant_delay),
    CAL_INFO(ant[1].ch[1].prf[1].tx_power),
    CAL_INFO(ant[1].ch[1].prf[1].pg_count),
    CAL_INFO(ant[1].ch[1].prf[1].pg_delay),
    CAL_INFO(ant[1].port),
    CAL_INFO(ant[1].selector_gpio),
    CAL_INFO(ant[1].selector_gpio_value),
    // antX.antW.*
    CAL_INFO(antpair[ANTPAIR_IDX(0, 1)].ch[0].pdoa_offset),
    CAL_INFO(antpair[ANTPAIR_IDX(0, 1)].ch[0].pdoa_lut),
    CAL_INFO(antpair[ANTPAIR_IDX(0, 1)].ch[1].pdoa_offset),
    CAL_INFO(antpair[ANTPAIR_IDX(0, 1)].ch[1].pdoa_lut),
    // chY.*
    CAL_INFO(ch[0].pll_locking_code),
    CAL_INFO(ch[1].pll_locking_code),
#if 0
    // other with defaults from OTP
    OTP_INFO(xtal_trim),
    OTP_INFO(tempP),
#endif
};
// clang-format on

/**
 * dw3000_calib_parse_key - parse key and find corresponding param
 * @dw: the DW device
 * @key: pointer to NUL terminated string to retrieve param address and len
 * @param: pointer where to store the corresponding parameter address
 *
 * This function lookup the NULL terminated table @dw3000_calib_keys and
 * if specified key is found, store the corresponding address in @param and
 *
 * Return: length of corresponding parameter if found, else a -ENOENT error.
 */
int dw3000_calib_parse_key(struct dwchip_s *dw, const char *key, void **param)
{
    int i;

    for (i = 0; dw3000_calib_keys[i]; i++)
    {
        const char *k = dw3000_calib_keys[i];

        if (strcmp(k, key) == 0)
        {
            /* Key found, calculate parameter address */
            *param = (void *)((uintptr_t)dw->calib_data + dw3000_calib_keys_info[i].offset);
            return dw3000_calib_keys_info[i].length;
        }
    }
    return -1;
}

/**
 * dw3000_calib_list_keys - return the @dw3000_calib_keys known key table
 * @dw: the DW device
 *
 * Return: pointer to known keys table.
 */
const char *const *dw3000_calib_list_keys(struct dwchip_s *dw)
{
    // dw->calib_data =
    return dw3000_calib_keys;
}

/**
 * dw3000_calib_update_config - update running configuration
 * @dw: the DW device
 *
 * This function update the required fields in struct dw3000_txconfig according
 * the channel and PRF and the corresponding calibration values.
 *
 * Also update RX/TX RMARKER offset according calibrated antenna delay.
 *
 * Other calibration parameters aren't used yet.
 *
 * Return: zero on success, else a negative error code.
 */
int dw3000_calib_update_config(struct dwchip_s *dw)
{
#if 0
    struct mcps802154_llhw *llhw = dw->llhw;
#endif
    dwt_config_t *config = dw->config->rxtx_config->pdwCfg;
    dwt_txconfig_t *txconfig = dw->config->rxtx_config->txConfig;
    struct dw3000_antenna_calib *ant_calib = &dw->calib_data->ant[0];
#if 0
    struct mcps802154_llhw *llhw = dw->llhw;
    struct dw3000_antenna_pair_calib *antpair_calib = &dw->calib_data->antpair[0];
#endif
    int chanidx = config->chan == 9;
    int prfidx = config->txCode > 9;
    /* Update TX configuration */
    txconfig->power = ant_calib->ch[chanidx].prf[prfidx].tx_power ? ant_calib->ch[chanidx].prf[prfidx].tx_power : 0xfefefefe;
    txconfig->PGdly = ant_calib->ch[chanidx].prf[prfidx].pg_delay ? ant_calib->ch[chanidx].prf[prfidx].pg_delay : 0x34;
    txconfig->PGcount = ant_calib->ch[chanidx].prf[prfidx].pg_count ? ant_calib->ch[chanidx].prf[prfidx].pg_count : 0;
#if 0
    /* Update RMARKER offsets */
    llhw->rx_rmarker_offset_rctu =
    	ant_calib->ch[chanidx].prf[prfidx].ant_delay;
    llhw->tx_rmarker_offset_rctu =
    	ant_calib->ch[chanidx].prf[prfidx].ant_delay;
    /* Update PDOA offset */
    config->pdoaOffset = antpair_calib->ch[chanidx].pdoa_offset;
    /* TODO: add support for port/gpios, etc... */
#endif
    return 0;
}
