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

#ifndef UWBMAC_PCTT_HELPER_H
#define UWBMAC_PCTT_HELPER_H

#include <stdint.h>
#include <stdbool.h>

#include <net/pctt_region_nl.h>
#include <net/pctt_region_params.h>

#include "uwbmac/uwbmac.h"

#define PCTT_PAYLOAD_MAX_LEN 4096

/**
 * struct pctt_parameters - PCTT parameters.
 */
struct pctt_parameters {
	/**
	 * @num_packets: number of packets.
	 */
	uint32_t num_packets;
	/**
	 * @t_gap: Gap between start of one packet to the next in micro seconds.
	 */
	uint32_t t_gap;
	/**
	 * @t_start: Max. time from the start of T_GAP to SFD found state in micro seconds.
	 */
	uint32_t t_start;
	/**
	 * @t_win: Max. time for which RX is looking for a packet from the start of T_GAP in micro seconds.
	 */
	uint32_t t_win;
	/**
	 * @randomize_psdu: 0 no randomization, 1 take first byte of data supplied by cmd is the seed.
	 */
	uint8_t randomize_psdu;
	/**
	 * @phr_ranging_bit: 0 disable, 1 enable. Configures ranging bit field of PHR.
	 */
	uint8_t phr_ranging_bit;
	/**
	 * @rmarker_tx_start: Start time of TX in 1/(128*499.2MHz) units.
	 */
	uint32_t rmarker_tx_start;
	/**
	 * @rmarker_rx_start: Start time of RX in 1/(128*499.2MHz) units.
	 */
	uint32_t rmarker_rx_start;
	/**
	 * @sts_index_auto_incr: 0x00: STS_INDEX config value is used for all PER Rx/ Periodic TX.
	 * 0x01: STS_INDEX value SHALL be incremented for every frame in PER Rx/Periodic TX test.
	 */
	uint8_t sts_index_auto_incr;
};

/**
 * struct pctt_test_payload - PCTT test payload.
 */
struct pctt_test_payload {
	/**
	 * @payload: PSDU data bytes used by certain cmd tests.
	 */
	uint8_t payload[PCTT_PAYLOAD_MAX_LEN];
	/**
	 * @payload_len: payload length.
	 */
	int payload_len;
};

//FIXME(Guillaume): I don't think we really need all those parameters from regular fira sessions.
/**
 * struct pctt_session_parameters - PCTT session parameters.
 */
