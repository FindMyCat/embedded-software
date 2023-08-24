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

#ifndef UCI_SPEC_FIRA_H
#define UCI_SPEC_FIRA_H

/* Version is 1.1. */
#define UCI_FIRA_VERSION_MAJOR 1
#define UCI_FIRA_VERSION_MINOR 1
/* Additional versions used by CORE_GET_DEVICE_INFO message. */
#define UCI_FIRA_MAC_VERSION_MAJOR 1
#define UCI_FIRA_MAC_VERSION_MINOR 1
#define UCI_FIRA_PHY_VERSION_MAJOR 1
#define UCI_FIRA_PHY_VERSION_MINOR 1
#define UCI_FIRA_TEST_VERSION_MAJOR 1
#define UCI_FIRA_TEST_VERSION_MINOR 1

/**
 * enum uci_common_packet_header - Common Packet Header layout description
 * @UCI_COMMON_PACKET_HEADER_MT_OFFSET:
 *     Offset in the Common Packet Header Octet 0 of the MT field.
 * @UCI_COMMON_PACKET_HEADER_MT_MASK:
 *     Mask of the MT field once shifted down.
 * @UCI_COMMON_PACKET_HEADER_PBF_OFFSET:
 *     Offset in the Common Packet Header Octet 0 of the PBF field.
 * @UCI_COMMON_PACKET_HEADER_PBF_MASK:
 *     Mask of the PBF field once shifted down.
 * @UCI_COMMON_PACKET_HEADER_INFO_OFFSET:
 *     Offset in the Common Packet Header Octet 0 of the INFO field.
 * @UCI_COMMON_PACKET_HEADER_INFO_MASK:
 *     Mask of the INFO field once shifted down.
 */
enum uci_common_packet_header {
	UCI_COMMON_PACKET_HEADER_MT_OFFSET = 5,
	UCI_COMMON_PACKET_HEADER_MT_MASK = 0b111,
	UCI_COMMON_PACKET_HEADER_PBF_OFFSET = 4,
	UCI_COMMON_PACKET_HEADER_PBF_MASK = 1,
	UCI_COMMON_PACKET_HEADER_INFO_OFFSET = 0,
	UCI_COMMON_PACKET_HEADER_INFO_MASK = 0b1111,
};

/**
 * enum uci_message_pbf - PBF flag definition.
 *
 * @UCI_MESSAGE_PBF_COMPLETE:
 *     If the Packet contains a complete Message, the PBF SHALL be set to 0b0.
 *     If the Packet contains the last segment of a segmented Message, the PBF
 *     SHALL be set to 0b0.
 * @UCI_MESSAGE_PBF_SEGMENT:
 *     If the packet does not contain the last segment of a segmented Message, the
 *     PBF SHALL be set to 0b1.
 */
enum uci_message_pbf {
	UCI_MESSAGE_PBF_COMPLETE = 0b0,
	UCI_MESSAGE_PBF_SEGMENT = 0b1,
};

/**
 * enum uci_message_type - Message type definition.
 *
 * @UCI_MESSAGE_TYPE_COMMAND:
 *     The message is a command.
 * @UCI_MESSAGE_TYPE_RESPONSE:
 *     The message is a response to a command.
 * @UCI_MESSAGE_TYPE_NOTIFICATION:
 *     The message is a notification.
 * @UCI_MESSAGE_TYPE_MASK
 *     The mask to extract the message type once shifted to the lower bits.
 */
enum uci_message_type {
	/* UCI_MESSAGE_TYPE_RFU = 0b000, */
	UCI_MESSAGE_TYPE_COMMAND = 0b001,
	UCI_MESSAGE_TYPE_RESPONSE = 0b010,
	UCI_MESSAGE_TYPE_NOTIFICATION = 0b011,
	/* RFU 0b100 - 0b111 */
};

/**
 * enum uci_message_gid - Group identifiers definition.
 *
 * @UCI_MESSAGE_GID_CORE:
 *     GID used for generic uci command on device.
 * @UCI_MESSAGE_GID_SESSION_CONFIG:
 *     GID used for fira session command.
 * @UCI_MESSAGE_GID_RANGING_SESSION_CONTROL:
 *     GID used for starting stoping a fira ranging.
 * @UCI_MESSAGE_GID_DATA_CONTROL_GROUP:
 *     Data session group.
 * @UCI_MESSAGE_GID_QORVO:
 *     GID used for qorvo's additional physical tests and diagnostics.
 * @UCI_MESSAGE_GID_QORVO_MCPS:
 *     GID used for uwbmac over uci feature.
 * @UCI_MESSAGE_GID_TEST:
 *     Test session group.
 * @UCI_MESSAGE_GID_ANDROID:
 *     GID used in Android HAL.
 */
enum uci_message_gid {
	UCI_MESSAGE_GID_CORE = 0b0000,
	UCI_MESSAGE_GID_SESSION_CONFIG = 0b0001,
	UCI_MESSAGE_GID_RANGING_SESSION_CONTROL = 0b0010,
	UCI_MESSAGE_GID_DATA_CONTROL_GROUP = 0b0011,
	/* RFU 0b0100 - 0b1000 */
	/* Vendor 0b1001 - 0b1100 */
	UCI_MESSAGE_GID_QORVO = 0b1011,
	UCI_MESSAGE_GID_QORVO_MCPS = 0b1100,
	/* spec PCTT */
	UCI_MESSAGE_GID_TEST = 0b1101,
	/* Vendor 0b1110 - 0b1111 */
	UCI_MESSAGE_GID_ANDROID = 0b1110,
};

/**
 * enum uci_message_core_oid - Opcode identifiers for UCI_MESSAGE_GID_CORE.
 *
 * @UCI_MESSAGE_OID_CORE_DEVICE_RESET_CMD_RSP:
 *     Reset device.
 * @UCI_MESSAGE_OID_CORE_DEVICE_STATUS_NTF:
 *     Device state notification (ready, active, error).
 * @UCI_MESSAGE_OID_CORE_GET_DEVICE_INFO_CMD_RSP:
 *     Get UCI version information as well as device specific ones.
 * @UCI_MESSAGE_OID_CORE_GET_CAPS_INFO_CMD_RSP:
 *     Get device capabilities.
 * @UCI_MESSAGE_OID_CORE_SET_CONFIG_CMD_RSP:
 *     Set device configuration.
 * @UCI_MESSAGE_OID_CORE_GET_CONFIG_CMD_RSP:
 *     Get device configuration.
 * @UCI_MESSAGE_OID_CORE_GENERIC_ERROR_NTF:
 *     Error notification.
 */
