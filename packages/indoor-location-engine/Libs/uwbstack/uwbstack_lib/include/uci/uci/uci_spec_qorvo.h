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

#ifndef UCI_SPEC_QORVO_H
#define UCI_SPEC_QORVO_H

/**
 * enum uci_qorvo_oid - MAC calls identifiers through UCI.
 *
 * @QORVO_FIRA_RANGE_DIAGNOSTICS:
 *	Diagnostics notifications.
 */
enum uci_qorvo_oid {
	QORVO_FIRA_RANGE_DIAGNOSTICS = 0b0000011,
};

#endif /* UCI_SPEC_QORVO_H */
