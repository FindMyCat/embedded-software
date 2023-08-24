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

#ifndef UCI_BACKEND_MAC_H
#define UCI_BACKEND_MAC_H

#include <uci/uci.h>
#include <uwbmac/uwbmac.h>

/**
 * DOC: Backend MAC overview
 *
 * The uci_backend_mac implements the MAC interface through UCI commands.
 * Using this UCI interface, all control over the MAC is possible like setting the scheduler and regions and calling them.
 * It is used by the SoftMac kernel to forward MAC commands in a plateform of type linux host coupled with a QMXX.
 *
 */

struct cbor_state;
struct uci_backend_mac_context;

/*
 * Private data that should not be used by the client.
 * Associates the client provided cookie with an uwbmac channel.
 */
struct uci_backend_mac_channel {
	bool is_initialized;
	struct uci_backend_mac_context *context;
	uint32_t client_cookie;
	struct uwbmac_channel uwbmac_channel;
	uint32_t last_activity;
};

/*
 * Private data that should not be used by the client.
 * Holds backend internal state.
 */
struct uci_backend_mac_context {
	struct uwbmac_context *uwbmac_context;
	struct uci *uci;
	struct uci_message_handlers uci_mac_handlers;
	struct uci_backend_mac_channel channels[UWBMAC_MAX_CHANNEL_COUNT];
	uint32_t channel_activity;
	/*
	 * TODO(cyril) cbor work buffer could be avoided
	 * by making the cbor library able to work on UCI messages directly
	 */
	struct cbor_state *cbor_preallocated;
	struct uwbmac_msg *params_preallocated;
	struct uwbmac_msg *reply_preallocated;
};

/**
 * uci_backend_mac_init() - Attach this backend to the uci context to bridge uci
 * communication to the MAC.
 * @context: internal backend context, to be allocated by the client.
 * @uci: UCI context.
 * @uwbmac_context: opaque, implementation dependant uwbmac context.
 * @cbor_preallocated: preallocated buffer to be used for cbor <-> uwbmac_msg,
 *                     NULL to automatically allocate on the heap at init.
 * @params_preallocated: preallocated buffer to temporarily store uwbmac params,
 *                       NULL to automatically allocate on the heap at init.
 * @reply_preallocated:  preallocated buffer to temporarily store uwbmac params,
 *                       NULL to automatically allocate on the heap at init.
 *
 * Return: UWBMAC_SUCCESS or error.
 */
int uci_backend_mac_init(struct uci_backend_mac_context *context,
			 struct uci *uci, struct uwbmac_context *uwbmac_context,
			 struct cbor_state *cbor_preallocated,
			 struct uwbmac_msg *params_preallocated,
			 struct uwbmac_msg *reply_preallocated);

/**
 * uci_backend_mac_release() - Free resources that have been initialized.
 * @context: internal backend context.
 */
void uci_backend_mac_release(struct uci_backend_mac_context *context);

#endif /* UCI_BACKEND_MAC_H */
