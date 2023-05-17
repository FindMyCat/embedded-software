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

#ifndef UWBMAC_ERROR_H
#define UWBMAC_ERROR_H

/**
 * enum uwbmac_error - Return values for most uwbmac functions.
 * @UWBMAC_SUCCESS: Operation successful.
 * @UWBMAC_EADDRNOTAVAIL: Address not available.
 * @UWBMAC_EAFNOSUPPORT: Address family not supported.
 * @UWBMAC_EAGAIN: Resource temporarily unavailable.
 * @UWBMAC_EBADF: Bad file descriptor.
 * @UWBMAC_EBADMSG: Bad message.
 * @UWBMAC_EBUSY: Device or resource busy.
 * @UWBMAC_ECONNREFUSED: Connection refused.
 * @UWBMAC_EEXIST: File exists.
 * @UWBMAC_EFAULT: Bad address.
 * @UWBMAC_EINTR: Interrupted system call.
 * @UWBMAC_EINVAL: Invalid argument.
 * @UWBMAC_EIO: I/O error.
 * @UWBMAC_EMSGSIZE: Message too long.
 * @UWBMAC_ENETDOWN: Network is down.
 * @UWBMAC_ENOBUFS : No buffer space available.
 * @UWBMAC_ENOENT: No such region or scheduler.
 * @UWBMAC_ENOMEM: Not enough memory.
 * @UWBMAC_ENOTSUP: Operation not supported.
 * @UWBMAC_EPERM: Permission denied.
 * @UWBMAC_EPIPE: Broken pipe.
 * @UWBMAC_EPROTO: Protocol error.
 * @UWBMAC_EPROTONOSUPPORT: Protocol not supported.
 * @UWBMAC_ERANGE: Result too large.
 * @UWBMAC_ETIME: Timer expired.
 * @UWBMAC_UCI_MSG_HANDLED: Handler wants to be the last one.
 * Handlers basically return either UCI_SUCCESS or an error value. This
 * particular value is similar to UCI_SUCCESS but will prevent further handlers
 * from being invoked.
*/
typedef enum {
	UWBMAC_SUCCESS = 0,
	UWBMAC_EADDRNOTAVAIL = -1,
	UWBMAC_EAFNOSUPPORT = -2,
	UWBMAC_EAGAIN = -3,
	UWBMAC_EBADF = -4,
	UWBMAC_EBADMSG = -5,
	UWBMAC_EBUSY = -6,
	UWBMAC_ECONNREFUSED = -7,
	UWBMAC_EEXIST = -8,
	UWBMAC_EFAULT = -9,
	UWBMAC_EINTR = -10,
	UWBMAC_EINVAL = -11,
	UWBMAC_EIO = -12,
	UWBMAC_EMSGSIZE = -13,
	UWBMAC_ENETDOWN = -14,
	UWBMAC_ENOBUFS = -15,
	UWBMAC_ENOENT = -16,
	UWBMAC_ENOMEM = -17,
	UWBMAC_ENOTSUP = -18,
	UWBMAC_EPERM = -19,
	UWBMAC_EPIPE = -20,
	UWBMAC_EPROTO = -21,
	UWBMAC_EPROTONOSUPPORT = -22,
	UWBMAC_ERANGE = -23,
	UWBMAC_ETIME = -24,
	UWBMAC_UCI_MSG_HANDLED = -255,
} uwbmac_error;

#endif /* UWBMAC_ERROR_H */
