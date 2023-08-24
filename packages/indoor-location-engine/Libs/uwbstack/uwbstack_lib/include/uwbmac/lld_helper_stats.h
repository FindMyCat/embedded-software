/*
 * Copyright (c) 2020–20212 Qorvo, Inc
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

#ifndef LLD_HELPER_STATS_H
#define LLD_HELPER_STATS_H

#include <stdint.h>
#include <stdbool.h>

#include "uwbmac/uwbmac.h"

/**
 * struct lld_mfs_stats - LLD MFS statistics.
 */
struct lld_mfs_stats {
	/**
	 * @rx_packets: Number of good data packets received.
	 */
	uint32_t rx_packets;
	/**
	 * @rx_bytes: Number of good received bytes, corresponding to
	 * rx_packets.
	 */
	uint32_t rx_bytes;
	/**
	 * @rx_errors: Number of bad data packets received.
	 */
	uint32_t rx_errors;
	/**
	 * @rx_ignored: Number of good data packets ignored because
	 * they are out of the reception window.
	 */
	uint32_t rx_ignored;
	/**
	 * @tx_packets: Number of data packets successfully transmitted
	 * (acknowledged successfully or no acknowledgement required).
	 */
	uint32_t tx_packets;
	/**
	 * @tx_bytes: Number of transmitted bytes, corresponding to
	 * tx_packet.
	 */
	uint32_t tx_bytes;
	/**
	 * @tx_errors: Number of data packets not transmitted due to problem
	 * with the packet.
	 */
	uint32_t tx_errors;
	/**
	 * @tx_dropped: Number of data packets dropped before successful
	 * transmission.
	 */
	uint32_t tx_dropped;
};

#endif //LLD_HELPER_STATS_H
