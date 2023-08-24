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

#ifndef COMPAT_LINUX_IEEE802154_H
#define COMPAT_LINUX_IEEE802154_H

#define IEEE802154_MTU 127
#define IEEE802154_FCS_LEN 2
#define IEEE802154_FC_LEN 2
#define IEEE802154_SEQ_LEN 1

#define IEEE802154_PAN_ID_BROADCAST 0xffff
#define IEEE802154_ADDR_SHORT_BROADCAST 0xffff
#define IEEE802154_ADDR_SHORT_UNSPEC 0xfffe

#define IEEE802154_EXTENDED_ADDR_LEN 8
#define IEEE802154_SHORT_ADDR_LEN 2
#define IEEE802154_PAN_ID_LEN 2

#define IEEE802154_FC_TYPE_BEACON 0
#define IEEE802154_FC_TYPE_DATA 1
#define IEEE802154_FC_TYPE_ACK 2
#define IEEE802154_FC_TYPE_MAC_CMD 3

#define IEEE802154_FC_TYPE_SHIFT 0
#define IEEE802154_FC_TYPE_MASK ((1 << 3) - 1)
#define IEEE802154_FC_TYPE(x) \
	(((x)&IEEE802154_FC_TYPE_MASK) >> IEEE802154_FC_TYPE_SHIFT)

#define IEEE802154_FC_SECEN_SHIFT 3
#define IEEE802154_FC_SECEN (1 << IEEE802154_FC_SECEN_SHIFT)
#define IEEE802154_FC_FRPEND_SHIFT 4
#define IEEE802154_FC_FRPEND (1 << IEEE802154_FC_FRPEND_SHIFT)
#define IEEE802154_FC_ACK_REQ_SHIFT 5
#define IEEE802154_FC_ACK_REQ (1 << IEEE802154_FC_ACK_REQ_SHIFT)
#define IEEE802154_FC_INTRA_PAN_SHIFT 6
#define IEEE802154_FC_INTRA_PAN (1 << IEEE802154_FC_INTRA_PAN_SHIFT)

#define IEEE802154_FC_SAMODE_SHIFT 14
#define IEEE802154_FC_DAMODE_SHIFT 10

#define IEEE802154_FC_VERSION_SHIFT 12

#define IEEE802154_SCF_SECLEVEL_ENC_MIC64 6

/* MAC's Command Frames Identifiers */
#define IEEE802154_CMD_ASSOCIATION_REQ 0x01
#define IEEE802154_CMD_ASSOCIATION_RESP 0x02
#define IEEE802154_CMD_DISASSOCIATION_NOTIFY 0x03
#define IEEE802154_CMD_DATA_REQ 0x04
#define IEEE802154_CMD_PANID_CONFLICT_NOTIFY 0x05
#define IEEE802154_CMD_ORPHAN_NOTIFY 0x06
#define IEEE802154_CMD_BEACON_REQ 0x07
#define IEEE802154_CMD_COORD_REALIGN_NOTIFY 0x08
#define IEEE802154_CMD_GTS_REQ 0x09

#endif /* COMPAT_LINUX_IEEE802154_H */
