/**
 * @file    HAL_power.c
 *
 * @brief   HAL functions for power
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
#include <stdbool.h>
#include "cmsis_os.h"
#include "nrf_rtc.h"
#include "boards.h"
#include "HAL_uart.h"
#include "HAL_timer.h"
#include "app_uart.h"
#ifdef SOFTDEVICE_PRESENT
#include "nrf_sdh.h"
#endif

// can't be in HAL_UART.h cause nRF specific.
extern void Restart_UART_timer();
extern void deca_discard_next_symbol(void);


__STATIC_INLINE void nrf_gpio_cfg_input_sense_none(uint32_t pin_number, nrf_gpio_pin_pull_t pull_config)
{
    nrf_gpio_cfg(
        pin_number,
        NRF_GPIO_PIN_DIR_INPUT,
        NRF_GPIO_PIN_INPUT_CONNECT,
        pull_config,
        NRF_GPIO_PIN_S0S1,
        NRF_GPIO_PIN_NOSENSE);
}

#if (configUSE_TICKLESS_IDLE == 2)

extern void xPortSysTickHandler(void);

/*
 * This wrapper is to not break the Nordic SDK while support of (configUSE_TICKLESS_IDLE == 2)
 * See FreeRtosConfig.h
 *
 * for NRF we want to execute the xPortSysTickHandler on the RTC IRQ, with nrf_rtc_event_clear()
 */
void xPortSysTickHandler2(void)
{
    nrf_rtc_event_clear(portNRF_RTC_REG, NRF_RTC_EVENT_COMPARE_0);
    xPortSysTickHandler();
}


/*
 * @fn      vPortSuppressTicksAndSleep
 *
 * @brief   The function that is called by portSUPPRESS_TICKS_AND_SLEEP()
 *
 *          This is a modified version of option "1" and even better lower powered.
 *
 * @param   [in] xExpectedIdleTime - time to sleep in FreeRTOS ticks
 *
 */
void vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime)
{
    /*
     * Implementation note:
     *
     * To help debugging the option configUSE_TICKLESS_IDLE_SIMPLE_DEBUG was presented.
     * This option would make sure that even if program execution was stopped inside
     * this function no more than expected number of ticks would be skipped.
     *
     * Normally RTC works all the time even if firmware execution was stopped
     * and that may lead to skipping too much of ticks.
     */

    TickType_t enterTime;

    /* Make sure the SysTick reload value does not overflow the counter. */
    if (xExpectedIdleTime > portNRF_RTC_MAXTICKS - configEXPECTED_IDLE_TIME_BEFORE_SLEEP)
    {
        xExpectedIdleTime = portNRF_RTC_MAXTICKS - configEXPECTED_IDLE_TIME_BEFORE_SLEEP;
    }
    /* Block all the interrupts globally */
#ifdef SOFTDEVICE_PRESENT
    do
    {
        uint8_t dummy = 0;
        uint32_t err_code = sd_nvic_critical_region_enter(&dummy);
        APP_ERROR_CHECK(err_code);
    } while (0);
#else
    __disable_irq();
#endif

    enterTime = nrf_rtc_counter_get(portNRF_RTC_REG);

    if (eTaskConfirmSleepModeStatus() != eAbortSleep)
    {
        TickType_t xModifiableIdleTime;
        TickType_t wakeupTime = (enterTime + xExpectedIdleTime) & portNRF_RTC_MAXTICKS;

        /* Stop tick events */
        nrf_rtc_int_disable(portNRF_RTC_REG, NRF_RTC_INT_TICK_MASK);

        /* Configure CTC interrupt */
        nrf_rtc_cc_set(portNRF_RTC_REG, 0, wakeupTime);
        nrf_rtc_event_clear(portNRF_RTC_REG, NRF_RTC_EVENT_COMPARE_0);
        nrf_rtc_int_enable(portNRF_RTC_REG, NRF_RTC_INT_COMPARE0_MASK);

        __DSB();

        /* Sleep until something happens.  configPRE_SLEEP_PROCESSING() can
         * set its parameter to 0 to indicate that its implementation contains
         * its own wait for interrupt or wait for event instruction, and so wfi
         * should not be executed again.  However, the original expected idle
         * time variable must remain unmodified, so a copy is taken. */
        xModifiableIdleTime = xExpectedIdleTime;
        configPRE_SLEEP_PROCESSING(xModifiableIdleTime);
        if (xModifiableIdleTime > 0)
        {

            NVIC_ClearPendingIRQ(RTC0_IRQn);
            NVIC_ClearPendingIRQ(RTC1_IRQn);

// errata #87, 3.12 Errata_v1.6
#if (__FPU_USED == 1)
            __set_FPSCR(__get_FPSCR() & ~(0x0000009F));
            (void)__get_FPSCR();
            NVIC_ClearPendingIRQ(FPU_IRQn);
#endif

// With FreeRTOS sd_app_evt_wait increases power consumption with FreeRTOS compared to _WFE (NRFFOSDK-11174)
            do
            {
#ifdef SOFTDEVICE_PRESENT
                if (nrf_sdh_is_enabled())
                {
                        uint32_t err_code = sd_app_evt_wait();
                        APP_ERROR_CHECK(err_code);
                }
                else
#endif
                {
                    /* No SD -  we would just block interrupts globally.
                    * BASEPRI cannot be used for that because it would prevent WFE from wake up.
                    */
                        __WFE();
                }
            } while (0 == (NVIC->ISPR[0] | NVIC->ISPR[1]));
        }

        configPOST_SLEEP_PROCESSING(xExpectedIdleTime);

        nrf_rtc_int_disable(portNRF_RTC_REG, NRF_RTC_INT_COMPARE0_MASK);
        nrf_rtc_event_clear(portNRF_RTC_REG, NRF_RTC_EVENT_COMPARE_0);

        /* Correct the system ticks */
        {
            TickType_t diff;
            TickType_t exitTime;

            nrf_rtc_event_clear(portNRF_RTC_REG, NRF_RTC_EVENT_TICK);
            nrf_rtc_int_enable(portNRF_RTC_REG, NRF_RTC_INT_TICK_MASK);

            exitTime = nrf_rtc_counter_get(portNRF_RTC_REG);
            diff = (exitTime - enterTime) & portNRF_RTC_MAXTICKS;

            /* It is important that we clear pending here so that our corrections are latest and in sync with tick_interrupt handler */
            NVIC_ClearPendingIRQ(portNRF_RTC_IRQn);

            if ((configUSE_TICKLESS_IDLE_SIMPLE_DEBUG) && (diff > xExpectedIdleTime))
            {
                diff = xExpectedIdleTime;
            }

            if (diff > 0)
            {
                vTaskStepTick(diff);
            }
        }
    }
#ifdef SOFTDEVICE_PRESENT
    uint32_t err_code = sd_nvic_critical_region_exit(0);
    APP_ERROR_CHECK(err_code);
#else

    if (IsUartDown())
    {
        if (nrf_gpio_pin_latch_get(UART_0_RX_PIN))
        {
            nrf_gpio_cfg_input_sense_none(UART_0_RX_PIN, NRF_GPIO_PIN_NOPULL);
            nrf_gpio_pin_latch_clear(UART_0_RX_PIN);
            SetUartDown(false);
            Restart_UART_timer();
            deca_uart_init();
            app_uart_flush();
            deca_discard_next_symbol();
        }
    }

    __enable_irq();
#endif
}


#endif // configUSE_TICKLESS_IDLE == 2