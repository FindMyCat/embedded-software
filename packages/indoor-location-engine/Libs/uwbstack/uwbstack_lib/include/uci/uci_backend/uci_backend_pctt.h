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

#ifndef UCI_BACKEND_PCTT_H
#define UCI_BACKEND_PCTT_H

#include <uci/uci.h>
#include <uci_backend/uci_backend_coordinator.h>
#include <uci_backend/uci_backend_manager.h>
#include <uwbmac/pctt_helper.h>
#include "uci_backend_fira.h"

/**
 * DOC: Backend PCTT overview
 *
 * The uci_backend_pctt implements the control of test session.
 *
 * For every UCI commands relative to a test session, the backend manager will dispatch it to this pctt backend if
 * the session is of type test. Then this pctt backend will realise the command through uwbmac API using the pctt helper interface.
 * This will then be realized by the pctt region inside the MAC.
 *
 */

/**
 * struct uci_backend_pctt_context - Definition of this backend context.
 *
 * NOTE: UCI spec defines several command to set the configuration for a test.
 * It is consumed in one helper call, hence the context hold the various part of the configuration.
 */
struct uci_backend_pctt_context {
	/**
	 * @pctt_context: context for pctt helper.
	 */
	struct pctt_context pctt_context;
	/**
	 * @uwbmac_context: UWB MAC context.
	 */
	struct uwbmac_context *uwbmac_context;
	/**
	 * @pctt_params: part of the test configuration held.
	 */
	struct pctt_parameters pctt_params;
	/**
	 * @pctt_test_payload: part of the test configuration held.
	 */
	struct pctt_test_payload pctt_test_payload;
	/**
	 * @pctt_session_params: part of the test configuration held.
	 */
	struct pctt_session_parameters pctt_session_params;
	/**
	 * @session_state: the unique session state as per uci spec.
	 */
	int session_state;
	/**
	 * @uci: uci context used in callbacks from the helper.
	 */
	struct uci *uci;
	/**
	 * @coord: uci backend coordinator.
	 */
	struct uci_backend_coordinator *coord;
	/**
	 * @sess_man: common FiRa sub-modules sessions manager.
	 */
	struct uci_backend_manager *sess_man;
	/**
	 * @enable_diag: whether diagnostic notification is used.
	 */
	bool enable_diag;
};

/**
 * uci_backend_pctt_init() - Attach this backend to the uci context to bridge uci communication to the MAC.
 * @context: This backend context.
 * @uci: UCI context.
 * @uwbmac_context: UWB MAC context.
 * @coord: backend coordinator.
 * @sess_man: session manager.
 *
 * NOTE: This uci_backend_pctt is meant to be used in addition of uci_backend_fira.
 * It cannot be used alone, most of common FiRa errors are dealt with in uci_backend_fira.
 *
 * Return: 0 or an error code.
 */
int uci_backend_pctt_init(struct uci_backend_pctt_context *context,
			  struct uci *uci,
			  struct uwbmac_context *uwbmac_context,
			  struct uci_backend_coordinator *coord,
			  struct uci_backend_manager *sess_man);

/**
 * uci_backend_pctt_release() - Free global ressources used.
 * @context: This backend context.
 */
void uci_backend_pctt_release(struct uci_backend_pctt_context *context);

#endif // UCI_BACKEND_PCTT_H
