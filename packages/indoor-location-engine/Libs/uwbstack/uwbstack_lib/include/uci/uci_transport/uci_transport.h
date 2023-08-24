/*
 * Copyright (c) 2021–2022 Qorvo, Inc
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

#ifndef UWB_STACK_UCI_INCLUDE_UCI_UCI_TRANSPORT_UART_H_
#define UWB_STACK_UCI_INCLUDE_UCI_UCI_TRANSPORT_UART_H_

#include <uci/uci.h>

typedef void (*receive_callback)(void);

struct uci_transport *uci_transport_get();
void uci_transport_init(receive_callback cb);
int uci_transport_read();

#endif /* UWB_STACK_UCI_INCLUDE_UCI_UCI_TRANSPORT_UART_H_ */
