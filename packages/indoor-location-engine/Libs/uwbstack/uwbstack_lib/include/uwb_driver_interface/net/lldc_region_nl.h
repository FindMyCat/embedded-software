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

#ifndef LLDC_REGION_NL_H
#define LLDC_REGION_NL_H

#include "lld_region_common_nl.h"

/**
 * enum lldc_call - LLD coordinator calls identifiers.
 *
 * @LLDC_CALL_START:
 *	Start LLD coordinator.
 * @LLDC_CALL_STOP:
 *	Stop LLD coordinator.
 * @LLDC_CALL_DEV_ASSOC_STATE_CHANGED:
 *	Device association state change notification.
 * @LLDC_CALL_DEV_PAIRING_REQUEST:
 *	Device pairing request notification.
 * @LLDC_CALL_ADD_PAIRED_DEV:
 *	Add a device to the list of paired devices.
 * @LLDC_CALL_DEL_PAIRED_DEV:
 *	Delete a device from the list of paired devices.
 * @LLDC_CALL_GET_PAIRED_DEV:
 *	Get the list of paired devices.
 * @LLDC_CALL_GET_TIME:
 *	Get local time and network time offset.
 * @LLDC_CALL_GET_COORD_STATS:
 *	Get global coordinator statistics statistics.
 * @LLDC_CALL_GET_DEVICE_STATS:
 *	Get device statistics.
 * @LLDC_CALL_RESET_STATS:
 *	Reset statistics.
 * @LLDC_CALL_MAX: Internal use.
 */
enum lldc_call {
	LLDC_CALL_START,
	LLDC_CALL_STOP,
	LLDC_CALL_DEV_ASSOC_STATE_CHANGED,
	LLDC_CALL_DEV_PAIRING_REQUEST,
	LLDC_CALL_ADD_PAIRED_DEV,
	LLDC_CALL_DEL_PAIRED_DEV,
	LLDC_CALL_GET_PAIRED_DEV,
	LLDC_CALL_GET_TIME,
	LLDC_CALL_GET_COORD_STATS,
	LLDC_CALL_GET_DEVICE_STATS,
	LLDC_CALL_RESET_STATS,
	LLDC_CALL_MAX,
};

/**
 * enum lldc_call_attrs - LLD coordinator call attributes.
 *
 * @LLDC_CALL_ATTR_NETWORK_TIME_OFFSET_DTU:
 *	Network time offset value in DTU, difference between coordinator and
 *      device time.
 * @LLDC_CALL_ATTR_LOCAL_TIME_DTU:
 *	Local time value in DTU.
 * @LLDC_CALL_ATTR_DEVICE_SHORT_ADDR:
 *	Device short address.
 * @LLDC_CALL_ATTR_STATS:
 *	Statistics.
 * @LLDC_CALL_ATTR_DEV_KEY:
 *	Device identity key.
 * @LLDC_CALL_ATTR_ASSOCIATED:
 *	Association state.
 * @LLDC_CALL_ATTR_ASSOCIATION_PAYLOAD:
 *	Association payload.
 * @LLDC_CALL_ATTR_DEVICES:
 *	Devices information.
 *
 * @LLDC_CALL_ATTR_UNSPEC: Invalid attribute.
 * @__LLDC_CALL_ATTR_AFTER_LAST: Internal use.
 * @LLDC_CALL_ATTR_MAX: Internal use.
 */
enum lldc_call_attrs {
	LLDC_CALL_ATTR_UNSPEC,
	LLDC_CALL_ATTR_NETWORK_TIME_OFFSET_DTU,
	LLDC_CALL_ATTR_LOCAL_TIME_DTU,
	LLDC_CALL_ATTR_DEVICE_SHORT_ADDR,
	LLDC_CALL_ATTR_STATS,
	LLDC_CALL_ATTR_DEV_KEY,
	LLDC_CALL_ATTR_ASSOCIATED,
	LLDC_CALL_ATTR_ASSOCIATION_PAYLOAD,
	LLDC_CALL_ATTR_DEVICES,

