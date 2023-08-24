/**
 * @file      custom_board.h
 *
 * @brief     pin mapping description corresponding to nRF52840DK module
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
 */
#ifndef BOARD_DWM3001_A1_H
#define BOARD_DWM3001_A1_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nrf_gpio.h"
#include "nrf_spim.h"


// LEDs definitions for PCA10056
#define LEDS_NUMBER    4

#define LED_1          NRF_GPIO_PIN_MAP(0,13)
#define LED_2          NRF_GPIO_PIN_MAP(0,14)
#define LED_3          NRF_GPIO_PIN_MAP(0,15)
#define LED_4          NRF_GPIO_PIN_MAP(0,16)
#define LED_START      LED_1
#define LED_STOP       LED_4

#define LEDS_ACTIVE_STATE 0

#define LEDS_LIST { LED_1, LED_2, LED_3, LED_4 }

#define LEDS_INV_MASK  LEDS_MASK

#define BSP_LED_0      13
#define BSP_LED_1      14
#define BSP_LED_2      15
#define BSP_LED_3      16

#define BUTTONS_NUMBER 4

#define BUTTON_1       11
#define BUTTON_2       12
#define BUTTON_3       24
#define BUTTON_4       25
#define BUTTON_PULL    NRF_GPIO_PIN_PULLUP

#define BUTTONS_ACTIVE_STATE 0

#define BUTTONS_LIST { BUTTON_1, BUTTON_2, BUTTON_3, BUTTON_4 }

#define BSP_BUTTON_0   BUTTON_1
#define BSP_BUTTON_1   BUTTON_2
#define BSP_BUTTON_2   BUTTON_3
#define BSP_BUTTON_3   BUTTON_4

#define RX_PIN_NUMBER  8
#define TX_PIN_NUMBER  6
#define CTS_PIN_NUMBER 7
#define RTS_PIN_NUMBER 5
#define HWFC           true

#define BSP_QSPI_SCK_PIN   19
#define BSP_QSPI_CSN_PIN   17
#define BSP_QSPI_IO0_PIN   20
#define BSP_QSPI_IO1_PIN   21
#define BSP_QSPI_IO2_PIN   22
#define BSP_QSPI_IO3_PIN   23


// serialization APPLICATION board - temp. setup for running serialized MEMU tests
#define SER_APP_RX_PIN              NRF_GPIO_PIN_MAP(1,13)    // UART RX pin number.
#define SER_APP_TX_PIN              NRF_GPIO_PIN_MAP(1,14)    // UART TX pin number.
#define SER_APP_CTS_PIN             NRF_GPIO_PIN_MAP(0,2)     // UART Clear To Send pin number.
#define SER_APP_RTS_PIN             NRF_GPIO_PIN_MAP(1,15)    // UART Request To Send pin number.

#define SER_APP_SPIM0_SCK_PIN       NRF_GPIO_PIN_MAP(0,27)    // SPI clock GPIO pin number.
#define SER_APP_SPIM0_MOSI_PIN      NRF_GPIO_PIN_MAP(0,2)     // SPI Master Out Slave In GPIO pin number
#define SER_APP_SPIM0_MISO_PIN      NRF_GPIO_PIN_MAP(0,26)    // SPI Master In Slave Out GPIO pin number
#define SER_APP_SPIM0_SS_PIN        NRF_GPIO_PIN_MAP(1,13)    // SPI Slave Select GPIO pin number
#define SER_APP_SPIM0_RDY_PIN       NRF_GPIO_PIN_MAP(1,15)    // SPI READY GPIO pin number
#define SER_APP_SPIM0_REQ_PIN       NRF_GPIO_PIN_MAP(1,14)    // SPI REQUEST GPIO pin number

// serialization CONNECTIVITY board
#define SER_CON_RX_PIN              NRF_GPIO_PIN_MAP(1,14)    // UART RX pin number.
#define SER_CON_TX_PIN              NRF_GPIO_PIN_MAP(1,13)    // UART TX pin number.
#define SER_CON_CTS_PIN             NRF_GPIO_PIN_MAP(1,15)    // UART Clear To Send pin number. Not used if HWFC is set to false.
#define SER_CON_RTS_PIN             NRF_GPIO_PIN_MAP(0,2)     // UART Request To Send pin number. Not used if HWFC is set to false.


