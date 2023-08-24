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

#ifndef UCI_TRANSPORT_SERIAL_H
#define UCI_TRANSPORT_SERIAL_H

#include "uci/uci.h"

/**
 * uci_transport_serial_create() - Create a serial transport object.
 * @path: Path to serial character device.
 * @baud: Baud rate of the serial port.
 * @fd: File decriptor for the serial device.
 *
 * Return: Transport object or null on error.
 */
struct uci_transport *uci_transport_serial_create(const char *path, int baud,
						  int *fd);

/**
 * uci_transport_serial_destroy() - Destroy a serial transport object.
 * @transport: Transport to destroy.
 */
void uci_transport_serial_destroy(struct uci_transport *transport);

/**
 * uci_transport_serial_read() - Read a packet from the transport.
 * @transport: Transport to read from.
 *
 * Return: UWBMAC_SUCCESS on success, or a negative error.
 */
uwbmac_error uci_transport_serial_read(struct uci_transport *transport);

#endif // UCI_TRANSPORT_SERIAL_H
