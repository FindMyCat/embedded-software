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

#ifndef COMPAT_LINUX_BITS_H
#define COMPAT_LINUX_BITS_H

#ifndef BIT
#define BIT(n) (1ul << (n))
#endif
#ifndef BIT_ULL
#define BIT_ULL(n) (1ull << (n))
#endif

#endif /* COMPAT_LINUX_BITS_H */
