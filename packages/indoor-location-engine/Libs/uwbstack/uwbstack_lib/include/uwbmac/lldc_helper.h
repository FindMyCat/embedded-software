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

/*
 * This file is generated automatically, edit with care.
 */

#ifndef LLDC_HELPER_H
#define LLDC_HELPER_H

#include <stdint.h>
#include <stdbool.h>

#include "uwbmac/uwbmac.h"
#include "lld_helper_stats.h"
#include "net/lldc_region_nl.h"

#define LLDC_HELPER_PAIRED_DEVICE_MAX 32

/**
 * struct lldc_helper_dev_assoc_state_changed - Information on device
 * association state changed.
 */
struct lldc_helper_dev_assoc_state_changed {
	/**
	 * @dev_key: Device identity key.
	 */
	uint8_t dev_key[LLD_DEVICE_IDENTITY_PUBLIC_KEY_LEN];
	/**
	 * @associated: True if the device is associated.
	 */
	bool associated;
	/**
	 * @device_short_addr: Short address of the device, if associated.
	 */
	uint16_t device_short_addr;
};

/**
 * struct lldc_helper_dev_pairing_request - Information on device pairing
 * request.
 */
struct lldc_helper_dev_pairing_request {
	/**
	 * @dev_key: Device identity key.
	 */
	uint8_t dev_key[LLD_DEVICE_IDENTITY_PUBLIC_KEY_LEN];
	/**
	 * @association_payload: Payload embedded in received association request.
	 */
	uint8_t association_payload[LLD_ASSOCIATION_PAYLOAD_LEN_MAX];
	/**
	 * @association_payload_len: Length of association payload.
	 */
	int association_payload_len;
};

/**
 * struct lldc_helper_ops - LLD coordinator operations.
 */
struct lldc_helper_ops {
	/**
	 * @dev_assoc_state_changed: Called when a device association state change. If NULL,
	 * ignored.
	 *
	 * This callback must return quickly and it must not reenter the MAC.
	 */
	void (*dev_assoc_state_changed)(
		void *user_data,
		const struct lldc_helper_dev_assoc_state_changed
			*dev_assoc_state_changed);
	/**
	 * @dev_pairing_request: Called when a device pairing request is received. If NULL,
	 * ignored.
	 *
	 * This callback must return quickly and it must not reenter the MAC.
	 */
	void (*dev_pairing_request)(void *user_data,
				    const struct lldc_helper_dev_pairing_request
					    *dev_pairing_request);
};

/*
 * struct lldc_context - LLD Coordinator context
 *
 * NOTE: content of this struct is considered private, do not use directly.
 */

struct lldc_context {
	struct uwbmac_channel channel;
	const struct lldc_helper_ops *ops;
	void *ops_user_data;
};

/**
 * struct lldc_stats - LLD coordinator per device statistics.
 */
struct lldc_stats {
	/**
	 * @interactive_mfs: Interactive MFS packet statistics.
	 */
	struct lld_mfs_stats interactive_mfs;
	/**
	 * @best_effort_mfs: Best effort MFS packet statistics.
	 */
	struct lld_mfs_stats best_effort_mfs;
	/**
	 * @rx_ul_frames: Number of good UL frames received.
	 */
	uint32_t rx_ul_frames;
	/**
	 * @rx_ul_bytes: Number of good UL frames bytes received, FCS not
	 * included, corresponding to rx_ul_frames.
	 */
	uint32_t rx_ul_bytes;
	/**
	 * @rx_ul_errors: Number of UL frames received with error, or not
	 * received.
	 */
	uint32_t rx_ul_errors;
	/**
	 * @rx_ul_retry_frames: Number of good UL frames received in a retry
	 * sub-block.
	 */
	uint32_t rx_ul_retry_frames;
	/**
	 * @rx_ul_retry_bytes: Number of good UL frames bytes received in a
	 * retry sub-block, FCS not included, corresponding to
	 * rx_ul_retry_frames.
	 */
	uint32_t rx_ul_retry_bytes;
	/**
	 * @rx_ul_retry_errors: Number of UL frames received with error in a
	 * retry sub-block, or not received.
	 */
	uint32_t rx_ul_retry_errors;
	/**
	 * @blocks: Number of blocks in which the device participated (tracking
	 * or associated).
	 */
	uint32_t blocks;
	/**
	 * @lqi: Link quality indicator (LQI, 0-100) of all connected devices.
	 */
	uint8_t lqi;
};

