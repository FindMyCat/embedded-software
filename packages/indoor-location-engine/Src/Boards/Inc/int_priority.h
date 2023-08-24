/**
 * @file      int_priority.h
 *
 * @brief     Task and interrupt definitions
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

#ifndef INT_PRIORITY_H
#define INT_PRIORITY_H

#include "cmsis_os.h"
#include "custom_board.h"

enum Priorities {
    /* Interrupt safe FreeRTOS API functions below is CMSIS IRQ's */
    PRIO_RTC_WKUP_IRQn      = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY, //equivalent to the highest in the FreeRTOS
    PRIO_OTG_FS_IRQn        = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY+1,
    PRIO_ETH_IRQn           = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY+1,
    PRIO_TIM1_CC_IRQn       = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY+1,
    PRIO_TIM3_IRQn          = configLIBRARY_LOWEST_INTERRUPT_PRIORITY,

    /* Application-specific priorities : CMSIS-OS priorities +3..0..-3
     * osPriorityRealtime  is not used by Application level
     * osPriorityHigh       => configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + ?
     * osPriorityAboveNormal
     *
     * */
    PRIO_FlushTask          = osPriorityAboveNormal, /* FlushTask should have higher priority than CalckTask */
    PRIO_CtrlTask           = osPriorityNormal,
    PRIO_StartDefaultTask   = osPriorityLow,

    PRIO_RxTask             = osPriorityHigh,
    PRIO_CalcTask           = osPriorityNormal,
    PRIO_TrilatTask         = osPriorityBelowNormal,

    PRIO_TagPollTask        = osPriorityHigh,
    PRIO_TagRxTask          = osPriorityHigh,
    PRIO_BlinkTask          = osPriorityNormal,

    PRIO_TcfmTask           = osPriorityNormal,
    PRIO_TcwmTask           = osPriorityNormal,
    PRIO_Usb2SpiTask        = osPriorityNormal,

    PRIO_UciTask            = osPriorityNormal


};
#endif
