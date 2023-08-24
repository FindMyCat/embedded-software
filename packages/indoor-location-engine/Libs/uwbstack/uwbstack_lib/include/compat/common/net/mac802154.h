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

#ifndef COMPAT_NET_MAC802154_H
#define COMPAT_NET_MAC802154_H

#include <linux/types.h>
#include <linux/bits.h>
#include <linux/skbuff.h>
#include <net/cfg802154.h>

/* Structure ieee802154_ops below have support for scanning. */
#define CONFIG_HAVE_IEEE802154_SCANNING 1

enum ieee802154_hw_addr_filt_flags {
	IEEE802154_AFILT_SADDR_CHANGED = BIT(0),
	IEEE802154_AFILT_IEEEADDR_CHANGED = BIT(1),
	IEEE802154_AFILT_PANID_CHANGED = BIT(2),
	IEEE802154_AFILT_PANC_CHANGED = BIT(3),
};

struct ieee802154_hw_addr_filt {
	__le64 ieee_addr;
	__le16 pan_id;
	__le16 short_addr;
	bool pan_coord;
};

enum ieee802154_hw_flags {
	IEEE802154_HW_TX_OMIT_CKSUM = BIT(0),
	IEEE802154_HW_LBT = BIT(1),
	IEEE802154_HW_CSMA_PARAMS = BIT(2),
	IEEE802154_HW_FRAME_RETRIES = BIT(3),
	IEEE802154_HW_AFILT = BIT(4),
	IEEE802154_HW_PROMISCUOUS = BIT(5),
	IEEE802154_HW_RX_OMIT_CKSUM = BIT(6),
	IEEE802154_HW_RX_DROP_BAD_CKSUM = BIT(7),
};

#define IEEE802154_HW_OMIT_CKSUM \
	(IEEE802154_HW_TX_OMIT_CKSUM | IEEE802154_HW_RX_OMIT_CKSUM)

struct ieee802154_hw {
	int extra_tx_headroom;
	u32 flags;
	struct device *parent;
	void *priv;
	struct wpan_phy *phy;
};

struct ieee802154_ops {
	struct module *owner;
	int (*start)(struct ieee802154_hw *hw);
	void (*stop)(struct ieee802154_hw *hw);
	int (*xmit_sync)(struct ieee802154_hw *hw, struct sk_buff *skb);
	int (*xmit_async)(struct ieee802154_hw *hw, struct sk_buff *skb);
	int (*ed)(struct ieee802154_hw *hw, u8 *level);
	int (*set_channel)(struct ieee802154_hw *hw, u8 page, u8 channel);
	int (*set_hw_addr_filt)(struct ieee802154_hw *hw,
				struct ieee802154_hw_addr_filt *filt,
				unsigned long changed);
	int (*set_txpower)(struct ieee802154_hw *hw, s32 mbm);
	int (*set_lbt)(struct ieee802154_hw *hw, bool on);
	int (*set_cca_mode)(struct ieee802154_hw *hw,
			    const struct wpan_phy_cca *cca);
	int (*set_cca_ed_level)(struct ieee802154_hw *hw, s32 mbm);
	int (*set_csma_params)(struct ieee802154_hw *hw, u8 min_be, u8 max_be,
			       u8 retries);
	int (*set_frame_retries)(struct ieee802154_hw *hw, s8 retries);
	int (*set_promiscuous_mode)(struct ieee802154_hw *hw, const bool on);
	void (*sw_scan_start)(struct ieee802154_hw *hw, __le64 addr);
	void (*sw_scan_complete)(struct ieee802154_hw *hw);
};

struct ieee802154_hw *ieee802154_alloc_hw(size_t priv_data_len,
					  const struct ieee802154_ops *ops);

void ieee802154_free_hw(struct ieee802154_hw *hw);

int ieee802154_register_hw(struct ieee802154_hw *hw);

void ieee802154_unregister_hw(struct ieee802154_hw *hw);

void ieee802154_rx_irqsafe(struct ieee802154_hw *hw, struct sk_buff *skb,
			   u8 lqi);

void ieee802154_wake_queue(struct ieee802154_hw *hw);

void ieee802154_xmit_complete(struct ieee802154_hw *hw, struct sk_buff *skb,
			      bool ifs_handling);

#endif /* COMPAT_NET_MAC802154_H */
