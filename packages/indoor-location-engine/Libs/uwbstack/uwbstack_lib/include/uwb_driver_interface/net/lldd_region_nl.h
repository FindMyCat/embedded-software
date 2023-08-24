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

/*
 * This file is generated automatically, edit with care.
 */

#ifndef LLDD_REGION_NL_H
#define LLDD_REGION_NL_H

#include "lld_region_common_nl.h"

/**
 * enum lldd_call - LLD device calls identifiers.
 *
 * @LLDD_CALL_START:
 *	Start LLD device.
 * @LLDD_CALL_TRACK:
 *	Start tracking a detected network.
 * @LLDD_CALL_STOP:
 *	Stop LLD device.
 * @LLDD_CALL_STATE_CHANGED:
 *	Device state change notification.
 * @LLDD_CALL_NETWORK_DETECTED:
 *	Network detected notification.
 * @LLDD_CALL_GET_TIME:
 *	Get local time and network time offset.
 * @LLDD_CALL_GET_STATS:
 *	Get device statistics.
 * @LLDD_CALL_RESET_STATS:
 *	Reset statistics.
 * @LLDD_CALL_ASSOCIATION_REQUEST:
 *	Device association request.
 * @LLDD_CALL_ASSOCIATION_FAILED:
 *	Device association failure notification.
 * @LLDD_CALL_MAX: Internal use.
 */
enum lldd_call {
	LLDD_CALL_START,
	LLDD_CALL_TRACK,
	LLDD_CALL_STOP,
	LLDD_CALL_STATE_CHANGED,
	LLDD_CALL_NETWORK_DETECTED,
	LLDD_CALL_GET_TIME,
	LLDD_CALL_GET_STATS,
	LLDD_CALL_RESET_STATS,
	LLDD_CALL_ASSOCIATION_REQUEST,
	LLDD_CALL_ASSOCIATION_FAILED,
	LLDD_CALL_MAX,
};

/**
 * enum lldd_call_attrs - LLD device call attributes.
 *
 * @LLDD_CALL_ATTR_STATE:
 *	Device state.
 * @LLDD_CALL_ATTR_PAN_ID:
 *	Network PAN ID.
 * @LLDD_CALL_ATTR_COORD_SHORT_ADDR:
 *	Coordinator short address.
 * @LLDD_CALL_ATTR_LQI:
 *	Link quality indicator (LQI, 0-100).
 * @LLDD_CALL_ATTR_PAIRING_ALLOWED:
 *	Whether the coordinator accepts pairing.
 * @LLDD_CALL_ATTR_BEACON_PAYLOAD:
 *	Payload embedded in beacon.
 * @LLDD_CALL_ATTR_NETWORK_TIME_OFFSET_DTU:
 *	Network time offset value in DTU, difference between coordinator and
 *      device time.
 * @LLDD_CALL_ATTR_LOCAL_TIME_DTU:
 *	Local time value in DTU.
 * @LLDD_CALL_ATTR_STATS:
 *  	Network device statistics.
 * @LLDD_CALL_ATTR_DEV_KEY:
 *  	Device identity key.
 * @LLDD_CALL_ATTR_ASSOCIATION_PAYLOAD:
 *  	Payload embedded in the association request.
 * @LLDD_CALL_ATTR_ASSOCIATION_FAILURE_REASON:
 *  	Association failure reason.
 *
 * @LLDD_CALL_ATTR_UNSPEC: Invalid attribute.
 * @__LLDD_CALL_ATTR_AFTER_LAST: Internal use.
 * @LLDD_CALL_ATTR_MAX: Internal use.
 */
enum lldd_call_attrs {
	LLDD_CALL_ATTR_UNSPEC,
	LLDD_CALL_ATTR_STATE,
	LLDD_CALL_ATTR_PAN_ID,
	LLDD_CALL_ATTR_COORD_SHORT_ADDR,
	LLDD_CALL_ATTR_LQI,
	LLDD_CALL_ATTR_PAIRING_ALLOWED,
	LLDD_CALL_ATTR_BEACON_PAYLOAD,
	LLDD_CALL_ATTR_NETWORK_TIME_OFFSET_DTU,
	LLDD_CALL_ATTR_LOCAL_TIME_DTU,
	LLDD_CALL_ATTR_STATS,
	LLDD_CALL_ATTR_DEV_KEY,
	LLDD_CALL_ATTR_ASSOCIATION_PAYLOAD,
	LLDD_CALL_ATTR_ASSOCIATION_FAILURE_REASON,