/**
 * struct lldc_coord_stats - LLD global coordinator statistics.
 */
struct lldc_coord_stats {
	/**
	 * @tx_frame_non_assocs: Number of non associative frames successfully transmitted.
	 */
	uint32_t tx_frame_non_assocs;
	/**
	 * @tx_frame_non_assocs_errors: Number of non associative frames not transmitted.
	 */
	uint32_t tx_frame_non_assocs_errors;
	/**
	 * @tx_frame_beacon: Number of beacon frames successfully transmitted.
	 */
	uint32_t tx_frame_beacon;
	/**
	 * @tx_frame_beacon_errors: Number of beacon frames not transmitted.
	 */
	uint32_t tx_frame_beacon_errors;
	/**
	 * @tx_dl_frames: Number of DL frames successfully transmitted.
	 */
	uint32_t tx_dl_frames;
	/**
	 * @tx_dl_errors: Number of DL frames not transmitted.
	 */
	uint32_t tx_dl_errors;
	/**
	 * @tx_dl_bytes: Number of DL frames bytes transmitted
	 * successfully, corresponding to tx_dl_frames.
	 */
	uint32_t tx_dl_bytes;
	/**
	 * @stats: See &struct lldc_stats.
	 */
	struct lldc_stats stats;
};

/**
 * struct paired_dev_parameters - Paired devices list parameters.
 */
struct paired_dev_parameters {
	/**
	 * @dev_keys: List of device identity keys.
	 */
	uint8_t dev_keys[LLDC_HELPER_PAIRED_DEVICE_MAX]
			[LLD_DEVICE_IDENTITY_PUBLIC_KEY_LEN];
	/**
	 * @n_dev_keys: Number of device identity keys in the list.
	 */
	int n_dev_keys;
};

/**
 * lldc_helper_open() - Initialize the internal resources of the helper.
 * @context: Lldc context to initialize.
 * @uwbmac_context: UWB MAC context.
 * @ops: Coordinator operations.
 * @ops_user_data: User data given to coordinator operations.
 *
 * NOTE: This function must be called first. @lldc_helper_close must be
 * called at the end of the application to ensure resources are freed.
 *
 * Return: 0 or error.
 */
int lldc_helper_open(struct lldc_context *context,
		     struct uwbmac_context *uwbmac_context,
		     const struct lldc_helper_ops *ops, void *ops_user_data);

/**
 * lldc_helper_close() - Free all internal resources of the helper.
 * @context: Lldc context to free.
 */
void lldc_helper_close(struct lldc_context *context);

/**
 * lldc_helper_set_scheduler() - Set the scheduler and the region.
 * @context: Lldc context.
 *
 * NOTE: This function must be called while the UWB MAC is stopped.
 *
 * Return: 0 or error.
 */
int lldc_helper_set_scheduler(struct lldc_context *context);

/**
 * lldc_helper_start() - Start the coordinator.
 * @context: Lldc context.
 *
 * Return: 0 or error.
 */
int lldc_helper_start(struct lldc_context *context);

/**
 * lldc_helper_stop() - Stop the coordinator, return to the STANDBY
 * state.
 * @context: Lldc context.
 *
 * Return: 0 or error.
 */
int lldc_helper_stop(struct lldc_context *context);

