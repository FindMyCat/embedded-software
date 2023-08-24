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

#ifndef LLDD_HELPER_H
#define LLDD_HELPER_H

#include <stdint.h>
#include <stdbool.h>

#include "uwbmac/uwbmac.h"
#include "lld_helper_stats.h"

/**
 * LLDD_HELPER_BEACON_PAYLOAD_LEN_MAX - Maximum length of beacon payload.
 */
#define LLDD_HELPER_BEACON_PAYLOAD_LEN_MAX 32

/**
 * enum lldd_helper_state - State of the device.
 * @LLDD_HELPER_STATE_STANDBY:
 * 	Standby state.
 * @LLDD_HELPER_STATE_SCANNING:
 * 	Scanning state.
 * @LLDD_HELPER_STATE_TRACKING:
 * 	Tracking state.
 * @LLDD_HELPER_STATE_ASSOCIATED_ACTIVE:
 * 	Active state.
 * @LLDD_HELPER_STATE_ASSOCIATED_INACTIVE:
 * 	Inactive state.
 */
enum lldd_helper_state {
	LLDD_HELPER_STATE_STANDBY,
	LLDD_HELPER_STATE_SCANNING,
	LLDD_HELPER_STATE_TRACKING,
	LLDD_HELPER_STATE_ASSOCIATED_ACTIVE,
	LLDD_HELPER_STATE_ASSOCIATED_INACTIVE,
};

/**
 * enum lldd_helper_association_failure_reason - LLD device association failure
 * reason.
 *
 * @LLDD_HELPER_ASSOCIATION_FAILURE_REASON_PAN_AT_CAPACITY:
 *  Not enough resources.
 * @LLDD_HELPER_ASSOCIATION_FAILURE_REASON_PAN_ACCESS_DENIED:
 *  Device not paired.
 */
enum lldd_helper_association_failure_reason {
	LLDD_HELPER_ASSOCIATION_FAILURE_REASON_PAN_AT_CAPACITY,
	LLDD_HELPER_ASSOCIATION_FAILURE_REASON_PAN_ACCESS_DENIED,
};

/**
 * struct lldd_helper_network - Information on a detected network.
 */
struct lldd_helper_network {
	/**
	 * @pan_id: Network PAN ID.
	 */
	uint16_t pan_id;
	/**
	 * @coord_short_addr: Coordinator short address.
	 */
	uint16_t coord_short_addr;
	/**
	 * @lqi: Link quality indicator (LQI, 0-100).
	 */
	uint8_t lqi;
	/**
	 * @pairing_allowed: Whether the coordinator accepts pairing.
	 */
	bool pairing_allowed;
	/**
	 * @beacon_payload: Payload embedded in beacon.
	 */
	uint8_t beacon_payload[LLDD_HELPER_BEACON_PAYLOAD_LEN_MAX];
	/**
	 * @beacon_payload_len: Length of beacon payload.
	 */
	size_t beacon_payload_len;
};

/**
 * struct lldd_stats - LLD device statistics.
 */
struct lldd_stats {
	/**
	 * @interactive_mfs: Interactive MFS packet statistics.
	 */
	struct lld_mfs_stats interactive_mfs;
	/**
	 * @best_effort_mfs: Best effort MFS packet statistics.
	 */
	struct lld_mfs_stats best_effort_mfs;
	/**
	 * @rx_dl_frames: Number of good DL frames received.
	 */
	uint32_t rx_dl_frames;
	/**
	 * @rx_dl_bytes: Number of good DL frames bytes received, FCS not
	 * included, corresponding to rx_dl_frames.
	 */
	uint32_t rx_dl_bytes;
	/**
	 * @rx_dl_errors: Number of DL frames received with error, or not
	 * received.
	 */
	uint32_t rx_dl_errors;
	/**
	 * @rx_dl_retry_frames: Number of good DL frames received in a retry
	 * sub-block.
	 */
	uint32_t rx_dl_retry_frames;
	/**
	 * @rx_dl_retry_bytes: Number of good DL frames bytes received in a
	 * retry sub-block, FCS not included, corresponding to rx_dl_retry_frames.
	 */
	uint32_t rx_dl_retry_bytes;
	/**
	 * @rx_dl_retry_errors: Number of DL frames received with error in a
	 * retry sub-block, or not received.
	 */
	uint32_t rx_dl_retry_errors;
	/**
	 * @tx_ul_frames: Number of UL frames transmitted successfully (acknowledged
	 * successfully or no acknowledgement required).
	 */
	uint32_t tx_ul_frames;
	/**
	 * @tx_ul_bytes: Number of UL frames bytes transmitted successfully,
	 * corresponding to tx_ul_frames.
	 */
	uint32_t tx_ul_bytes;
	/**
	 * @tx_ul_errors: Number of UL frames transmitted with error.
	 */
	uint32_t tx_ul_errors;
	/**
	 * @rx_beacon_frames: Number of beacon frames received.
	 */
	uint32_t rx_beacon_frames;
	/**
	 * @rx_beacon_errors: Number of beacon frames received with errors.
	 */
	uint32_t rx_beacon_errors;
	/**
	 * @rx_unrelateds: Number of unrelated frames received.
	 */
	uint32_t rx_unrelateds;
	/**
	 * @rx_frame_non_assocs: Number of non associative frames received.
	 */
	uint32_t rx_frame_non_assocs;
	/**
	 * @rx_frame_non_assocs_errors: Number of non associative frames errors.
	 */
	uint32_t rx_frame_non_assocs_errors;
	/**
	 * @blocks: Number of blocks in which the device participated (tracking
	 * or associated).
	 */
	uint32_t blocks;
	/**
	 * @lqi: Link quality indicator (LQI, 0-100).
	 */
	uint8_t lqi;
};