	__LLDD_CALL_ATTR_AFTER_LAST,
	LLDD_CALL_ATTR_MAX = __LLDD_CALL_ATTR_AFTER_LAST - 1,
};

/**
 * enum lldd_param_attrs - LLD device parameters attributes.
 *
 * @LLDD_PARAM_ATTR_AGG_LIMIT_BYTES:
 *	Limit MPDU aggregated payload size after the first sub-frame.
 * @LLDD_PARAM_ATTR_INACTIVE_EXPONENT:
 *	If not 0, the device will behave as an inactive device. It will
 *	participate only in blocks with an index which is a multiple of 2 to the
 *	power of inactiveExponent.
 * @LLDD_PARAM_ATTR_PHY_PREAMBLE_CODE:
 *	Preamble code index in use. 9-24: BPRF, 25-32: HPRF.
 * @LLDD_PARAM_ATTR_PHY_PSDU_SIZE:
 *	For HPRF, specify the use of A0 and A1 fields of the PHR to extend the
 *	maximum PSDU size. 0: 1023, 1: 2047, 2: 4095.
 * @LLDD_PARAM_ATTR_SMART_POWER:
 *	True to enable smart power feature, false otherwise.
 * @LLDD_PARAM_ATTR_TRACKING_LOSS_THRESHOLD:
 *	Number of consecutive failed blocks for a device to declare tracking
 *	loss.
 * @LLDD_PARAM_ATTR_MFS:
 *	MFS configuration.
 *
 * @LLDD_PARAM_ATTR_UNSPEC: Invalid command.
 * @__LLDD_PARAM_ATTR_AFTER_LAST: Internal use.
 * @LLDD_PARAM_ATTR_MAX: Internal use.
 */
enum lldd_param_attrs {
	LLDD_PARAM_ATTR_UNSPEC,
	LLDD_PARAM_ATTR_AGG_LIMIT_BYTES,
	LLDD_PARAM_ATTR_INACTIVE_EXPONENT,
	LLDD_PARAM_ATTR_PHY_PREAMBLE_CODE,
	LLDD_PARAM_ATTR_PHY_PSDU_SIZE,
	LLDD_PARAM_ATTR_SMART_POWER,
	LLDD_PARAM_ATTR_TRACKING_LOSS_THRESHOLD,
	LLDD_PARAM_ATTR_MFS,

	__LLDD_PARAM_ATTR_AFTER_LAST,
	LLDD_PARAM_ATTR_MAX = __LLDD_PARAM_ATTR_AFTER_LAST - 1
};

/**
 * enum lldd_mfs_attrs - LLD device mfs attributes.
 *
 * @LLDD_MFS_ATTR_MFS_ID:
 *	MFS id (0: interactive, 1: best effort).
 * @LLDD_MFS_ATTR_MFS_EXPIRATION_MS:
 *	MSDU expiration delay in milliseconds. Expiration is disabled if 0.
 * @LLDD_MFS_ATTR_MFS_WINDOW_MSDU:
 *	Size of the MFS window in number of MSDUs.
 * @LLDD_MFS_ATTR_MFS_WINDOW_BYTES:
 *	Size of the MFS window in bytes.
 *
 * @LLDD_MFS_ATTR_UNSPEC: Invalid command.
 * @__LLDD_MFS_ATTR_AFTER_LAST: Internal use.
 * @LLDD_MFS_ATTR_MAX: Internal use.
 */
enum lldd_mfs_attrs {
	LLDD_MFS_ATTR_UNSPEC,
	LLDD_MFS_ATTR_MFS_ID,
	LLDD_MFS_ATTR_MFS_EXPIRATION_MS,
	LLDD_MFS_ATTR_MFS_WINDOW_MSDU,
	LLDD_MFS_ATTR_MFS_WINDOW_BYTES,

	__LLDD_MFS_ATTR_AFTER_LAST,
	LLDD_MFS_ATTR_MAX = __LLDD_MFS_ATTR_AFTER_LAST - 1
};