/**
 * lldc_helper_add_paired_dev() - Add a device to the list of paired devices.
 * @context: Lldc context.
 * @paired_dev_parameters: Paired devices parameters.
 *
 * Return: 0 or error.
 */
int lldc_helper_add_paired_dev(
	struct lldc_context *context,
	const struct paired_dev_parameters *paired_dev_parameters);

/**
 * lldc_helper_del_paired_dev() - Delete a device from the list of paired
 * devices.
 * @context: Lldc context.
 * @paired_dev_parameters: Paired devices parameters.
 *
 * Return: 0 or error.
 */
int lldc_helper_del_paired_dev(
	struct lldc_context *context,
	const struct paired_dev_parameters *paired_dev_parameters);

/**
 * lldc_helper_get_paired_dev() - Get the list of paired devices.
 * @context: Lldc context.
 * @paired_dev_parameters: Paired devices parameters.
 *
 * Return: 0 or error.
 */
int lldc_helper_get_paired_dev(
	struct lldc_context *context,
	struct paired_dev_parameters *paired_dev_parameters);

/**
 * lldc_helper_set_allow_pairing() - Set ALLOW_PAIRING parameter.
 * @context: Lldc context.
 * @value: Parameter value.
 *
 * Allow pairing with unknown devices, requests will be forwarded to higher
 * layers.
 *
 * Return: 0 or error.
 */
int lldc_helper_set_allow_pairing(struct lldc_context *context, bool value);

/**
 * lldc_helper_set_agg_limit_bytes() - Set AGG_LIMIT_BYTES parameter.
 * @context: Lldc context.
 * @value: Parameter value.
 *
 * Limit MPDU aggregated payload size after the first sub-frame.
 *
 * Return: 0 or error.
 */
int lldc_helper_set_agg_limit_bytes(struct lldc_context *context, int value);

/**
 * lldc_helper_set_beacon_ifs() - Set BEACON_IFS parameter.
 * @context: Lldc context.
 * @slot_based: Use slot based mode if true, else gap based mode.
 * @value_rstu: Interval value in RSTU.
 *
 * Interval between the last sub-block DL frame and the beacon frame.
 *
 * Return: 0 or error.
 */
int lldc_helper_set_beacon_ifs(struct lldc_context *context, bool slot_based,
			       int value_rstu);

/**
 * lldc_helper_set_beacon_interval_rstu() - Set BEACON_INTERVAL_RSTU parameter.
 * @context: Lldc context.
 * @value_rstu: Parameter value.
 *
 * Interval between two beacons transmissions when the coordinator is in ACTIVE
 * state. Set to 0 to disable beacon transmission.
 *
 * Return: 0 or error.
 */
int lldc_helper_set_beacon_interval_rstu(struct lldc_context *context,
					 int value_rstu);

/**
 * lldc_helper_set_beacon_payload() - Set BEACON_PAYLOAD parameter.
 * @context: Lldc context.
 * @data: Data value.
 * @size: Data size.
 *
 * Payload embedded in transmitted beacons, maximum size of 32 bytes.
 *
 * Return: 0 or error.
 */
int lldc_helper_set_beacon_payload(struct lldc_context *context,
				   const uint8_t *data, size_t size);

/**
 * lldc_helper_set_block_rstu() - Set BLOCK_RSTU parameter.
 * @context: Lldc context.
 * @value_rstu: Parameter value.
 *
 * Duration of a block.
 *
 * Return: 0 or error.
 */
int lldc_helper_set_block_rstu(struct lldc_context *context, int value_rstu);

/**
 * lldc_helper_set_burst_ifs() - Set BURST_IFS parameter.
 * @context: Lldc context.
 * @slot_based: Use slot based mode if true, else gap based mode.
 * @value_rstu: Interval value in RSTU.
 *
 * Interval between burst DL frames, or between the last sub-block DL frame and
 * the first burst DL frame.
 *
 * Return: 0 or error.
 */
int lldc_helper_set_burst_ifs(struct lldc_context *context, bool slot_based,
			      int value_rstu);

