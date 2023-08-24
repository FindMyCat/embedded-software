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

#ifndef FIRA_HELPER_H
#define FIRA_HELPER_H

#include <stdint.h>
#include <stdbool.h>

#include "net/fira_region_params.h"
#include "net/fira_region_nl.h"

#include "uwbmac/uwbmac.h"
#include "uwbmac/uwbmac_writer.h"

struct uwbmac_channel;

/*
 * struct fira_capabilities - FiRa capabilities.
 *
 * This structure contains the FiRa capabilities as reported by the MAC.
 */
struct fira_capabilities {
#define F(name) bool has_##name

#define P(name, type) \
	F(name);      \
	type name

	P(fira_phy_version_range, uint32_t);
	P(fira_mac_version_range, uint32_t);
	P(device_class, uint8_t);
	F(device_type_controlee_responder);
	F(device_type_controlee_initiator);
	F(device_type_controller_responder);
	F(device_type_controller_initiator);
	F(multi_node_mode_unicast);
	F(multi_node_mode_one_to_many);
	F(ranging_round_usage_ss_twr);
	F(ranging_round_usage_ds_twr);
	P(number_of_controlees_max, uint32_t);
	/* Behaviour. */
	F(round_hopping);
	F(block_striding);
	F(embedded_mode_non_deferred);
	/* Radio. */
	P(channel_number, uint16_t);
	F(rframe_config_sp0);
	F(rframe_config_sp1);
	F(rframe_config_sp3);
	F(prf_mode_bprf);
	F(prf_mode_hprf);
	F(preamble_duration_64);
	F(preamble_duration_32);
	F(sfd_id_0);
	F(sfd_id_1);
	F(sfd_id_2);
	F(sfd_id_3);
	F(sfd_id_4);
	F(number_of_sts_segments_0);
	F(number_of_sts_segments_1);
	F(number_of_sts_segments_2);
	F(number_of_sts_segments_3);
	F(number_of_sts_segments_4);
	F(psdu_data_rate_6m81);
	F(psdu_data_rate_7m80);
	F(psdu_data_rate_27m2);
	F(psdu_data_rate_31m2);
	F(bprf_phr_data_rate_850k);
	F(bprf_phr_data_rate_6m81);
	F(mac_fcs_type_crc32);
	F(tx_adaptive_payload_power);
	/* Antenna. */
	P(rx_antenna_pairs, uint32_t);
	P(tx_antennas, uint32_t);
	/* STS and crypto capabilities. */
	F(sts_config_static);
	F(sts_config_dynamic);
	F(sts_config_dynamic_individual);
	F(sts_config_provisioned);
	F(sts_config_provisioned_individual);
	/* Report. */
	F(aoa_azimuth);
	F(aoa_azimuth_full);
	F(aoa_elevation);
	F(aoa_fom);

#undef P
#undef F
};

/**
 * struct measurement_sequence_step - Fira measurement sequence step.
 *
 * This structure contains a step of the measurement sequence executed by the
 * region. It can be used to configure the region when inserted in a
 * measurement sequence.
 */
struct measurement_sequence_step {
	/**
	 * @type:
	 * Kind of ranging measurement done in this step.
	 */
	enum fira_measurement_type type;
	/**
	 * @n_measurements:
	 * Number ranging round done in this step.
	 */
	uint8_t n_measurements;
	/**
	 * @rx_ant_set_nonranging:
	 * Antenna set ID, used to receive non-RFRAMEs.
	 */
	uint8_t rx_ant_set_nonranging;
	/**
	 * @rx_ant_sets_ranging:
	 * Array of sets used to receive RFRAMEs.
	 */
	uint8_t rx_ant_sets_ranging[2];
	/**
	 * @tx_ant_set_nonranging:
	 * Antenna set ID, used to tansmit non-RFRAMEs.
	 */
	uint8_t tx_ant_set_nonranging;
	/**
	 * @tx_ant_set_ranging:
	 * Array of sets used to tansmit RFRAMEs.
	 */
	uint8_t tx_ant_set_ranging;
};

/**
 * struct measurement_sequence - Fira measurement sequence.
 *
 * This structure contains the measurement sequence executed by the
 * region.
 */