enum uci_message_core_oid {
	UCI_MESSAGE_OID_CORE_DEVICE_RESET_CMD_RSP = 0b000000,
	UCI_MESSAGE_OID_CORE_DEVICE_STATUS_NTF = 0b000001,
	UCI_MESSAGE_OID_CORE_GET_DEVICE_INFO_CMD_RSP = 0b000010,
	UCI_MESSAGE_OID_CORE_GET_CAPS_INFO_CMD_RSP = 0b000011,
	UCI_MESSAGE_OID_CORE_SET_CONFIG_CMD_RSP = 0b000100,
	UCI_MESSAGE_OID_CORE_GET_CONFIG_CMD_RSP = 0b000101,
	/* RFU = 0b000110 */
	UCI_MESSAGE_OID_CORE_GENERIC_ERROR_NTF = 0b000111,
	/* RFU = 0b111000 - 0b111111 */
};

/**
 * enum uci_message_session_config_oid - Opcode identifiers for UCI_MESSAGE_GID_SESSION_CONFIG.
 *
 * @UCI_MESSAGE_OID_SESSION_INIT_CMD_RSP:
 *     Init session.
 * @UCI_MESSAGE_OID_SESSION_DEINIT_CMD_RSP:
 *     Deinit session.
 * @UCI_MESSAGE_OID_SESSION_STATUS_NTF:
 *     Session state notification (init, idle, active, deinit).
 * @UCI_MESSAGE_OID_SESSION_SET_APP_CONFIG_CMD_RSP:
 *     Set session configuration.
 * @UCI_MESSAGE_OID_SESSION_GET_APP_CONFIG_CMD_RSP:
 *     Get session configuration.
 * @UCI_MESSAGE_OID_SESSION_GET_COUNT_CMD_RSP:
 *     Get the number of session in use.
 * @UCI_MESSAGE_OID_SESSION_GET_STATE_CMD_RSP:
 *     Get the state of one session.
 * @UCI_MESSAGE_OID_SESSION_UPDATE_CONTROLLER_MULTICAST_LIST_CMD_RSP_NTF:
 *     Update the list of controlee of a session.
 */
enum uci_message_session_config_oid {
	UCI_MESSAGE_OID_SESSION_INIT_CMD_RSP = 0b000000,
	UCI_MESSAGE_OID_SESSION_DEINIT_CMD_RSP = 0b000001,
	UCI_MESSAGE_OID_SESSION_STATUS_NTF = 0b000010,
	UCI_MESSAGE_OID_SESSION_SET_APP_CONFIG_CMD_RSP = 0b000011,
	UCI_MESSAGE_OID_SESSION_GET_APP_CONFIG_CMD_RSP = 0b000100,
	UCI_MESSAGE_OID_SESSION_GET_COUNT_CMD_RSP = 0b000101,
	UCI_MESSAGE_OID_SESSION_GET_STATE_CMD_RSP = 0b000110,
	UCI_MESSAGE_OID_SESSION_UPDATE_CONTROLLER_MULTICAST_LIST_CMD_RSP_NTF =
		0b000111,
	/* RFU 0b100000 - 0b111111 */
};

/**
 * enum uci_message_ranging_session_control_oid - Opcode identifiers for UCI_MESSAGE_GID_RANGING_SESSION_CONTROL.
 *
 * @UCI_MESSAGE_OID_RANGE_START_CMD_RSP_NTF:
 *     Start session.
 * @UCI_MESSAGE_OID_RANGE_STOP_CMD_RSP:
 *     Stop session.
 * @UCI_MESSAGE_OID_RANGE_INTERVAL_UPDATE_REQ_CMD_RSP:
 *     Update ranging interval of active session.
 * @UCI_MESSAGE_OID_RANGE_GET_RANGING_COUNT_CMD_RSP:
 *     Get the number of ranging done by an active session.
 */
enum uci_message_ranging_session_control_oid {
	UCI_MESSAGE_OID_RANGE_START_CMD_RSP_NTF = 0b000000,
	UCI_MESSAGE_OID_RANGE_STOP_CMD_RSP = 0b000001,
	UCI_MESSAGE_OID_RANGE_INTERVAL_UPDATE_REQ_CMD_RSP = 0b000010,
	UCI_MESSAGE_OID_RANGE_GET_RANGING_COUNT_CMD_RSP = 0b000011,
	/* RFU 0b000100 - 0b111111 */
};

/**
 * enum uci_message_test_control_oid - Opcode identifiers for UCI_MESSAGE_GID_TEST.
 *
 * @UCI_MESSAGE_OID_TEST_CONFIG_SET_RSP:
 *     Set configuration specific to testing.
 * @UCI_MESSAGE_OID_TEST_CONFIG_GET_RSP:
 *     Get test configuration.
 * @UCI_MESSAGE_OID_TEST_PERIODIC_TX_RSP_NTF:
 *     Execute periodic tx test.
 * @UCI_MESSAGE_OID_TEST_PER_RX_RSP_NTF:
 *     Execute per rx test.
 * @UCI_MESSAGE_OID_TEST_RX_RSP_NTF:
 *     Execute rx test.
 * @UCI_MESSAGE_OID_TEST_LOOPBACK_RSP_NTF:
 *     Execute loopback test.
 * @UCI_MESSAGE_OID_TEST_STOP_SESSION_RSP:
 *     Stop a test.
 * @UCI_MESSAGE_OID_TEST_SS_TWR_RSP_NTF:
 *     Execute ss twr test.
 */
enum uci_message_test_control_oid {
	UCI_MESSAGE_OID_TEST_CONFIG_SET_RSP = 0b000000,
	UCI_MESSAGE_OID_TEST_CONFIG_GET_RSP = 0b000001,
	UCI_MESSAGE_OID_TEST_PERIODIC_TX_RSP_NTF = 0b000010,
	UCI_MESSAGE_OID_TEST_PER_RX_RSP_NTF = 0b000011,
	UCI_MESSAGE_OID_TEST_RX_RSP_NTF = 0b000101,
	UCI_MESSAGE_OID_TEST_LOOPBACK_RSP_NTF = 0b000110,
	UCI_MESSAGE_OID_TEST_STOP_SESSION_RSP = 0b000111,
	UCI_MESSAGE_OID_TEST_SS_TWR_RSP_NTF = 0b001000,
	/* RFU 0b000100 */
	/* RFU 0b001001 - 0b111111 */
};

/**
 * enum uci_message_data_control_oid - Opcode identifiers for UCI_MESSAGE_GID_DATA_CONTROL_GROUP.
 *
 * @UCI_MESSAGE_OID_APP_DATA_TX_CMD_RSP_NTF:
 *     Transmit data.
 * @UCI_MESSAGE_OID_APP_DATA_RX_CMD_RSP_NTF:
 *     Receive data.
 */