	__LLDC_CALL_ATTR_AFTER_LAST,
	LLDC_CALL_ATTR_MAX = __LLDC_CALL_ATTR_AFTER_LAST - 1,
};

/**
 * enum lldc_param_attrs - LLD coordinator parameters attributes.
 *
 * @LLDC_PARAM_ATTR_ALLOW_PAIRING:
 *	Allow pairing with unknown devices, requests will be forwarded to higher
 *	layers.
 * @LLDC_PARAM_ATTR_AGG_LIMIT_BYTES:
 *	Limit MPDU aggregated payload size after the first sub-frame.
 * @LLDC_PARAM_ATTR_BEACON_IFS:
 *	Interval between the last sub-block DL frame and the beacon frame.
 * @LLDC_PARAM_ATTR_BEACON_INTERVAL_RSTU:
 *	Interval between two beacons transmissions when the coordinator is in
 *	ACTIVE state. Set to 0 to disable beacon transmission.
 * @LLDC_PARAM_ATTR_BEACON_PAYLOAD:
 *	Payload embedded in transmitted beacons, maximum size of 32 bytes.
 * @LLDC_PARAM_ATTR_BLOCK_RSTU:
 *	Duration of a block.
 * @LLDC_PARAM_ATTR_BURST_IFS:
 *	Interval between burst DL frames, or between the last sub-block DL frame
 *	and the first burst DL frame.
 * @LLDC_PARAM_ATTR_CAP_INTERVAL_MAX_RSTU:
 *	Maximum interval between two CAP when the coordinator is in ACTIVE
 *	state. Set to 0 to disable CAP.
 * @LLDC_PARAM_ATTR_CAP_MAX_BE:
 *	Maximum backoff exponent.
 * @LLDC_PARAM_ATTR_CAP_MIN_BE:
 *	Minimum backoff exponent.
 * @LLDC_PARAM_ATTR_CAP_SLOTS_PER_SECOND:
 *	Average number of contention slots per second.
 * @LLDC_PARAM_ATTR_DEVICE_LOSS_THRESHOLD:
 *	Number of consecutive failed blocks for a coordinator to declare a
 *	device loss.
 * @LLDC_PARAM_ATTR_FAST_DS_TWR:
 *	TRUE to use the fast DS-TWR ranging procedure: disable smart power and
 *	ignore SB_MIN_RSTU.
 * @LLDC_PARAM_ATTR_NON_ASSOC_IFS:
 *	Interval between the last sub-block DL frame and the first frame of non-
 *	associated exchange.
 * @LLDC_PARAM_ATTR_PHY_PREAMBLE_CODE:
 *	Preamble code index in use. 9-24: BPRF, 25-32: HPRF.
 * @LLDC_PARAM_ATTR_PHY_PSDU_SIZE:
 *	For HPRF, specify the use of A0 and A1 fields of the PHR to extend the
 *	maximum PSDU size. 0: 1023, 1: 2047, 2: 4095.
 * @LLDC_PARAM_ATTR_PROBING_BEACON_AS_DL:
 *	True to use a beacon frame to fill the role of both DL frame and beacon
 *	frame when the coordinator is in the PROBING state.
 * @LLDC_PARAM_ATTR_PROBING_CAP_SLOTS:
 *	Override CAP_SLOTS_PER_SECOND when the coordinator is in PROBING state.
 * @LLDC_PARAM_ATTR_PROBING_INTERVAL_RSTU:
 *	Interval between two beacons transmissions when the coordinator is in
 *	PROBING state. The same interval is used between two CAP.
 * @LLDC_PARAM_ATTR_SB_BEACON_CAP_SLOT_IFS:
 *	Interval between the beacon frame acting as a DL frame and the first
 *	additional CAP slot of a combined main sub-block and CAP.
 * @LLDC_PARAM_ATTR_SB_CAP_SLOT_RSTU:
 *	Duration of the additional CAP slots in a combined sub-block and CAP.
 * @LLDC_PARAM_ATTR_SB_DL_CAP_SLOT_IFS:
 *	Interval between the last sub-block DL frame and the first additional
 *	CAP slot of a combined sub-block and CAP.
 * @LLDC_PARAM_ATTR_SB_DL_SLOT_RSTU:
 *	Duration of the DL slot in a sub-block.
 * @LLDC_PARAM_ATTR_SB_MIN_RSTU:
 *	Minimum duration of a sub-block.
 * @LLDC_PARAM_ATTR_SB_RETRY_MAX:
 *	Maximum number of retry sub-block per block.
 * @LLDC_PARAM_ATTR_SB_UL_SLOT_LAST_RSTU:
 *	Duration of the last UL slot in a sub-block.
 * @LLDC_PARAM_ATTR_SB_UL_SLOT_RSTU:
 *	Duration of the UL slot in a sub-block, except the last one.
 * @LLDC_PARAM_ATTR_SMART_POWER:
 *	True to enable smart power feature, false otherwise.
 * @LLDC_PARAM_ATTR_RESYNC_THRESHOLD_EXPONENT:
 *	Threshold on the number of blocks since last successful DL frame
 *	reception to require a resynchronization for a device. The actual
 *	threshold value is 2 to the power of resyncThresholdExponent.
 * @LLDC_PARAM_ATTR_MFS:
 *	MFS configuration.
 *
 * @LLDC_PARAM_ATTR_UNSPEC: Invalid command.
 * @__LLDC_PARAM_ATTR_AFTER_LAST: Internal use.
 * @LLDC_PARAM_ATTR_MAX: Internal use.
 */
