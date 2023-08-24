/*
 * Copyright (c) 2020–2021 Qorvo, Inc
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

#ifndef UCI_BACKEND_COORDINATOR_H
#define UCI_BACKEND_COORDINATOR_H

#include <uwbmac/uwbmac.h>

/**
 * DOC: Coordinator overview
 *
 * The uci_backend_coordinator implements MAC scheduler and region selection for the protocol backends (Fira, PCTT, Radar, NFCC).
 * It also implements device state management.
 *
 * The various backends are not aware of the state of scheduler or region when a session of a certain type is required, or what state the device is in (active or not).
 * This interface provides a way for them to request a certain state. The coordinator implementation is meant to be custom depending on the project.
 * It is meant to vary depending on the use case, target and so on. A default implementation is provided.
 *
 */

struct uci_backend_coordinator;

/**
 * struct uci_backend_coordinator_ops - Operations provided by the coordinator.
 */
struct uci_backend_coordinator_ops {
	/**
	 * @request_region: Callback invoked when a backend is requesting a certain region.
	 */
	uwbmac_error (*request_region)(struct uci_backend_coordinator *coord,
				       struct uwbmac_context *context,
				       const char *region_name);
	/**
	 * @release_region: Callback invoked when a backend no longer needs a certain region.
	 */
	void (*release_region)(struct uci_backend_coordinator *coord,
			       struct uwbmac_context *context,
			       const char *region_name);
	/**
	 * @request_start: Callback invoked when a backend needs the device to be started.
	 */
	uwbmac_error (*request_start)(struct uci_backend_coordinator *coord,
				      struct uwbmac_context *context);
	/**
	 * @suggest_stop: Callback invoked when a backend no longer needs the device.
	 */
	void (*suggest_stop)(struct uci_backend_coordinator *coord,
			     struct uwbmac_context *context);
	/**
	 * @get_scheduler: Get the scheduler used for the given region.
	 */
	const char *(*get_scheduler)(struct uci_backend_coordinator *coord,
				     const char *region_name);
	/**
	 * @get_region_id: Get the region id used for the given region.
	 */
	int (*get_region_id)(struct uci_backend_coordinator *coord,
			     const char *region_name);
};

/**
 * struct uci_backend_coordinator - UCI generic backend coordinator.
 *
 * Coordinator is a mechanism for backends to handle device state and region state without knowing each others.
 */
struct uci_backend_coordinator {
	/**
	 * @ops: Coordinator operations.
	 */
	const struct uci_backend_coordinator_ops *ops;
};

#endif // UCI_BACKEND_PCTT_H