struct pctt_session_parameters {
	/**
	 * @device_role: see enum device_role.
	 */
	uint8_t device_role;
	/**
	 * @short_addr: Device short address
	 */
	uint16_t short_addr;
	/**
	 * @destination_short_address: Address of controller.
	 */
	uint16_t destination_short_address;
	/**
	 * @tx_antenna_selection: Selection of TX antenna configuration for this session.
	 */
	uint8_t tx_antenna_selection;
	/**
	 * @rx_antenna_selection: Selection of RX antenna configuration for this session.
	 */
	uint8_t rx_antenna_selection;
	/**
	 * @slot_duration_rstu: Duration of a slot in RSTU. (1200RSTU=1ms)
	 */
	uint32_t slot_duration_rstu;
	/**
	 * @channel_number: Uwb channel for this session.
	 */
	uint8_t channel_number;
	/**
	 * @preamble_code_index: Uwb preamble code index.
	 * BPRF (9-24), HPRF (25-32)
	 */
	uint8_t preamble_code_index;
	/**
	 * @rframe_config: see enum fira_rframe_config.
	 */
	uint8_t rframe_config;
	/**
	 * @prf_mode: prf_mode
	 * 0x00 = 62.4 MHz PRF. BPRF mode (default)
	 * 0x01 = 124.8 MHz PRF. HPRF mode.
	 * 0x02 – 249.6 MHz PRF. HPRF mode with data
	 * rate 27.2 and 31.2 Mbps
	 * Values 0x03 to 0xFF = RFU
	 */
	uint8_t prf_mode;
	/**
	 * @preamble_duration: preamble_duration
	 * 0x00 = 32 symbols
	 * 0x01 = 64 symbols (default)
	 * Values 0x02 to 0xFF = RFU
	 */
	uint8_t preamble_duration;
	/**
	 * @sfd_id: BPRF (O or 2), HPRF (1-4).
	 */
	uint8_t sfd_id;
	/**
	 * @number_of_sts_segments: number_of_sts_segments
	 * 0x01 = 1 STS Segment (default)
	 * 0x02 = 2 STS Segments (HPRF only)
	 * 0x03 = 3 STS Segments (HPRF only)
	 * 0x04 = 4 STS Segments (HPRF only)
	 * Values 0x05 to 0xFF = RFU
	 */
	uint8_t number_of_sts_segments;
	/**
	 * @psdu_data_rate: psdu_data_rate
	 * 0x00 - > 6.81Mbps (Default)
	 * 0x01 = 7.80 Mbps
	 * 0x02 = 27.2 Mbps
	 * 0x03 = 31.2 Mbps
	 * 0x04 = 850 Kbps
	 * Values 0x00, 0x02, 0x04 map to K=3 and
	 * 0x01, 0x03 map to K=7.
	 * Values 0x05 to 0xFF = RFU
	 */
	uint8_t psdu_data_rate;
	/**
	 * @mac_fcs_type: mac_fcs_type
	 * 0x00 = CRC 16 (default)
	 * 0x01 = CRC 32
	 * Values 0x02 to 0xFF = RFU
	*/
	uint8_t mac_fcs_type;
	/**
	 * @tx_adaptive_payload_power: tx_adaptive_payload_power
	 * 0x00 = Disable
	 * 0x01 = Enable
	 * Values 0x02 to 0xFF = RFU
	 */
	uint8_t tx_adaptive_payload_power;
	/**
	 * @sts_index: sts_index. default = 0.
	 */
	uint32_t sts_index;
	/**
	 * @sts_length: Number of symbols in a STS segment.
	 *
	 * Possible values:
	 *
	 * - 0x00: 32 symbols
	 * - 0x01: 64 symbols (default)
	 * - 0x02: 128 symbols
	 * - Values 0x03 to 0xFF: RFU
	 */
	uint8_t sts_length;
};

/**
 * struct pctt_result_data - PCTT result data.
 */
