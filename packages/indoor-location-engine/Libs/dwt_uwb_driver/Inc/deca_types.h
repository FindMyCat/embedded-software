/*! ----------------------------------------------------------------------------
 *  @file   deca_types.h
 *  @brief  Decawave general type definitions
 *
 * @attention
 *
 * Copyright 2013 - 2021 (c) Decawave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 */

#ifndef _DECA_TYPES_D0_H_
#define _DECA_TYPES_D0_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>
#include <stdint.h>

#ifdef STM32F429xx
#ifndef uint8_t
#ifndef _DECA_UINT8_
#define _DECA_UINT8_
    typedef unsigned char uint8_t;
#endif
#endif

#ifndef uint16_t
#ifndef _DECA_UINT16_
#define _DECA_UINT16_
    typedef unsigned short uint16_t;
#endif
#endif

#ifndef uint32_t
#ifndef _DECA_UINT32_
#define _DECA_UINT32_
    typedef unsigned long uint32_t;
#endif
#endif

#ifndef int8_t
#ifndef _DECA_INT8_
#define _DECA_INT8_
    typedef signed char int8_t;
#endif
#endif

#ifndef int16_t
#ifndef _DECA_INT16_
#define _DECA_INT16_
    typedef signed short int16_t;
#endif
#endif

#ifndef int32_t
#ifndef _DECA_INT32_
#define _DECA_INT32_
    typedef signed long int32_t;
#endif
#endif

#ifndef uint64_t
#ifndef _DECA_UINT64_
#define _DECA_UINT64_
    typedef unsigned long long uint64_t;
#endif
#endif

#ifndef int64_t
#ifndef _DECA_INT64_
#define _DECA_INT64_
    typedef signed long long int64_t;
#endif
#endif
#endif // STM32F429xx

#ifndef NULL
#define NULL ((void *)0UL)
#endif

#ifdef __cplusplus
}
#endif

#endif /* DECA_TYPES_H_ */
