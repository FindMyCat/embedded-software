/**
 * @file      thread_fn.c
 *
 * @brief     Thread function
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
#include <stdio.h>
#include <inttypes.h>

#include "cmsis_os.h"
#include "reporter.h"
#include "thread_fn.h"

const char THREAD_FN_RET_OK[] = "ok\r\n";
const char THREAD_FN_RET_KO[] = "KO\r\n";

/**
 * @brief show current threads and stack depth
 *
 * */
const char *thread_fn(void)
{
    const char *ret = THREAD_FN_RET_OK;

    char *pcWriteBuffer = malloc(1024);

    if (pcWriteBuffer)
    {
        TaskStatus_t *pxTaskStatusArray;
        volatile UBaseType_t uxArraySize, x;
        uint32_t ulTotalRunTime;

        /* Make sure the write buffer does not contain a string. */
        *pcWriteBuffer = 0x00;

        /* Take a snapshot of the number of tasks in case it changes while this
        function is executing. */
        uxArraySize = uxTaskGetNumberOfTasks();

        /* Allocate a TaskStatus_t structure for each task.  An array could be
        allocated statically at compile time. */
        pxTaskStatusArray = pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));

        int sz = 0;
        if (pxTaskStatusArray != NULL)
        {
            /* Generate raw status information about each task. */
            uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, (uint32_t *const)&ulTotalRunTime);

            /* For each populated position in the pxTaskStatusArray array,
            format the raw data as human readable ASCII data. */
            sz += sprintf(&pcWriteBuffer[sz], "%-16s\t%s\r\n", "THREAD NAME", "Stack usage");
            for (x = 0; x < uxArraySize; x++)
            {
                uint32_t *p = pxTaskStatusArray[x].pxStackBase;
                uint32_t total = (p[-1] & 0xFFFFUL) - 8;
                sz += sprintf(&pcWriteBuffer[sz], "%-16s\t%" PRIu32 "/%" PRIu32 "\r\n",
                              pxTaskStatusArray[x].pcTaskName,
                              total - pxTaskStatusArray[x].usStackHighWaterMark * 4,
                              total);
            }
            /* The array is no longer needed, free the memory it consumes. */
            vPortFree(pxTaskStatusArray);

            sz += sprintf(&pcWriteBuffer[sz], "%-16s\t%d\r\n", "Total HEAP", configTOTAL_HEAP_SIZE);
            sz += sprintf(&pcWriteBuffer[sz], "%-16s\t%d\r\n", "Current HEAP used", configTOTAL_HEAP_SIZE - xPortGetFreeHeapSize());
            sz += sprintf(&pcWriteBuffer[sz], "%-16s\t%d\r\n", "Max HEAP used", configTOTAL_HEAP_SIZE - xPortGetMinimumEverFreeHeapSize());
        }
        else
        {
            ret = THREAD_FN_RET_KO;
        }

        reporter_instance.print((char *)pcWriteBuffer, sz);

        free(pcWriteBuffer);
    }

    ret = THREAD_FN_RET_OK;
    return (ret);
}