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

#ifndef COMPAT_NET_CFG802154_H
#define COMPAT_NET_CFG802154_H

struct wpan_phy_cca {
};

struct wpan_phy_supported {
	u32 channels[5];
};

struct wpan_phy {
	u8 current_page;
	u8 current_channel;
	struct wpan_phy_supported supported;
	__le64 perm_extended_addr;
};

static inline const char *
wpan_phy_name(__attribute__((unused)) struct wpan_phy *phy)
{
	return "phy0";
}

#endif /* COMPAT_NET_CFG802154_H */
