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

#ifndef NET_MCPS802154_NL_DEFS_H
#define NET_MCPS802154_NL_DEFS_H

#define MCPS802154_GENL_NAME "mcps802154"

/**
 * enum mcps802154_commands - MCPS net link commands.
 *
 * @MCPS802154_CMD_GET_HW:
 *	Request information about a device, or dump request to get a list of all
 *	devices.
 * @MCPS802154_CMD_NEW_HW:
 *	Result from information request.
 * @MCPS802154_CMD_SET_SCHEDULER:
 *	Set the scheduler used to manage the schedule.
 * @MCPS802154_CMD_SET_SCHEDULER_PARAMS:
 *	Set the parameters of the current scheduler.
 * @MCPS802154_CMD_CALL_SCHEDULER:
 *	Call specific scheduler procedure.
 * @MCPS802154_CMD_SET_SCHEDULER_REGIONS:
 *	Set and configure the scheduler regions.
 * @MCPS802154_CMD_SET_REGIONS_PARAMS:
 *	Set the parameters of a specific region.
 * @MCPS802154_CMD_CALL_REGION:
 *	Call specific region procedure.
 * @MCPS802154_CMD_SET_CALIBRATIONS:
 *	Attempts to write the given value to the indicated calibration item.
 * @MCPS802154_CMD_GET_CALIBRATIONS:
 *	Requests information about a given calibration items.
 * @MCPS802154_CMD_LIST_CALIBRATIONS:
 *	Reports all calibrations existing.
 * @MCPS802154_CMD_TESTMODE:
 *	Run a testmode command with TESTDATA blob attribute to pass through
 *	to the driver.
 * @MCPS802154_CMD_CLOSE_SCHEDULER:
 *	Close current scheduler and its regions.
 * @MCPS802154_CMD_GET_PWR_STATS:
 *	Get the power statistics.
 *
 * @MCPS802154_CMD_UNSPEC: Invalid command.
 * @__MCPS802154_CMD_AFTER_LAST: Internal use.
 * @MCPS802154_CMD_MAX: Internal use.
 */
enum mcps802154_commands {
	MCPS802154_CMD_UNSPEC,

	MCPS802154_CMD_GET_HW, /* can dump */
	MCPS802154_CMD_NEW_HW,
	MCPS802154_CMD_SET_SCHEDULER,
	MCPS802154_CMD_SET_SCHEDULER_PARAMS,
	MCPS802154_CMD_CALL_SCHEDULER,
	MCPS802154_CMD_SET_SCHEDULER_REGIONS,
	MCPS802154_CMD_SET_REGIONS_PARAMS,
	MCPS802154_CMD_CALL_REGION,
	MCPS802154_CMD_SET_CALIBRATIONS,
	MCPS802154_CMD_GET_CALIBRATIONS,
	MCPS802154_CMD_LIST_CALIBRATIONS,
	MCPS802154_CMD_TESTMODE,
	MCPS802154_CMD_CLOSE_SCHEDULER,
	MCPS802154_CMD_GET_PWR_STATS,
	__MCPS802154_CMD_AFTER_LAST,
	MCPS802154_CMD_MAX = __MCPS802154_CMD_AFTER_LAST - 1
};

/**
 * enum mcps802154_attrs - Top level MCPS net link attributes.
 *
 * @MCPS802154_ATTR_HW:
 *	Hardware device index, internal to MCPS.
 * @MCPS802154_ATTR_WPAN_PHY_NAME:
 *	Name of corresponding wpan_phy device.
 * @MCPS802154_ATTR_SCHEDULER_NAME:
 *	Name of the scheduler.
 * @MCPS802154_ATTR_SCHEDULER_PARAMS:
 *	Parameters of the scheduler.
 * @MCPS802154_ATTR_SCHEDULER_REGIONS:
 *	Parameters of the regions.
 * @MCPS802154_ATTR_SCHEDULER_CALL:
 *	Call id of the scheduler's procedure, scheduler specific.
 * @MCPS802154_ATTR_SCHEDULER_CALL_PARAMS:
 *	Parameters of the scheduler's procedure, scheduler specific.
 * @MCPS802154_ATTR_SCHEDULER_REGION_CALL:
 *	Parameters of the region call, scheduler specific.
 * @MCPS802154_ATTR_TESTDATA:
 *	Testmode's data blob, passed through to the driver. It contains
 *	driver-specific attributes.
 * @MCPS802154_ATTR_CALIBRATIONS:
 *	Nested array of calibrations.
 * @MCPS802154_ATTR_PWR_STATS:
 * 	Nested power statistics data.
 *
 * @MCPS802154_ATTR_UNSPEC: Invalid command.
 * @__MCPS802154_ATTR_AFTER_LAST: Internal use.
 * @MCPS802154_ATTR_MAX: Internal use.
 */
enum mcps802154_attrs {
	MCPS802154_ATTR_UNSPEC,

	MCPS802154_ATTR_HW,
	MCPS802154_ATTR_WPAN_PHY_NAME,

	MCPS802154_ATTR_SCHEDULER_NAME,
	MCPS802154_ATTR_SCHEDULER_PARAMS,
	MCPS802154_ATTR_SCHEDULER_REGIONS,

	MCPS802154_ATTR_SCHEDULER_CALL,
	MCPS802154_ATTR_SCHEDULER_CALL_PARAMS,

	MCPS802154_ATTR_SCHEDULER_REGION_CALL,

	MCPS802154_ATTR_TESTDATA,

	MCPS802154_ATTR_CALIBRATIONS,

	MCPS802154_ATTR_PWR_STATS,

