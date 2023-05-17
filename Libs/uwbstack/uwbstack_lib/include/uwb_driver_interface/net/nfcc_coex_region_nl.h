/*
 * Copyright (c) 2020–2021 Qorvo, Inc
 *
 * All rights reserved.
 *
 * NOTICE: All information contained herein is, and remains the property of
 * Qorvo, Inc. and its suppliers, if any. The intellectual and technical
 * concepts herein are proprietary to Qorvo, Inc. and its suppliers, and may
 * be covered by patents, patent applications, and are protected by trade
 * secret and/or copyright law. Dissemination of this information or
 * reproduction of this material is strictly forbidden unless prior written
 * permission is obtained from Qorvo, Inc.
 */

#ifndef NFCC_COEX_REGION_NL_H
#define NFCC_COEX_REGION_NL_H

/**
 * enum nfcc_coex_call - NFCC coexistence calls identifiers.
 *
 * @NFCC_COEX_CALL_CCC_SESSION_START:
 *	Start CCC session.
 * @NFCC_COEX_CALL_CCC_SESSION_STOP:
 *	Stop CCC session.
 * @NFCC_COEX_CALL_CCC_SESSION_NOTIFICATION:
 *	Notify session reports.
 * @NFCC_COEX_CALL_MAX: Internal use.
 */
enum nfcc_coex_call {
	NFCC_COEX_CALL_CCC_SESSION_START,
	NFCC_COEX_CALL_CCC_SESSION_STOP,
	NFCC_COEX_CALL_CCC_SESSION_NOTIFICATION,
	NFCC_COEX_CALL_MAX,
};

/**
 * enum nfcc_coex_call_attrs - NFCC coexistence call attributes.
 *
 * @NFCC_COEX_CALL_ATTR_CCC_SESSION_PARAMS:
 *	Session parameters.
 * @NFCC_COEX_CALL_ATTR_CCC_WATCHDOG_TIMEOUT:
 *	Watchdog trigged.
 * @NFCC_COEX_CALL_ATTR_CCC_STOPPED:
 *	Session stopped.
 *
 * @NFCC_COEX_CALL_ATTR_UNSPEC: Invalid command.
 * @__NFCC_COEX_CALL_ATTR_AFTER_LAST: Internal use.
 * @NFCC_COEX_CALL_ATTR_MAX: Internal use.
 */
enum nfcc_coex_call_attrs {
	NFCC_COEX_CALL_ATTR_UNSPEC,

	NFCC_COEX_CALL_ATTR_CCC_SESSION_PARAMS,
	NFCC_COEX_CALL_ATTR_CCC_WATCHDOG_TIMEOUT,
	NFCC_COEX_CALL_ATTR_CCC_STOPPED,

	__NFCC_COEX_CALL_ATTR_AFTER_LAST,
	NFCC_COEX_CALL_ATTR_MAX = __NFCC_COEX_CALL_ATTR_AFTER_LAST - 1
};

/**
 * enum nfcc_coex_ccc_session_param_attrs - NFCC coexistence session parameters attributes.
 *
 * @NFCC_COEX_CCC_SESSION_PARAM_ATTR_TIME0_NS:
 *	Initiation time in unit of ns, default 0.
 * @NFCC_COEX_CCC_SESSION_PARAM_ATTR_CHANNEL_NUMBER:
 *	Override channel for this session: 5, 6, 8, 9, 10, 12, 13 or 14.
  * @NFCC_COEX_CCC_SESSION_PARAM_ATTR_VERSION:
 *	Protocol version to be used.
 *
 * @NFCC_COEX_CCC_SESSION_PARAM_ATTR_UNSPEC: Invalid command.
 * @__NFCC_COEX_CCC_SESSION_PARAM_ATTR_AFTER_LAST: Internal use.
 * @NFCC_COEX_CCC_SESSION_PARAM_ATTR_MAX: Internal use.
 */
enum nfcc_coex_ccc_session_param_attrs {
	NFCC_COEX_CCC_SESSION_PARAM_ATTR_UNSPEC,

	NFCC_COEX_CCC_SESSION_PARAM_ATTR_TIME0_NS,
	NFCC_COEX_CCC_SESSION_PARAM_ATTR_CHANNEL_NUMBER,
	NFCC_COEX_CCC_SESSION_PARAM_ATTR_VERSION,

	__NFCC_COEX_CCC_SESSION_PARAM_ATTR_AFTER_LAST,
	NFCC_COEX_CCC_SESSION_PARAM_ATTR_MAX =
		__NFCC_COEX_CCC_SESSION_PARAM_ATTR_AFTER_LAST - 1
};

#endif /* NFCC_COEX_REGION_NL_H */