enum uci_message_data_control_oid {
	UCI_MESSAGE_OID_APP_DATA_TX_CMD_RSP_NTF = 0b000000,
	UCI_MESSAGE_OID_APP_DATA_RX_CMD_RSP_NTF = 0b000001,
};

/**
 * enum qorvo_test_uci_oid - Opcode identifiers for uci commands for Qorvo's tests.
 *
 * @UCI_OID_TEST_DEBUG_NTF:
 *     Opcode for tests additional notification.
 * @UCI_OID_TEST_TX_CW:
 *     Opcode for continous wave transmission test.
 * @UCI_OID_TEST_PLLRF:
 *     Opcode for pll status test.
 */
enum qorvo_test_uci_oid {
	UCI_OID_TEST_DEBUG_NTF = 0b000000,
	UCI_OID_TEST_TX_CW = 0b000001,
	UCI_OID_TEST_PLLRF = 0b000010,
};

/**
 * enum uci_status_code - Status of a command.
 *
 * @UCI_STATUS_OK:
 *     Success.
 * @UCI_STATUS_REJECTED:
 *     Rejected.
 * @UCI_STATUS_FAILED:
 *     Unknown failure.
 * @UCI_STATUS_SYNTAX_ERROR:
 *     Syntax error in the UCI command.
 * @UCI_STATUS_INVALID_PARAM:
 *     A parameter was not valid.
 * @UCI_STATUS_INVALID_RANGE:
 *     A known parameter value was out of possible range.
 * @UCI_STATUS_INVALID_MESSAGE_SIZE:
 *     The size of the command message was wrong.
 * @UCI_STATUS_UNKNOWN_GID:
 *     Unknow GID.
 * @UCI_STATUS_UNKNOWN_OID:
 *     Unknow OID.
 * @UCI_STATUS_READ_ONLY:
 *     Tried to set a read only value.
 * @UCI_STATUS_COMMAND_RETRY:
 *     UWBS request retransmission from Host.
 * @UCI_STATUS_ERROR_SESSION_NOT_EXIST:
 *     Given session does not exist.
 * @UCI_STATUS_ERROR_SESSION_DUPLICATE:
 *    Session is already created/exist.
 * @UCI_STATUS_ERROR_SESSION_ACTIVE:
 *     Session is active.
 * @UCI_STATUS_ERROR_MAX_SESSIONS_EXCEEDED:
 *     Max. number of sessions already created.
 * @UCI_STATUS_ERROR_SESSION_NOT_CONFIGURED:
 *     Session is not configured with required app configurations.
 * @UCI_STATUS_ERROR_ACTIVE_SESSIONS_ONGOING:
 *     Sessions are actively running in UWBS.
 * @UCI_STATUS_ERROR_MULTICAST_LIST_FULL:
 *     Indicates when multicast list is full during one to many ranging.
 * @UCI_STATUS_ERROR_ADDRESS_NOT_FOUND:
 *     Short address is not available.
 * @UCI_STATUS_ERROR_ADDRESS_ALREADY_PRESENT:
 *     Short address is already present in multicat list.
 * @UCI_STATUS_RANGING_TX_FAILED:
 *     Failed to transmit UWB packet.
 * @UCI_STATUS_RANGING_RX_TIMEOUT:
 *     No UWB packet detected by the receiver.
 * @UCI_STATUS_RANGING_RX_PHY_DEC_FAILED:
 *     UWB packet channel decoding error.
 * @UCI_STATUS_RANGING_RX_PHY_TOA_FAILED:
 *     Failed to detect time of arrival of the UWB packet from CIR samples.
 * @UCI_STATUS_RANGING_RX_PHY_STS_FAILED:
 *     UWB packet STS segment mismatch.
 * @UCI_STATUS_RANGING_RX_MAC_DEC_FAILED:
 *     MAC CRC or syntax error.
 * @UCI_STATUS_RANGING_RX_MAC_IE_DEC_FAILED:
 *     IE syntax error.
 * @UCI_STATUS_RANGING_RX_MAC_IE_MISSING:
 *     Expected IE missing in the packet.
 * @UCI_STATUS_DATA_MAX_TX_APDU_SIZE_EXCEEDED:
 *     Max tx apdu size exceeded.
 * @UCI_STATUS_DATA_RX_CRC_ERROR:
 *     rx crc error.
 * @UCI_STATUS_ERROR_SE_BUSY:
 *     Proprietary CCC error: Secure Element is busy.
 * @UCI_STATUS_ERROR_CCC_LIFECYCLE:
 *     Proprietary CCC error: lifecycle has not been respected.
 * @UCI_STATUS_LAST:
 *     Internal use.
 */
enum uci_status_code {
	UCI_STATUS_OK = 0x00,
	UCI_STATUS_REJECTED = 0x01,
	UCI_STATUS_FAILED = 0x02,
	UCI_STATUS_SYNTAX_ERROR = 0x03,
	UCI_STATUS_INVALID_PARAM = 0x04,
	UCI_STATUS_INVALID_RANGE = 0x05,
	UCI_STATUS_INVALID_MESSAGE_SIZE = 0x06,
	UCI_STATUS_UNKNOWN_GID = 0x07,
	UCI_STATUS_UNKNOWN_OID = 0x08,
	UCI_STATUS_READ_ONLY = 0x09,
	UCI_STATUS_COMMAND_RETRY = 0x0a,
	/* RFU 0x0b - 0x0f */
	/* Note: 0x10 is missing from the documentation */
	UCI_STATUS_ERROR_SESSION_NOT_EXIST = 0x11,
	UCI_STATUS_ERROR_SESSION_DUPLICATE = 0x12,
	UCI_STATUS_ERROR_SESSION_ACTIVE = 0x13,
	UCI_STATUS_ERROR_MAX_SESSIONS_EXCEEDED = 0x14,
	UCI_STATUS_ERROR_SESSION_NOT_CONFIGURED = 0x15,
	UCI_STATUS_ERROR_ACTIVE_SESSIONS_ONGOING = 0x16,
	UCI_STATUS_ERROR_MULTICAST_LIST_FULL = 0x17,
	UCI_STATUS_ERROR_ADDRESS_NOT_FOUND = 0x18,
	UCI_STATUS_ERROR_ADDRESS_ALREADY_PRESENT = 0x19,
	/* RFU 0x1a - 0x1f */
	/* Note: the range is 0x20 - 0x1f in the documentation */
	UCI_STATUS_RANGING_TX_FAILED = 0x20,
	UCI_STATUS_RANGING_RX_TIMEOUT = 0x21,
	UCI_STATUS_RANGING_RX_PHY_DEC_FAILED = 0x22,
	UCI_STATUS_RANGING_RX_PHY_TOA_FAILED = 0x23,
	UCI_STATUS_RANGING_RX_PHY_STS_FAILED = 0x24,
	UCI_STATUS_RANGING_RX_MAC_DEC_FAILED = 0x25,
	UCI_STATUS_RANGING_RX_MAC_IE_DEC_FAILED = 0x26,
	UCI_STATUS_RANGING_RX_MAC_IE_MISSING = 0x27,
	/* RFU 0x28 - 0x2f */
	UCI_STATUS_DATA_MAX_TX_APDU_SIZE_EXCEEDED = 0x30,
	UCI_STATUS_DATA_RX_CRC_ERROR = 0x31,
	/* RFU 0x32 - 0x4f */
	/* Proprietary 0x50 - 0xff */
	UCI_STATUS_ERROR_SE_BUSY = 0x50,
	UCI_STATUS_ERROR_CCC_LIFECYCLE = 0x51,
	UCI_STATUS_LAST,
};

