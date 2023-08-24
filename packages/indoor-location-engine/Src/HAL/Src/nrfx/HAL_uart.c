/**
 * @file    HAL_uart.c
 *
 * @brief   HAL implementation of UART functionalities
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

#include "boards.h"
#include "nrf_uart.h"
#include "app_uart.h"
#include "circular_buffers.h"
#include "HAL_error.h"
#include "HAL_timer.h"
#include "HAL_uart.h"

extern void NotifyControlTask(void);

/******************************************************************************
 *
 *                              Local variables
 *
 ******************************************************************************/

static bool discard_next_symbol = false;
static uint32_t UART_timeout;
static bool UART_is_down = false;

static data_circ_buf_t _uartRx;
data_circ_buf_t *uartRx = &_uartRx;


void Restart_UART_timer()
{
    Timer.start(&UART_timeout);
}

bool IsUartDown(void)
{
    return UART_is_down;
}

void SetUartDown(bool val)
{
    UART_is_down = val;
}


/* @fn  uart_error_handle
 *
 * @param[in] p_event Pointer to UART event.
 * */
void deca_uart_error_handle(app_uart_evt_t *p_event)
{
    if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
    {
        error_handler(0, _ERR_General_Error);
    }
    else if (p_event->evt_type == APP_UART_FIFO_ERROR)
    {
        error_handler(0, _ERR_General_Error);
    }
    /* This event indicates that UART data has been received  */
    else if (p_event->evt_type == APP_UART_DATA_READY)
    {
        deca_uart_receive();
    }
}

/* @fn  deca_uart_init
 *
 * @brief Function for initializing the UART module.
 *
 * @param[in] void
 * */
void deca_uart_init(void)
{
    uint32_t err_code;
    const app_uart_comm_params_t comm_params = {
        UART_0_RX_PIN,
        UART_0_TX_PIN,
        RTS_PIN_NUMBER,
        CTS_PIN_NUMBER,
        APP_UART_FLOW_CONTROL_DISABLED,
        false,
        UART_BAUDRATE_BAUDRATE_Baud115200};

    // Tx and Rx buffers have the same size
    APP_UART_FIFO_INIT(&comm_params,
                       UART_RX_BUF_SIZE,
                       UART_RX_BUF_SIZE,
                       deca_uart_error_handle,
                       APP_IRQ_PRIORITY_LOW,
                       err_code);
    UNUSED_VARIABLE(err_code);
}

void deca_uart_close(void)
{
    app_uart_flush();
    app_uart_close();
}

/* @fn  deca_uart_transmit
 *
 * @brief Function for transmitting data on UART
 *
 * @param[in] ptr Pointer is contain base address of data.
 * */
int deca_uart_transmit(uint8_t *ptr, uint16_t size)
{
    uint32_t ret = NRF_SUCCESS;
    for (int i = 0; i < size; i++)
    {
        if (app_uart_put(ptr[i]) != NRF_SUCCESS)
        {
            ret = _ERR_UART_TX;
            break;
        }
    }
    return ret == NRF_SUCCESS;
}

int deca_uart_putc(uint8_t car)
{
    app_uart_put(car);
    return 1;
}

/* @fn  deca_uart_receive
 *
 * @brief Function for receive data from UART and store into rx_buf
 *        (global array).
 *
 * @param[in] void
 * */
void deca_uart_receive(void)
{
    uint32_t err_code;
    uint8_t rx_data;
    int head, tail, size;

    head = uartRx->head;
    tail = uartRx->tail;
    size = UART_RX_BUF_SIZE;

    err_code = app_uart_get(&rx_data);
    err_code = err_code; // for warning
    if (discard_next_symbol)
    {
        discard_next_symbol = false;
    }
    else
    {
        if (CIRC_SPACE(head, tail, size) > 0)
        {
            uartRx->buf[head] = rx_data;
            head = (head + 1) & (size - 1);
            uartRx->head = head;
        }
        else
        {
            /* USB RX packet can not fit free space in the buffer */
        }
        NotifyControlTask();
    }
}

/* @fn  deca_discard_next_symbol
 *
 * @brief Discard next incoming symbol (used while wakening up UART from sleep as the first receiving symbol is a garbage)
 *
 * @param[in] void
 * */
void deca_discard_next_symbol(void)
{
    discard_next_symbol = true;
}
