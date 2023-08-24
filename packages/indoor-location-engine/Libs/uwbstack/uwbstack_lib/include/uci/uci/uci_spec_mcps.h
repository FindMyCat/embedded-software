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

#ifndef UCI_SPEC_MCPS_H
#define UCI_SPEC_MCPS_H

#define UCI_MCPS_GID 0xc

/**
 * enum uci_mcps_oid - MAC calls identifiers through UCI.
 *
 * @MAC_START:
 *	Start device.
 * @MAC_STOP:
 *	Stop device.
 * @MAC_TX:
 *	Send data.
 * @MAC_TX_DONE:
 *	Data sent notification.
 * @MAC_TX_QUEUE:
 *	Send queue notification.
 * @MAC_RX:
 *	Data reception notification.
 * @MAC_RX_QUEUE:
 *	Control reception queue.
 * @MAC_SCAN:
 *	Set scanning mode.
 * @MAC_SET_SCHEDULER:
 *	Set scheduler.
 * @MAC_GET_SCHEDULER:
 *	Get scheduler.
 * @MAC_SET_SCHEDULER_PARAMS:
 *	Set scheduler parameters.
 * @MAC_GET_SCHEDULER_PARAMS:
 *	Get scheduler parameters.
 * @MAC_SET_SCHEDULER_REGIONS:
 *	Change scheduler regions.
 * @MAC_GET_SCHEDULER_REGIONS:
 *	Get scheduler regions.
 * @MAC_CALL_SCHEDULER:
 *	Call to scheduler.
 * @MAC_SET_REGION_PARAMS:
 *	Set region parameters.
 * @MAC_GET_REGION_PARAMS:
 *	Get region parameters.
 * @MAC_CALL_REGION:
 *	Call to region.
 * @MAC_SET_CALIBRATIONS:
 *	Set calibration values.
 * @MAC_GET_CALIBRATIONS:
 *	Get calibration values.
 * @MAC_LIST_CALIBRATIONS:
 *	List calibration keys.
 * @MAC_CLOSE_SCHEDULER:
 *	Close scheduler and regions.
 * @MAC_TESTMODE:
 *	Call test mode function.
 * @MAC_CALL_MAX:
 *	Internal use.
 */
enum uci_mcps_oid {
	MAC_START = 0x00,
	MAC_STOP = 0x01,
	MAC_TX = 0x02,
	MAC_TX_DONE = 0x03,
	MAC_TX_QUEUE = 0x04,
	MAC_RX = 0x05,
	MAC_RX_QUEUE = 0x06,
	MAC_SCAN = 0x07,
	MAC_SET_SCHEDULER = 0x20,
	MAC_GET_SCHEDULER = 0x21,
	MAC_SET_SCHEDULER_PARAMS = 0x22,
	MAC_GET_SCHEDULER_PARAMS = 0x23,
	MAC_SET_SCHEDULER_REGIONS = 0x24,
	MAC_GET_SCHEDULER_REGIONS = 0x25,
	MAC_CALL_SCHEDULER = 0x26,
	MAC_SET_REGION_PARAMS = 0x27,
	MAC_GET_REGION_PARAMS = 0x28,
	MAC_CALL_REGION = 0x29,
	MAC_SET_CALIBRATIONS = 0x2a,
	MAC_GET_CALIBRATIONS = 0x2b,
	MAC_LIST_CALIBRATIONS = 0x2c,
	MAC_CLOSE_SCHEDULER = 0x2d,
	MAC_TESTMODE = 0x3f,
	MAC_CALL_MAX,
};

/**
 * enum device_config - Device configurations that are set through CORE_SET_CONFIG cmd.
 *
 * @DEVICE_CHANNEL:
 *	The device default channel.
 * @DEVICE_PREAMBLE_CODE:
 *  The device default preamble code.
 * @DEVICE_PAN_ID:
 *	The identifier of the PAN on which the device is operating, if 0xffff, device is not associated.
 * @DEVICE_SHORT_ADDR:
 *	The address the device uses to communicate in the PAN.
 * @DEVICE_EXTENDED_ADDR:
 *	The extended address assigned to the device.
 * @DEVICE_PAN_COORD:
 *  Device is a PAN coordinator if 1.
 * @DEVICE_PROMISCUOUS:
 *  Enable (1) or disable (0) promiscuous mode.
 * @DEVICE_FRAME_RETRIES:
 *	The maximum number of retries after a transmission failure.
 */
enum device_config {
	DEVICE_CHANNEL = 0xa0,
	DEVICE_PREAMBLE_CODE,
	DEVICE_PAN_ID,
	DEVICE_SHORT_ADDR,
	DEVICE_EXTENDED_ADDR,
	DEVICE_PAN_COORD,
	DEVICE_PROMISCUOUS,
	DEVICE_FRAME_RETRIES,
};

#endif /* UCI_SPEC_MCPS_H */