struct measurement_sequence {
	/**
	 * @n_steps:
	 * Number of steps in the schedule.
	 */
	size_t n_steps;
	/**
	 * @steps:
	 * Steps of the schedule.
	 */
	struct measurement_sequence_step
		steps[FIRA_MEASUREMENT_SEQUENCE_STEP_MAX];
};

/**
 * struct session_parameters - Fira session parameters.
 *
 * This structure contains the session parameters sent to the
 * Fira region.
 * Current implementation does not use all the parameters defined
 * below.
 */
struct session_parameters {
	/**
	 * @device_type:
	 * Type of the device.
	 *
	 * See &enum fira_device_type.
	 */
	uint8_t device_type;
	/**
	 * @device_role:
	 * **[NOT IMPLEMENTED]** Role played by the device.
	 *
	 * This parameter is not used in the current implementation.
	 *
	 * Current implementation does not support decorrelation between the
	 * device's role and the device's type. The controller is always
	 * the initiator and the controlee is always the responder.
	 *
	 * See &enum fira_device_role.
	 */
	uint8_t device_role;
	/**
	 * @ranging_round_usage:
	 * The ranging mode used during a round.
	 *
	 * See &enum fira_ranging_round_usage.
	 */
	uint8_t ranging_round_usage;
	/**
	 * @sts_config: it configures how system shall generate the STS.
	 *
	 * Possible values:
	 *
	 * - 0x00: Static STS (default).
	 * - 0x01: Dynamic STS.
	 * - 0x02: RFU (Dynamic STS - Individual Key).
	 * - 0x03: Provisioned STS.
	 * - 0x04: RFU (Provisioned STS - Individual Key).
	 *
	 * See &enum fira_sts_config.
	 */
	uint8_t sts_config;
	/**
	 * @multi_node_mode:
	 * The multi-node mode used during a round.
	 *
	 * Current implementation does not support
	 * FIRA_MULTI_NODE_MODE_MANY_TO_MANY mode.
	 *
	 * See enum &fira_multi_node_mode.
	 */
	uint8_t multi_node_mode;
	/**
	 * @short_addr: Short address of the local device.
	 */
	uint16_t short_addr;
	/**
	 * @destination_short_address: Short address of the destination controller.
	 */
	uint16_t destination_short_address;
	/**
	 * @initiation_time_ms: Initiation time of the session in milliseconds.
	 */
	uint32_t initiation_time_ms;
	/**
	 * @slot_duration_rstu: Duration of a slot in RSTU (1200RSTU=1ms).
	 */
	uint32_t slot_duration_rstu;
	/**
	 * @round_duration_slots: Number of slots per ranging round.
	 */
	uint32_t round_duration_slots;
	/**
	 * @block_duration_ms: Block size in unit of 1200 RSTU (same as ms).
	 */
	uint32_t block_duration_ms;
	/**
	 * @block_stride_length: Number of blocks to stride.
	 */
	uint32_t block_stride_length;
	/**
	 * @round_hopping: Enable FiRa round hopping.
	 */
	bool round_hopping;
	/**
	 * @priority: Priority of the session.
	 */
	uint8_t priority;
	/**
	 * @result_report_phase:
	 * Enable result report phase.
	 *
	 * Current implementation does not support enabling RRRM on
	 * controller/initiator, it works only on controlee/responder.
	 */
	bool result_report_phase;
	/**
	 * @embedded_mode: Message embedding behaviour.
	 *
	 * Possible values:
	 *
	 * - 0: MODE_DEFERRED - Ranging messages do not embed control messages. Additional messages are required.
	 * - 1: MODE_NON_DEFERRED - Ranging messages embed control messages
	 */
	uint8_t embedded_mode;
	/**
	 * @max_number_of_measurements: Max number of measurements
	 */
	uint16_t max_number_of_measurements;
	/**
	 * @max_rr_retry:
	 * Number of failed ranging round attempts before stopping the session.
	 *
	 * The value zero disable the feature.
	 */
	uint32_t max_rr_retry;
	/**
	 * @channel_number: UWB channel for this session.
	 */
	uint8_t channel_number;
	/**
	 * @preamble_code_index:
	 * UWB preamble code index.
	 *
	 * Possible values:
	 *
	 * - 9-24: BPRF
	 * - 25-32: HPRF
	 */
	uint8_t preamble_code_index;
	/**
	 * @rframe_config:
	 * The configuration of the frame.
	 *
	 * Current implementation only supports deferred mode.
	 *
	 * see enum &fira_rframe_config.
	 */
	uint8_t rframe_config;
	/**
	 * @preamble_duration:
	 *
	 * Possible values:
	 *
	 * - 0x00: 32 symbols
	 * - 0x01: 64 symbols (default)
	 *
	 * See &enum fira_preambule_duration.
	 */
	uint8_t preamble_duration;
	/**
	 * @sfd_id:
	 *
	 * Possible values:
	 *
	 * - 0 or 2 in BPRF
	 * - 1-4 in HPRF
	 *
	 * See &enum fira_sfd_id.
	 */
	uint8_t sfd_id;
	/**
	 * @psdu_data_rate:
	 *
	 * Possible values:
	 *
	 * - 0: 6.81Mbps (default)
	 * - 1: 7.80 Mbps
	 * - 2: 27.2 Mbps
	 * - 3: 31.2 Mbps
	 *
	 * See &enum fira_psdu_data_rate.
	 */
	uint8_t psdu_data_rate;
	/**
	 * @phr_data_rate:
	 *
	 * Possible values:
	 *
	 * - 0: 850 kbit/s
	 * - 1: 6.81 Mbit/s.
	 *
	 * See &enum fira_phr_data_rate.
	 */
	uint8_t phr_data_rate;
	/**
	 * @vupper64: vUpper64 used during Static STS ranging.
	 */
	uint8_t vupper64[FIRA_VUPPER64_SIZE];
	/**
	 * @session_key: session key used during Provisioned STS ranging.
	 */
	uint8_t session_key[FIRA_KEY_SIZE_MIN];
	/**
	 * @key_rotation: Enable/disable key rotation feature during
	 * Dynamic or Provisioned STS ranging.
	 *
	 * Possible values:
	 *
	 * - false: No key rotation.
	 * - true: Key rotation enabled and period set by key_rotation_rate.
	 */
	uint8_t key_rotation;
	/**
	 * @key_rotation_rate: defines n, with 2^n being the rotation rate of some
	 * keys used during Dynamic or Provisioned STS Ranging,
	 * n shall be in the range of 0<=n<=15.
	 */
	uint8_t key_rotation_rate;
	/**
	 * @aoa_result_req: Activate local AoA report
	 *
	 * Possible values:
	 *
	 * - false: No local AoA report
	 * - true: -90 to +90
	 */
	uint8_t aoa_result_req;
	/**
	 * @report_rssi: Activate rssi report
	 *
	 * Possible values:
	 *
	 * - 0: no rssi report
	 * - 1: activate rssi report
	 */
	uint8_t report_rssi;
	/**
	 * @report_tof: Activate ToF report in RRRM.
	 *
	 * Possible values:
	 *
	 * - false: No ToF report in RRRM
	 * - true: ToF Report in RRRM
	 */
	uint8_t report_tof;
	/**
	 * @report_aoa_azimuth: Activate AoA azimuth report in RRRM.
	 *
	 * Possible values:
	 *
	 * - false: No AoA azimuth report in RRRM
	 * - true:  AoA azimuth Report in RRRM
	 */
	uint8_t report_aoa_azimuth;
	/**
	 * @report_aoa_elevation: Activate AoA elevation report in RRRM.
	 *
	 * Possible values:
	 *
	 * - false: No AoA elevation report in RRRM
	 * - true:  AoA elevation Report in RRRM
	 */
	uint8_t report_aoa_elevation;
	/**
	 * @report_aoa_fom: No Report AoA FOM in result message (0)
	 *
	 * Possible values:
	 *
	 * - false: No AoA FOM report in RRRM
	 * - true:  AoA FOM Report in RRRM
	 */
	uint8_t report_aoa_fom;
	/**
	 * @data_vendor_oui: Vendor OUI used to send and receive data using the
	 * ranging frames.
	 */
	uint32_t data_vendor_oui;
	/**
	 * @mac_fcs_type:
	 * **[NOT IMPLEMENTED]** The length of the Frame Check Sequence in the
	 * session.
	 *
	 * Possible values:
	 *
	 * - 0x00: CRC 16 (default)
	 * - 0x01: CRC 32
	 * - Values 0x02 to 0xFF: RFU
	 *
	 * This parameter is not used in the current implementation.
	 *
	 * See &enum fira_mac_fcs_type.
	*/
	uint8_t mac_fcs_type;
	/**
	 * @tx_adaptative_payload_power:
	 * **[NOT IMPLEMENTED]** Activate TX adaptive power.
	 *
	 * Possible values:
	 *
	 * - 0x00: Disabled
	 * - 0x01: Enabled
	 * - Values 0x02 to 0xFF: RFU
	 *
	 * This parameter is not used in the current implementation.
	 */
	uint8_t tx_adaptative_payload_power;
	/**
	 * @prf_mode:
	 *
	 * Possible values:
	 *
	 * - 0x00: 62.4 MHz PRF. BPRF mode (default)
	 * - 0x01: 124.8 MHz PRF. HPRF mode.
	 * - 0x02: 249.6 MHz PRF. HPRF mode with data rate 27.2 and 31.2 Mbps
	 *
	 * See &enum fira_prf_mode.
	 */
	uint8_t prf_mode;
	/**
	 * @cap_size_min: Contention access period minimum value.
	 *
	 * Default: 5
	 */
	uint8_t cap_size_min;
	/**
	 * @cap_size_max: Contention access period maximum value.
	 *
	 * Default: round_duration_slots - 1
	 */
	uint8_t cap_size_max;
	/**
	 * @number_of_sts_segments:
	 * **[NOT IMPLEMENTED]** Number of STS segments.
	 *
	 * Possible values:
	 *
	 * - 0x01: 1 STS Segment (default)
	 * - 0x02: 2 STS Segments (HPRF only)
	 * - 0x03: 3 STS Segments (HPRF only)
	 * - 0x04: 4 STS Segments (HPRF only)
	 * - Values 0x05 to 0xFF: RFU
	 *
	 * This parameter is not used in the current implementation.
	 */
	uint8_t number_of_sts_segments;
	/**
	 * @meas_seq: sequence of measurement sequence steps,
	 * configures the Antenna Flexibility features.
	 */
	struct measurement_sequence meas_seq;
	/**
	 * @enable_diagnostics: activate the diagnostics for each round.
	 */
	bool enable_diagnostics;
	/**
	 * @diags_frame_reports_fields: select the fields to activate in the frame
	 * reports stored in the diags. If the ENABLE_DIAGNOSTICS is not true this
	 * parameters does not activate the diags itself.
	 */
	uint32_t diags_frame_reports_fields;
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
 * struct data_parameters - Data parameters.
 */
struct data_parameters {
	/**
	 * @data_payload: Data payload to send.
	 */
	uint8_t data_payload[FIRA_DATA_PAYLOAD_SIZE_MAX];
	/**
	 * @data_payload_len: Length of data to send.
	 */
	int data_payload_len;
};

/**
 * struct controlee_parameters - Controlee parameters.
 */
struct controlee_parameters {
	/**
	 * @address: Controlee short address.
	 */
	uint16_t address;
};

/**
 * struct controlees_parameters - Controlees list parameters.
 */
struct controlees_parameters {
	/**
	 * @controlees: List of controlees.
	 */
	struct controlee_parameters controlees[FIRA_CONTROLEES_MAX];
	/**
	 * @n_controlees: Number of controlees in the list.
	 */
	int n_controlees;
};

/**
 * enum aoa_measurements_index - AOA measurements.
 * @FIRA_HELPER_AOA_AZIMUTH: Retrieve AOA azimuth.
 * @FIRA_HELPER_AOA: Retrieve AOA (same as azimuth).
 * @FIRA_HELPER_AOA_ELEVATION: Retrieve AOA elevation.
 * @FIRA_HELPER_AOA_NB: Enum members number.
 */
enum aoa_measurements_index {
	FIRA_HELPER_AOA_AZIMUTH,
	FIRA_HELPER_AOA = FIRA_HELPER_AOA_AZIMUTH,
	FIRA_HELPER_AOA_ELEVATION,
	FIRA_HELPER_AOA_NB
};

/**
 * struct aoa_measurements - Fira Angle of Arrival measurements.
 *
 * Contains the different results of the AOA measurements.
 */
struct aoa_measurements {
	/**
	 * @rx_antenna_pair: Antenna pair index.
	 */
	uint8_t rx_antenna_pair;
	/**
	 * @aoa_fom: Estimation of local AoA reliability.
	 */
	uint8_t aoa_fom;
	/**
	 * @aoa_2pi: Estimation of reception angle.
	 */
	int16_t aoa_2pi;
	/**
	 * @pdoa_2pi: Estimation of reception phase difference.
	 */
	int16_t pdoa_2pi;
};

struct cir_window_sample {
	struct cir_window_sample *next;
	const uint8_t *data;
};

struct cir {
	struct cir *next;
	uint16_t fpathindex;
	int16_t fpathsnr;
	uint16_t fpathns;
	uint16_t ppathindex;
	int16_t ppathsnr;
	uint16_t ppathns;
	uint16_t fpath_sample_offset;
	uint16_t nb_window_sample;
	uint8_t sample_size;
	struct cir_window_sample *window_samples;
};

struct aoa_measurement {
	struct aoa_measurement *next;
	uint16_t tdoa;
	uint16_t pdoa;
	uint16_t aoa;
	uint8_t fom;
	uint8_t type;
};

#define MAX_RSSI 2
struct frame_report {
	struct frame_report *next;
	uint8_t msg_id;
	uint8_t action;
	uint8_t antenna_set;
	uint8_t nb_rssi;
	uint8_t rssis[MAX_RSSI];
	uint8_t nb_aoa;
	struct aoa_measurement *aoas;
	uint8_t nb_cir;
	struct cir *cirs;
};

struct diagnostic_info {
	uint8_t nb_reports;
	struct frame_report *reports;
};

/**
 * struct ranging_measurements - Fira ranging measurements.
 */
struct ranging_measurements {
	/**
	 * @short_addr: Address of the participating device.
	 */
	uint16_t short_addr;
	/**
	 * @status: Zero if ok, or error reason.
	 */
	uint8_t status;
	/**
	 * @slot_index: in case of error, slot index where the error was detected.
	 */
	uint8_t slot_index;
	/**
	 * @stopped: Ranging was stopped as requested [controller only].
	 */
	bool stopped;
	/**
	 * @nlos:
	 * **[NOT IMPLEMENTED]** Not in line of sight indicator.
	 *
	 * The current implementation does not compute nLOS.
	 */
	bool nlos;
	/**
	 * @los:
	 * **[NOT IMPLEMENTED]** Line of sight indicator.
	 *
	 * The current implementation does not compute LOS.
	 */
	bool los;
	/**
	 * @distance_mm: Distance in mm.
	 */
	int32_t distance_mm;
	/**
	 * @remote_aoa_azimuth_2pi: Estimation of reception angle in the azimuth
	 * of the participating device.
	 */
	int16_t remote_aoa_azimuth_2pi;
	/**
	 * @remote_aoa_elevation_pi: Estimation of reception angle in the
	 * elevation of the participating device.
	 */
	int16_t remote_aoa_elevation_pi;
	/**
	 * @remote_aoa_azimuth_fom: Estimation of azimuth reliability of the
	 * participating device.
	 */
	uint8_t remote_aoa_azimuth_fom;
	/**
	 * @remote_aoa_elevation_fom: Estimation of elevation of the
	 * participating device.
	 */
	uint8_t remote_aoa_elevation_fom;
	/**
	 * @local_aoa_measurements: Table of estimations of local measurements.
	 */
	struct aoa_measurements local_aoa_measurements[FIRA_HELPER_AOA_NB];
	/**
	 * @sp1_data: SP1 received data payload
	 */
	uint8_t sp1_data[FIRA_DATA_PAYLOAD_SIZE_MAX];
	/**
	 * @sp1_data_len: Length of received data.
	 */
	int sp1_data_len;
	/**
	 * @rssi: computed rssi
	 */
	uint8_t rssi;
	/**
	 * @payload_seq_sent: Data sequence number.
	 */
	uint32_t payload_seq_sent;
};

/**
 * struct ranging_results - Fira ranging results.
 */
struct ranging_results {
	/**
	 * @stopped_reason:
	 * 0x00: Session was stopped due to stop request (0).
	 *
	 * 0x01 (controlee only): Session was stopped using in band signaling from the controller.
	 *
	 * 0x02 (controller only): Session was stopped due to maximum attempts reached with no response.
	 *
	 * 0xff: Session is running.
	 */
	uint8_t stopped_reason;
	/**
	 * @session_id: Session id of the ranging result.
	 */
	uint32_t session_id;
	/**
	 * @block_index: Current block index.
	 */
	uint32_t block_index;
	/**
	 * @ranging_interval_ms: Current ranging interval in unit of ms.
	 * formula: (block size * (stride + 1))
	 */
	uint32_t ranging_interval_ms;
	/**
	 * @timestamp_ns:
	 * **[NOT IMPLEMENTED]** Timestamp in nanoseconds in
	 * the CLOCK_MONOTONIC time reference.
	 *
	 * The current implementation does not provide any timestamp.
	 */
	uint64_t timestamp_ns;
	/**
	 * @n_measurements:
	 * Number of measurements stored in the measurements
	 * table.
	 */
	int n_measurements;
	/**
	 * @measurements: Ranging measurements information.
	 */
	struct ranging_measurements measurements[FIRA_CONTROLEES_MAX];
	/**
	 * @diagnostic: Debug informations
	 */
	struct diagnostic_info *diagnostic;
};

/**
 * ranging_result_free() - Free diagnostics data attached to ranging results.
 * @ranging_results: Pointer to Fira ranging results.
 *
 * NOTE: ranging results structure itself will no be freed.
 *
 * Return: Nothing.
 */
void ranging_result_free(struct ranging_results *ranging_results);

/**
 * typedef fira_helper_notification_cb_t - Notification callback type.
 * @results: Fira ranging results.
 * @user_data: User data pointer given to fira_helper_open.
 *
 * Return: Nothing.
 */
typedef void (*fira_helper_notification_cb_t)(
	const struct ranging_results *results, void *user_data);

/*
 * struct fira_context - Fira context
 *
 * NOTE: content of this struct is considered private, do not use directly.
 */

struct fira_context {
	/* private: internal use only */
	struct uwbmac_channel channel;
	fira_helper_notification_cb_t notification_cb;
	uint8_t *call_buffer;
	char *scheduler;
	int region_id;
	void *user_data;
};

/**
 * fira_helper_open() - Initialize the internal resources of the helper.
 * @context: Fira context to initialize.
 * @uwbmac_context: UWB MAC context.
 * @notification_cb: Callback to call when a notification is available.
 * @scheduler: In which scheduler the region will be
 * @region_id: Which id the region will have in the scheduler
 * @user_data: User data pointer to give back in callback.
 *
 * NOTE: This function must be called first. @fira_helper_close must be called
 * at the end of the application to ensure resources are freed.
 * The channel will be managed by the helper, this means you should neither use
 * uwbmac_channel_create nor uwbmac_channel_release.
 *
 * Return: 0 or error.
 */
int fira_helper_open(struct fira_context *context,
		     struct uwbmac_context *uwbmac_context,
		     fira_helper_notification_cb_t notification_cb,
		     const char *scheduler, int region_id, void *user_data);

/**
 * fira_helper_close() - Free all internal resources of the helper.
 * @context: Fira context to free.
 */
void fira_helper_close(struct fira_context *context);

/**
 * fira_helper_set_scheduler() - Set the scheduler and the region of fira.
 * @context: Fira context.
 *
 * NOTE: This function must be called while the UWB MAC is stopped.
 *
 * Return: 0 or error.
 */
int fira_helper_set_scheduler(struct fira_context *context);

/**
 * fira_helper_get_capabilities() - Get the FiRa region capabilities.
 * @context: Fira context.
 * @capabilites: Fira capabilites.
 *
 * Return: 0 or error.
 */
int fira_helper_get_capabilities(struct fira_context *context,
				 struct fira_capabilities *capabilites);

/**
 * fira_helper_init_session() - Initialize a fira session.
 * @context: Fira context.
 * @session_id: Session identifier.
 *
 * This function must be called first to create and initialize the fira session.
 *
 * Return: 0 or error.
 */
int fira_helper_init_session(struct fira_context *context, uint32_t session_id);

/**
 * fira_helper_start_session() - Start a fira session.
 * @context: Fira context.
 * @session_id: Session identifier.
 *
 * This function must be called after fira session was initialized.
 *
 * Return: 0 or error.
 */
int fira_helper_start_session(struct fira_context *context,
			      uint32_t session_id);

/**
 * fira_helper_stop_session() - Stop a fira session.
 * @context: Fira context.
 * @session_id: Session identifier.
 *
 * This function stop the session ranging.
 *
 * Return: 0 or error.
 */
int fira_helper_stop_session(struct fira_context *context, uint32_t session_id);

/**
 * fira_helper_deinit_session() - Deinitialize a fira session.
 * @context: Fira context.
 * @session_id: Session identifier.
 *
 * This function is called to free all memory allocated by the session.
 * This function must be called when the session is stopped.
 *
 * Return: 0 or error.
 */
int fira_helper_deinit_session(struct fira_context *context,
			       uint32_t session_id);

/**
 * fira_helper_set_session_parameters() - Set session parameters.
 * @context: Fira context.
 * @session_id: Session identifier.
 * @session_params: Session parameters.
 *
 * Return: 0 or error.
 */
int fira_helper_set_session_parameters(
	struct fira_context *context, uint32_t session_id,
	const struct session_parameters *session_params);

/**
 * fira_helper_get_session_parameters() - Get session parameters.
 * @context: Fira context.
 * @session_id: Session identifier.
 * @session_params: Session parameters.
 *
 * Return: 0 or error.
 */
int fira_helper_get_session_parameters(
	struct fira_context *context, uint32_t session_id,
	struct session_parameters *session_params);

/**
 * fira_helper_session_get_count() - Get sessions count.
 * @context: Fira context.
 * @count: Session count.
 *
 * Return: 0 or error.
 */
int fira_helper_session_get_count(struct fira_context *context, int *count);

/**
 * fira_helper_session_get_state() - Get session state.
 * @context: Fira context.
 * @session_id: Session ID.
 * @state: Session state.
 *
 * Return: 0 or error.
 */
int fira_helper_session_get_state(struct fira_context *context,
				  uint32_t session_id, int *state);

/**
 * fira_helper_set_controlees() - Set controlees to a specific session.
 * This API can be used only when session is not active.
 * @context: Fira context.
 * @session_id: Session identifier.
 * @controlees: List of controlees to add.
 *
 * Return: 0 or error.
 */
int fira_helper_set_controlees(struct fira_context *context,
			       uint32_t session_id,
			       const struct controlees_parameters *controlees);

/**
 * fira_helper_add_controlees() - Add controlees to a specific session.
 * @context: Fira context.
 * @session_id: Session identifier.
 * @controlees: List of controlees to add.
 *
 * Return: 0 or error.
 */
int fira_helper_add_controlees(struct fira_context *context,
			       uint32_t session_id,
			       const struct controlees_parameters *controlees);

/**
 * fira_helper_delete_controlees() - Delete controlees from a specific session.
 * @context: Fira context.
 * @session_id: Session identifier.
 * @controlees: List of controlees to delete.
 *
 * Return: 0 or error.
 */
int fira_helper_delete_controlees(
	struct fira_context *context, uint32_t session_id,
	const struct controlees_parameters *controlees);

/**
 * fira_helper_get_controlees() - Get controlees list.
 * @context: Fira context.
 * @session_id: Session identifier.
 * @controlees: List of controlees to write.
 *
 * Return: 0 or error.
 */
int fira_helper_get_controlees(struct fira_context *context,
			       uint32_t session_id,
			       struct controlees_parameters *controlees);

/**
 * fira_helper_send_data() - Send custom parameters.
 * @context: Fira context.
 * @session_id: Session identifier.
 * @data_params: Custom data parameters.
 *
 * Return: 0 or error.
 */
int fira_helper_send_data(struct fira_context *context, uint32_t session_id,
			  const struct data_parameters *data_params);

/**
 * struct session_parameters_builder - Tool to send partial session
 * configuration.
 */
struct session_parameters_builder {
	/**
	 * @msg: msg constructed by the builder.
	 */
	struct uwbmac_msg msg;
	/**
	 * @request: first nested part of msg constructed.
	 */
	struct uwbmac_msg request;
	/**
	 * @session_params: second nested part of msg constructed.
	 */
	struct uwbmac_msg session_params;
};

/**
 * session_parameters_builder_init() - Initialize the UWB MAC message stored in
 * the builder.
 * @context: Fira context.
 * @builder: Builder to initialize.
 * @session_id: Session identifier.
 *
 * Return: 0 or error.
 */
int session_parameters_builder_init(struct fira_context *context,
				    struct session_parameters_builder *builder,
				    uint32_t session_id);

/**
 * session_parameters_builder_finish() - Generate the UWB MAC message to send.
 * @builder: Builder which will build the message.
 *
 * Return: Built message, or NULL on error.
 */
struct uwbmac_msg *
session_parameters_builder_finish(struct session_parameters_builder *builder);

#define BUILDER_SETTER_DECL(parameter_name, type)            \
	int session_parameters_builder_set_##parameter_name( \
		struct session_parameters_builder *builder, type v)