/**
 * struct lldd_helper_ops - LLD device operations.
 */
struct lldd_helper_ops {
	/**
	 * @state_changed: Called when the device state change. If NULL,
	 * ignored.
	 *
	 * This callback must return quickly and it must not reenter the MAC.
	 */
	void (*state_changed)(void *user_data,
			      enum lldd_helper_state new_state);
	/**
	 * @network_detected: Notify the detection of a network. If NULL,
	 * ignored.
	 *
	 * This callback must return quickly and it must not reenter the MAC.
	 */
	void (*network_detected)(void *user_data,
				 const struct lldd_helper_network *network);
	/**
	 * @association_failed: Notify the association with the network failed.
	 * If NULL, ignored.
	 *
	 * This callback must return quickly and it must not reenter the MAC.
	 */
	void (*association_failed)(
		void *user_data,
		enum lldd_helper_association_failure_reason reason);
};

/*
 * struct lldd_context - LLD Device context
 *
 * NOTE: content of this struct is considered private, do not use directly.
 */

struct lldd_context {
	struct uwbmac_channel channel;
	const struct lldd_helper_ops *ops;
	void *ops_user_data;
};

/**
 * lldd_helper_open() - Initialize the internal resources of the helper.
 * @context: Lldd context to initialize.
 * @uwbmac_context: UWB MAC context.
 * @ops: Device operations.
 * @ops_user_data: User data given to device operations.
 *
 * NOTE: This function must be called first. @lldd_helper_close must be
 * called at the end of the application to ensure resources are freed.
 *
 * Return: 0 or error.
 */
int lldd_helper_open(struct lldd_context *context,
		     struct uwbmac_context *uwbmac_context,
		     const struct lldd_helper_ops *ops, void *ops_user_data);

/**
 * lldd_helper_close() - Free all internal resources of the helper.
 * @context: Lldd context to free.
 */
void lldd_helper_close(struct lldd_context *context);

/**
 * lldd_helper_set_scheduler() - Set the scheduler and the region.
 * @context: Lldd context.
 *
 * NOTE: This function must be called while the UWB MAC is stopped.
 *
 * Return: 0 or error.
 */
int lldd_helper_set_scheduler(struct lldd_context *context);

/**
 * lldd_helper_start() - Start the device and scan for networks.
 * @context: Lldd context.
 *
 * When the device is started, it goes to the SCANNING state in order to detect
 * a network. Every detected networks will be reported thanks to the network
 * notification callback.
 *
 * If device is not currently in the STANDBY state, it first leaves its previous
 * state then goes to the SCANNING state.
 *
 * Return: 0 or error.
 */
int lldd_helper_start(struct lldd_context *context);

/**
 * lldd_helper_track() - Track a detected network.
 * @context: Lldd context.
 * @pan_id: PAN ID of the network to track.
 * @coord_short_addr: Address of the coordinator of the network to track.
 *
 * If device is not currently in the SCANNING state, it goes to the SCANNING
 * state to detect the requested network.
 *
 * Once the network has been detected, it goes to the TRACKING state and tracks
 * the network. In this state, an association attempt can be made.
 *
 * Return: 0 or error.
 */
int lldd_helper_track(struct lldd_context *context, uint16_t pan_id,
		      uint16_t coord_short_addr);

/**
 * lldd_helper_stop() - Stop the device, return to the STANDBY state.
 * @context: Lldd context.
 *
 * Return: 0 or error.
 */
int lldd_helper_stop(struct lldd_context *context);