/**
 * lldc_helper_set_cap_interval_max_rstu() - Set CAP_INTERVAL_MAX_RSTU
 * parameter.
 * @context: Lldc context.
 * @value_rstu: Parameter value.
 *
 * Maximum interval between two CAP when the coordinator is in ACTIVE state. Set
 * to 0 to disable CAP.
 *
 * Return: 0 or error.
 */
int lldc_helper_set_cap_interval_max_rstu(struct lldc_context *context,
					  int value_rstu);

/**
 * lldc_helper_set_cap_max_be() - Set CAP_MAX_BE parameter.
 * @context: Lldc context.
 * @value: Parameter value.
 *
 * Maximum backoff exponent.
 *
 * Return: 0 or error.
 */
int lldc_helper_set_cap_max_be(struct lldc_context *context, int value);

/**
 * lldc_helper_set_cap_min_be() - Set CAP_MIN_BE parameter.
 * @context: Lldc context.
 * @value: Parameter value.
 *
 * Minimum backoff exponent.
 *
 * Return: 0 or error.
 */
int lldc_helper_set_cap_min_be(struct lldc_context *context, int value);

/**
 * lldc_helper_set_cap_slots_per_second() - Set CAP_SLOTS_PER_SECOND parameter.
 * @context: Lldc context.
 * @value: Parameter value.
 *
 * Average number of contention slots per second.
 *
 * Return: 0 or error.
 */
int lldc_helper_set_cap_slots_per_second(struct lldc_context *context,
					 int value);

/**
 * lldc_helper_set_device_loss_threshold() - Set DEVICE_LOSS_THRESHOLD
 * parameter.
 * @context: Lldc context.
 * @value: Parameter value.
 *
 * Number of consecutive failed blocks for a coordinator to declare a device
 * loss.
 *
 * Return: 0 or error.
 */
int lldc_helper_set_device_loss_threshold(struct lldc_context *context,
					  int value);

/**
 * lldc_helper_set_fast_ds_twr() - Set FAST_DS_TWR parameter.
 * @context: Lldc context.
 * @value: Parameter value.
 *
 * TRUE to use the fast DS-TWR ranging procedure: disable smart power and ignore
 * SB_MIN_RSTU.
 *
 * Return: 0 or error.
 */
int lldc_helper_set_fast_ds_twr(struct lldc_context *context, bool value);

/**
 * lldc_helper_set_non_assoc_ifs() - Set NON_ASSOC_IFS parameter.
 * @context: Lldc context.
 * @slot_based: Use slot based mode if true, else gap based mode.
 * @value_rstu: Interval value in RSTU.
 *
 * Interval between the last sub-block DL frame and the first frame of non-
 * associated exchange.
 *
 * Return: 0 or error.
 */
int lldc_helper_set_non_assoc_ifs(struct lldc_context *context, bool slot_based,
				  int value_rstu);

/**
 * lldc_helper_set_phy_preamble_code() - Set PHY_PREAMBLE_CODE parameter.
 * @context: Lldc context.
 * @value: Parameter value.
 *
 * Preamble code index in use. 9-24: BPRF, 25-32: HPRF.
 *
 * Return: 0 or error.
 */
int lldc_helper_set_phy_preamble_code(struct lldc_context *context, int value);

/**
 * lldc_helper_set_phy_psdu_size() - Set PHY_PSDU_SIZE parameter.
 * @context: Lldc context.
 * @value: Parameter value.
 *
 * For HPRF, specify the use of A0 and A1 fields of the PHR to extend the
 * maximum PSDU size. 0: 1023, 1: 2047, 2: 4095.
 *
 * Return: 0 or error.
 */
int lldc_helper_set_phy_psdu_size(struct lldc_context *context, int value);

