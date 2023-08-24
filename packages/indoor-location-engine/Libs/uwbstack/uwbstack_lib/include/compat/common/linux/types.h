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

#ifndef COMPAT_LINUX_TYPES_H
#define COMPAT_LINUX_TYPES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint16_t __le16;
typedef uint32_t __le32;
typedef uint64_t __le64;

typedef uint8_t u8;
typedef uint8_t __u8;
typedef int8_t s8;
typedef int8_t __s8;
typedef uint16_t u16;
typedef uint16_t __u16;
typedef int16_t s16;
typedef int16_t __s16;
typedef uint32_t u32;
typedef uint32_t __u32;
typedef int32_t s32;
typedef int32_t __s32;
typedef uint64_t u64;
typedef uint64_t __u64;
typedef int64_t s64;
typedef int64_t __s64;

typedef int gfp_t;

#endif /* COMPAT_LINUX_TYPES_H */