BUILDER_SETTER_DECL(device_type, uint8_t);
BUILDER_SETTER_DECL(ranging_round_usage, uint8_t);
BUILDER_SETTER_DECL(sts_config, uint8_t);
BUILDER_SETTER_DECL(multi_node_mode, uint8_t);
BUILDER_SETTER_DECL(channel_number, uint8_t);
BUILDER_SETTER_DECL(short_addr, uint16_t);
BUILDER_SETTER_DECL(destination_short_address, uint16_t);
BUILDER_SETTER_DECL(slot_duration_rstu, uint32_t);
BUILDER_SETTER_DECL(block_duration_ms, uint32_t);
BUILDER_SETTER_DECL(mac_fcs_type, uint8_t);
BUILDER_SETTER_DECL(aoa_result_req, uint8_t);
BUILDER_SETTER_DECL(report_tof, uint8_t);
BUILDER_SETTER_DECL(report_rssi, uint8_t);
BUILDER_SETTER_DECL(key_rotation, uint8_t);
BUILDER_SETTER_DECL(key_rotation_rate, uint8_t);
BUILDER_SETTER_DECL(report_aoa_azimuth, uint8_t);
BUILDER_SETTER_DECL(report_aoa_elevation, uint8_t);
BUILDER_SETTER_DECL(report_aoa_fom, uint8_t);
BUILDER_SETTER_DECL(device_role, uint8_t);
BUILDER_SETTER_DECL(rframe_config, uint8_t);
BUILDER_SETTER_DECL(preamble_code_index, uint8_t);
BUILDER_SETTER_DECL(sfd_id, uint8_t);
BUILDER_SETTER_DECL(psdu_data_rate, uint8_t);
BUILDER_SETTER_DECL(preamble_duration, uint8_t);
BUILDER_SETTER_DECL(round_duration_slots, uint32_t);
BUILDER_SETTER_DECL(tx_adaptative_payload_power, uint8_t);
BUILDER_SETTER_DECL(prf_mode, uint8_t);
BUILDER_SETTER_DECL(cap_size_max, uint8_t);
BUILDER_SETTER_DECL(cap_size_min, uint8_t);
BUILDER_SETTER_DECL(priority, uint8_t);
BUILDER_SETTER_DECL(number_of_sts_segments, uint8_t);
BUILDER_SETTER_DECL(max_rr_retry, uint32_t);
BUILDER_SETTER_DECL(initiation_time_ms, uint32_t);
BUILDER_SETTER_DECL(round_hopping, bool);
BUILDER_SETTER_DECL(result_report_phase, bool);
BUILDER_SETTER_DECL(embedded_mode, uint8_t);
BUILDER_SETTER_DECL(max_number_of_measurements, uint32_t);
BUILDER_SETTER_DECL(block_stride_length, uint32_t);
BUILDER_SETTER_DECL(data_vendor_oui, uint32_t);
BUILDER_SETTER_DECL(phr_data_rate, uint8_t);
BUILDER_SETTER_DECL(enable_diagnostics, bool);
BUILDER_SETTER_DECL(diags_frame_reports_fields, uint32_t);
BUILDER_SETTER_DECL(sts_length, uint8_t);

#undef BUILDER_SETTER_DECL

int session_parameters_builder_set_vupper64(
	struct session_parameters_builder *builder,
	uint8_t vupper64[FIRA_VUPPER64_SIZE]);

int session_parameters_builder_set_session_key(
	struct session_parameters_builder *builder,
	uint8_t session_key[FIRA_KEY_SIZE_MIN]);

int session_parameters_builder_set_meas_seq(
	struct session_parameters_builder *builder,
	const struct measurement_sequence *meas_seq);

/**
 * fira_helper_set_partial_session_parameters() - Set session parameters defined with builder.
 * @context: Fira context.
 * @builder: Session parameters.
 *
 * Return: 0 or error.
 */
int fira_helper_set_partial_session_parameters(
	struct fira_context *context,
	struct session_parameters_builder *builder);

#endif /* FIRA_HELPER_H */