/**
 * lldd_helper_request_association() - Request a network association.
 * @context: Lldd context.
 * @dev_key_data: Device identity key.
 * @dev_key_size: Device identity key length.
 * @payload_data: Payload embedded in the association request.
 * @payload_size: Payload length.
 *
 * Return: 0 or error.
 */
int lldd_helper_request_association(struct lldd_context *context,
				    const uint8_t *dev_key_data,
				    size_t dev_key_size,
				    const uint8_t *payload_data,
				    size_t payload_size);
/**
 * lldd_helper_set_agg_limit_bytes() - Set AGG_LIMIT_BYTES parameter.
 * @context: Lldd context.
 * @value: Parameter value.
 *
 * Limit MPDU aggregated payload size after the first sub-frame.
 *
 * Return: 0 or error.
 */
int lldd_helper_set_agg_limit_bytes(struct lldd_context *context, int value);

/**
 * lldd_helper_set_inactive_exponent() - Set INACTIVE_EXPONENT parameter.
 * @context: Lldd context.
 * @value: Parameter value.
 *
 * If not 0, the device will behave as an inactive device. It will participate
 * only in blocks with an index which is a multiple of 2 to the power of
 * inactiveExponent.
 *
 * Return: 0 or error.
 */
int lldd_helper_set_inactive_exponent(struct lldd_context *context, int value);

/**
 * lldd_helper_set_phy_preamble_code() - Set PHY_PREAMBLE_CODE parameter.
 * @context: Lldd context.
 * @value: Parameter value.
 *
 * Preamble code index in use. 9-24: BPRF, 25-32: HPRF.
 *
 * Return: 0 or error.
 */
int lldd_helper_set_phy_preamble_code(struct lldd_context *context, int value);

/**
 * lldd_helper_set_phy_psdu_size() - Set PHY_PSDU_SIZE parameter.
 * @context: Lldd context.
 * @value: Parameter value.
 *
 * For HPRF, specify the use of A0 and A1 fields of the PHR to extend the
 * maximum PSDU size. 0: 1023, 1: 2047, 2: 4095.
 *
 * Return: 0 or error.
 */
int lldd_helper_set_phy_psdu_size(struct lldd_context *context, int value);

/**
 * lldd_helper_set_smart_power() - Set SMART_POWER parameter.
 * @context: Lldd context.
 * @value: Parameter value.
 *
 * True to enable smart power feature, false otherwise.
 *
 * Return: 0 or error.
 */
int lldd_helper_set_smart_power(struct lldd_context *context, bool value);

/**
 * lldd_helper_set_tracking_loss_threshold() - Set TRACKING_LOSS_THRESHOLD
 * parameter.
 * @context: Lldd context.
 * @value: Parameter value.
 *
 * Number of consecutive failed blocks for a device to declare tracking loss.
 *
 * Return: 0 or error.
 */
int lldd_helper_set_tracking_loss_threshold(struct lldd_context *context,
					    int value);

/**
 * lldd_helper_set_mfs_parameters() - Set MFS parameters.
 * @context: Lldd context.
 * @mfs_id: MFS id (0: interactive, 1: best effort).
 * @mfs_expiration_ms: MSDU expiration delay in milliseconds. Expiration is
 * disabled if 0.
 * @mfs_window_msdu: Size of the MFS window in number of MSDUs.
 * @mfs_window_bytes: Size of the MFS window in bytes.
 *
 * Return: 0 or error.
 */
int lldd_helper_set_mfs_parameters(struct lldd_context *context, int mfs_id,
				   int mfs_expiration_ms, int mfs_window_msdu,
				   int mfs_window_bytes);

/**
 * lldd_helper_get_time() - Get current time and last time offset.
 * @context: Lldd context.
 * @local_time_dtu: Local time value in DTU.
 * @network_time_offset_dtu: Network time offset value in DTU, difference
 * between coordinator and device time.
 *
 * Return: UWBMAC_SUCCESS or error.
 */
int lldd_helper_get_time(struct lldd_context *context, uint32_t *local_time_dtu,
			 uint32_t *network_time_offset_dtu);

/**
 * lldd_helper_get_stats() - Get Device network statistics.
 * @context: Lldd context.
 * @stats: Device statistics.
 *
 * Return: UWBMAC_SUCCESS or error.
 */
int lldd_helper_get_stats(struct lldd_context *context,
			  struct lldd_stats *stats);

/**
 * lldd_helper_reset_stats() - Reset statistics immediately. Caller must note
 * that the reset can occur during an lld sub-block.
 * @context: Lldd context.
 *
 * Return: UWBMAC_SUCCESS or error.
 */
int lldd_helper_reset_stats(struct lldd_context *context);

#endif /* LLDD_HELPER_H */
