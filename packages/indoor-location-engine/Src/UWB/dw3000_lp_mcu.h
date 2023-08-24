/**
 * @file    dw3000_lp_mcu.h
 *
 * @brief   DW3000 low-power header
 *
 * @author Decawave Applications
 *
 * @version 1.0.0
 *          implementation of DeepSleep LP
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

#ifndef __DW3000_LP_MCU_H
#define __DW3000_LP_MCU_H 1

#include "HAL_gpio.h"
#include "HAL_uwb.h"
#include "HAL_timer.h"

#include "dw3000_mcps_mcu.h"

#ifdef SOFTDEVICE_PRESENT

/* With SOFTDEVICE sleeping inside non-ranging slots may be not stable */

#define TMR_WAKE_TIME_US  (400) /* time to assert CS low: min 150, guaranteed 200. */
#define TMR_IDLE_RC_US    (700) /* time required for the chip to go to IDLE_RC: min 700, guaranteed 900 */
#define TMR_IDLE_PLL_US   (200) /* time required for the chip to go to IDLE_PLL */
#define TMR_CALIB_DW3_US  (200) /* time required for the DW3000 chip to calibrate after deep sleep */
#define TMR_CALIB_QM33_US (950) /* time required for the QM33120 chip to calibrate after deep sleep */
#define TIME_MARGIN_RX_TX (800)

#else

#define TMR_WAKE_TIME_US  (200) /* time to assert CS low: min 150, guaranteed 200. */
#define TMR_IDLE_RC_US    (700) /* time required for the chip to go to IDLE_RC: min 700, guaranteed 900 */
#define TMR_IDLE_PLL_US   (100) /* time required for the chip to go to IDLE_PLL */
#define TMR_CALIB_DW3_US  (200) /* time required for the DW3000 chip to calibrate after deep sleep */
#define TMR_CALIB_QM33_US (950) /* time required for the QM33120 chip to calibrate after deep sleep */
#define TIME_MARGIN_RX_TX (700)

#endif

#define TMR_ABC_DW3000_US (TMR_WAKE_TIME_US + TMR_IDLE_RC_US + TMR_IDLE_PLL_US + TMR_CALIB_DW3_US)
#define TMR_ABC_QM33_US   (TMR_WAKE_TIME_US + TMR_IDLE_RC_US + TMR_IDLE_PLL_US + TMR_CALIB_QM33_US)

#define MIN_TIME_FOR_CONFIG_US (20000) /* Minimum time if want update the configuration: before chip is going to sleep */

/* Time required to set, wake, and program the DW3xxx from the DeepSleep */
#define DEEPSLEEP_WAKE_CONSTANT_DW3000_us (TMR_ABC_DW3000_US+TIME_MARGIN_RX_TX)

/* Time required to set, wake, calibrate and program the QM33xxx from the DeepSleep */
#define DEEPSLEEP_WAKE_CONSTANT_QM33000_us (TMR_ABC_QM33_US+TIME_MARGIN_RX_TX)



/**/
void lp_timer_fira_init(struct dwchip_s *dw);
void lp_timer_fira_deinit(struct dwchip_s *dw);

uint32_t get_timebase_dtu(struct dwchip_s *dw);

int dw3000_rx_enable(struct dwchip_s *dw, int rx_delayed, uint32_t date_dtu, uint32_t timeout_pac);
int dw3000_tx_frame(struct dwchip_s *dw, struct sk_buff *skb, int tx_delayed, uint32_t tx_date_dtu, int rx_delay_dly, uint32_t rx_timeout_pac);

/* Timer expiration callback need to be implemented to trigger the event for mcps task*/
typedef struct uwbmac_timer_s
{
    void (*timer_expires_callback)(void);
} uwbmac_timer_t;

extern uwbmac_timer_t uwbmac_timer;
#endif
