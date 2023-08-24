/**
 * @file    HAL_SPI.c
 *
 * @brief   Hardware abstraction layer for SPI
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
#include <string.h>

#include "HAL_lock.h"
#include "HAL_SPI.h"
#include "deca_error.h"
#include "boards.h"
#include "nrf_delay.h"
#include "nrf_drv_spi.h"
#include "nrf_error.h"

static void spi_slow_rate_(void *handler);
static void spi_fast_rate_(void *handler);
static void spi_cs_low_(void *handler);
static void spi_cs_high_(void *handler);
static int readfromspi_(void *handler, uint16_t headerLength, const uint8_t *headerBuffer, uint16_t readlength, uint8_t *readBuffer);
static int writetospi_(void *handler, uint16_t headerLength, const uint8_t *headerBuffer, uint16_t bodylength, const uint8_t *bodyBuffer);

typedef struct
{
    nrf_drv_spi_t spi_inst;
    uint32_t frequency_slow;
    uint32_t frequency_fast;
    uint32_t cs_pin;
    nrf_drv_spi_config_t spi_config;
    dw_hal_lockTypeDef lock;
} spi_handle_t;

static spi_handle_t spi_handler0 = {
    /* below will be configured in the port_init_dw_chip() */
    .spi_inst = {0},
    .frequency_slow = 0,
    .frequency_fast = 0,
    .spi_config = {0},
    .cs_pin = 0,
    .lock = DW_HAL_NODE_UNLOCKED};

#if NRFX_SPIM3_ENABLED == 1
static spi_handle_t spi_handler3 = {
    /* below will be configured in the port_init_dw_chip() */
    .spi_inst = {0},
    .frequency_slow = 0,
    .frequency_fast = 0,
    .spi_config = {0},
    .cs_pin = 0,
    .lock = DW_HAL_NODE_UNLOCKED};
#endif

#if NRFX_SPIM0_ENABLED == 1
static const struct spi_s spim0 = {
    .cs_high = spi_cs_high_,
    .cs_low = spi_cs_low_,
    .slow_rate = spi_slow_rate_,
    .fast_rate = spi_fast_rate_,
    .read = readfromspi_,
    .write = writetospi_,
    .write_with_crc = NULL,
    .handler = &spi_handler0};
#endif

#if NRFX_SPIM3_ENABLED == 1
static const struct spi_s spim3 = {
    .cs_high = spi_cs_high_,
    .cs_low = spi_cs_low_,
    .slow_rate = spi_slow_rate_,
    .fast_rate = spi_fast_rate_,
    .read = readfromspi_,
    .write = writetospi_,
    .write_with_crc = NULL,
    .handler = &spi_handler3};
#endif

#define DATALEN1 200
static uint8_t idatabuf[DATALEN1] = {0}; // Never define this inside the Spi read/write
static uint8_t itempbuf[DATALEN1] = {0}; // As that will use the stack from the Task, which are not such long!!!!
                                         // You will face a crashes which are not expected!
uint8_t spi_init_stat = 0; // use 1 for slow, use 2 for fast;


