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

#ifndef UWBMAC_REPORT_H
#define UWBMAC_REPORT_H

#include <linux/skbuff.h>
#include <linux/types.h>
#include <net/mac802154.h>
#include <uwbmac/uwbmac_buf.h>

struct report_data {
	u32 port_id;
	struct uwbmac_buf *report;
};

/**
 * uwbmac_region_call_reply() - Reply to a region call.
 * @hw: Pointer to MCPS hw instance.
 * @reply: Reply message.
 *
 * Return: 0 or error.
 */
int uwbmac_region_call_reply(struct ieee802154_hw *hw, struct sk_buff *reply);

/**
 * uwbmac_event_report() - Report an event.
 * @hw: Pointer to MCPS hw instance.
 * @port_id: Port id to use to notify upper layer.
 * @report: Event report.
 *
 * Return: 0 or error.
 */
int uwbmac_event_report(struct ieee802154_hw *hw, u32 port_id,
			struct sk_buff *report);

/**
 * uwbmac_handle_report() - Handle reported event.
 * @report: Event report.
 *
 * NOTE: This method is only used by embedded flavor. It must be used
 * by the platform to process an event received from the MAC.
 * It is recommended to handle events from a dedicated thread,
 * as processing the event may imply reentrancy in the MAC.
 *
 */
void uwbmac_handle_report(struct report_data *report);

/**
 * uwbmac_testmode_reply() - Reply to a testmode call.
 * @hw: Pointer to MCPS hw instance.
 * @reply: Reply message.
 *
 * NOTE: This method is only used by embedded flavor.
 *
 * Return: 0 or error.
 */
int uwbmac_testmode_reply(struct ieee802154_hw *hw, struct uwbmac_buf *reply);

#endif /* UWBMAC_REPORT_H */