enum lldc_param_attrs {
	LLDC_PARAM_ATTR_UNSPEC,
	LLDC_PARAM_ATTR_ALLOW_PAIRING,
	LLDC_PARAM_ATTR_AGG_LIMIT_BYTES,
	LLDC_PARAM_ATTR_BEACON_IFS,
	LLDC_PARAM_ATTR_BEACON_INTERVAL_RSTU,
	LLDC_PARAM_ATTR_BEACON_PAYLOAD,
	LLDC_PARAM_ATTR_BLOCK_RSTU,
	LLDC_PARAM_ATTR_BURST_IFS,
	LLDC_PARAM_ATTR_CAP_INTERVAL_MAX_RSTU,
	LLDC_PARAM_ATTR_CAP_MAX_BE,
	LLDC_PARAM_ATTR_CAP_MIN_BE,
	LLDC_PARAM_ATTR_CAP_SLOTS_PER_SECOND,
	LLDC_PARAM_ATTR_DEVICE_LOSS_THRESHOLD,
	LLDC_PARAM_ATTR_FAST_DS_TWR,
	LLDC_PARAM_ATTR_NON_ASSOC_IFS,
	LLDC_PARAM_ATTR_PHY_PREAMBLE_CODE,
	LLDC_PARAM_ATTR_PHY_PSDU_SIZE,
	LLDC_PARAM_ATTR_PROBING_BEACON_AS_DL,
	LLDC_PARAM_ATTR_PROBING_CAP_SLOTS,
	LLDC_PARAM_ATTR_PROBING_INTERVAL_RSTU,
	LLDC_PARAM_ATTR_SB_BEACON_CAP_SLOT_IFS,
	LLDC_PARAM_ATTR_SB_CAP_SLOT_RSTU,
	LLDC_PARAM_ATTR_SB_DL_CAP_SLOT_IFS,
	LLDC_PARAM_ATTR_SB_DL_SLOT_RSTU,
	LLDC_PARAM_ATTR_SB_MIN_RSTU,
	LLDC_PARAM_ATTR_SB_RETRY_MAX,
	LLDC_PARAM_ATTR_SB_UL_SLOT_LAST_RSTU,
	LLDC_PARAM_ATTR_SB_UL_SLOT_RSTU,
	LLDC_PARAM_ATTR_SMART_POWER,
	LLDC_PARAM_ATTR_RESYNC_THRESHOLD_EXPONENT,
	LLDC_PARAM_ATTR_MFS,

	__LLDC_PARAM_ATTR_AFTER_LAST,
	LLDC_PARAM_ATTR_MAX = __LLDC_PARAM_ATTR_AFTER_LAST - 1
};