/**
 * enum uci_device_state - Device state.
 *
 * @UCI_DEVICE_STATE_READY:
 *     Device is ready.
 * @UCI_DEVICE_STATE_ACTIVE:
 *     Device is performing a communication or ranging.
 * @UCI_DEVICE_STATE_ERROR:
 *     Device needs to be reset.
 */
enum uci_device_state {
	/* RFU 0x00 */
	UCI_DEVICE_STATE_READY = 0x01,
	UCI_DEVICE_STATE_ACTIVE = 0x02,
	/* RFU 0x03 - 0xfe */
	UCI_DEVICE_STATE_ERROR = 0xff,
};

/**
 * enum uci_session_type - Session type.
 *
 * @UCI_SESSION_TYPE_RANGING:
 *     Fira ranging session.
 * @UCI_SESSION_TYPE_DATA:
 *     Data session.
 * @UCI_SESSION_TYPE_ANDROID_CCC:
 *     CCC session.
 * @UCI_SESSION_TYPE_TEST:
 *     Test session.
 */
enum uci_session_type {
	UCI_SESSION_TYPE_RANGING = 0x00,
	UCI_SESSION_TYPE_DATA = 0x01,
	/* RFU 0x02 - 0x9f */
	UCI_SESSION_TYPE_ANDROID_CCC = 0xa0,
	/* CCC 0xa1 - 0cf */
	UCI_SESSION_TYPE_TEST = 0xd0,
	/* RFU 0xd1 - 0xdf */
	/* Proprietary 0xe0 - 0xff */
};

/**
 * enum uci_session_state - Session state.
 *
 * @UCI_SESSION_STATE_INIT:
 *     Session is created but not configured.
 * @UCI_SESSION_STATE_DEINIT:
 *     Session does not exist.
 * @UCI_SESSION_STATE_ACTIVE:
 *     Ranging or communication is underway.
 * @UCI_SESSION_STATE_IDLE:
 *     Session is configured and communication ready to start.
 * @UCI_SESSION_STATE_ERROR:
 *     Session is broken.
 */
enum uci_session_state {
	UCI_SESSION_STATE_INIT = 0x00,
	UCI_SESSION_STATE_DEINIT = 0x01,
	UCI_SESSION_STATE_ACTIVE = 0x02,
	UCI_SESSION_STATE_IDLE = 0x03,
	/* RFU 0x04 - 0xfe */
	UCI_SESSION_STATE_ERROR = 0xff,
};

/**
 * enum uci_session_reason_code - Reason of session state change.
 *
 * @UCI_SESSION_REASON_STATE_CHANGE_WITH_SESSION_MANAGEMENT_COMMANDS:
 *     State was changed by a user command.
 * @UCI_SESSION_REASON_MAX_RANGING_ROUND_RETRY_COUNT_REACHED:
 *     State has changed after too much retry.
 * @UCI_SESSION_REASON_MAX_NUMBER_OF_MEASUREMENTS_REACHED:
 *     State changed because measurement is over as per configuration.
 * @UCI_SESSION_REASON_ERROR_SLOT_LENGTH_NOT_SUPPORTED:
 *     New slot length not supported.
 * @UCI_SESSION_REASON_ERROR_INSUFFICIENT_SLOTS_PER_RR:
 *     New slots per rr not supported.
 * @UCI_SESSION_REASON_ERROR_MAC_ADDRESS_MODE_NOT_SUPPORTED:
 *     Mac adress mode not supported.
 */
enum uci_session_reason_code {
	UCI_SESSION_REASON_STATE_CHANGE_WITH_SESSION_MANAGEMENT_COMMANDS = 0x00,
	/* Below codes shall be reported with UCI_SESSION_STATE_IDLE state only */
	UCI_SESSION_REASON_MAX_RANGING_ROUND_RETRY_COUNT_REACHED = 0x01,
	UCI_SESSION_REASON_MAX_NUMBER_OF_MEASUREMENTS_REACHED = 0x02,
	/* RFU 0x03 - 0x1f */
	UCI_SESSION_REASON_ERROR_SLOT_LENGTH_NOT_SUPPORTED = 0x20,
	UCI_SESSION_REASON_ERROR_INSUFFICIENT_SLOTS_PER_RR = 0x21,
	UCI_SESSION_REASON_ERROR_MAC_ADDRESS_MODE_NOT_SUPPORTED = 0x22,
	/* RFU 0x23 - 0x7f */
	/* Proprietary 0x80 - 0xff */
};

/**
 * enum uci_device_configuration_parameters - Device configuration.
 *
 * @UCI_DEVICE_PARAMETER_DEVICE_STATE:
 *     Read only device state.
 * @UCI_DEVICE_PARAMETER_LOW_POWER_MODE:
 *     Low power mode.
 * @UCI_DEVICE_PARAMETER_CHANNEL:
 *     Default channel.
 */
enum uci_device_configuration_parameters {
	UCI_DEVICE_PARAMETER_DEVICE_STATE = 0x00,
	UCI_DEVICE_PARAMETER_LOW_POWER_MODE = 0x01,
	/* RFU 0x02 - 0x9f */
	UCI_DEVICE_PARAMETER_CHANNEL = 0xA0,
	/* CCC 0xa1 - 0xdf */
	/* RFU 0xe0 - 0xe2 */
	/* Proprietary 0xe3 - 0xff */
};