struct pctt_result_data {
	/**
	 * @status: Generic status code.
	 */
	uint8_t status;
	/**
	 * @attempts: Number of RX attempts.
	 */
	uint32_t attempts;
	/**
	 * @acq_detect: Number of times signal was detected.
	 */
	uint32_t acq_detect;
	/**
	 * @acq_reject: Number of times signal was rejected.
	 */
	uint32_t acq_reject;
	/**
	 * @rx_fail: Number of times RX did not go beyond ACQ stage.
	 */
	uint32_t rx_fail;
	/**
	 * @sync_cir_ready: Number of times sync. CIR ready event was received.
	 */
	uint32_t sync_cir_ready;
	/**
	 * @sfd_fail: Number of times RX was stuck at either ACQ detect or sync CIR ready.
	 */
	uint32_t sfd_fail;
	/**
	 * @sfd_found: Number of times SFD was found.
	 */
	uint32_t sfd_found;
	/**
	 * @phr_dec_error: No. of times PHR decode failed.
	 */
	uint32_t phr_dec_error;
	/**
	 * @phr_bit_error: No. of times PHR bits in error.
	 */
	uint32_t phr_bit_error;
	/**
	 * @psdu_dec_error: No. of times payload decode failed.
	 */
	uint32_t psdu_dec_error;
	/**
	 * @psdu_bit_error: No. of times payload bits in error.
	 */
	uint32_t psdu_bit_error;
	/**
	 * @sts_found: No. of times STS detection was successful.
	 */
	uint32_t sts_found;
	/**
	 * @eof: No. of times end of frame event was triggered
	 */
	uint32_t eof;
	/**
	 * @rx_done_ts_int: Integer part of timestamp 1/124.8Mhz ticks.
	 */
	uint32_t rx_done_ts_int;
	/**
	 * @rx_done_ts_frac: Fractional part of timestamp 1/(128*499.2Mhz) ticks.
	 */
	uint16_t rx_done_ts_frac;
	/**
	 * @aoa_azimuth: AoA Azimuth in degrees and it is a signed value in Q9.7 format.
	 * This field is zero if AOA_RESULT_REQ = 0.
	 */
	uint16_t aoa_azimuth;
	/**
	 * @aoa_elevation: AoA Elevation in degrees and it is a signed value in Q9.7 format.
	 * This field is zero if AOA_RESULT_REQ = 0.
	 */
	uint16_t aoa_elevation;
	/**
	 * @toa_gap: ToA of main path minus ToA of first path in nanoseconds.
	 */
	uint8_t toa_gap;
	/**
	 * @phr: Received PHR (bits 0-12 as per IEEE spec).
	 */
	uint8_t phr;
	/**
	 * @psdu_data: Length of PSDU Data(N) to follow.
	 */
	uint8_t psdu_data[PCTT_PAYLOAD_MAX_LEN];
	/**
	 * @psdu_data_len: Length of psdu_data.
	 */
	uint16_t psdu_data_len;
	/**
	 * @tx_ts_int: Integer part of timestamp in 1/124.8 us resolution.
	 */
	uint32_t tx_ts_int;
	/**
	 * @tx_ts_frac: Fractional part of timestamp in 1/124.8/512 us resolution.
	 */
	uint16_t tx_ts_frac;
	/**
	 * @rx_ts_int: Integer part of timestamp in 1/124.8 us resolution.
	 */
	uint32_t rx_ts_int;
	/**
	 * @rx_ts_frac: Fractional part of timestamp in 1/124.8/512 us resolution.
	 */
	uint16_t rx_ts_frac;
	/**
	 * @measurement: For TEST_SS_TWR_NTF. Contains Tround time of Initiator or Treply time of Responder depending on DEVICE_ROLE option.
	 * This is expressed in 1/(128 * 499.2Mhz) ticks.
	 */
	uint32_t measurement;
	/**
	 * @pdoa_azimuth_deg_q7: Phase difference on Arrival,azimuth,coded as Q7
	 */
	int16_t pdoa_azimuth_deg_q7;
	/**
	 * @pdoa_elevation_deg_q7: Phase difference on Arrival,elevation, coded as Q7
	 */
	int16_t pdoa_elevation_deg_q7;
	/**
	 * @rssi_q1: Calculated Received Signal Strength Indicator, coded as Q7.1
	 */
	uint8_t rssi_q1;
	/**
	 * @aoa_azimuth_deg_q7: AoA Azimuth, coded as Q7
	 */
	int16_t aoa_azimuth_deg_q7;
	/**
	 * @aoa_elevation_deg_q7: AoA Elevation, coded as Q7
	 */
	int16_t aoa_elevation_deg_q7;
};

/**
 * typedef pctt_helper_notification_cb_t - Notification callback type.
 * @user_data: User data pointer given to pctt_helper_open.
 * @cmd_id: The type of cmd corresponding to the results.
 * @results: pctt results.
 * Return: nothing
 */
typedef void (*pctt_helper_notification_cb_t)(
	void *user_data, uint8_t cmd_id,
	const struct pctt_result_data *results);

/*
 * struct pctt_context - Contextual data for the different pctt calls
 *
 * NOTE: content of this struct is considered private, do not use directly.
 */

struct pctt_context {
	int region_id;
	struct uwbmac_channel channel;
	pctt_helper_notification_cb_t notification_cb;
	char *scheduler;
	void *user_data;
};

