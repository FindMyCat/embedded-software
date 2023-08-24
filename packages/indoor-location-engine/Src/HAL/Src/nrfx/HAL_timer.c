/**
 * @file    HAL_timer.c
 *
 * @brief   HAL functions for timer
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

#include <stdint.h>

#include "sdk_config.h"
#include "nrf_error.h"
#include "nrf_timer.h"
#include "nrf_drv_timer.h"
#include "assert.h"

#include "HAL_timer.h"
#include "HAL_gpio.h"

#if (NRFX_TIMER1_ENABLED == 1)
#define TIMERC_ID 1 // SD using 0;
#else
#define TIMERC_ID 0
#endif

uint32_t HAL_GetTick(void);
void SysTick_Handler(void);

/******************************************************************************
 *                              Time Tick section
 */

static uint32_t time32_incr = 0;

/* @fn    HAL_GetTick
 * @brief wrapper for to read a SysTickTimer, which is incremented with
 *        CLOCKS_PER_SEC frequency.
 *        The resolution of time32_incr is usually 1/1000 sec.
 * */
uint32_t HAL_GetTick(void)
{
    return time32_incr;
}

/**@brief Systick handler
 *
 */
void SysTick_Handler(void)
{
    time32_incr++;
}

int64_t mcps_get_uptime_us(void)
{
    //TODO  This is required since MAC R11.3.0. Needs to be implemented for DW3000
	return 0;
}

/******************************************************************************
 *              timer for tick counting
 */
const nrf_drv_timer_t timer1 = NRF_DRV_TIMER_INSTANCE(TIMERC_ID); // softdevice uses Timer0

/**
 * @brief Handler for timer events.
 */
static void
timer1_event_handler(nrf_timer_event_t event_type, void *p_context)
{

    switch (event_type)
    {
    case NRF_TIMER_EVENT_COMPARE0:
        SysTick_Handler();
        break;

    default:
        // Do nothing.
        break;
    }
}

/* @fn       init_timer(void)
 * @brief    initiate timestamp (in CLOCKS_PER_SEC)
 * @parm     p_timestamp pointer on current system timestamp
 */

static uint32_t init_timer(void)
{
    uint32_t time_ms = 1; // Time(in miliseconds) between consecutive compare events.
    uint32_t time_ticks;
    uint32_t err_code = NRF_SUCCESS;

    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
    timer_cfg.frequency = NRF_TIMER_FREQ_125kHz;
    err_code = nrf_drv_timer_init(&timer1, &timer_cfg, timer1_event_handler);
    APP_ERROR_CHECK(err_code);

    time_ticks = nrf_drv_timer_ms_to_ticks(&timer1, time_ms);

    nrf_drv_timer_extended_compare(&timer1, NRF_TIMER_CC_CHANNEL0, time_ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);

    nrf_drv_timer_enable(&timer1);

    return err_code;
}

/* @fn         start_timer(uint32 *p_timestamp)
 * @brief     save system timestamp (in CLOCKS_PER_SEC)
 * @parm     p_timestamp pointer on current system timestamp
 */
static void start_timer(volatile uint32_t *p_timestamp)
{
    *p_timestamp = HAL_GetTick();
}

/* @fn         check_timer(uint32 timestamp , uint32 time)
 * @brief     check if time from current timestamp over expectation
 * @param     [in] timestamp - current timestamp
 * @param     [in] time - time expectation (in CLOCKS_PER_SEC)
 * @return     true - time is over
 *             false - time is not over yet
 */
bool check_timer(uint32_t timestamp, uint32_t time)
{
    uint32_t time_passing;
    uint32_t temp_tick_time = HAL_GetTick();

    if (temp_tick_time >= timestamp)
    {
        time_passing = temp_tick_time - timestamp;
    }
    else
    {
        time_passing = 0xffffffffUL - timestamp + temp_tick_time;
    }

    if (time_passing >= time)
    {
        return (true);
    }

    return (false);
}


/******************************************************************************
 * START Fast Sleep Timer section
 * Timer used for low-power UWB
 */

#if (TIMER4_ENABLED == 1)

/* This struct is to decouple fs_timer_cb() from other API */
static struct fs_timer_context_s
{
    void *tmr;
    void *dw;
} fs_timer_context;

/* @brief Internal instance callback
 *        Calls "timerIsr" on CC event, if present
 */