	__MCPS802154_ATTR_AFTER_LAST,
	MCPS802154_ATTR_MAX = __MCPS802154_ATTR_AFTER_LAST - 1
};

/**
 * enum mcps802154_region_attrs - Regions attributes.
 *
 * @MCPS802154_REGION_ATTR_ID:
 *	ID of the region, scheduler specific.
 * @MCPS802154_REGION_ATTR_NAME:
 *	Name of the region, caller specific.
 * @MCPS802154_REGION_ATTR_PARAMS:
 *	Parameters of the region.
 * @MCPS802154_REGION_ATTR_CALL:
 *	Call id of the region's procedure, scheduler specific.
 * @MCPS802154_REGION_ATTR_CALL_PARAMS:
 *	Parameters of the region's procedure, scheduler specific.
 *
 * @MCPS802154_REGION_UNSPEC: Invalid command.
 * @__MCPS802154_REGION_AFTER_LAST: Internal use.
 * @MCPS802154_REGION_MAX: Internal use.
 */
enum mcps802154_region_attrs {
	MCPS802154_REGION_UNSPEC,

	MCPS802154_REGION_ATTR_ID,
	MCPS802154_REGION_ATTR_NAME,
	MCPS802154_REGION_ATTR_PARAMS,
	MCPS802154_REGION_ATTR_CALL,
	MCPS802154_REGION_ATTR_CALL_PARAMS,

	__MCPS802154_REGION_AFTER_LAST,
	MCPS802154_REGION_MAX = __MCPS802154_REGION_AFTER_LAST - 1
};

/**
 * enum mcps802154_calibrations_attrs - Calibration item.
 *
 * @MCPS802154_CALIBRATIONS_ATTR_KEY:
 *	Calibration name.
 * @MCPS802154_CALIBRATIONS_ATTR_VALUE:
 *	Calibration value.
 * @MCPS802154_CALIBRATIONS_ATTR_STATUS:
 *	Status in case of error on write or read.
 *
 * @MCPS802154_CALIBRATIONS_ATTR_UNSPEC: Invalid command.
 * @__MCPS802154_CALIBRATIONS_ATTR_AFTER_LAST: Internal use.
 * @MCPS802154_CALIBRATIONS_ATTR_MAX: Internal use.
 */
enum mcps802154_calibrations_attrs {
	MCPS802154_CALIBRATIONS_ATTR_UNSPEC,

	MCPS802154_CALIBRATIONS_ATTR_KEY,
	MCPS802154_CALIBRATIONS_ATTR_VALUE,
	MCPS802154_CALIBRATIONS_ATTR_STATUS,

	__MCPS802154_CALIBRATIONS_ATTR_AFTER_LAST,
	MCPS802154_CALIBRATIONS_ATTR_MAX =
		__MCPS802154_CALIBRATIONS_ATTR_AFTER_LAST - 1
};

/**
 * enum mcps802154_nl_pwr_stats_state_attrs - Power state item.
 *
 * @MCPS802154_PWR_STATS_STATE_ATTR_TIME:
 * 	Time spent in this state.
 * @MCPS802154_PWR_STATS_STATE_ATTR_COUNT:
 * 	Number of transitions to this state.
 * @MCPS802154_PWR_STATS_STATE_ATTR_UNSPEC: Invalid command.
 * @__MCPS802154_PWR_STATS_STATE_ATTR_AFTER_LAST: Internal use.
 * @MCPS802154_PWR_STATS_STATE_ATTR_MAX: Internal use.
 */
enum mcps802154_nl_pwr_stats_state_attrs {
	MCPS802154_PWR_STATS_STATE_ATTR_UNSPEC,

	MCPS802154_PWR_STATS_STATE_ATTR_TIME,
	MCPS802154_PWR_STATS_STATE_ATTR_COUNT,

	__MCPS802154_PWR_STATS_STATE_ATTR_AFTER_LAST,
	MCPS802154_PWR_STATS_STATE_ATTR_MAX =
		__MCPS802154_PWR_STATS_STATE_ATTR_AFTER_LAST - 1
};

/**
 * enum mcps802154_pwr_stats_attrs - Power statistics item.
 *
 * @MCPS802154_PWR_STATS_ATTR_SLEEP:
 * 	Sleep state nested attribute.
 * @MCPS802154_PWR_STATS_ATTR_IDLE:
 * 	Idle state nested attribute.
 * @MCPS802154_PWR_STATS_ATTR_RX:
 * 	Rx state nested attribute.
 * @MCPS802154_PWR_STATS_ATTR_TX:
 * 	Tx state nested attribute.
 * @MCPS802154_PWR_STATS_ATTR_INTERRUPTS:
 * 	Interrupts count attribute.
 * @MCPS802154_PWR_STATS_ATTR_UNSPEC: Invalid command.
 * @__MCPS802154_PWR_STATS_ATTR_AFTER_LAST: Internal use.
 * @MCPS802154_PWR_STATS_ATTR_MAX: Internal use.
 */
enum mcps802154_pwr_stats_attrs {
	MCPS802154_PWR_STATS_ATTR_UNSPEC,

	MCPS802154_PWR_STATS_ATTR_SLEEP,
	MCPS802154_PWR_STATS_ATTR_IDLE,
	MCPS802154_PWR_STATS_ATTR_RX,
	MCPS802154_PWR_STATS_ATTR_TX,
	MCPS802154_PWR_STATS_ATTR_INTERRUPTS,

	__MCPS802154_PWR_STATS_ATTR_AFTER_LAST,
	MCPS802154_PWR_STATS_ATTR_MAX =
		__MCPS802154_PWR_STATS_ATTR_AFTER_LAST - 1
};

#endif /* NET_MCPS802154_NL_DEFS_H */
