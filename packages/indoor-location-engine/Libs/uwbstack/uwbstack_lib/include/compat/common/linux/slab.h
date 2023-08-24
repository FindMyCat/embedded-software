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

#ifndef COMPAT_LINUX_SLAB_H
#define COMPAT_LINUX_SLAB_H

#include <linux/string.h>
#include <linux/types.h>
#include <stdlib.h>
#include <string.h>

/* NOTE: This assumes the size of the allocation is the size of the pointee
 * type. This is currently always the case in the UWB stack but nothing
 * enforces this. */
#define kfree_sensitive(arg)                             \
	do {                                             \
		memzero_explicit((arg), sizeof(*(arg))); \
		free((arg));                             \
	} while (0)

enum gfp_e {
	GFP_KERNEL,
};

static inline void *kmalloc(size_t size, __attribute__((unused)) gfp_t flags)
{
	return malloc(size);
}

static inline void *kzalloc(size_t size, __attribute__((unused)) gfp_t flags)
{
	return calloc(1, size);
}

static inline void kfree(void *ptr)
{
	free(ptr);
}

static inline void *krealloc(void *ptr, size_t size,
			     __attribute__((unused)) gfp_t flags)
{
	return realloc(ptr, size);
}

#endif /* COMPAT_LINUX_SLAB_H */