/**
 * pctt_helper_open() - Initialize the internal resources of the helper.
 * @context: Context to initialize.
 * @uwbmac_context: UWB MAC context.
 * @notification_cb: Callback to call when a notification is available.
 * @scheduler: In which scheduler the region will be
 * @region_id: Which id the region will have in the scheduler
 * @user_data: User data pointer to give back in callbacks.
 *
 * NOTE: This function must be called first. @pctt_helper_close must be called
 * at the end of the application to ensure resources are freed.
 *
 * Return: 0 or error.
 */
int pctt_helper_open(struct pctt_context *context,
		     struct uwbmac_context *uwbmac_context,
		     pctt_helper_notification_cb_t notification_cb,
		     const char *scheduler, int region_id, void *user_data);

/**
 * pctt_helper_set_scheduler() - Set the scheduler and the region.
 * @context: Context of this helper.
 *
 * NOTE: This function must be called while the UWB MAC is stopped.
 *
 * Return: 0 or error.
 */
int pctt_helper_set_scheduler(struct pctt_context *context);

/**
 * pctt_helper_close() - Free all internal resources of the helper.
 * @context: Context to release.
 */
void pctt_helper_close(struct pctt_context *context);

/**
 * pctt_helper_session_init() - Initialize a pctt session.
 * @context: Context of this helper.
 *
 * This function must be called first to create and initialize the pctt session.
 *
 * Return: 0 or error.
 */
int pctt_helper_session_init(struct pctt_context *context);

/**
 * pctt_helper_session_start() - Start a pctt session.
 * @context: Context of this helper.
 * @cmd_id: The cmd being executed.
 * @params: specific pctt_parameters of the test.
 *
 * This function must be called after pctt session was initialized.
 *
 * Return: 0 or error.
 */
int pctt_helper_session_start(struct pctt_context *context, uint8_t cmd_id,
			      const struct pctt_parameters *params);
/**
 * pctt_helper_set_test_payload() - Set payload.
 * @context: Context of this helper.
 * @test_payload: data payload for the test.
 *
 * This function must be called after pctt session was initialized.
 *
 * Return: 0 or error.
 */
int pctt_helper_set_test_payload(struct pctt_context *context,
				 const struct pctt_test_payload *test_payload);
/**
 * pctt_helper_session_deinit() - Deinitialize a pctt session.
 * @context: Context of this helper.
 *
 * This function is called to free all memory allocated by the session.
 * This function must be called when the session is stopped.
 *
 * Return: 0 or error.
 */
int pctt_helper_session_deinit(struct pctt_context *context);

/**
 * pctt_helper_session_get_state() - Request to get a state.
 * @context: Context of this helper.
 * @state: The result state of the session.
 *
 * Return: 0 or error.
 */
int pctt_helper_session_get_state(struct pctt_context *context, int *state);

/**
 * pctt_helper_session_get_params() - Request pctt parameters.
 * @context: Context of this helper.
 * @params: the output parameters.
 *
 * Return: 0 or error.
 */
int pctt_helper_session_get_params(struct pctt_context *context,
				   struct pctt_session_parameters *params);

/*  TODO(Guillaume): change that to individual setters UWB-4419 */
/*  TODO(Guillaume): this temporary solution is to fix UWB-4471 */
/**
 * pctt_helper_session_set_params() - Set pctt session parameters.
 * @context: Context of this helper.
 * @params: the session parameters to set.
 *
 * Return: 0 or error.
 */
int pctt_helper_session_set_params(struct pctt_context *context,
				   struct pctt_session_parameters *params);

/*  TODO(Guillaume): change that when this test is put in region */
/**
 * pctt_helper_tx_cw() - Start/stop continuous wave test.
 * @context: Context of this helper.
 * @params: Specific pctt_parameters of the test, used to get TX antenna to transit from
 * @start: Whether to start or not.
 *
 * Return: 0 or error.
 */
int pctt_helper_tx_cw(struct pctt_context *context,
		      struct pctt_session_parameters *params, bool start);

#endif /* UWBMAC_PCTT_HELPER_H */
