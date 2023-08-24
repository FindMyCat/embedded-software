/*
 * Copyright (c) 2021 Qorvo, Inc
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

#ifndef UCI_COMPAT_H
#define UCI_COMPAT_H

#ifdef __KERNEL__
#include <linux/types.h>
#include <linux/string.h>
#else
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <string.h>
#endif

#ifdef __KERNEL__
#include <linux/kernel.h>
#include <linux/bug.h>

#define UCI_ASSERT(x)                                                       \
	do {                                                                \
		if (WARN(!(x), "### UCI ASSERTION FAILED %s: %s: %d: %s\n", \
			 __FILE__, __func__, __LINE__, #x))                 \
			dump_stack();                                       \
	} while (0)

#else

#include <assert.h>
#define UCI_ASSERT(x) assert(x)

#endif

#endif /* UCI_COMPAT_H */
