/**
 * @file   dw3000.h
 *
 * @brief  Definition of DW3000 parameters
 *
 * @author Decawave Applications
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

#ifndef __DW3000_H
#define __DW3000_H

#include <linux/skbuff.h>
#include <net/mcps802154.h>
#include "deca_device_api.h"

/* Time units and conversion factor for MCU implementation */
// #define DW3000_CHIP_FREQ 499200000
// #define DW3000_CHIP_PER_DTU 2 /* this is "true" driver dtu time, i.e. 4.0064ns */

// RCTU: resolution 15.65ps
// DTU: resolution 4.0064ns
// DTU_TX: resolution 2*DTU
// DLY: resolution 4*RCTU
// RSTU: resolution is 0.8333333 us

#define DW3000_RCTU_PER_CHIP 128
#define DW3000_RCTU_PER_DTU  (DW3000_RCTU_PER_CHIP * DW3000_CHIP_PER_DTU)
#define DW3000_RCTU_PER_DLY  (DW3000_CHIP_PER_DLY / DW3000_RCTU_PER_CHIP)

/* 6.9.1.5 in 4z, for HRP UWB PHY:
   416 chips = 416 / (499.2 * 10^6) ~= 833.33 ns */
#define DW3000_DTU_PER_RSTU (416 / DW3000_CHIP_PER_DTU)
#define DW3000_DTU_PER_DLY  (DW3000_CHIP_PER_DLY / DW3000_CHIP_PER_DTU)

#define DW3000_ANTICIP_DTU (16 * (DW3000_DTU_FREQ / 1000))

#define DW3000_RX_ENABLE_STARTUP_DLY 16 /* time to power-up the chip*/
#define DW3000_RX_ENABLE_STARTUP_DTU \
    (DW3000_RX_ENABLE_STARTUP_DLY * DW3000_CHIP_PER_DLY / DW3000_CHIP_PER_DTU)

/* Additional information on rx. */
enum dw3000_rx_flags
{
    /* Set if an automatix ack is send. */
    DW3000_RX_FLAG_AACK = BIT(0),
    /* Set if no data. */
    DW3000_RX_FLAG_ND = BIT(1),
};

#define device dwchip_s

#endif /* __DW3000_H */
