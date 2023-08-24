/*
 * Copyright (c) 2020–2022 Qorvo, Inc
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

#ifndef DEFAULT_COORDINATOR_H
#define DEFAULT_COORDINATOR_H

#include <uci_backend/uci_backend_coordinator.h>
#include <uwbmac/uwbmac.h>

/**
 * enum uwbmac_scheduler - Internal use.
 *
 * @UWBMAC_SCHEDULER_ENDLESS: Endless scheduler.
 * @UWBMAC_SCHEDULER_ON_DEMAND: On demand scheduler.
 * @UWBMAC_SCHEDULER_NONE: No scheduler.
 */
enum uwbmac_scheduler {
	UWBMAC_SCHEDULER_ENDLESS,
	UWBMAC_SCHEDULER_ON_DEMAND,
	UWBMAC_SCHEDULER_NONE,
};

/**
 * enum uwbmac_region - Internal use.
 *
 * @UWBMAC_REGION_FIRA: FiRa region.
 * @UWBMAC_REGION_NFCC: NFCC region.
 * @UWBMAC_REGION_PCTT: PCTT region.
 * @UWBMAC_REGION_NONE: No region.
 */
enum uwbmac_region {
	UWBMAC_REGION_FIRA,
	UWBMAC_REGION_NFCC,
	UWBMAC_REGION_PCTT,
	UWBMAC_REGION_NONE,
};

/**
 * struct default_coordinator - A coordinator implementation.
 *
 * NOTE: This implementation only works with one device.
 *
 * @start_count: Start counter.
 * @current_region: Current region.
 * @current_scheduler: Current scheduler.
 * @base: UCI generic backend coordinator.
 */
struct default_coordinator {
	/* private */
	int start_count;
	enum uwbmac_region current_region;
	enum uwbmac_scheduler current_scheduler;
	struct uci_backend_coordinator base;
};

/**
 * default_coordinator_init - Initialise default coordinator.
 * @coord: The default coordinator.
 * @context: The UWB MAC context.
 *
 */
void default_coordinator_init(struct default_coordinator *coord,
			      struct uwbmac_context *context);

/**
 * default_request_region - Implementation of request_region interface.
 * @coord: The default coordinator.
 * @context: The UWB MAC context.
 * @region_name: The region being requested
 *
 * NOTE: This implementation only works when device is down.
 *
 * Return: UWBMAC_SUCCESS on success or an error code.
 */
uwbmac_error default_request_region(struct uci_backend_coordinator *coord,
				    struct uwbmac_context *context,
				    const char *region_name);

/**
 * default_release_region - Implementation of release_region interface.
 * @coord: The default coordinator.
 * @context: The UWB MAC context.
 * @region_name: The region that is not requested anymore.
 *
 * NOTE: This implementation only does something when device is down.
 * It will set default scheduler to reset it completely.
 *
 */
void default_release_region(struct uci_backend_coordinator *coord,
			    struct uwbmac_context *context,
			    const char *region_name);

/**
 * default_request_start - Implementation of request_start interface.
 * @coord: The default coordinator.
 * @context: The UWB MAC context.
 *
 * NOTE: This implementation only works with one device.
 *
 * Return: UWBMAC_SUCCESS on success or an error code.
 */
uwbmac_error default_request_start(struct uci_backend_coordinator *coord,
				   struct uwbmac_context *context);

/**
 * default_suggest_stop - Implementation of suggest_stop interface.
 * @coord: The default coordinator.
 * @context: The UWB MAC context.
 *
 * NOTE: This implementation only works with one device.
 * Device will be stopped if all active sessions have been stopped.
 *
 */
void default_suggest_stop(struct uci_backend_coordinator *coord,
			  struct uwbmac_context *context);

/**
 * default_get_scheduler - Implementation of get_scheduler interface.
 * @coord: The default coordinator.
 * @region_name: The region for which we want to know the scheduler.
 *
 * Return: cstring representing the chosen scheduler.
 */
const char *default_get_scheduler(struct uci_backend_coordinator *coord,
				  const char *region_name);

/**
 * default_get_region_id - Implementation of get_region_id interface.
 * @coord: The default coordinator.
 * @region_name: The region for which we want to know the scheduler.
 *
 * Return: int representing the chosen region id.
 */
int default_get_region_id(struct uci_backend_coordinator *coord,
			  const char *region_name);

#endif // DEFAULT_COORDINATOR_H
