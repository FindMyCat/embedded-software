/*
 * Copyright (c) 2022 Qorvo, Inc
 *
 * All rights reserved.
 *
 * NOTICE: All information contained herein is, and remains the property
 * of Qorvo, Inc. and its suppliers, if any. The intellectual and technical
 * concepts herein are proprietary to Qorvo, Inc. and its suppliers, and
 * may be covered by patents, patent applications, and are protected by
 * trade secret and/or copyright law. Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written
 * permission is obtained from Qorvo, Inc.
 *
 */

#ifndef _UWBMAC_PLATFORM_H_
#define _UWBMAC_PLATFORM_H_

#include <uwbmac/uwbmac_report.h>

/**
 * uwbmac_platform_init() - Init platform used for UWBMAC.
 *
 * NOTE: This function is not defined and must be implemented by the user to
 * perform platform-specific initializations, such as creating a thread...
 * It is called from uwbmac_init.
 *
 * Return: 0 or error.
 */
int uwbmac_platform_init(void);

/**
 * uwbmac_platform_exit() - Exit platform used for UWBMAC.
 *
 * NOTE: This function is not defined and must be implemented by the user to
 * perform platform-specific de-initializations, such as deleting a thread...
 * It is called from uwbmac_exit.
 */
void uwbmac_platform_exit(void);

/**
 * uwbmac_platform_report() - Ask platform to handle UWBMAC event.
 * @rep: pointer to the event report.
 *
 * NOTE: This function is not defined and must be implemented by the user to
 * handle events received from the MAC. It is up to the user to perform
 * event processing synchronously or asynchronously.
 * In order to process the event, the function uwbmac_handle_report must
 * be called.
 *
 * Return: 0 or error.
 */
int uwbmac_platform_report(struct report_data *rep);

#endif /* _UWBMAC_PLATFORM_H_ */