/**
 * lldc_helper_set_probing_beacon_as_dl() - Set PROBING_BEACON_AS_DL parameter.
 * @context: Lldc context.
 * @value: Parameter value.
 *
 * True to use a beacon frame to fill the role of both DL frame and beacon frame
 * when the coordinator is in the PROBING state.
 *
 * Return: 0 or error.
 */
int lldc_helper_set_probing_beacon_as_dl(struct lldc_context *context,
					 bool value);

/**
 * lldc_helper_set_probing_cap_slots() - Set PROBING_CAP_SLOTS parameter.
 * @context: Lldc context.
 * @value: Parameter value.
 *
 * Override CAP_SLOTS_PER_SECOND when the coordinator is in PROBING state.
 *
 * Return: 0 or error.
 */
int lldc_helper_set_probing_cap_slots(struct lldc_context *context, int value);

/**
 * lldc_helper_set_probing_interval_rstu() - Set PROBING_INTERVAL_RSTU
 * parameter.
 * @context: Lldc context.
 * @value_rstu: Parameter value.
 *
 * Interval between two beacons transmissions when the coordinator is in PROBING
 * state. The same interval is used between two CAP.
 *
 * Return: 0 or error.
 */
int lldc_helper_set_probing_interval_rstu(struct lldc_context *context,
					  int value_rstu);

/**
 * lldc_helper_set_sb_beacon_cap_slot_ifs() - Set SB_BEACON_CAP_SLOT_IFS
 * parameter.
 * @context: Lldc context.
 * @slot_based: Use slot based mode if true, else gap based mode.
 * @value_rstu: Interval value in RSTU.
 *
 * Interval between the beacon frame acting as a DL frame and the first
 * additional CAP slot of a combined main sub-block and CAP.
 *
 * Return: 0 or error.
 */
int lldc_helper_set_sb_beacon_cap_slot_ifs(struct lldc_context *context,
					   bool slot_based, int value_rstu);

/**
 * lldc_helper_set_sb_cap_slot_rstu() - Set SB_CAP_SLOT_RSTU parameter.
 * @context: Lldc context.
 * @value_rstu: Parameter value.
 *
 * Duration of the additional CAP slots in a combined sub-block and CAP.
 *
 * Return: 0 or error.
 */
int lldc_helper_set_sb_cap_slot_rstu(struct lldc_context *context,
				     int value_rstu);

/**
 * lldc_helper_set_sb_dl_cap_slot_ifs() - Set SB_DL_CAP_SLOT_IFS parameter.
 * @context: Lldc context.
 * @slot_based: Use slot based mode if true, else gap based mode.
 * @value_rstu: Interval value in RSTU.
 *
 * Interval between the last sub-block DL frame and the first additional CAP
 * slot of a combined sub-block and CAP.
 *
 * Return: 0 or error.
 */
int lldc_helper_set_sb_dl_cap_slot_ifs(struct lldc_context *context,
				       bool slot_based, int value_rstu);

/**
 * lldc_helper_set_sb_dl_slot_rstu() - Set SB_DL_SLOT_RSTU parameter.
 * @context: Lldc context.
 * @value_rstu: Parameter value.
 *
 * Duration of the DL slot in a sub-block.
 *
 * Return: 0 or error.
 */
int lldc_helper_set_sb_dl_slot_rstu(struct lldc_context *context,
				    int value_rstu);

/**
 * lldc_helper_set_sb_min_rstu() - Set SB_MIN_RSTU parameter.
 * @context: Lldc context.
 * @value_rstu: Parameter value.
 *
 * Minimum duration of a sub-block.
 *
 * Return: 0 or error.
 */
int lldc_helper_set_sb_min_rstu(struct lldc_context *context, int value_rstu);

/**
 * lldc_helper_set_sb_retry_max() - Set SB_RETRY_MAX parameter.
 * @context: Lldc context.
 * @value: Parameter value.
 *
 * Maximum number of retry sub-block per block.
 *
 * Return: 0 or error.
 */
int lldc_helper_set_sb_retry_max(struct lldc_context *context, int value);

