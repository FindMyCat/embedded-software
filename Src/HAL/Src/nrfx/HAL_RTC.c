/**
 * @file    HAL_RTC.c
 *
 * @brief   Hardware abstaction layer for RTC
 *
 * @author  Decawave Applications
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

/* nRF5340dk Hardware Abstraction Layer for RTC */
#include <stdint.h>

#include "HAL_rtc.h"
#include "int_priority.h"

#include <nrfx_rtc.h>

/* WKUP timer counts Super Frame period.
 * The WKUP timer resolution is (30517.5) counts in 1 ns.
 */
#define WKUP_RESOLUTION_NS (1e9f / 32768.0f)

/* WKUP timer used at Ranging phase.
 * It is counts the Super Frame period, received in the Ranging Config message
 * in Tag's local time domain.
 * */
#define WKUP_RESOLUTION_US (WKUP_RESOLUTION_NS / 1e3f)

#define WKUP_RESOLUTION_MS (WKUP_RESOLUTION_US / 1e3f)

/* RTC WKUP timer counts Super Frame period.
 * The RTC WKUP timer prescaler is configured as each Tick count is 30.517 us.
 */
#define RTC_WKUP_PRESCALER (0)

/* RTC WKUP timer counts Super Frame period.
 * The RTC WKUP timer is 24 bit counter. Counter oveflows at 2^24 - 16777216
 */
#define RTC_WKUP_CNT_OVFLW      (16777216)
#define RTC_WKUP_CNT_OVFLW_MASK (RTC_WKUP_CNT_OVFLW - 1)


#if !defined(NRFX_RTC_ENABLED)
#error "RTC flag not defined"
#endif

#if !NRFX_RTC_ENABLED
#error "RTC defined but not enabled"
#endif

#if (NRFX_RTC2_ENABLED == 1)
#define RTC_ID 2 // SD using 0; FreeRTOS using 1;
#else
#define RTC_ID 0
#endif
#define RTC_CHAN        0
#define RTC_COMP_EVT    NRFX_RTC_INT_COMPARE0
#define RTC_IRQ         RTC0_IRQn
#define RTC_HDLR        nrfx_rtc_0_irq_handler

static nrfx_rtc_t grtc = NRFX_RTC_INSTANCE(RTC_ID);

static void (*rtc_callback)(void);
static uint32_t sRTC_SF_PERIOD = 3276;

static void rtc_disable_irq(void)
{
    uint32_t msk = RTC_CHANNEL_INT_MASK(RTC_CHAN);
    nrfx_rtc_int_disable(&grtc, &msk);
}

static void rtc_enable_irq(void)
{
    uint32_t msk = RTC_CHANNEL_INT_MASK(RTC_CHAN);
    nrfx_rtc_int_enable(&grtc, msk);
}

static void rtc_set_priority_irq(void)
{
    /* configure the RTC Wakeup timer with a high priority;
     * this timer is saving global Super Frame Timestamp,
     * so we want this timestamp as stable as we can.
     *
     * */
    NVIC_SetPriority(RTC_IRQ, PRIO_RTC_WKUP_IRQn);
}

static uint32_t rtc_get_counter(void)
{
    return nrfx_rtc_counter_get(&grtc);
}

static uint32_t rtc_get_time_elapsed(uint32_t start, uint32_t stop)
{
    // RTC is counting up, so we do stop - start to get elapsed time
    int32_t tmp = stop - start;

    // check roll over
    if (tmp < 0)
    {
        tmp += RTC_WKUP_CNT_OVFLW; // RTC Timer overflow - 24 bit counter
    }
    return tmp;
}

//-----------------------------------------------------------------------------
/*
 * @brief   reload RTC registers after interrupt
 *          to prepare next compare match event
 * */
static void rtc_reload(uint32_t base_cnt)
{
    uint32_t new_cc = base_cnt + sRTC_SF_PERIOD;
    new_cc &= RTC_WKUP_CNT_OVFLW_MASK;
    nrfx_rtc_cc_set(&grtc, RTC_CHAN, new_cc, true);
}

