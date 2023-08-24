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
 *
 * 802.15.4 mac common part sublayer, pctt ranging region.
 *
 */

#ifndef NET_PCTT_REGION_PARAMS_H
#define NET_PCTT_REGION_PARAMS_H

#include <linux/types.h>

#define PCTT_VUPPER64_SIZE 8
#define PCTT_KEY_SIZE_MAX 32
#define PCTT_KEY_SIZE_MIN 16
#define PCTT_CONTROLEES_MAX 16
#define PCTT_RX_ANTENNA_PAIR_INVALID 0xff
/*
 * In BPRF, frame is at most 127
 * 127 - (MHR + HIE + HT + PIE_Header + V_OUI + MIC + CRC)
 */
#define PCTT_DATA_PAYLOAD_SIZE_MAX 84

/**
 * enum pctt_device_role - **[NOT IMPLEMENTED]**  Role played by a device.
 * @PCTT_DEVICE_ROLE_RESPONDER: The device acts as a responder.
 * @PCTT_DEVICE_ROLE_INITIATOR: The device acts as an initiator.
 *
 * Current implementation does not support decorrelation between the
 * device's role and the device's type. The controller is always
 * the initiator and the controlee is always the responder.
 *
 * This enum is not used in the current implementation.
 */
enum pctt_device_role {
	PCTT_DEVICE_ROLE_RESPONDER,
	PCTT_DEVICE_ROLE_INITIATOR,
};

/**
 * enum pctt_rframe_config - Rframe configuration used to transmit/receive
 * ranging messages.
 * @PCTT_RFRAME_CONFIG_SP0: Use SP0 mode.
 * @PCTT_RFRAME_CONFIG_SP1: Use SP1 mode.
 * @PCTT_RFRAME_CONFIG_SP2: RFU
 * @PCTT_RFRAME_CONFIG_SP3: Use SP3 mode.
 */
enum pctt_rframe_config {
	PCTT_RFRAME_CONFIG_SP0,
	PCTT_RFRAME_CONFIG_SP1,
	PCTT_RFRAME_CONFIG_SP2,
	PCTT_RFRAME_CONFIG_SP3,
};

/**
 * enum pctt_prf_mode - Pulse Repetition Frequency mode.
 * @PCTT_PRF_MODE_BPRF: Base Pulse Repetition Frequency.
 * @PCTT_PRF_MODE_HPRF: Higher Pulse Repetition Frequency.
 * @PCTT_PRF_MODE_HPRF_HIGH_RATE: Higher Pulse Repetition Frequency allowing
 * higher data rates (27M2 and 31M2).
 *
 * This enum is not used in the current implementation.
 */
enum pctt_prf_mode {
	PCTT_PRF_MODE_BPRF,
	PCTT_PRF_MODE_HPRF,
	PCTT_PRF_MODE_HPRF_HIGH_RATE,
};

/**
 * enum pctt_preamble_duration - Duration of preamble in symbols.
 * @PCTT_PREAMBLE_DURATION_32: 32 symbols duration.
 * @PCTT_PREAMBLE_DURATION_64: 64 symbols duration.
 */
enum pctt_preamble_duration {
	PCTT_PREAMBLE_DURATION_32,
	PCTT_PREAMBLE_DURATION_64,
};

/**
 * enum pctt_sfd_id - Start-of-frame delimiter.
 * @PCTT_SFD_ID_0: Delimiter is [0 +1 0 –1 +1 0 0 –1]
 * @PCTT_SFD_ID_1: Delimiter is [ –1 –1 +1 –1 ]
 * @PCTT_SFD_ID_2: Delimiter is [ –1 –1 –1 +1 –1 –1 +1 –1 ]
 * @PCTT_SFD_ID_3: Delimiter is
 * [ –1 –1 –1 –1 –1 +1 +1 –1 –1 +1 –1 +1 –1 –1 +1 –1 ]
 * @PCTT_SFD_ID_4: Delimiter is
 * [ –1 –1 –1 –1 –1 –1 –1 +1 –1 –1 +1 –1 –1 +1 –1 +1 –1 +1
 * –1 –1 –1 +1 +1 –1 –1 –1 +1 –1 +1 +1 –1 –1 ]
 */
enum pctt_sfd_id {
	PCTT_SFD_ID_0,
	PCTT_SFD_ID_1,
	PCTT_SFD_ID_2,
	PCTT_SFD_ID_3,
	PCTT_SFD_ID_4,
};

/**
 * enum pctt_number_of_sts_segments - Number of STS segments.
 * @PCTT_NUMBER_OF_STS_SEGMENTS_NONE: No STS Segment (Rframe config SP0).
 * @PCTT_NUMBER_OF_STS_SEGMENTS_1_SEGMENT: 1 STS Segment.
 * @PCTT_NUMBER_OF_STS_SEGMENTS_2_SEGMENTS: 2 STS Segments.
 * @PCTT_NUMBER_OF_STS_SEGMENTS_3_SEGMENTS: 3 STS Segments.
 * @PCTT_NUMBER_OF_STS_SEGMENTS_4_SEGMENTS: 4 STS Segments.
 */
