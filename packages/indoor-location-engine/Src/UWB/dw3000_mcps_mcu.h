/**
 * @file   dw3000_mcps_mcu.h
 *
 * @brief  DW3000 MAC Common Part Sublayer (MCPS)
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

#ifndef __DW3000_MCPS_MCU_H
#define __DW3000_MCPS_MCU_H

#include <stdint.h>

#include <deca_interface.h>
#include <net/mcps802154.h>

enum operational_state
{
    DW3000_OP_STATE_OFF = 0,
    DW3000_OP_STATE_DEEP_SLEEP,
    DW3000_OP_STATE_SLEEP,
    DW3000_OP_STATE_WAKE_UP,
    DW3000_OP_STATE_INIT_RC,
    DW3000_OP_STATE_IDLE_RC,
    DW3000_OP_STATE_IDLE_PLL,
    DW3000_OP_STATE_TX_WAIT,
    DW3000_OP_STATE_TX,
    DW3000_OP_STATE_RX_WAIT,
    DW3000_OP_STATE_RX,
    DW3000_OP_STATE_MAC_EXIT_IDLE,
    DW3000_OP_STATE_MAX,
};

/**
 * struct dw3000_deep_sleep_state - Useful data to restore on wake up
 * @next_operational_state: operational state to enter after DEEP SLEEP mode or during waking up
 * @config_changed: bitfield of configuration changed during DEEP-SLEEP
 * @tx_skb: saved frame to transmit for deferred TX
 * @txops: saved ops to use for deferred TX after wake from deep sleep
 * @rx_info: saved ops to use for deferred RX after wake from deep sleep
 */
struct dw3000_deep_sleep_state
{
    enum operational_state next_operational_state;

	unsigned long config_changed;

    struct sk_buff *tx_skb;

    union
    {
        struct dw_tx_frame_info_s txops;
        struct dw_rx_frame_info_s rxops;
    };
};

struct dwt_mcps_rx_s
{
    uint64_t timeStamp; /* Full TimeStamp */
    uint32_t rtcTimeStamp;
    uint32_t flags;
    uint8_t *data;
    unsigned int len;
    int16_t cfo;
};
typedef struct dwt_mcps_rx_s dwt_mcps_rx_t;

struct mcps_diag_s
{
    bool enable;
    float rssi;
    float non_line_of_sight;
    int32_t cfo_ppm;
    uint32_t CIA_TDOA;
};

struct dwt_mcps_runtime_s
{
    int chips_per_pac;   /* Chips per PAC unit. */
    int pre_timeout_pac; /* Preamble timeout in PAC unit. */
    int16_t pdoa_offset;
    struct mcps_diag_s diag;

    // Below is 10.x.x+

    /* Internal operational state of the chip */
    enum operational_state current_operational_state;

    /* Deep Sleep & MCPS Idle management */
    struct dw3000_deep_sleep_state deep_sleep_state;
    void   *deep_sleep_timer;

    uint32_t tmbase_dtu; /* Current timebase in DW chip system time(4ns). Adjusted every wakeup from Deep Sleep */
    uint32_t corr_4ns;   /* 0 or 1 */

    bool need_ranging_clock;
};

typedef struct dwt_mcps_runtime_s dwt_mcps_runtime_t;

struct ieee802154_hw *ieee802154_hw_alloc(void);
struct ieee802154_hw *ieee802154_hw_free(void);

struct dwchip_s *dw3000_mcps_alloc(void);
int dw3000_mcps_register(struct dwchip_s *dw);
void dw3000_mcps_unregister(struct dwchip_s *dw);
void dw3000_mcps_free(struct dwchip_s *dw);

#endif /* __DW3000_MCPS_MCU_H */