//-----------------------------------------------------------------------------
/*
 * @brief   RTC callback called by the RTC interrupt
 * */
static void rtc_handler(nrfx_rtc_int_type_t int_type)
{
    if (int_type == RTC_COMP_EVT)
    {
        if (rtc_callback)
        {
            rtc_callback();
        }
    }
}

static void rtc_set_callback(void (*cb)(void))
{
    rtc_callback = cb;
}

static float rtc_get_wakup_resolution_ns(void)
{
    return WKUP_RESOLUTION_NS;
}

//-----------------------------------------------------------------------------
/*
 * @brief   setup RTC Wakeup timer
 *          period_ns is awaiting time in ns
 * */
static void rtc_configure_wakeup(void)
{
    nrfx_err_t err_code;
    uint32_t old_cc, new_cc;
    nrfx_rtc_config_t config = NRFX_RTC_DEFAULT_CONFIG;

    config.prescaler = RTC_WKUP_PRESCALER;

    // the timer was intialized in the rtc_init()
    nrfx_rtc_disable(&grtc);

    // rtc_handler
    nrfx_rtc_uninit(&grtc);
    err_code = nrfx_rtc_init(&grtc, &config, rtc_handler);
    if (err_code != NRFX_SUCCESS)
    {
        return;
    }

    // Disable tick interrupt
    nrfx_rtc_tick_disable(&grtc);

    // Enable Counter Compare interrupt
    old_cc = nrfx_rtc_counter_get(&grtc);
    new_cc = old_cc + sRTC_SF_PERIOD;
    new_cc &= RTC_WKUP_CNT_OVFLW_MASK;
    nrfx_rtc_cc_set(&grtc, 0, new_cc, true);

    // Power on RTC instance
    nrfx_rtc_enable(&grtc);
}

//-----------------------------------------------------------------------------
/*
 * @brief   setup RTC Wakeup timer
 *          period_ms is awaiting time in ms
 * */
void rtc_configure_wakeup_ns(uint32_t period_ns)
{
    sRTC_SF_PERIOD = period_ns / WKUP_RESOLUTION_NS;
    rtc_configure_wakeup();
}

//-----------------------------------------------------------------------------
/*
 * @brief   setup RTC Wakeup timer
 *          period_ms is awaiting time in ms
 * */
void rtc_configure_wakeup_ms(uint32_t period_ms)
{
    sRTC_SF_PERIOD = period_ms / WKUP_RESOLUTION_MS;
    rtc_configure_wakeup();
}

/** @brief Initialization of the RTC driver instance
 */
static void rtc_init(void)
{
    nrfx_err_t err_code;
    nrfx_rtc_config_t config = NRFX_RTC_DEFAULT_CONFIG;

    config.prescaler = RTC_WKUP_PRESCALER; // WKUP_RESOLUTION_US counter period

    rtc_callback = NULL;

    err_code = nrfx_rtc_init(&grtc, &config, rtc_handler);
    if (err_code != NRFX_SUCCESS)
    {
        return;
    }

    // Power on RTC instance
    nrfx_rtc_enable(&grtc);
}

static void rtc_deinit(void)
{
    // stop the RTC timer
    nrfx_rtc_disable(&grtc);
    nrfx_rtc_uninit(&grtc);
    rtc_callback = NULL;
}

/*********************************************************************************/
/** @brief HAL RTC API structure
 */
const struct hal_rtc_s Rtc = {
    .init = &rtc_init,
    .deinit = &rtc_deinit,
    .enableIRQ = &rtc_enable_irq,
    .disableIRQ = &rtc_disable_irq,
    .setPriorityIRQ = &rtc_set_priority_irq,
    .getTimestamp = &rtc_get_counter,
    .getTimeElapsed = &rtc_get_time_elapsed,
    .reload = &rtc_reload,
    .configureWakeup_ms = &rtc_configure_wakeup_ms,
    .configureWakeup_ns = &rtc_configure_wakeup_ns,
    .getWakeupResolution_ns = &rtc_get_wakup_resolution_ns,
    .setCallback = &rtc_set_callback
};