/**
 * enum uci_application_configuration_parameters - Session configuration.
 *
 * @UCI_APPLICATION_PARAMETER_DEVICE_TYPE:
 *     Device type (controller or controlee).
 * @UCI_APPLICATION_PARAMETER_RANGING_ROUND_USAGE:
 *     Ranging round usage.
 * @UCI_APPLICATION_PARAMETER_STS_CONFIG:
 *     STS config.
 * @UCI_APPLICATION_PARAMETER_MULTI_NODE_MODE:
 *     Multi node mode.
 * @UCI_APPLICATION_PARAMETER_CHANNEL_NUMBER:
 *     Channel number (5 or 9).
 * @UCI_APPLICATION_PARAMETER_NUMBER_OF_CONTROLEES:
 *     Number of controlees.
 * @UCI_APPLICATION_PARAMETER_DEVICE_MAC_ADDRESS:
 *     Device mac address.
 * @UCI_APPLICATION_PARAMETER_DST_MAC_ADDRESS:
 *     List of distant mac addresses.
 * @UCI_APPLICATION_PARAMETER_SLOT_DURATION:
 *     Slot duration.
 * @UCI_APPLICATION_PARAMETER_RANGING_INTERVAL:
 *     Ranging interval.
 * @UCI_APPLICATION_PARAMETER_STS_INDEX:
 *     STS index.
 * @UCI_APPLICATION_PARAMETER_MAC_FCS_TYPE:
 *     MAC fcs type.
 * @UCI_APPLICATION_PARAMETER_RANGING_ROUND_CONTROL:
 *     Ranging round control.
 * @UCI_APPLICATION_PARAMETER_AOA_RESULT_REQ:
 *     AOA result requirement.
 * @UCI_APPLICATION_PARAMETER_RANGE_DATA_NTF_CONFIG:
 *     Range data ntf config.
 * @UCI_APPLICATION_PARAMETER_RANGE_DATA_NTF_PROXIMITY_NEAR:
 *     Range data ntf proximity near.
 * @UCI_APPLICATION_PARAMETER_RANGE_DATA_NTF_PROXIMITY_FAR:
 *     Range data ntf proximity far.
 * @UCI_APPLICATION_PARAMETER_DEVICE_ROLE:
 *     Device role (initiator or responder).
 * @UCI_APPLICATION_PARAMETER_RFRAME_CONFIG:
 *     RFrame config.
 * @UCI_APPLICATION_PARAMETER_RX_MODE:
 *     RX mode.
 * @UCI_APPLICATION_PARAMETER_PREAMBLE_CODE_INDEX:
 *     Preamble code index.
 * @UCI_APPLICATION_PARAMETER_SFD_ID:
 *     SDF id.
 * @UCI_APPLICATION_PARAMETER_PSDU_DATA_RATE:
 *     PSDU data rate.
 * @UCI_APPLICATION_PARAMETER_PREAMBLE_DURATION:
 *     Preamble duration.
 * @UCI_APPLICATION_PARAMETER_MAC_CFG:
 *     MAC cfg.
 * @UCI_APPLICATION_PARAMETER_RANGING_TIME_STRUCT:
 *     Ranging time struct.
 * @UCI_APPLICATION_PARAMETER_SLOTS_PER_RR:
 *     Slots per ranging round.
 * @UCI_APPLICATION_PARAMETER_TX_ADAPTIVE_PAYLOAD_POWER:
 *     TX adaptive payload power.
 * @UCI_APPLICATION_PARAMETER_RESPONDER_SLOT_INDEX:
 *     Responder slot index.
 * @UCI_APPLICATION_PARAMETER_PRF_MODE:
 *     PRF mode.
 * @UCI_APPLICATION_PARAMETER_CAP_SIZE_RANGE:
 *     Maximum and minimum CAP size for contention-based ranging.
 * @UCI_APPLICATION_PARAMETER_SCHEDULE_MODE:
 *     Schedule mode.
 * @UCI_APPLICATION_PARAMETER_KEY_ROTATION:
 *     Key rotation.
 * @UCI_APPLICATION_PARAMETER_KEY_ROTATION_RATE:
 *     Key rotation rate.
 * @UCI_APPLICATION_PARAMETER_SESSION_PRIORITY:
 *     Session priority.
 * @UCI_APPLICATION_PARAMETER_MAC_ADDRESS_MODE:
 *     MAC adress mode.
 * @UCI_APPLICATION_PARAMETER_VENDOR_ID:
 *     Vendor id.
 * @UCI_APPLICATION_PARAMETER_STATIC_STS_IV:
 *     Static STS IV.
 * @UCI_APPLICATION_PARAMETER_NUMBER_OF_STS_SEGMENTS:
 *     Number of STS segments.
 * @UCI_APPLICATION_PARAMETER_MAX_RR_RETRY:
 *     Max number of ranging round retry.
 * @UCI_APPLICATION_PARAMETER_UWB_INITIATION_TIME:
 *     UWB initiation time.
 * @UCI_APPLICATION_PARAMETER_HOPPING_MODE:
 *     Hopping mode.
 * @UCI_APPLICATION_PARAMETER_BLOCK_STRIDE_LENGTH:
 *     Block stride length.
 * @UCI_APPLICATION_PARAMETER_RESULT_REPORT_CONFIG:
 *     Result report config.
 * @UCI_APPLICATION_PARAMETER_IN_BAND_TERMINATION_ATTEMPT_COUNT:
 *     In band termination attempt count.
 * @UCI_APPLICATION_PARAMETER_SUB_SESSION_ID:
 *     Sub session id.
 * @UCI_APPLICATION_PARAMETER_BPRF_PHR_DATA_RATE:
 *     BPRF PHR data rate.
 * @UCI_APPLICATION_PARAMETER_MAX_NUMBER_OF_MEASUREMENTS:
 *     Max number of measurements.
 * @UCI_APPLICATION_PARAMETER_STS_LENGTH:
 *     STS length.
 * @UCI_APPLICATION_PARAMETER_RSSI_REPORTING:
 *     RSSI report
 * @UCI_APPLICATION_PARAMETER_NB_OF_RANGE_MEASUREMENTS:
 *     Number of range measurements in interleaved mode.
 * @UCI_APPLICATION_PARAMETER_NB_OF_AZIMUTH_MEASUREMENTS:
 *     Number of azimuth measurements in interleaved mode.
 * @UCI_APPLICATION_PARAMETER_NB_OF_ELEVATION_MEASUREMENTS:
 *     Number of elevation measurements in interleaved mode.
 * @UCI_APPLICATION_PARAMETER_RX_ANTENNA_SELECTION:
 *     RX antenna set id.
 * @UCI_APPLICATION_PARAMETER_TX_ANTENNA_SELECTION:
 *     TX antenna set id.
 * @UCI_APPLICATION_PARAMETER_ENABLE_DIAGNOSTICS:
 *     Enable diagnostic notification in FIRA or PCTT region.
 * @UCI_APPLICATION_PARAMETER_DIAGS_FRAME_REPORTS_FIELDS:
 *     Select the fields to activate in the frame reports
 *     stored in the diags. If the ENABLE_DIAGNOSTICS is
 *     not true this parameters does not activate the diags
 *     itself.
 * @UCI_APPLICATION_PARAMETER_AFTER_LAST_NOT_VENDOR:
 *     Internal use.
 */