/**
 * enum lldd_state_id - State of the device.
 * @LLDD_STATE_ID_STANDBY:
 * 	Standby state.
 * @LLDD_STATE_ID_SCANNING:
 * 	Scanning state.
 * @LLDD_STATE_ID_TRACKING:
 * 	Tracking state.
 * @LLDD_STATE_ID_ASSOCIATED_ACTIVE:
 * 	Active state.
 * @LLDD_STATE_ID_ASSOCIATED_INACTIVE:
 * 	Inactive state.
 */
enum lldd_state_id {
	LLDD_STATE_ID_STANDBY,
	LLDD_STATE_ID_SCANNING,
	LLDD_STATE_ID_TRACKING,
	LLDD_STATE_ID_ASSOCIATED_ACTIVE,
	LLDD_STATE_ID_ASSOCIATED_INACTIVE,
};

/**
 * enum lldd_stats_attrs - LLD device statistics attributes.
 *
 * @LLDD_STATS_ATTR_INTERACTIVE_MFS:
 *  Interactive MFS packets statistics.
 * @LLDD_STATS_ATTR_BEST_EFFORT_MFS:
 *  Best effort MFS packets statistics.
 * @LLDD_STATS_ATTR_RX_DL_FRAMES:
 *  Number of good DL frames received.
 * @LLDD_STATS_ATTR_RX_DL_BYTES:
 *  Number of good DL frames bytes received,
 *  corresponding to LLDD_STATS_ATTR_RX_DL_FRAMES.
 * @LLDD_STATS_ATTR_RX_DL_ERRORS:
 *  Number of DL frames received with error.
 * @LLDD_STATS_ATTR_RX_DL_RETRY_FRAMES:
 *  Number of good DL frames received in a retry sub-block.
 * @LLDD_STATS_ATTR_RX_DL_RETRY_BYTES:
 *  Number of good DL frames bytes received in a retry sub-block, corresponding
 *  to LLDD_STATS_ATTR_RX_DL_RETRY_FRAMES.
 * @LLDD_STATS_ATTR_RX_DL_RETRY_ERRORS:
 *  Number of DL frames received with error in a retry sub-block.
 * @LLDD_STATS_ATTR_TX_UL_FRAMES:
 *  Number of UL frames transmitted successfully.
 * @LLDD_STATS_ATTR_TX_UL_BYTES:
 *  Number of UL frames bytes transmitted successfully,
 *  corresponding to LLDD_STATS_ATTR_TX_UL_FRAMES.
 * @LLDD_STATS_ATTR_TX_UL_ERRORS:
 *  Number of UL frames transmitted with error.
 * @LLDD_STATS_ATTR_RX_BEACON_FRAMES:
 *  Number of beacon frames received.
 * @LLDD_STATS_ATTR_RX_BEACON_ERRORS:
 *  Number of beacon frames received with errors.
 * @LLDD_STATS_ATTR_RX_UNRELATEDS:
 *  Number of unrelated frames received.
 * @LLDD_STATS_ATTR_RX_FRAME_NON_ASSOCS:
 *  Number of non associative frames received.
 * @LLDD_STATS_ATTR_RX_FRAME_NON_ASSOCS_ERRORS:
 *  Number of non associative frames errors.
 * @LLDD_STATS_ATTR_LQI:
 *  Link quality indicator (LQI, 0-100).
 * @LLDD_STATS_ATTR_BLOCKS:
 *  Number of blocks in which the device participated (tracking or associated).
 *
 * @LLDD_STATS_ATTR_UNSPEC: Invalid command.
 * @__LLDD_STATS_ATTR_AFTER_LAST: Internal use.
 * @LLDD_STATS_ATTR_MAX: Internal use.
 */
