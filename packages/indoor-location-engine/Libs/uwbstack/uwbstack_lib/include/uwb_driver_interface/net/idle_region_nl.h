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

#ifndef IDLE_REGION_NL_H
#define IDLE_REGION_NL_H

/**
 * enum idle_param_attrs - Idle parameters attributes.
 *
 * @IDLE_PARAM_ATTR_MIN_DURATION_DTU:
 *	Minimum duration of an access.
 * @IDLE_PARAM_ATTR_MAX_DURATION_DTU:
 *	Maximum duration of an access.
 *
 * @IDLE_PARAM_ATTR_UNSPEC: Invalid command.
 * @__IDLE_PARAM_ATTR_AFTER_LAST: Internal use.
 * @IDLE_PARAM_ATTR_MAX: Internal use.
 */
enum idle_param_attrs {
	IDLE_PARAM_ATTR_UNSPEC,

	IDLE_PARAM_ATTR_MIN_DURATION_DTU,
	IDLE_PARAM_ATTR_MAX_DURATION_DTU,

	__IDLE_PARAM_ATTR_AFTER_LAST,
	IDLE_PARAM_ATTR_MAX = __IDLE_PARAM_ATTR_AFTER_LAST - 1
};

#endif /* IDLE_REGION_NL_H */