static void
fs_timer_cb(nrf_timer_event_t event_type, void *p_context)
{
    struct fs_timer_context_s *context = (struct fs_timer_context_s *)p_context;
    hal_fs_timer_t *htimer = (hal_fs_timer_t *)context->tmr;

    switch (event_type)
    {
    case NRF_TIMER_EVENT_COMPARE0:
        if (htimer->timerIsr)
        {
            LP_DEBUG_D1();
            htimer->timerIsr(context->dw);
        }
        break;
    default:
        // Do nothing.
        break;
    }
}

/** @brief Initialization of the NRF Fast Sleep Timer driver instance
 *         timer is running on 1MHz
 *         - in continous mode with Counter Compare events
 *         - clearing its counter (and keep counting) on CC
 */
static void
fs_timer_init(void *tmr, void *dw, void (*cb)(void *))
{
    hal_fs_timer_t *htimer = (hal_fs_timer_t *)tmr;
    nrf_drv_timer_t *timer = htimer->timer;
    nrf_drv_timer_config_t timer_cfg;
    nrfx_err_t err_code;

    assert(htimer->freq == 1000000);

    fs_timer_context.tmr = tmr;
    fs_timer_context.dw = dw;

    timer_cfg.bit_width = NRF_TIMER_BIT_WIDTH_32;
    timer_cfg.frequency = NRF_TIMER_FREQ_1MHz;
    timer_cfg.mode = NRF_TIMER_MODE_TIMER;
    timer_cfg.interrupt_priority = 7;
    timer_cfg.p_context = &fs_timer_context;

    if (cb)
    {
        htimer->timerIsr = cb;
        err_code = nrf_drv_timer_init(timer, &timer_cfg, fs_timer_cb);

        if (err_code == NRFX_ERROR_INVALID_STATE)
        {
            nrfx_timer_uninit(timer);
            nrf_drv_timer_init(timer, &timer_cfg, fs_timer_cb);
        }

        /* Starting fs timer to run without IRQ to avoid jitter on Start/Stop */
        htimer->start(htimer, false, UINT32_MAX, 0);
    }
    else
    {
        htimer->stop(htimer);
        nrfx_timer_uninit(timer);
    }
}

static uint32_t
fs_timer_get_tick(void *tmr)
{
    hal_fs_timer_t *htimer = (hal_fs_timer_t *)tmr;
    nrf_drv_timer_t *timer = (nrf_drv_timer_t *)htimer->timer;

    return (uint32_t)(nrf_drv_timer_capture(timer, NRF_TIMER_CC_CHANNEL0));
}


static void
fs_timer_stop(void *tmr)
{
    hal_fs_timer_t *htimer = (hal_fs_timer_t *)tmr;
    nrf_drv_timer_t *timer = (nrf_drv_timer_t *)htimer->timer;
    nrf_drv_timer_disable(timer);
}


static void
fs_timer_start_us(void *tmr, bool int_en, uint32_t time_us, int corr_tick)
{
    hal_fs_timer_t *htimer = (hal_fs_timer_t *)tmr;
    nrf_drv_timer_t *timer = (nrf_drv_timer_t *)htimer->timer;
    uint32_t time_ticks;

    time_ticks = nrf_drv_timer_us_to_ticks(timer, time_us) + corr_tick;
    nrf_drv_timer_extended_compare(timer, NRF_TIMER_CC_CHANNEL0, time_ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, int_en);

    if (!nrf_drv_timer_is_enabled(timer))
    {
        nrf_drv_timer_enable(timer);
    }
}

/* Instance of the fast sleep timer
 * this timer is used to enable low-power sleep of UWB subsystem for the price of its own power consumption
 */
static nrf_drv_timer_t fs_timer = NRF_DRV_TIMER_INSTANCE(4);

hal_fs_timer_t hal_fs_timer = {
    .timer = &fs_timer,
    .timerIsr = NULL,
    .init = fs_timer_init,
    .start = fs_timer_start_us,
    .stop = fs_timer_stop,
    .get_tick = fs_timer_get_tick,
    .freq = 1000000,
};

#else
hal_fs_timer_t hal_fs_timer = {NULL, NULL, NULL, NULL, NULL, NULL, 0};
#endif

/*********************************************************************************/
/** @brief HAL Timer API structure
 */
const struct hal_timer_s Timer = {
    .init = &init_timer,
    .start = &start_timer,
    .check = &check_timer};
/*
 * END Fast Sleep timer section
 ******************************************************************************/