enum lldd_stats_attrs {
	LLDD_STATS_ATTR_UNSPEC,
	LLDD_STATS_ATTR_INTERACTIVE_MFS,
	LLDD_STATS_ATTR_BEST_EFFORT_MFS,
	LLDD_STATS_ATTR_RX_DL_FRAMES,
	LLDD_STATS_ATTR_RX_DL_BYTES,
	LLDD_STATS_ATTR_RX_DL_ERRORS,
	LLDD_STATS_ATTR_RX_DL_RETRY_FRAMES,
	LLDD_STATS_ATTR_RX_DL_RETRY_BYTES,
	LLDD_STATS_ATTR_RX_DL_RETRY_ERRORS,
	LLDD_STATS_ATTR_TX_UL_FRAMES,
	LLDD_STATS_ATTR_TX_UL_BYTES,
	LLDD_STATS_ATTR_TX_UL_ERRORS,
	LLDD_STATS_ATTR_RX_BEACON_FRAMES,
	LLDD_STATS_ATTR_RX_BEACON_ERRORS,
	LLDD_STATS_ATTR_RX_UNRELATEDS,
	LLDD_STATS_ATTR_RX_FRAME_NON_ASSOCS,
	LLDD_STATS_ATTR_RX_FRAME_NON_ASSOCS_ERRORS,
	LLDD_STATS_ATTR_LQI,
	LLDD_STATS_ATTR_BLOCKS,

	__LLDD_STATS_ATTR_AFTER_LAST,
	LLDD_STATS_ATTR_MAX = __LLDD_STATS_ATTR_AFTER_LAST - 1
};

/**
 * enum lldd_msdu_stats_attrs - LLD device MSDU statistics attributes.
 *
 * @LLDD_STATS_MSDU_ATTR_RX_PACKETS:
 *  Number of good data packets received.
 * @LLDD_STATS_MSDU_ATTR_RX_BYTES:
 *  Number of good received bytes, corresponding to
 *  LLDD_STATS_ATTR_RX_PACKETS.
 * @LLDD_STATS_MSDU_ATTR_RX_ERRORS:
 *  Number of bad data packets received.
 * @LLDD_STATS_MSDU_ATTR_RX_IGNORED:
 *  Number of good data packets ignored because
 *  they are out of the reception window.
 * @LLDD_STATS_MSDU_ATTR_TX_PACKETS:
 *  Number of data packets successfully transmitted.
 * @LLDD_STATS_MSDU_ATTR_TX_BYTES:
 *  Number of transmitted bytes, corresponding to LLDD_STATS_ATTR_RX_PACKETS.
 * @LLDD_STATS_MSDU_ATTR_TX_ERRORS:
 *  Number of data packets not transmitted due to problem.
 * @LLDD_STATS_MSDU_ATTR_TX_DROPPED:
 *  Number of data packets dropped before successful transmission.
 *
 * @LLDD_STATS_MSDU_ATTR_UNSPEC: Invalid command.
 * @__LLDD_STATS_MSDU_ATTR_AFTER_LAST: Internal use.
 * @LLDD_STATS_MSDU_ATTR_MAX: Internal use.
 */
enum lldd_msdu_stats_attrs {
	LLDD_STATS_MSDU_ATTR_UNSPEC,
	LLDD_STATS_MSDU_ATTR_RX_PACKETS,
	LLDD_STATS_MSDU_ATTR_RX_BYTES,
	LLDD_STATS_MSDU_ATTR_RX_ERRORS,
	LLDD_STATS_MSDU_ATTR_RX_IGNORED,
	LLDD_STATS_MSDU_ATTR_TX_PACKETS,
	LLDD_STATS_MSDU_ATTR_TX_BYTES,
	LLDD_STATS_MSDU_ATTR_TX_ERRORS,
	LLDD_STATS_MSDU_ATTR_TX_DROPPED,

	__LLDD_STATS_MSDU_ATTR_AFTER_LAST,
	LLDD_STATS_MSDU_ATTR_MAX = __LLDD_STATS_MSDU_ATTR_AFTER_LAST - 1
};

/**
 * enum lldd_association_failure_reason - LLD device association failure reason.
 *
 * @LLDD_ASSOCIATION_FAILURE_REASON_PAN_AT_CAPACITY:
 *  Not enough resources.
 * @LLDD_ASSOCIATION_FAILURE_REASON_PAN_ACCESS_DENIED:
 *  Device not paired.
 */
enum lldd_association_failure_reason {
	LLDD_ASSOCIATION_FAILURE_REASON_PAN_AT_CAPACITY,
	LLDD_ASSOCIATION_FAILURE_REASON_PAN_ACCESS_DENIED,
};

#endif /* LLDD_REGION_NL_H */