/**
 * lldc_helper_set_sb_ul_slot_last_rstu() - Set SB_UL_SLOT_LAST_RSTU parameter.
 * @context: Lldc context.
 * @value_rstu: Parameter value.
 *
 * Duration of the last UL slot in a sub-block.
 *
 * Return: 0 or error.
 */
int lldc_helper_set_sb_ul_slot_last_rstu(struct lldc_context *context,
					 int value_rstu);

/**
 * lldc_helper_set_sb_ul_slot_rstu() - Set SB_UL_SLOT_RSTU parameter.
 * @context: Lldc context.
 * @value_rstu: Parameter value.
 *
 * Duration of the UL slot in a sub-block, except the last one.
 *
 * Return: 0 or error.
 */
int lldc_helper_set_sb_ul_slot_rstu(struct lldc_context *context,
				    int value_rstu);

/**
 * lldc_helper_set_smart_power() - Set SMART_POWER parameter.
 * @context: Lldc context.
 * @value: Parameter value.
 *
 * True to enable smart power feature, false otherwise.
 *
 * Return: 0 or error.
 */
int lldc_helper_set_smart_power(struct lldc_context *context, bool value);

/**
 * lldc_helper_set_resync_threshold_exponent() - Set RESYNC_THRESHOLD_EXPONENT
 * parameter.
 * @context: Lldc context.
 * @value: Parameter value.
 *
 * Threshold on the number of blocks since last successful DL frame reception to
 * require a resynchronization for a device. The actual threshold value is 2 to
 * the power of resyncThresholdExponent.
 *
 * Return: 0 or error.
 */
int lldc_helper_set_resync_threshold_exponent(struct lldc_context *context,
					      int value);

/**
 * lldc_helper_set_mfs_parameters() - Set MFS parameters.
 * @context: Lldc context.
 * @mfs_id: MFS id (0: interactive, 1: best effort).
 * @mfs_expiration_ms: MSDU expiration delay in milliseconds. Expiration is
 * disabled if 0.
 * @mfs_window_msdu: Size of the MFS window in number of MSDUs.
 * @mfs_window_bytes: Size of the MFS window in bytes.
 *
 * Return: 0 or error.
 */
int lldc_helper_set_mfs_parameters(struct lldc_context *context, int mfs_id,
				   int mfs_expiration_ms, int mfs_window_msdu,
				   int mfs_window_bytes);

/**
 * lldc_helper_get_time() - Get current time and last time offset.
 * @context: Lldc context.
 * @local_time_dtu: Local time value in DTU.
 * @network_time_offset_dtu: Network time offset value in DTU, difference
 * between coordinator and device time.
 *
 * Return: UWBMAC_SUCCESS or error.
 */
int lldc_helper_get_time(struct lldc_context *context, uint32_t *local_time_dtu,
			 uint32_t *network_time_offset_dtu);

/**
 * lldc_helper_get_stats() - Get coordinator network statistics.
 * @context: Lldc context.
 * @stats: Coordinator statistics.
 *
 * Return: UWBMAC_SUCCESS or error.
 */
int lldc_helper_get_stats(struct lldc_context *context,
			  struct lldc_coord_stats *stats);

/**
 * lldc_helper_get_device_stats() - Get device network statistics.
 * @context: Lldc context.
 * @short_addr: device's short address.
 * @stats: Device statistics.
 *
 * Return: UWBMAC_SUCCESS or error.
 */
int lldc_helper_get_device_stats(struct lldc_context *context,
				 uint16_t short_addr, struct lldc_stats *stats);

/**
 * lldc_helper_reset_stats() - Reset statistics immediately. Caller must note
 * that the reset can occur during an lld sub-block.
 * @context: Lldc context.
 *
 * Return: UWBMAC_SUCCESS or error.
 */
int lldc_helper_reset_stats(struct lldc_context *context);

#endif /* LLDC_HELPER_H */
