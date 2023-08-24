/**
 * @file      HAL_SPI_SE.c
 *
 * @brief     Header for HAL for SPI connected to Thales SE
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

#ifndef HAL_SPI_SE_H
#define HAL_SPI_SE_H

#include <stdint.h>

/* APDU Transport over SPI / I2C – Public Release v1.0
   § 3.1.7 Default Parameter Values
*/
#define SE_SPI_DPWT             25          // Default Power Wake-Up Time (ms)
#define SE_SPI_DMCF             1000        // Default Maximum Clock Frequency (kHz)
#define SE_SPI_DMPOT            1000        // Default Minimum Polling Time (us)
#define SE_SPI_DSEGT            200         // Default SE Guard Time (us)
#define SE_SPI_DWUT             200         // Default Wake-Up Time (us)
#define SE_SPI_DSEAL            16          // Default SEAL (byte)
#define SE_SPI_DBWT             300         // Default Block Waiting Time (ms)

/* APDU Transport over SPI / I2C – Public Release v1.0
   § 3.1.2.1 Signal Conventions

   The SPI communication shall use the “mode 0” configuration as described below:
   * The SPI Clock Polarity bit (CPOL) shall be 0 (non-inverted clock), meaning that the
     idle state of the clock is low and that trigger is initiated at a raising clock signal
     (Active-high clocks selected).
   * The SPI Clock Phase bit (CPHA) shall be 0, meaning that the sampling of data occurs
     at a raising clock signal of the SCL signal. The data signals are set at chip selection
     and falling clock signals.
   * The SCL frequency shall not exceed the Maximum Clock Frequency (MCF) defined by the CIP.
     If the HD doesn’t know which MCF to use (e.g. CIP not retrieved yet), the Default
     Maximum Clock Frequency (DMCF) shall be used.
   * The number of bits transmitted via MOSI or MISO signal during a single SPI access shall
     be a multiple of eight.
   * The Most Significant Bit (msb) shall be sent first.
*/
#define SE_SPI_OP (SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8) | SPI_LINES_SINGLE)

/**
 * @fn const struct spi_se_s *init_spi_se(int spiIdx)
 *
 * @brief   initialisation of SPI connected to SE
 *
 * init the SDK HAL SPI driver and configure the SPI pins
 *
 * @param spiIdx spi index from 0 to n depending on target
 *
 * @return pointer on SPI SE driver *
 */
const struct spi_se_s *init_spi_se();

/**
 * @struct spi_se_s
 *
 * @brief   HAL SPI SE function pointer table
 *
 * any SDK depending implentation must define this table
 *
 * @var cs_low set SPI pin CS low
 *
 * @var cs_high set SPI pin CS HIGH
 *
 */
struct spi_se_s
{
    int (*read)(void *handler,
                uint16_t readlength, uint8_t *readBuffer);
    int (*write)(void *handler,
                 uint16_t readlength, const uint8_t *readBuffer);
    int (*write_with_crc)(void *handler,
                          uint16_t bodyLength, const uint8_t *bodyBuffer,
                          uint8_t crc8);
    void *handler;
};

#endif /* HAL_SPI_SE_H */