enum uci_application_configuration_parameters {
	UCI_APPLICATION_PARAMETER_DEVICE_TYPE = 0x00,
	UCI_APPLICATION_PARAMETER_RANGING_ROUND_USAGE = 0x01,
	UCI_APPLICATION_PARAMETER_STS_CONFIG = 0x02,
	UCI_APPLICATION_PARAMETER_MULTI_NODE_MODE = 0x03,
	UCI_APPLICATION_PARAMETER_CHANNEL_NUMBER = 0x04,
	UCI_APPLICATION_PARAMETER_NUMBER_OF_CONTROLEES = 0x05,
	UCI_APPLICATION_PARAMETER_DEVICE_MAC_ADDRESS = 0x06,
	UCI_APPLICATION_PARAMETER_DST_MAC_ADDRESS = 0x07,
	UCI_APPLICATION_PARAMETER_SLOT_DURATION = 0x08,
	UCI_APPLICATION_PARAMETER_RANGING_INTERVAL = 0x09,
	UCI_APPLICATION_PARAMETER_STS_INDEX = 0x0a,
	UCI_APPLICATION_PARAMETER_MAC_FCS_TYPE = 0x0b,
	UCI_APPLICATION_PARAMETER_RANGING_ROUND_CONTROL = 0x0c,
	UCI_APPLICATION_PARAMETER_AOA_RESULT_REQ = 0x0d,
	UCI_APPLICATION_PARAMETER_RANGE_DATA_NTF_CONFIG = 0x0e,
	UCI_APPLICATION_PARAMETER_RANGE_DATA_NTF_PROXIMITY_NEAR = 0x0f,
	UCI_APPLICATION_PARAMETER_RANGE_DATA_NTF_PROXIMITY_FAR = 0x10,
	UCI_APPLICATION_PARAMETER_DEVICE_ROLE = 0x11,
	UCI_APPLICATION_PARAMETER_RFRAME_CONFIG = 0x12,
	UCI_APPLICATION_PARAMETER_RX_MODE = 0x13,
	UCI_APPLICATION_PARAMETER_PREAMBLE_CODE_INDEX = 0x14,
	UCI_APPLICATION_PARAMETER_SFD_ID = 0x15,
	UCI_APPLICATION_PARAMETER_PSDU_DATA_RATE = 0x16,
	UCI_APPLICATION_PARAMETER_PREAMBLE_DURATION = 0x17,
	UCI_APPLICATION_PARAMETER_MAC_CFG = 0x19,
	UCI_APPLICATION_PARAMETER_RANGING_TIME_STRUCT = 0x1a,
	UCI_APPLICATION_PARAMETER_SLOTS_PER_RR = 0x1b,
	UCI_APPLICATION_PARAMETER_TX_ADAPTIVE_PAYLOAD_POWER = 0x1c,
	UCI_APPLICATION_PARAMETER_RESPONDER_SLOT_INDEX = 0x1e,
	UCI_APPLICATION_PARAMETER_PRF_MODE = 0x1f,
	UCI_APPLICATION_PARAMETER_CAP_SIZE_RANGE = 0x20,
	/* UCI_APPLICATION_PARAMETER_CONTENTION_PHASE_UPDATE_LENGTH = 0x21, */
	UCI_APPLICATION_PARAMETER_SCHEDULE_MODE = 0x22,
	UCI_APPLICATION_PARAMETER_KEY_ROTATION = 0x23,
	UCI_APPLICATION_PARAMETER_KEY_ROTATION_RATE = 0x24,
	UCI_APPLICATION_PARAMETER_SESSION_PRIORITY = 0x25,
	UCI_APPLICATION_PARAMETER_MAC_ADDRESS_MODE = 0x26,
	UCI_APPLICATION_PARAMETER_VENDOR_ID = 0x27,
	UCI_APPLICATION_PARAMETER_STATIC_STS_IV = 0x28,
	UCI_APPLICATION_PARAMETER_NUMBER_OF_STS_SEGMENTS = 0x29,
	UCI_APPLICATION_PARAMETER_MAX_RR_RETRY = 0x2a,
	UCI_APPLICATION_PARAMETER_UWB_INITIATION_TIME = 0x2b,
	UCI_APPLICATION_PARAMETER_HOPPING_MODE = 0x2c,
	UCI_APPLICATION_PARAMETER_BLOCK_STRIDE_LENGTH = 0x2d,
	UCI_APPLICATION_PARAMETER_RESULT_REPORT_CONFIG = 0x2e,
	UCI_APPLICATION_PARAMETER_IN_BAND_TERMINATION_ATTEMPT_COUNT = 0x2f,
	UCI_APPLICATION_PARAMETER_SUB_SESSION_ID = 0x30,
	UCI_APPLICATION_PARAMETER_BPRF_PHR_DATA_RATE = 0x31,
	UCI_APPLICATION_PARAMETER_MAX_NUMBER_OF_MEASUREMENTS = 0x32,
	/* UNDEX = 0x33 */
	/* UNDEX = 0x34 */
	UCI_APPLICATION_PARAMETER_STS_LENGTH = 0x35,
	UCI_APPLICATION_PARAMETER_RSSI_REPORTING = 0x36,
	UCI_APPLICATION_PARAMETER_AFTER_LAST_NOT_VENDOR,
	/* RFU 0x37 - 0x9f */
	/* CCC 0xa0 - 0xdf */
	/* RFU 0xe0 - 0xe2 */
	/* Proprietary 0xe3 - 0xff */
	UCI_APPLICATION_PARAMETER_NB_OF_RANGE_MEASUREMENTS = 0xe3,
	UCI_APPLICATION_PARAMETER_NB_OF_AZIMUTH_MEASUREMENTS = 0xe4,
	UCI_APPLICATION_PARAMETER_NB_OF_ELEVATION_MEASUREMENTS = 0xe5,
	/* Antenna set selection for physical testing */
	UCI_APPLICATION_PARAMETER_RX_ANTENNA_SELECTION = 0xe6,
	UCI_APPLICATION_PARAMETER_TX_ANTENNA_SELECTION = 0xe7,
	UCI_APPLICATION_PARAMETER_ENABLE_DIAGNOSTICS = 0xe8,
	UCI_APPLICATION_PARAMETER_DIAGS_FRAME_REPORTS_FIELDS = 0xe9
};

