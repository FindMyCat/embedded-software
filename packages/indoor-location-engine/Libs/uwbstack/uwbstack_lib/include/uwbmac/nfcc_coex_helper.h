/*
 * Copyright (c) 2021 Qorvo, Inc
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

#ifndef NFCC_COEX_HELPER_H
#define NFCC_COEX_HELPER_H

#include <net/nfcc_coex_region_nl.h>

#include <uwbmac/uwbmac.h>
#include <uwbmac/uwbmac_writer.h>

/**
 * enum stopped_reason - Reason why the nfcc coex region can be stopped.
 *
 * @REQUESTED:
 *  Stop has been requested by application.
 * @WATCHDOG:
 *  Stop was triggered by a watchdog because NFCC did not release the SPI.
 */
enum stopped_reason {
	REQUESTED,
	WATCHDOG,
};

/**
 * typedef nfcc_coex_helper_notification_cb_t - Notification callback type for stop.
 * @user_data: User data pointer given to nfcc_coex_helper_open.
 * @reason: Cause of the notification.
 *
 * Return: none.
 */
typedef void (*nfcc_coex_helper_notification_cb_t)(void *user_data,
						   enum stopped_reason reason);

/*
 * struct nfcc_coex_context - NFCC coex context
 *
 * NOTE: content of this struct is considered private, do not use directly.
 */
struct nfcc_coex_context {
	/* private: internal use only */
	struct uwbmac_channel channel;
	nfcc_coex_helper_notification_cb_t notification_cb;
	char *scheduler;
	int region_id;
	void *user_data;
};

/**
 * nfcc_coex_helper_open() - Initialize the internal resources of the helper.
 * @context: Context to initialize.
 * @uwbmac_context: UWB MAC context.
 * @notification_cb: Callback to call when a notification is available.
 * @scheduler: In which scheduler the region will be.
 * @region_id: Which id the region will have in the scheduler.
 * @user_data: User data pointer to give back in callback.
 *
 * NOTE: This function must be called first. @nfcc_coex_helper_close must be called
 * at the end of the application to ensure resources are freed.
 * The channel will be managed by the helper, this means you should neither use
 * uwbmac_channel_create nor uwbmac_channel_release.
 *
 * Return: UWBMAC_SUCCESS or error.
 */
int nfcc_coex_helper_open(struct nfcc_coex_context *context,
			  struct uwbmac_context *uwbmac_context,
			  nfcc_coex_helper_notification_cb_t notification_cb,
			  const char *scheduler, int region_id,
			  void *user_data);

/**
 * nfcc_coex_helper_close() - Free all internal resources of the helper.
 * @context: Context to free.
 */
void nfcc_coex_helper_close(struct nfcc_coex_context *context);

/**
 * nfcc_coex_helper_start() - Start nfcc_coex region.
 * @context: NFCC coex context.
 * @uwbtime0: Timestamp in nanoseconds in the CLOCK_MONOTONIC time reference.
 * @nfcc_channel: Radio channel to use for the session, 5 or 9
 *
 * Return: UWBMAC_SUCCESS or error.
 */
int nfcc_coex_helper_start(struct nfcc_coex_context *context, uint64_t uwbtime0,
			   uint8_t nfcc_channel);

/**
 * nfcc_coex_helper_stop() - Stop nfcc_coex region.
 * @context: NFCC coex context.
 *
 * NOTE: This function should trigger a notification with reason REQUESTED.
 *
 * Return: UWBMAC_SUCCESS or error.
 */
int nfcc_coex_helper_stop(struct nfcc_coex_context *context);

#endif /* NFCC_COEX_HELPER_H */
