/**
 * @file      uwbmac_platform.c
 *
 * @brief     
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

#include "create_report_task.h"
#include "uwbmac/uwbmac.h"
#include "uwbmac/uwbmac_report.h"


osMailQDef(report_mail, CONFIG_UWBMAC_REPORT_QUEUE_LEN, struct report_data);

static task_signal_t reportTask;
static osMailQId mail_id = 0;


static void report_task(const void *arg)
{
    osEvent evt;

    reportTask.Exit = 0;

    while (reportTask.Exit == 0)
    {
        /* Wait for report events */
        /* Message queue should retrieve a pointer and a port id */
        evt = osMailGet(mail_id, osWaitForever);
        if (reportTask.Exit == 1)
        {
			osMailFree(mail_id, evt.value.p);
            break;
        }

        if (evt.status == osEventMail)
        {
            struct report_data *rep_data = evt.value.p;
            uwbmac_handle_report(rep_data);
            osMailFree(mail_id, rep_data);
        }
    }
    reportTask.Exit = 2;
    while (reportTask.Exit == 2)
    {
        osDelay(1);
    };
}

int uwbmac_platform_init(void)
{
    reportTask.Exit = 0;
    reportTask.task_stack = NULL;
	
	if (!mail_id)
		mail_id = osMailCreate(osMailQ(report_mail), NULL);

	if (!mail_id)
		return UWBMAC_EINVAL;

    /* Create report thread */
    error_e ret = create_report_task(report_task, &reportTask, (uint16_t)CONFIG_UWBMAC_REPORT_TASK_STACK_SIZE);
    if (ret != _NO_ERR)
    {
        error_handler(1, _ERR_Create_Task_Bad);
    }

    return UWBMAC_SUCCESS;
}

void uwbmac_platform_exit(void)
{
    terminate_task_with_mail(&reportTask);
}

/* used by uwbmac_event_report(), needs to return negative errno */
int uwbmac_platform_report(struct report_data *report)
{
    struct report_data *rep;

    rep = osMailAlloc(mail_id, 0);
    if (!rep)
    {
        return -UWBMAC_ENOMEM;
    }

    rep->port_id = report->port_id;
    rep->report  = report->report;

    if (osMailPut(mail_id, rep))
        return -UWBMAC_EINVAL;

    return 0;
}
