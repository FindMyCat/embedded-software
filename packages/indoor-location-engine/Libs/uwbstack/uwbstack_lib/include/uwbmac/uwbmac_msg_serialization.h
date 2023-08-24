/*
 * Copyright (c) 2022 Qorvo, Inc
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

#ifndef UWBMAC_MSG_SERIALIZATION_H
#define UWBMAC_MSG_SERIALIZATION_H

#include <uwbmac/uwbmac_msg.h>
#include <cbor/cbor.h>

int uwbmac_msg_serialize(struct uwbmac_msg *msg, struct cbor_state *cbor_state);

int uwbmac_msg_unserialize(struct uwbmac_msg *msg,
			   struct cbor_state *cbor_state);

#endif // UWBMAC_MSG_SERIALIZATION_H
