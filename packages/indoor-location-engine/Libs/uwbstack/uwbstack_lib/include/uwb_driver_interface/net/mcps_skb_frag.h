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

#include <linux/skbuff.h>

/**
 * mcps_skb_frags_len() - Return the total length of the fragments attached to this buffer.
 * @skb: Pointer to buffer.
 *
 * Return: Attached fragments length.
 *
 * NOTE: The parent length is NOT included in the computed value
 */
int mcps_skb_frags_len(struct sk_buff *skb);