#define SER_CON_SPIS_SCK_PIN        NRF_GPIO_PIN_MAP(0,27)    // SPI SCK signal.
#define SER_CON_SPIS_MOSI_PIN       NRF_GPIO_PIN_MAP(0,2)     // SPI MOSI signal.
#define SER_CON_SPIS_MISO_PIN       NRF_GPIO_PIN_MAP(0,26)    // SPI MISO signal.
#define SER_CON_SPIS_CSN_PIN        NRF_GPIO_PIN_MAP(1,13)    // SPI CSN signal.
#define SER_CON_SPIS_RDY_PIN        NRF_GPIO_PIN_MAP(1,15)    // SPI READY GPIO pin number.
#define SER_CON_SPIS_REQ_PIN        NRF_GPIO_PIN_MAP(1,14)    // SPI REQUEST GPIO pin number.

#define SER_CONN_CHIP_RESET_PIN     NRF_GPIO_PIN_MAP(1,1)     // Pin used to reset connectivity chip

// Arduino board mappings
#define ARDUINO_SCL_PIN             27    // SCL signal pin
#define ARDUINO_SDA_PIN             26    // SDA signal pin
#define ARDUINO_AREF_PIN            2     // Aref pin

#define ARDUINO_13_PIN              NRF_GPIO_PIN_MAP(1, 15)   // Digital pin 13
#define ARDUINO_12_PIN              NRF_GPIO_PIN_MAP(1, 14)   // Digital pin 12
#define ARDUINO_11_PIN              NRF_GPIO_PIN_MAP(1, 13)   // Digital pin 11
#define ARDUINO_10_PIN              NRF_GPIO_PIN_MAP(1, 12)   // Digital pin 10
#define ARDUINO_9_PIN               NRF_GPIO_PIN_MAP(1, 11)   // Digital pin 9
#define ARDUINO_8_PIN               NRF_GPIO_PIN_MAP(1, 10)   // Digital pin 8
 
#define ARDUINO_7_PIN               NRF_GPIO_PIN_MAP(1, 8)    // Digital pin 7
#define ARDUINO_6_PIN               NRF_GPIO_PIN_MAP(1, 7)    // Digital pin 6
#define ARDUINO_5_PIN               NRF_GPIO_PIN_MAP(1, 6)    // Digital pin 5
#define ARDUINO_4_PIN               NRF_GPIO_PIN_MAP(1, 5)    // Digital pin 4
#define ARDUINO_3_PIN               NRF_GPIO_PIN_MAP(1, 4)    // Digital pin 3
#define ARDUINO_2_PIN               NRF_GPIO_PIN_MAP(1, 3)    // Digital pin 2
#define ARDUINO_1_PIN               NRF_GPIO_PIN_MAP(1, 2)    // Digital pin 1
#define ARDUINO_0_PIN               NRF_GPIO_PIN_MAP(1, 1)    // Digital pin 0

#define ARDUINO_A0_PIN              3     // Analog channel 0
#define ARDUINO_A1_PIN              4     // Analog channel 1
#define ARDUINO_A2_PIN              28    // Analog channel 2
#define ARDUINO_A3_PIN              29    // Analog channel 3
#define ARDUINO_A4_PIN              30    // Analog channel 4
#define ARDUINO_A5_PIN              31    // Analog channel 5

#define DW3000_RST_Pin              ARDUINO_7_PIN
#define DW3000_IRQ_Pin              ARDUINO_8_PIN
#define DW3000_WUP_Pin              ARDUINO_9_PIN

// SPI defs
#define DW3000_CS_Pin               ARDUINO_10_PIN
#define DW3000_CLK_Pin              ARDUINO_13_PIN            // DWM3000 shield SPIM1 sck connected to DW1000
#define DW3000_MOSI_Pin             ARDUINO_11_PIN            // DWM3000 shield SPIM1 mosi connected to DW1000
#define DW3000_MISO_Pin             ARDUINO_12_PIN            // DWM3000 shield SPIM1 miso connected to DW1000
#define DW3000_SPI_IRQ_PRIORITY     APP_IRQ_PRIORITY_LOW

// UART symbolic constants
#define UART_0_TX_PIN               TX_PIN_NUMBER             // DWM1001 module pin 20, DEV board name RXD
#define UART_0_RX_PIN               RX_PIN_NUMBER             // DWM1001 module pin 18, DEV board name TXD
#define DW3000_RTS_PIN_NUM          UART_PIN_DISCONNECTED
#define DW3000_CTS_PIN_NUM          UART_PIN_DISCONNECTED

#define LED_ERROR BSP_LED_0

#define DW3000_MIN_SPI_FREQ         (NRF_SPIM_FREQ_4M)
#define DW3000_MAX_SPI_FREQ         (NRF_SPIM_FREQ_32M)
#define DW3000_SPI_IDX	            (3)


#ifdef __cplusplus
}
#endif

#endif // PCA10100_H
