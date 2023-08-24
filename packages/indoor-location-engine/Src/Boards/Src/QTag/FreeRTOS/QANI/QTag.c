/**
 * @file      DWM3001CDK.c
 *
 * @brief     Board specific initialization
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
#include "HAL_SPI.h"
#include "HAL_error.h"
#include "HAL_rtc.h"
#include "HAL_uwb.h"
#include "HAL_watchdog.h"
#include "app_error.h"
#include "boards.h"
#include "deca_device_api.h"
#include "deca_interface.h"
#include "nrf_delay.h"
#include "nrf_drv_clock.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_spim.h"
#include "thisBoard.h"
#include "rf_tuning_config.h"

/* @fn  peripherals_init
 *
 * @param[in] void
 * */
void peripherals_init(void)
{
    ret_code_t err_code;

    /* With this change, Reset After Power Cycle is not required */
    nrf_gpio_cfg_input(UART_0_RX_PIN, NRF_GPIO_PIN_PULLUP);

    if (!nrf_drv_clock_init_check())
    {
        err_code = nrf_drv_clock_init();
        APP_ERROR_CHECK(err_code);
    }

    /* Request HF clock for precise block timing */
    nrf_drv_clock_hfclk_request(NULL);
    while (!nrf_drv_clock_hfclk_is_running())
        ;

    nrf_drv_clock_lfclk_request(NULL);

    /* Watchdog 60sec */
    Watchdog.init(60000);
}

void BoardInit(void)
{
    bsp_board_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS);
    peripherals_init();
}

void board_interface_init(void)
{
}

int uwb_init(void)
{
    hal_uwb.init();

    hal_uwb.irq_init();

    hal_uwb.disableIRQ();
    
    hal_uwb.reset();

    int status = hal_uwb.probe();

    /* This section is added to initialize the local in dw_drivers
       Without this initialization The command like xtaltrim will fail*/
    if (status == DWT_SUCCESS && hal_uwb.uwbs->dw)
    {
        hal_uwb.uwbs->spi->slow_rate(hal_uwb.uwbs->spi->handler);

        struct dwchip_s *dw = hal_uwb.uwbs->dw;

        status = dw->dwt_driver->dwt_ops->initialize(dw, DWT_DW_IDLE_RC);

        dwt_txconfig_t *txConfig = get_dwt_txconfig();

        rf_tuning_set_tx_power_pg_delay(hal_uwb.uwbs->devid);

        dwt_set_alternative_pulse_shape(1);
        dwt_enable_disable_eq(DWT_EQ_ENABLED);
        dwt_configuretxrf(txConfig);

        hal_uwb.uwbs->spi->fast_rate(hal_uwb.uwbs->spi->handler);

        hal_uwb.sleep_config();
        hal_uwb.sleep_enter();        
        hal_uwb.mcu_suspend_clk();

    }

    return status;
}