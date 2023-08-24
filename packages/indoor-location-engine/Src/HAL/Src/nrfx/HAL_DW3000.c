/**
 * @file    HAL_DW3000.c
 *
 * @brief   Hardware abstaction layer for DW30000 / NRF
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
#include "nrf_drv_gpiote.h"
#include "boards.h"
#include "deca_device_api.h"
#include "deca_interface.h"
#include "HAL_error.h"
#include "HAL_uwb.h"
#include "HAL_gpio.h"
#include "HAL_SPI.h"
#include "nrf_delay.h"
#include "nrf_drv_clock.h"
#include "sdk_config.h"
#include "cmsis_os.h"
#include "rf_tuning_config.h"
#ifdef SOFTDEVICE_PRESENT
#include "nrf_sdh.h"
#endif

extern struct dw_s uwbs;

static struct dwchip_s local_hal_dw = {
    .SPI = NULL,
    .wakeup_device_with_io = NULL,
    .dwt_driver = NULL,
    .callbacks = {0},
    .config = NULL,
    .llhw = NULL,
    .mcps_ops = NULL,
    .calib_data = NULL,
    .mcps_runtime = NULL,
    .rx = NULL,
    .coex_gpio_pin = 0,
    .coex_gpio_active_state = 0,
    .priv = NULL
};

static struct dwt_spi_s dw3000_spi_fct;
static struct dwt_probe_s dw3000_probe_interf;

/* UWB SLEEP */
void hal_uwb_sleep_mode_set(sleep_mode_t mode)
{
    hal_uwb.vsleep_mode = mode;
}

sleep_mode_t hal_uwb_sleep_mode_get(void)
{
    return hal_uwb.vsleep_mode;
}

static sleep_status_t hal_uwb_sleep_status_get(void)
{
    return hal_uwb.vsleep_status;
}

static void hal_uwb_sleep_status_set(sleep_status_t status)
{
    hal_uwb.vsleep_status = status;
}

static void hal_uwb_sleep_config(void)
{
    struct dwchip_s *dw = hal_uwb.uwbs->dw;
    struct dwt_configure_sleep_s sleep_config;

    sleep_config.mode = DWT_CONFIG | DWT_PGFCAL;
    sleep_config.wake = DWT_PRES_SLEEP | DWT_WAKE_CSN | DWT_SLP_EN;

    dw->dwt_driver->dwt_ops->ioctl(dw, DWT_CONFIGURESLEEP, 0, (void *)&sleep_config);
}

static void hal_uwb_sleep_enter(void)
{
    struct dwchip_s *dw = hal_uwb.uwbs->dw;

    if (hal_uwb_sleep_status_get() != UWB_IS_SLEEPING)
    {
        dw->dwt_driver->dwt_ops->ioctl(dw, DWT_ENTERSLEEP, 0, NULL);
        hal_uwb_sleep_status_set(UWB_IS_SLEEPING);

        // dw->mcps_runtime->current_operational_state = DW3000_OP_STATE_DEEP_SLEEP;
    }
}

/* Wakeup wrappers */
static inline void hal_uwb_wakeup_start(void)
{
    hal_uwb.uwbs->spi->cs_low(hal_uwb.uwbs->spi->handler);
    hal_uwb_sleep_status_set(UWB_IS_NOT_SLEEPING);
}

static inline void hal_uwb_wakeup_end(void)
{
    hal_uwb.uwbs->spi->cs_high(hal_uwb.uwbs->spi->handler);
}

static inline void hal_uwb_wakeup_fast(void)
{
    hal_uwb_wakeup_start();
    nrf_delay_us(200);
    hal_uwb_wakeup_end();
    hal_uwb_sleep_status_set(UWB_IS_NOT_SLEEPING);
}

static void hal_uwb_wakeup_with_io(void)
{
    hal_uwb.wakeup_fast();
    nrf_delay_us(1500);
}

/*SPI */
static struct spi_s *hal_uwb_init_spi(const spi_port_config_t *spi_port_cfg)
{
    struct spi_s *spi = (struct spi_s *)init_spi(spi_port_cfg);

    if (spi == NULL)
    {
        error_handler(1, _ERR_SPI);
    }

    spi->slow_rate(spi->handler);

    return spi;
}


/* direct SPI */
static void spi_slow_rate(void)
{
    struct spi_s *spi = hal_uwb.uwbs->spi;
    spi->slow_rate(spi->handler);
}

