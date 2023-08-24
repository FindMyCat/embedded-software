/**
 * @file      HAL_timer.h
 *
 * @brief     Header for HAL_timer
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

#ifndef HAL_TIMER_H
#define HAL_TIMER_H 1

#include <stdbool.h>
#include <stdint.h>

struct hal_fs_timer_s
{
    /* timer instance */
    void *timer;

    /* timer external ISR */
    void (*timerIsr)(void *dwchip);

    /* @brief Initialization/uninit of the Fast Sleep Timer
     *        cb - is the "timerIsr" or NULL to stop and disable the timer
     */
    void (*init)(void *self, void *dwchip, void (*cb)(void *));

    /* @brief  start the timer
     *         int_en    - True/False to generate an interrupt on counter compare event
     *         time_us   - is time to wait in microseconds
     *         corr_tick - the number of timer's ticks it needs to start the counting from
     */
    void (*start)(void *self, bool int_en, uint32_t next_cc_us, int corr_tick);

    /* @brief stop the timer
     */
    void (*stop)(void *self);

    /* @brief returns the fast sleep timer current tick value
     * this will be the number of ticks since its last reset (since last CC event)
     */
    uint32_t const (*get_tick)(void *self);

    /* timer tick frequency */
    int freq;
};

typedef struct hal_fs_timer_s hal_fs_timer_t;

/* public fast sleep timer */
extern hal_fs_timer_t hal_fs_timer;

/* public function prototypes */
int64_t mcps_get_uptime_us(void);

struct hal_timer_s
{
    uint32_t (*init)(void);
    void (*start)(volatile uint32_t *p_timestamp);
    bool (*check)(uint32_t timestamp, uint32_t time);
};

extern const struct hal_timer_s Timer;
#endif