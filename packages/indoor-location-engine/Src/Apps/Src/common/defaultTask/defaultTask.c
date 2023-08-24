/**
 * @file      defaultTask.c
 *
 * @brief     Default Task
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

#include <stdlib.h>
#include <string.h>

#include "EventManager.h"
#include "HAL_uwb.h"
#include "HAL_watchdog.h"
#include "app.h"
#include "apps_config.h"
#include "cmsis_os.h"
#include "create_default_task.h"
#include "critical_section.h"
#include "deca_dbg.h"
#include "defaultTask.h"
#include "int_priority.h"
#ifdef USB_ENABLE
#include "HAL_usb.h"
#endif

#define DEFAULT_TASK_STACK_SIZE_BYTES 4300 // worst case is RESPF followed by STOP command on a Nordic platform
static osThreadId defaultTaskHandle;

__attribute__((weak)) const app_definition_t *DefaultTaskHookEvent()
{
    return AppGetDefaultEvent();
}

__attribute__((weak)) void DefaultTaskTimeoutHook(void)
{
}

/* StartDefaultTask function */
void StartDefaultTask(void const *argument)
{
    const app_definition_t *queue_message;
    const app_definition_t *default_app = DefaultTaskHookEvent();

    if (default_app != NULL)
    {
        EventManagerRegisterApp(&default_app);
    }
    /* Infinite loop */
    while (1)
    {

        Watchdog.refresh();
        /* Contract: App registred should point to a valid application from the flash */
        if (EventManagerWaitAppRegistration((app_definition_t *)&queue_message, MESSAGE_QUEUE_TIMEOUT_MS) != -1)
        {
            /* clean the ressources of the current executed task */
            if (AppGet()->terminate != NULL)
            {
                AppGet()->terminate();
            }

            AppSetLastError(_NO_ERR);
            /* Set current running application to NULL */
            AppSet(NULL);

            osThreadYield(); // force switch of context

            enter_critical_section();

            /* Start appropriate RTOS top-level application:
             Contract: App registred should point to a valid application from the flash */
            AppSet(queue_message);
            if (queue_message->helper != NULL)
            {
                queue_message->helper(NULL);
            }
            leave_critical_section(); // ready to switch to a created task
        }
        else
        {
            DefaultTaskTimeoutHook();
        }
#ifdef USB_ENABLE
        Usb.update();
#endif
        osThreadYield();
    }
}

/* Create the thread(s) */
/* definition and creation of defaultTask */
/* Note. The DefaultTask is responsible for starting & stopping of TOP Level applications. */
void DefaultTaskInit(void)
{
    error_e ret = create_default_task(StartDefaultTask, &defaultTaskHandle, (uint16_t)DEFAULT_TASK_STACK_SIZE_BYTES);
    if (ret != _NO_ERR)
    {
        error_handler(1, _ERR_Create_Task_Bad);
    }
}