static void spi_fast_rate(void)
{
    struct spi_s *spi = hal_uwb.uwbs->spi;
    spi->fast_rate(spi->handler);
}

static int read_from_spi(uint16_t headerLength, uint8_t *headerBuffer, uint16_t readlength, uint8_t *readBuffer)
{
    struct spi_s *spi = hal_uwb.uwbs->spi;
    return spi->read(spi->handler, headerLength, headerBuffer, readlength, readBuffer);
}

static int write_to_spi(uint16_t headerLength, const uint8_t *headerBuffer, uint16_t bodyLength, const uint8_t *bodyBuffer)
{
    struct spi_s *spi = hal_uwb.uwbs->spi;
    return spi->write(spi->handler, headerLength, headerBuffer, bodyLength, bodyBuffer);
}

static int write_to_spi_with_crc(uint16_t headerLength, const uint8_t *headerBuffer, uint16_t bodyLength, const uint8_t *bodyBuffer, uint8_t crc8)
{
    return _ERR;
}

/* MCU IRQ */
static void hal_uwb_disable_irq(void)
{
    /* Unfortunately NRF chip has only 1 IRQ for all GPIO pins */
    nrf_drv_gpiote_in_event_disable(hal_uwb.uwbs->ext_io_cfg->irqPin);
}

static void hal_uwb_enable_irq(void)
{
    nrf_drv_gpiote_in_event_enable(hal_uwb.uwbs->ext_io_cfg->irqPin, true);
}


/* @fn      hal_uwb_reset_chip
 * @brief   DW_RESET pin has 2 functions
 *          In general it is output, but it also can be used to reset the
 *          digital part of the UWB chip by driving this pin low.
 *          Note, the DW_RESET pin should not be driven high externally.
 * */
static void hal_uwb_reset_chip(void)
{
    nrf_gpio_cfg_output(hal_uwb.uwbs->ext_io_cfg->rstPin);
    nrf_gpio_pin_clear(hal_uwb.uwbs->ext_io_cfg->rstPin);
    nrf_delay_us(200);
    nrf_gpio_cfg_input(hal_uwb.uwbs->ext_io_cfg->rstPin, NRF_GPIO_PIN_NOPULL);
    hal_uwb_sleep_status_set(UWB_IS_NOT_SLEEPING);
    nrf_delay_us(1500);
}

/*
 * @brief disable DW_IRQ, reset DW3000
 *        and set
 *        app.DwCanSleep = DW_CANNOT_SLEEP;
 *        app.DwEnterSleep = DW_NOT_SLEEPING;
 * */
error_e hal_uwb_disable_irq_and_reset(int reset)
{
    taskENTER_CRITICAL();

    hal_uwb_disable_irq(); /**< disable NVIC IRQ until we configure the device */

    // this is called to reset the DW device
    if (reset)
    {
        hal_uwb_reset_chip();
    }

    hal_uwb_sleep_mode_set(UWB_CANNOT_SLEEP);
    hal_uwb_sleep_status_set(UWB_IS_NOT_SLEEPING);

    taskEXIT_CRITICAL();

    return _NO_ERR;
}

static void hal_uwb_port_stop_all_UWB(void)
{
    hal_uwb_disable_irq_and_reset(1);

    struct dwchip_s *dw = hal_uwb.uwbs->dw;

    dw->callbacks.cbTxDone = NULL;
    dw->callbacks.cbRxOk = NULL;
    dw->callbacks.cbRxTo = NULL;
    dw->callbacks.cbRxErr = NULL;
    dw->callbacks.cbSPIErr = NULL;
    dw->callbacks.cbSPIRdy = NULL;
    dw->callbacks.cbDualSPIEv = NULL;
}


/*! ----------------------------------------------------------------------------
 * Function: decamutexon()
 *
 * Description: This function should disable interrupts.
 *
 *
 * input parameters: void
 *
 * output parameters: uint16
 * returns the state of the DW1000 interrupt
 */
decaIrqStatus_t decamutexon(void)
{
    /* NRF chip has only 1 IRQ for all GPIO pins.
     * Disablin of the NVIC would not be of the best ideas.
     */
    uint32_t s = nrf_drv_gpiote_in_is_set(hal_uwb.uwbs->ext_io_cfg->irqPin);
    if (s)
    {
        nrf_drv_gpiote_in_event_disable(hal_uwb.uwbs->ext_io_cfg->irqPin);
    }
    return 0;
}

