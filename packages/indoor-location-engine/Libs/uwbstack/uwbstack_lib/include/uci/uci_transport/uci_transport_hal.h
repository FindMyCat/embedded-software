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

#ifndef UCI_TRANSPORT_HAL_H
#define UCI_TRANSPORT_HAL_H

#include <stdlib.h>
#include <stdint.h>

#include "uci/uci.h"

typedef ssize_t (*on_uci_message_t)(const uint8_t buffer[], size_t length,
				    void *user_data);
/**
 * uci_transport_hal_create() - Create a transport object for the Android HAL.
 * @on_uci_message: Callback in charge of sending bytes.
 * @user_data: User data to send to the callback.
 *
 * Return: Transport object or null on error.
 */
struct uci_transport *uci_transport_hal_create(on_uci_message_t on_uci_message,
					       void *user_data);

/**
 * uci_transport_hal_destroy() - Destroy a serial transport object.
 * @transport: Transport to destroy.
 */
void uci_transport_hal_destroy(struct uci_transport *transport);

/**
 * uci_transport_hal_read() - Read a packet from the transport.
 * @transport: Transport to read from.
 * @data: The buffer where the bytes read are stored.
 * @length: The size of the data buffer.
 *
 * Return: bytes read or a negative error.
 */
int uci_transport_hal_read(struct uci_transport *transport,
			   const uint8_t data[], size_t length);

#endif // UCI_TRANSPORT_HAL_H
