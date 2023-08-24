/*! ----------------------------------------------------------------------------
 * @file    deca_version.h
 * @brief   Defines the version info for the DW3XXX device driver including its API
 *
 * @attention
 *
 * Copyright 2017 - 2021 (c) Decawave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 */

#ifndef _DECA_VERSION_DW3XXX_H_
#define _DECA_VERSION_DW3XXX_H_ 1

//
// The DW3XXX device driver is separately version numbered to any version the application using it may have
//
// Two symbols are defined here: one hexadecimal value and one string that includes the hex bytes.
// Both should be updated together in a consistent way when the software is being modified.
//
// The format of the hex version is 0xAABBCC and the string ends with AA.BB.CC, where...
//
// Quantity CC is updated for minor changes/bug fixes that should not need user code changes
// Quantity BB is updated for changes/bug fixes that may need user code changes
// Quantity AA is updated for major changes that will need user code changes
//

#define DRIVER_NAME        "DW3XXX"

#define DRIVER_VERSION_HEX 0x060014
#define DRIVER_VERSION_STR "DW3XXX Device Driver Version 06.00.14"

#endif