/*! ----------------------------------------------------------------------------
 * Function: decamutexoff()
 *
 * Description: This function should re-enable interrupts, or at least restore
 *              their state as returned(&saved) by decamutexon
 * This is called at the end of a critical section
 *
 * input parameters:
 * @param s - the state of the DW1000 interrupt as returned by decamutexon
 *
 * output parameters
 *
 * returns the state of the DW1000 interrupt
 */
void decamutexoff(decaIrqStatus_t s)
{
    if (s)
    {
        nrf_drv_gpiote_in_event_enable(hal_uwb.uwbs->ext_io_cfg->irqPin, true);
    }
}


/* @fn      port_CheckEXT_IRQ
 * @brief   wrapper to read DW_IRQ input pin state
 * */
static inline GPIO_PinState port_CheckEXT_IRQ(void)
{
    GPIO_PinState ret = GPIO_PIN_RESET;

    if (nrf_gpio_pin_read(hal_uwb.uwbs->ext_io_cfg->irqPin))
    {
        ret = GPIO_PIN_SET;
    }

    return ret;
}

/* @fn      process_deca_irq
 * @brief   main call-back for processing of DW3000 IRQ
 *          it re-enters the IRQ routing and processes all events.
 *          After processing of all events, DW3000 will clear the IRQ line.
 * */
static inline void process_deca_irq(void)
{
    while (port_CheckEXT_IRQ() == GPIO_PIN_SET)
    {
        dwt_isr();
    } // while DW3000 IRQ line active

    if (hal_uwb_sleep_status_get() & UWB_CAN_SLEEP_IN_IRQ)
    {
        hal_uwb_sleep_enter();
    }
}


static void deca_irq_handler(nrf_drv_gpiote_pin_t irqPin, nrf_gpiote_polarity_t irq_action)
{
    process_deca_irq();
}

void hal_uwb_init_irq(void)
{
    ret_code_t err_code;

    if (!nrf_drv_gpiote_is_init())
    {
        err_code = nrf_drv_gpiote_init();
        APP_ERROR_CHECK(err_code);

        nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
        in_config.pull = NRF_GPIO_PIN_PULLDOWN;

        err_code = nrf_drv_gpiote_in_init(hal_uwb.uwbs->ext_io_cfg->irqPin, &in_config, deca_irq_handler);
        APP_ERROR_CHECK(err_code);

        nrf_drv_gpiote_in_event_enable(hal_uwb.uwbs->ext_io_cfg->irqPin, false);
    }
}


void hal_uwb_mcu_sleep_config(void)
{
    // Activate deep sleep mode.
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    NRF_POWER->DCDCEN = 0;
}


void hal_uwb_mcu_hfclk_off(void)
{
#ifdef SOFTDEVICE_PRESENT
    if (nrf_sdh_is_enabled())
    {
        uint32_t hfclk_is_running = 1;
        sd_clock_hfclk_release();
        while (hfclk_is_running)
        {
            sd_clock_hfclk_is_running(&hfclk_is_running);
        }
    }
    else
#endif
    {
        nrf_drv_clock_hfclk_release();
        while (nrf_drv_clock_hfclk_is_running());
    }
}


void hal_uwb_mcu_hfclk_on(void)
{
    if (nrf_drv_clock_hfclk_is_running() == 0)
    {
#ifdef SOFTDEVICE_PRESENT
        if (nrf_sdh_is_enabled())
        {
            (void)sd_clock_hfclk_request();
        }
        else
#endif
        {
            nrf_drv_clock_hfclk_request(NULL);
            while (!nrf_drv_clock_hfclk_is_running());
        }
    }
}


aoa_enable_t hal_uwb_is_AoA(void)
{    
    rf_tuning_t *rf_tuning = get_rf_tuning_config();

#if (AOA_CHIP_ON_NON_AOA_PCB == 1)
    return AOA_DISABLED;
#endif

    /* First, check current antenna AoA Capabilities */
    switch (rf_tuning->antenna.port1)
    {
    /* AoA not supported */
    case ANT_TYPE_MAN5:
    case ANT_TYPE_CPWING5:
    case ANT_TYPE_CPWING9:
        return AOA_DISABLED;
        break;

    /* AoA supported, continue to check the chip */
    case ANT_TYPE_MONALISA5:
    case ANT_TYPE_MONALISA9:
    case ANT_TYPE_JOLIE5:
    case ANT_TYPE_JOLIE9:
        break;

    default:
        return AOA_ERROR;
    }

    /* Check Chip AoA Capabilities */
    switch (hal_uwb.uwbs->devid)
    {
    case DWT_DW3000_PDOA_DEV_ID:
    case DWT_DW3700_PDOA_DEV_ID:
    case DWT_DW3720_PDOA_DEV_ID:

        return AOA_ENABLED;
        break;

    case DWT_DW3000_DEV_ID:
    case DWT_QM33110_DEV_ID:
        return AOA_DISABLED;
        break;
    default:
        return AOA_ERROR;
    }
}


