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

#ifndef COMPAT_LINUX_STRING_H
#define COMPAT_LINUX_STRING_H

#include <string.h>

static inline void memzero_explicit(void *s, size_t count)
{
	memset(s, 0, count);
	__asm__ __volatile__("" : : "r"(s) : "memory");
}

#endif /* COMPAT_LINUX_STRING_H */