enum pctt_number_of_sts_segments {
	PCTT_NUMBER_OF_STS_SEGMENTS_NONE,
	PCTT_NUMBER_OF_STS_SEGMENTS_1_SEGMENT,
	PCTT_NUMBER_OF_STS_SEGMENTS_2_SEGMENTS,
	PCTT_NUMBER_OF_STS_SEGMENTS_3_SEGMENTS,
	PCTT_NUMBER_OF_STS_SEGMENTS_4_SEGMENTS,
};

/**
 * enum pctt_psdu_data_rate - Data rate used to exchange PSDUs.
 * @PCTT_PSDU_DATA_RATE_6M81: 6.8Mb/s rate.
 * @PCTT_PSDU_DATA_RATE_7M80: 7.8Mb/s rate.
 * @PCTT_PSDU_DATA_RATE_27M2: 27.2Mb/s rate.
 * @PCTT_PSDU_DATA_RATE_31M2: 31.2Mb/s rate.
 */
enum pctt_psdu_data_rate {
	PCTT_PSDU_DATA_RATE_6M81,
	PCTT_PSDU_DATA_RATE_7M80,
	PCTT_PSDU_DATA_RATE_27M2,
	PCTT_PSDU_DATA_RATE_31M2,
};

/**
 * enum pctt_phr_data_rate - Data rate used to exchange PHR.
 * @PCTT_PHR_DATA_RATE_850K: 850kb/s rate.
 * @PCTT_PHR_DATA_RATE_6M81: 6.8Mb/s rate.
 *
 * This enum is not used in the current implementation.
 */
enum pctt_phr_data_rate {
	PCTT_PHR_DATA_RATE_850K,
	PCTT_PHR_DATA_RATE_6M81,
};

enum pctt_mac_fcs_type {
	PCTT_MAC_FCS_TYPE_CRC_16,
	PCTT_MAC_FCS_TYPE_CRC_32,
};

/**
 * enum pctt_status_ranging - Ranging status: success or failure reason.
 * @PCTT_STATUS_RANGING_INTERNAL_ERROR: Implementation specific error.
 * @PCTT_STATUS_RANGING_SUCCESS: Ranging info are valid.
 * @PCTT_STATUS_RANGING_TX_FAILED: Failed to transmit UWB packet.
 * @PCTT_STATUS_RANGING_RX_TIMEOUT: No UWB packet detected by the receiver.
 * @PCTT_STATUS_RANGING_RX_PHY_DEC_FAILED: UWB packet channel decoding error.
 * @PCTT_STATUS_RANGING_RX_PHY_TOA_FAILED: Failed to detect time of arrival of
 * the UWB packet from CIR samples.
 * @PCTT_STATUS_RANGING_RX_PHY_STS_FAILED: UWB packet STS segment mismatch.
 * @PCTT_STATUS_RANGING_RX_MAC_DEC_FAILED: MAC CRC or syntax error.
 * @PCTT_STATUS_RANGING_RX_MAC_IE_DEC_FAILED: IE syntax error.
 * @PCTT_STATUS_RANGING_RX_MAC_IE_MISSING: Expected IE missing in the packet.
 */
enum pctt_status_ranging {
	PCTT_STATUS_RANGING_INTERNAL_ERROR = -1,
	PCTT_STATUS_RANGING_SUCCESS = 0,
	PCTT_STATUS_RANGING_TX_FAILED = 1,
	PCTT_STATUS_RANGING_RX_TIMEOUT = 2,
	PCTT_STATUS_RANGING_RX_PHY_DEC_FAILED = 3,
	PCTT_STATUS_RANGING_RX_PHY_TOA_FAILED = 4,
	PCTT_STATUS_RANGING_RX_PHY_STS_FAILED = 5,
	PCTT_STATUS_RANGING_RX_MAC_DEC_FAILED = 6,
	PCTT_STATUS_RANGING_RX_MAC_IE_DEC_FAILED = 7,
	PCTT_STATUS_RANGING_RX_MAC_IE_MISSING = 8,
};

/**
 * enum pctt_session_state - Session state.
 * @PCTT_SESSION_STATE_INIT: Initial state, session is not ready yet.
 * @PCTT_SESSION_STATE_DEINIT: Session does not exist.
 * @PCTT_SESSION_STATE_ACTIVE: Session is currently active.
 * @PCTT_SESSION_STATE_IDLE: Session is ready to start, but not currently
 * active.
 */
enum pctt_session_state {
	PCTT_SESSION_STATE_INIT,
	PCTT_SESSION_STATE_DEINIT,
	PCTT_SESSION_STATE_ACTIVE,
	PCTT_SESSION_STATE_IDLE,
};

/**
 * enum pctt_sts_length - Number of symbols in a STS segment.
 * @PCTT_STS_LENGTH_32: The STS length is 32 symbols.
 * @PCTT_STS_LENGTH_64: The STS length is 64 symbols.
 * @PCTT_STS_LENGTH_128: The STS length is 128 symbols.
 */
enum pctt_sts_length {
	PCTT_STS_LENGTH_32 = 0,
	PCTT_STS_LENGTH_64 = 1,
	PCTT_STS_LENGTH_128 = 2,
};

#endif /* NET_PCTT_REGION_PARAMS_H */