//==============================================================================
const struct spi_s *init_spi(const spi_port_config_t *port_cfg)
{
    nrf_drv_spi_t *spi_inst;
    nrf_drv_spi_config_t *spi_config;
    spi_handle_t *spi_handler;

    const struct spi_s *spi_ret = NULL;

#if NRFX_SPIM0_ENABLED == 1
    if (port_cfg->idx == 0)
    {
        spi_handler = &spi_handler0;
        spi_inst = &spi_handler0.spi_inst;
        spi_config = &spi_handler0.spi_config;
        spi_inst->inst_idx = SPI0_INSTANCE_INDEX;
        spi_inst->use_easy_dma = SPI0_USE_EASY_DMA;
        spi_inst->u.spim.p_reg = NRF_SPIM0;
        spi_inst->u.spim.drv_inst_idx = NRFX_SPIM0_INST_IDX;
        spi_ret = &spim0;
    }
#endif
#if NRFX_SPIM3_ENABLED == 1
    if (port_cfg->idx == 3)
    {
        spi_handler = &spi_handler3;
        spi_inst = &spi_handler3.spi_inst;
        spi_config = &spi_handler3.spi_config;
        spi_inst->inst_idx = SPI3_INSTANCE_INDEX;
        spi_inst->use_easy_dma = SPI3_USE_EASY_DMA;
        spi_inst->u.spim.p_reg = NRF_SPIM3;
        spi_inst->u.spim.drv_inst_idx = NRFX_SPIM3_INST_IDX;
        spi_ret = &spim3;
    }
#endif
    spi_handler->lock = DW_HAL_NODE_UNLOCKED;
    spi_handler->frequency_slow = port_cfg->min_freq;
    spi_handler->frequency_fast = port_cfg->max_freq;
    spi_handler->cs_pin = port_cfg->cs;

    spi_config->sck_pin = port_cfg->clk;
    spi_config->mosi_pin = port_cfg->mosi;
    spi_config->miso_pin = port_cfg->miso;
    spi_config->ss_pin = NRFX_SPIM_PIN_NOT_USED; // pin driven manually, not by the driver

#ifdef SOFTDEVICE_PRESENT
    spi_config->irq_priority = (APP_IRQ_PRIORITY_MID - 1);
#else
    spi_config->irq_priority = (APP_IRQ_PRIORITY_MID - 2);
#endif
    spi_config->orc = 0xFF;
    spi_config->frequency = spi_handler->frequency_slow;
    spi_config->mode = NRF_DRV_SPI_MODE_0;
    spi_config->bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;
    nrf_gpio_cfg_output(spi_handler->cs_pin);
    nrf_gpio_pin_set(spi_handler->cs_pin);
    return spi_ret;
}

void close_spi(nrf_drv_spi_t *p_instance)
{
    NRF_SPIM_Type *p_spi = p_instance->u.spim.p_reg;
    nrf_spim_disable(p_spi);
}

void open_spi(nrf_drv_spi_t *p_instance)
{
    NRF_SPIM_Type *p_spi = p_instance->u.spim.p_reg;
    nrf_spim_enable(p_spi);
}

static void spi_cs_low_(void *handler)
{
    spi_handle_t *spi_handler = handler;
    nrf_gpio_pin_clear(spi_handler->cs_pin);
}

static void spi_cs_high_(void *handler)
{
    spi_handle_t *spi_handler = handler;
    nrf_gpio_pin_set(spi_handler->cs_pin);
}

/* @fn      spi_slow_rate
 * @brief   set 2MHz
 *
 * */
static void spi_slow_rate_(void *handler)
{
    spi_handle_t *spi_handler = handler;
    // set_SPI_master();
    if (spi_init_stat == 1)
    {
        return;
    }
    else
    {
        if (spi_init_stat == 2)
        {
            nrf_drv_spi_uninit(&spi_handler->spi_inst);
        }

        spi_handler->spi_config.frequency = spi_handler->frequency_slow;

        APP_ERROR_CHECK(nrf_drv_spi_init(&spi_handler->spi_inst, &spi_handler->spi_config, NULL, NULL));
        spi_init_stat = 1;
    }
}

/* @fn      spi_fast_rate
 * @brief   set 16MHz
 *
 * */