/**
 * enum lldc_mfs_attrs - LLD coordinator mfs attributes.
 *
 * @LLDC_MFS_ATTR_MFS_ID:
 *	MFS id (0: interactive, 1: best effort).
 * @LLDC_MFS_ATTR_MFS_EXPIRATION_MS:
 *	MSDU expiration delay in milliseconds. Expiration is disabled if 0.
 * @LLDC_MFS_ATTR_MFS_WINDOW_MSDU:
 *	Size of the MFS window in number of MSDUs.
 * @LLDC_MFS_ATTR_MFS_WINDOW_BYTES:
 *	Size of the MFS window in bytes.
 *
 * @LLDC_MFS_ATTR_UNSPEC: Invalid command.
 * @__LLDC_MFS_ATTR_AFTER_LAST: Internal use.
 * @LLDC_MFS_ATTR_MAX: Internal use.
 */
enum lldc_mfs_attrs {
	LLDC_MFS_ATTR_UNSPEC,
	LLDC_MFS_ATTR_MFS_ID,
	LLDC_MFS_ATTR_MFS_EXPIRATION_MS,
	LLDC_MFS_ATTR_MFS_WINDOW_MSDU,
	LLDC_MFS_ATTR_MFS_WINDOW_BYTES,

	__LLDC_MFS_ATTR_AFTER_LAST,
	LLDC_MFS_ATTR_MAX = __LLDC_MFS_ATTR_AFTER_LAST - 1
};

/**
 * enum lldc_stats_attrs - LLD coordinator statistics attributes.
 *
 * @LLDC_STATS_ATTR_TX_FRAME_NON_ASSOCS:
 *  Number of non associative frames successfully transmitted.
 * @LLDC_STATS_ATTR_TX_FRAME_NON_ASSOCS_ERRORS:
 *  Number of non associative frames not transmitted.
 * @LLDC_STATS_ATTR_TX_FRAME_BEACON:
 *  Number of beacon frames successfully transmitted.
 * @LLDC_STATS_ATTR_TX_FRAME_BEACON_ERRORS:
 *  Number of beacon frames not transmitted.
 * @LLDC_STATS_ATTR_INTERACTIVE_MFS:
 *  Interactive MFS statistics.
 * @LLDC_STATS_ATTR_BEST_EFFORT_MFS:
 *  Best effort MFS statistics.
 * @LLDC_STATS_ATTR_TX_DL_FRAMES:
 *  Number of DL frames transmitted successfully.
 * @LLDC_STATS_ATTR_TX_DL_BYTES:
 *  Number of DL frames bytes transmitted successfully,
 *  corresponding to LLDC_STATS_ATTR_TX_DL_FRAMES.
 * @LLDC_STATS_ATTR_TX_DL_ERRORS:
 *  Number of DL frames transmitted but not acknowledged.
 * @LLDC_STATS_ATTR_RX_UL_FRAMES:
 *  Number of good UL frames received.
 * @LLDC_STATS_ATTR_RX_UL_BYTES:
 *  Number of good UL frames bytes received,
 *  corresponding to LLDC_STATS_ATTR_RX_UL_FRAMES.
 * @LLDC_STATS_ATTR_RX_UL_ERRORS:
 *  Number of UL frames received with error.
 * @LLDC_STATS_ATTR_RX_UL_RETRY_FRAMES:
 *  Number of good UL frames received in a retry sub-block.
 * @LLDC_STATS_ATTR_RX_UL_RETRY_BYTES:
 *  Number of good UL frames bytes received in a retry sub-block, corresponding
 *  to LLDC_STATS_ATTR_RX_UL_RETRY_FRAMES.
 * @LLDC_STATS_ATTR_RX_UL_RETRY_ERRORS:
 *  Number of UL frames received with error in a retry sub-block.
 * @LLDC_STATS_ATTR_LQI:
 *  Link quality indicator (LQI, 0-100).
 * @LLDC_STATS_ATTR_BLOCKS:
 *  Number of blocks in which the device participated.
 *
 * @LLDC_STATS_ATTR_UNSPEC: Invalid command.
 * @__LLDC_STATS_ATTR_AFTER_LAST: Internal use.
 * @LLDC_STATS_ATTR_MAX: Internal use.
 */