/* */
static inline void init(void)
{
    hal_uwb.uwbs = &uwbs;
    hal_uwb.uwbs->dw = &local_hal_dw;
    hal_uwb.uwbs->spi = hal_uwb_init_spi(hal_uwb.uwbs->spi_io_cfg);
    hal_uwb.wakeup_with_io();
}


static inline int probe(void)
{
    int ret;

    /**/
    dw3000_spi_fct.readfromspi = read_from_spi;
    dw3000_spi_fct.writetospi = write_to_spi;
    dw3000_spi_fct.writetospiwithcrc = write_to_spi_with_crc;
    dw3000_spi_fct.setslowrate = spi_slow_rate;
    dw3000_spi_fct.setfastrate = spi_fast_rate;


    /**/
    dw3000_probe_interf.dw = hal_uwb.uwbs->dw;
    dw3000_probe_interf.spi = (void *)&dw3000_spi_fct;
    dw3000_probe_interf.wakeup_device_with_io = hal_uwb.wakeup_with_io;

    ret = dwt_probe((struct dwt_probe_s *)&dw3000_probe_interf);

    if (ret == 0)
    {
        // Device ID address is common in between all DW chips
        uint8_t addr = 0x00; // DW3XXX_DEVICE_ID;
        uint8_t buf[4];
        read_from_spi(sizeof(uint8_t), &addr, sizeof(buf), buf);
        hal_uwb.uwbs->devid = buf[3] << 24 | buf[2] << 16 | buf[1] << 8 | buf[0];
    }

    return ret;
}

static void hal_deinit_callback(void)
{
    dwt_setcallbacks(NULL, NULL, NULL, NULL, NULL, NULL, NULL); // DW_IRQ is disabled: safe to cancel all user call-backs
}

static bool hal_uwb_return_false(void)
{
    return false;
}

struct hal_uwb_s hal_uwb = {
    .init = init,   // init spi/comm to the chip
    .probe = probe, // probe driver chip

    .irq_init = hal_uwb_init_irq,      // init the UWB IRQ of the MCU
    .disableIRQ = hal_uwb_disable_irq, // disable UWB IRQ on MCU
    .enableIRQ = hal_uwb_enable_irq,   // enable UWB IRQ on MCU
    .reset = hal_uwb_reset_chip,       // HW reset of UWB

    /* decoupled from MCU to chip IO */
    .wakeup_start = hal_uwb_wakeup_start,
    .wakeup_end   = hal_uwb_wakeup_end,
    .wakeup_fast  = hal_uwb_wakeup_fast,
    .wakeup_with_io = hal_uwb_wakeup_with_io,

    .sleep_config = hal_uwb_sleep_config,
    .sleep_enter = hal_uwb_sleep_enter,
    .sleep_status_set = hal_uwb_sleep_status_set,
    .sleep_status_get = hal_uwb_sleep_status_get,
    .sleep_mode_set = hal_uwb_sleep_mode_set,
    .sleep_mode_get = hal_uwb_sleep_mode_get,


    .mcu_sleep_config = hal_uwb_mcu_sleep_config, /* MCU specific preconfig */
    .mcu_suspend_clk = hal_uwb_mcu_hfclk_off,     /* MCU XTAL -> MCU RC  */
    .mcu_resume_clk = hal_uwb_mcu_hfclk_on,       /* MCU RC -> MCU XTAL */

    // review need of below
    //.disable_wake_init = hal_uwb_disable_wake_init,
    .stop_all_uwb = hal_uwb_port_stop_all_UWB,
    .disable_irq_and_reset = hal_uwb_disable_irq_and_reset,
    .deinit_callback = hal_deinit_callback,

    .is_aoa = hal_uwb_is_AoA,

    .is_sip = hal_uwb_return_false,
    .sip_configure = NULL,

    .uwbs = NULL
};