/**
 * enum uci_device_type - Device type.
 *
 * @UCI_DEVICE_TYPE_CONTROLEE:
 *     Controlee.
 * @UCI_DEVICE_TYPE_CONTROLLER:
 *     Controller.
 */
enum uci_device_type {
	UCI_DEVICE_TYPE_CONTROLEE = 0x00,
	UCI_DEVICE_TYPE_CONTROLLER = 0x01,
	/* RFU 0x02 - 0xff */
};

/**
 * enum uci_ranging_round_usage - Ranging Round usage
 *
 * @UCI_SSTWR_DEFERRED:
 *     SS-TWR with deferred mode.
 * @UCI_DSTWR_DEFERRED:
 *     DS-TWR with deferred mode.
 * @UCI_SSTWR_NON_DEFERRED:
 *     SS-TWR with non-deferred mode.
 * @UCI_DSTWR_NON_DEFERRED:
 *     DS-TWR with non-deferred mode.
 */
enum uci_ranging_round_usage {
	/* RFU 0x00 */
	UCI_SSTWR_DEFERRED = 0x01,
	UCI_DSTWR_DEFERRED = 0x02,
	UCI_SSTWR_NON_DEFERRED = 0x03,
	UCI_DSTWR_NON_DEFERRED = 0x04,
	/* RFU 0x05 - 0xFF */
};

/**
 * enum uci_device_role - Device role.
 *
 * @UCI_DEVICE_ROLE_RESPONDER:
 *     Responder.
 * @UCI_DEVICE_ROLE_INITIATOR:
 *     Initiator.
 */
enum uci_device_role {
	UCI_DEVICE_ROLE_RESPONDER = 0x00,
	UCI_DEVICE_ROLE_INITIATOR = 0x01,
	/* RFU 0x02 - 0xff */
};

/**
 * enum uci_test_configuration_parameters - Test configuration.
 *
 * @UCI_TEST_PARAMETER_NUM_PACKETS:
 *     Number of packets.
 * @UCI_TEST_PARAMETER_T_GAP:
 *     T gap.
 * @UCI_TEST_PARAMETER_T_START:
 *     T start.
 * @UCI_TEST_PARAMETER_T_WIN:
 *     T win.
 * @UCI_TEST_PARAMETER_RANDOMIZE_PSDU:
 *     Randomize psdu.
 * @UCI_TEST_PARAMETER_PHR_RANGING_BIT:
 *     PHR ranging bit.
 * @UCI_TEST_PARAMETER_RMARKER_TX_START:
 *     R marker tx start.
 * @UCI_TEST_PARAMETER_RMARKER_RX_START:
 *     R marker rx start.
 * @UCI_TEST_PARAMETER_STS_INDEX_AUTO_INCR:
 *     STS index auto incr.
 */
enum uci_test_configuration_parameters {
	UCI_TEST_PARAMETER_NUM_PACKETS = 0x00,
	UCI_TEST_PARAMETER_T_GAP = 0x01,
	UCI_TEST_PARAMETER_T_START = 0x02,
	UCI_TEST_PARAMETER_T_WIN = 0x03,
	UCI_TEST_PARAMETER_RANDOMIZE_PSDU = 0x04,
	UCI_TEST_PARAMETER_PHR_RANGING_BIT = 0x05,
	UCI_TEST_PARAMETER_RMARKER_TX_START = 0x06,
	UCI_TEST_PARAMETER_RMARKER_RX_START = 0x07,
	UCI_TEST_PARAMETER_STS_INDEX_AUTO_INCR = 0x08
	/* RFU 0x09 - 0xff */
	/* Reserved for extension of ID 0xe0 - 0xe4 */
	/* Proprietary 0xe5 - 0xff */
};

/**
 * enum uci_controller_action - Controller multicast action.
 *
 * @UCI_CONTROLLER_ACTION_ADD:
 *     Add controlee.
 * @UCI_CONTROLLER_ACTION_DELETE:
 *     Delete controlee.
 */
enum uci_controller_action {
	UCI_CONTROLLER_ACTION_ADD = 0x00,
	UCI_CONTROLLER_ACTION_DELETE = 0x01,
};

/**
 * enum uci_controller_status - Controller multicast update status.
 *
 * @UCI_CONTROLLER_STATUS_OK:
 *     OK.
 * @UCI_CONTROLLER_STATUS_MULTICAST_LIST_FULL:
 *     Maximum number of controlee reached.
 * @UCI_CONTROLLER_STATUS_KEY_FETCH_FAIL:
 *     Key not found.
 * @UCI_CONTROLLER_STATUS_SUB_SESSION_ID_NOT_FOUND:
 *    Subsession id not found.
 */
enum uci_controller_status {
	UCI_CONTROLLER_STATUS_OK = 0x00,
	UCI_CONTROLLER_STATUS_MULTICAST_LIST_FULL = 0x01,
	UCI_CONTROLLER_STATUS_KEY_FETCH_FAIL = 0x02,
	UCI_CONTROLLER_STATUS_SUB_SESSION_ID_NOT_FOUND = 0x03,
	/* RFU 0x04 - 0x1f */
};

enum uci_device_capability_parameters {
	UCI_CAP_FIRA_PHY_VERSION_RANGE = 0x00,
	UCI_CAP_FIRA_MAC_VERSION_RANGE = 0x01,
	UCI_CAP_DEVICE_ROLE = 0x02,
	UCI_CAP_RANGING_METHOD = 0x03,
	UCI_CAP_STS_CONFIG = 0x04,
	UCI_CAP_MULTIMODE_NODE = 0x05,
	UCI_CAP_RANGING_TIME_STRUCT = 0x06,
	UCI_CAP_SCHEDULED_MODE = 0x07,
	UCI_CAP_HOPPING_MODE = 0x08,
	UCI_CAP_BLOCK_STRIDING = 0x09,
	UCI_CAP_UWB_INITIATION_TIME = 0x0a,
	UCI_CAP_CHANNELS = 0x0b,
	UCI_CAP_RFRAME_CONFIG = 0x0c,
	UCI_CAP_CC_CONSTRAINT_LENGTH = 0x0d,
	UCI_CAP_BPRF_PARAMETERS_SET = 0x0e,
	UCI_CAP_HPRF_PARAMETERS_SET = 0x0f,
	UCI_CAP_AOA_SUPPORT = 0x10,
	UCI_CAP_EXTENDED_MAC_ADDRESS = 0x11,
	/* RFU 0x12 - 0x9f */
	/* VENDOR SPECIFIC 0xa0 - 0xdf */
	/* RESERVED FOR EXTENSION OF IDS 0xe0 - 0xe2 */
	/* VENDOR SPECIFIC APP CONFIG 0xe3 - 0xff */
};