static void spi_fast_rate_(void *handler)
{
    spi_handle_t *spi_handler = handler;
    if (spi_init_stat == 2)
    {
        return;
    }
    else
    {
        if (spi_init_stat == 1)
        {
            nrf_drv_spi_uninit(&spi_handler->spi_inst);
        }
        spi_handler->spi_config.frequency = spi_handler->frequency_fast;

        APP_ERROR_CHECK(nrf_drv_spi_init(&spi_handler->spi_inst, &spi_handler->spi_config, NULL, NULL));

        // HIGH drive is mandatory when operating @ 32MHz
        nrf_gpio_cfg(spi_handler->spi_config.sck_pin,
                     NRF_GPIO_PIN_DIR_OUTPUT,
                     NRF_GPIO_PIN_INPUT_CONNECT,
                     NRF_GPIO_PIN_NOPULL,
                     NRF_GPIO_PIN_H0H1,
                     NRF_GPIO_PIN_NOSENSE);

        nrf_gpio_cfg(spi_handler->spi_config.mosi_pin,
                     NRF_GPIO_PIN_DIR_OUTPUT,
                     NRF_GPIO_PIN_INPUT_DISCONNECT,
                     NRF_GPIO_PIN_NOPULL,
                     NRF_GPIO_PIN_H0H1,
                     NRF_GPIO_PIN_NOSENSE);

        spi_init_stat = 2;
    }
}

static int readfromspi_(void *handler, uint16_t headerLength, const uint8_t *headerBuffer, uint16_t readlength, uint8_t *readBuffer)
{
    spi_handle_t *spi_handler = handler;

    while (spi_handler->lock) {}

    QHAL_LOCK(spi_handler);

    open_spi(&spi_handler->spi_inst);

    nrf_gpio_pin_clear(spi_handler->cs_pin);

    if (headerLength + readlength <= 8)
    {
        uint8_t out[8] = {0};
        uint8_t in[8];
        int idx = 0;
        for (idx = 0; idx < headerLength; idx++)
        {
            out[idx] = headerBuffer[idx];
        }
        nrfx_spim_xfer_desc_t const spim_xfer_desc_header = {
            .p_tx_buffer = out,
            .tx_length = headerLength + readlength,
            .p_rx_buffer = in,
            .rx_length = headerLength + readlength};
        nrfx_spim_xfer(&spi_handler->spi_inst.u.spim, &spim_xfer_desc_header, 0);
        for (int i = 0; idx < headerLength + readlength; idx++, i++)
        {
            readBuffer[i] = in[idx];
        }
    }
    else
    {
        nrfx_spim_xfer_desc_t const spim_xfer_desc_header = {
            .p_tx_buffer = headerBuffer,
            .tx_length = headerLength,
            .p_rx_buffer = NULL,
            .rx_length = 0};
        nrfx_spim_xfer(&spi_handler->spi_inst.u.spim, &spim_xfer_desc_header, 0);

        nrfx_spim_xfer_desc_t const spim_xfer_desc_body = {
            .p_tx_buffer = NULL,
            .tx_length = 0,
            .p_rx_buffer = readBuffer,
            .rx_length = readlength,
        };
        nrfx_spim_xfer(&spi_handler->spi_inst.u.spim, &spim_xfer_desc_body, 0);
    }
    nrf_gpio_pin_set(spi_handler->cs_pin);

    close_spi(&spi_handler->spi_inst);

    QHAL_UNLOCK(spi_handler);

    return 0;
}

static int writetospi_(void *handler, uint16_t headerLength, const uint8_t *headerBuffer, uint16_t bodylength, const uint8_t *bodyBuffer)
{
    spi_handle_t *spi_handler = handler;
    uint8_t *p1;
    uint32_t idatalength = headerLength + bodylength;

    if (idatalength > DATALEN1)
    {
        return NRF_ERROR_NO_MEM;
    }

    while (spi_handler->lock) {}

    QHAL_LOCK(spi_handler);

    open_spi(&spi_handler->spi_inst);

    nrf_gpio_pin_clear(spi_handler->cs_pin);

    p1 = idatabuf;
    memcpy(p1, headerBuffer, headerLength);
    p1 += headerLength;
    memcpy(p1, bodyBuffer, bodylength);

    idatalength = headerLength + bodylength;

    nrf_drv_spi_transfer(&spi_handler->spi_inst, idatabuf, idatalength, itempbuf, idatalength);

    nrf_gpio_pin_set(spi_handler->cs_pin);

    close_spi(&spi_handler->spi_inst);

    QHAL_UNLOCK(spi_handler);

    return 0;
}