enum lldc_stats_attrs {
	LLDC_STATS_ATTR_UNSPEC,
	LLDC_STATS_ATTR_TX_FRAME_NON_ASSOCS,
	LLDC_STATS_ATTR_TX_FRAME_NON_ASSOCS_ERRORS,
	LLDC_STATS_ATTR_TX_FRAME_BEACON,
	LLDC_STATS_ATTR_TX_FRAME_BEACON_ERRORS,
	LLDC_STATS_ATTR_INTERACTIVE_MFS,
	LLDC_STATS_ATTR_BEST_EFFORT_MFS,
	LLDC_STATS_ATTR_TX_DL_FRAMES,
	LLDC_STATS_ATTR_TX_DL_BYTES,
	LLDC_STATS_ATTR_TX_DL_ERRORS,
	LLDC_STATS_ATTR_RX_UL_FRAMES,
	LLDC_STATS_ATTR_RX_UL_BYTES,
	LLDC_STATS_ATTR_RX_UL_ERRORS,
	LLDC_STATS_ATTR_RX_UL_RETRY_FRAMES,
	LLDC_STATS_ATTR_RX_UL_RETRY_BYTES,
	LLDC_STATS_ATTR_RX_UL_RETRY_ERRORS,
	LLDC_STATS_ATTR_LQI,
	LLDC_STATS_ATTR_BLOCKS,

	__LLDC_STATS_ATTR_AFTER_LAST,
	LLDC_STATS_ATTR_MAX = __LLDC_STATS_ATTR_AFTER_LAST - 1
};

/**
 * enum lldc_stats_msdu_attrs - LLD msdu statistics attributes.
 *
 * @LLDC_STATS_MSDU_ATTR_RX_PACKETS:
 *  Number of good data packets received.
 * @LLDC_STATS_MSDU_ATTR_RX_BYTES:
 *  Number of good received bytes, corresponding to
 *  LLDC_STATS_MSDU_ATTR_RX_PACKETS.
 * @LLDC_STATS_MSDU_ATTR_RX_ERRORS:
 *  Number of bad data packets received.
 * @LLDC_STATS_MSDU_ATTR_RX_IGNORED:
 *  Number of good data packets ignored.
 * @LLDC_STATS_MSDU_ATTR_TX_PACKETS:
 *  Number of data packets successfully transmitted.
 * @LLDC_STATS_MSDU_ATTR_TX_BYTES:
 *  Number of transmitted bytes, corresponding to
 *  LLDC_STATS_MSDU_ATTR_TX_PACKETS.
 * @LLDC_STATS_MSDU_ATTR_TX_ERRORS:
 *  Number of data packets not transmitted due to problem
 *  with the packet.
 * @LLDC_STATS_MSDU_ATTR_TX_DROPPED:
 *  Number of data packets dropped before successful
 *  transmission.
 *
 * @LLDC_STATS_MSDU_ATTR_UNSPEC: Invalid command.
 * @__LLDC_STATS_MSDU_ATTR_AFTER_LAST: Internal use.
 * @LLDC_STATS_MSDU_ATTR_MAX: Internal use.
 */
enum lldc_stats_msdu_attrs {
	LLDC_STATS_MSDU_ATTR_UNSPEC,
	LLDC_STATS_MSDU_ATTR_RX_PACKETS,
	LLDC_STATS_MSDU_ATTR_RX_BYTES,
	LLDC_STATS_MSDU_ATTR_RX_ERRORS,
	LLDC_STATS_MSDU_ATTR_RX_IGNORED,
	LLDC_STATS_MSDU_ATTR_TX_PACKETS,
	LLDC_STATS_MSDU_ATTR_TX_BYTES,
	LLDC_STATS_MSDU_ATTR_TX_ERRORS,
	LLDC_STATS_MSDU_ATTR_TX_DROPPED,

	__LLDC_STATS_MSDU_ATTR_AFTER_LAST,
	LLDC_STATS_MSDU_ATTR_MAX = __LLDC_STATS_MSDU_ATTR_AFTER_LAST - 1
};

#endif /* LLDC_REGION_NL_H */