/**
 * enum uci_device_capability_aosp - Vendor device capabilities for Android.
 *
 * Up-to-date documentation and definition is available in
 * hardware/uwb/fira_android/UwbVendorCapabilityTlvTypes.aidl.
 *
 * All values are prefixed with AOSP_CAPS compared to their original definition.
 */
enum uci_device_capability_aosp {
	/**
	 * @AOSP_CAPS_CCC_SUPPORTED_CHAPS_PER_SLOT: 1 byte bitmask with a list of
	 *		supported chaps per slot
	 *
	 * Bitmap of supported values of Slot durations as a multiple of TChap,
	 * NChap_per_Slot as defined in CCC Specification.
	 * Each “1” in this bit map corresponds to a specific
	 * value of NChap_per_Slot where:
	 * 0x01 = “3”,
	 * 0x02 = “4”,
	 * 0x04= “6”,
	 * 0x08 =“8”,
	 * 0x10 =“9”,
	 * 0x20 = “12”,
	 * 0x40 = “24”,
	 * 0x80 is reserved.
	 */
	AOSP_CAPS_CCC_SUPPORTED_CHAPS_PER_SLOT = 0xA0,
	/**
	 * @AOSP_CAPS_CCC_SUPPORTED_SYNC_CODES: 4 byte bitmask with a list of
	 *		supported sync codes.
	 *
	 * Bitmap of SYNC code indices that can be used.
	 * The position of each “1” in this bit pattern
	 * corresponds to the index of a SYNC code that
	 * can be used, where:
	 * 0x00000001 = “1”,
	 * 0x00000002 = “2”,
	 * 0x00000004 = “3”,
	 * 0x00000008 = “4”,
	 * ….
	 * 0x40000000 = “31”,
	 * 0x80000000 = “32”
	 * Refer to IEEE 802.15.4-2015 and CCC
	 * Specification for SYNC code index definition
	 */
	AOSP_CAPS_CCC_SUPPORTED_SYNC_CODES = 0xA1,
	/**
	 * @AOSP_CAPS_CCC_SUPPORTED_HOPPING_CONFIG_MODES_AND_SEQUENCES: 1 byte
	 *		bitmask with a list of supported hopping config modes and sequences.
	 *
	 * [b7 b6 b5] : bitmask of hopping modes the
	 * device offers to use in the ranging session
	 * 100 - No Hopping
	 * 010 - Continuous Hopping
	 * 001 - Adaptive Hopping
	 * [b4 b3 b2 b1 b0] : bit mask of hopping
	 * sequences the device offers to use in the
	 * ranging session
	 * b4=1 is always set because of the default
	 * hopping sequence. Support for it is mandatory.
	 * b3=1 is set when the optional AES based
	 * hopping sequence is supported.
	 */
	AOSP_CAPS_CCC_SUPPORTED_HOPPING_CONFIG_MODES_AND_SEQUENCES = 0xA2,
	/**
	 * @AOSP_CAPS_CCC_SUPPORTED_CHANNELS: 1 byte bitmask with list of supported
	 *		channels.
	 *
	 * Bitmap of supported UWB channels. Each “1” in
	 * this bit map corresponds to a specific value of
	 * UWB channel where:
	 * 0x01 = "Channel 5"
	 * 0x02 = "Channel 9"
	 */
	AOSP_CAPS_CCC_SUPPORTED_CHANNELS = 0xA3,
	/**
	 * @AOSP_CAPS_CCC_SUPPORTED_VERSIONS: Supported CCC version.
	 *
	 * 2 byte tuple {major_version (1 byte), minor_version (1 byte)} array with
	 * list of supported CCC versions
	 */
	AOSP_CAPS_CCC_SUPPORTED_VERSIONS = 0xA4,
	/**
	 * @AOSP_CAPS_CCC_SUPPORTED_UWB_CONFIGS: byte array with a list of supported
	 *		UWB configs.
	 *
	 * UWB configurations are define in chapter
	 * "21.4 UWB Frame Elements" of the CCC
	 * specification. Configuration 0x0000 is
	 * mandatory for device and vehicle, configuration
	 * 0x0001 is mandatory for the device, optional for
	 * the vehicle.
	 */
	AOSP_CAPS_CCC_SUPPORTED_UWB_CONFIGS = 0xA5,
	/**
	 * @AOSP_CAPS_CCC_SUPPORTED_PULSE_SHAPE_COMBOS: supported CCC pulse shape
	 *		combo.
	 *
	 * 1 byte tuple {initiator_tx (4 bits), responder_tx (4 bits)} array with
	 * list of supported pulse shape combos
	 *
	 * Values:
	 *  PULSE_SHAPE_SYMMETRICAL_ROOT_RAISED_COSINE = 1
	 *  PULSE_SHAPE_PRECURSOR_FREE = 2
	 *  PULSE_SHAPE_PRECURSOR_FREE_SPECIAL = 3
	 */
	AOSP_CAPS_CCC_SUPPORTED_PULSE_SHAPE_COMBOS = 0xA6,
	/**
	 * @AOSP_CAPS_CCC_SUPPORTED_RAN_MULTIPLIER: Int value for indicating
	 *		supported ran multiplier.
	 */
	AOSP_CAPS_CCC_SUPPORTED_RAN_MULTIPLIER = 0xA7,
	/**
	 * @AOSP_CAPS_SUPPORTED_AOA_RESULT_REQ_ANTENNA_INTERLEAVING: 1 byte value
	 *		to indicate support for antenna interleaving feature.
	 *
	 * Values:
	 *  1 - Feature supported.
	 *  0 - Feature not supported.
	 */
	AOSP_CAPS_SUPPORTED_AOA_RESULT_REQ_ANTENNA_INTERLEAVING = 0xE3,
};

/**
 * enum uci_diagnostic_notif_frame_report_field - Vendor diagnostic report fields
 *
 * @UCI_DIAG_REPORT_RSSIS: type of report is rssis
 * @UCI_DIAG_REPORT_AOAS: type of report is aoas
 * @UCI_DIAG_REPORT_CIRS: type of report is cirs
 *
 */
enum uci_diagnostic_notif_frame_report_field {
	UCI_DIAG_REPORT_RSSIS = 0x00,
	UCI_DIAG_REPORT_AOAS = 0x01,
	UCI_DIAG_REPORT_CIRS = 0x02,
};

#endif /* UCI_SPEC_FIRA_H */
