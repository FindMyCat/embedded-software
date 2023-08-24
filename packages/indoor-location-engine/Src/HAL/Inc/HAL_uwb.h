/**
 * @file      HAL_uwb.h
 *
 * @brief     Headers for abstraction layer of uwb
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


#ifndef HAL_UWB_H
#define HAL_UWB_H

#include <stdint.h>
#include <stdbool.h>
#include "deca_error.h"
#include "deca_device_api.h"
#include "HAL_SPI.h"

struct uwbs_port_config_s
{
    const uint16_t irqPin;
    const uint16_t irqN;
    const uint16_t rstPin;
    const uint16_t rstIrqN;
    const uint16_t wakeUpPin;
};

struct uwbs_sip_config_s
{
    uint8_t tx_ant;
    uint8_t rxa_ant;
    uint8_t rxb_ant;
    uint8_t lna1;
    uint8_t lna2;
    uint8_t pa;
    uint8_t dual_rx;
    uint8_t channel;
};

struct dw_s
{
    uint32_t devid;
    const struct spi_port_config_s *spi_io_cfg;
    const struct uwbs_port_config_s *ext_io_cfg;
    const void *void_cfg;
    struct spi_s *spi;   /* spi instance to this chip */
    struct dwchip_s *dw; /* driver instance to the chip */
};

typedef struct dw_s dw_t;


typedef enum
{
    UWB_CANNOT_SLEEP     = 0x00,
    UWB_CAN_SLEEP_IN_IRQ = 0x01, // UWB can be put to sleep immediately after IRQ
    UWB_CAN_SLEEP_IN_APP = 0x02  // DW can be put to sleep by APP
} sleep_mode_t;

typedef enum
{
    UWB_IS_NOT_SLEEPING,
    UWB_IS_SLEEPING,
} sleep_status_t;


typedef enum
{
    AOA_ERROR,
    AOA_ENABLED,
    AOA_DISABLED
} aoa_enable_t;

/*Abstraction structur*/
struct hal_uwb_s
{
    void (*init)(void);             // init MCU I/O to the DW chip
    int (*probe)(void);             // probe the driver

    void (*irq_init)(void);         // init MCU IRQ line
    void (*enableIRQ)(void);        // enable IRQ from UWBS
    void (*disableIRQ)(void);       // disable IRQ from UWBS

    void (*reset)(void);            // HW reset of UWBS, Note: this will update the sleep status
    
    // UWB various wakeup mechanisms
    void (*wakeup_start)(void);     // start waking up UWBS, Note: this will update the sleep status
    void (*wakeup_end)(void);       // stop waking up UWBS
    void (*wakeup_fast)(void);      // wakeup UWBS without final pause, Note: this will update the sleep status
    void (*wakeup_with_io)(void);   // wakeup UWBS with guarantee. Note: this will update the sleep status

    // UWB sleeping
    sleep_status_t vsleep_status;
    void (*sleep_status_set)(sleep_status_t); // set local sleep status to the new value. Avoid to use this fn() directly.
    sleep_status_t (*sleep_status_get)(void); // get local UWB sleeping status
    void (*sleep_config)(void);               // configure UWB for sleeping
    void (*sleep_enter)(void);                // enter deep sleep if not already. Note: this will update the status

    sleep_mode_t vsleep_mode;
    void (*sleep_mode_set)(sleep_mode_t);     // specify in which mode UWB can sleep NONE/IRQ/APP
    sleep_mode_t (*sleep_mode_get)(void);     // get the current UWB can sleep mode

    // MCU sleeping
    void (*mcu_sleep_config)(void); // enable mcu-specific init features
    void (*mcu_suspend_clk)(void);  // turn off MCU clock XTAL
    void (*mcu_resume_clk)(void);   // turn on MCU clock XTAL

    aoa_enable_t (*is_aoa)(void);   // chip + PCB design both support AOA

    // SIP support
    bool (*is_sip)(void);           // check if it is a SIP device
    void (*sip_configure)(struct uwbs_sip_config_s *sip_cfg);    // configuring SIP device

    struct dw_s *uwbs;              // UWB Subsystem connection: SPI, I/O and Driver

    void (*stop_all_uwb)(void);
    error_e (*disable_irq_and_reset)(int);
    void (*deinit_callback)(void);
};

extern struct hal_uwb_s hal_uwb;

#endif // HAL_UWB_H
