/**
 * @file      fira_device_config.h
 *
 * @brief     fira configurations
 *
 * @author    Decawave Applications
 *
 * @attention Copyright (c) 2021 - 2022, Qorvo US, Inc.
 * All rights reserved
 * Redistribution and use in source and binary forms, with or without modification,
 *  are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this
 *  list of conditions, and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 * 3. You may only use this software, with or without any modification, with an
 *  integrated circuit developed by Qorvo US, Inc. or any of its affiliates
 *  (collectively, "Qorvo"), or any module that contains such integrated circuit.
 * 4. You may not reverse engineer, disassemble, decompile, decode, adapt, or
 *  otherwise attempt to derive or gain access to the source code to any software
 *  distributed under this license in binary or object code form, in whole or in
 *  part.
 * 5. You may not use any Qorvo name, trademarks, service marks, trade dress,
 *  logos, trade names, or other symbols or insignia identifying the source of
 *  Qorvo's products or services, or the names of any of Qorvo's developers to
 *  endorse or promote products derived from this software without specific prior
 *  written permission from Qorvo US, Inc. You must not call products derived from
 *  this software "Qorvo", you must not have "Qorvo" appear in their name, without
 *  the prior permission from Qorvo US, Inc.
 * 6. Qorvo may publish revised or new version of this license from time to time.
 *  No one other than Qorvo US, Inc. has the right to modify the terms applicable
 *  to the software provided under this license.
 * THIS SOFTWARE IS PROVIDED BY QORVO US, INC. "AS IS" AND ANY EXPRESS OR IMPLIED
 *  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. NEITHER
 *  QORVO, NOR ANY PERSON ASSOCIATED WITH QORVO MAKES ANY WARRANTY OR
 *  REPRESENTATION WITH RESPECT TO THE COMPLETENESS, SECURITY, RELIABILITY, OR
 *  ACCURACY OF THE SOFTWARE, THAT IT IS ERROR FREE OR THAT ANY DEFECTS WILL BE
 *  CORRECTED, OR THAT THE SOFTWARE WILL OTHERWISE MEET YOUR NEEDS OR EXPECTATIONS.
 * IN NO EVENT SHALL QORVO OR ANYBODY ASSOCIATED WITH QORVO BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 *
 */

#ifndef FIRA_DEVICE_CONFIG_H
#define FIRA_DEVICE_CONFIG_H

#include "niq.h"
#include "net/fira_region_params.h"
#include "fira_helper.h"
#include "app.h"

/*Defines from common_fira*/
#define FIRA_BPRF_SET_1 (1) //SP0 IEEE SFD
#define FIRA_BPRF_SET_2 (2) //SP0 4z SFD
#define FIRA_BPRF_SET_3 (3) //SP1 4z SFD
#define FIRA_BPRF_SET_4 (4) //SP3 4z SFD
#define FIRA_BPRF_SET_5 (5) //SP1 IEEE SFD
#define FIRA_BPRF_SET_6 (6) //SP3 IEEE SFD

#define FIRA_ROLE_MASK      0x01

#define FIRA_DEFAULT_DEVICE_CONFIG                               \
    {                                                            \
      .role = FIRA_DEVICE_ROLE_INITIATOR,                        \
      .enc_payload = 1,                                          \
      .Session_ID = 1111,                                        \
      .Ranging_Round_Usage = FIRA_RANGING_ROUND_USAGE_DSTWR,     \
      .Multi_Node_Mode = FIRA_MULTI_NODE_MODE_UNICAST,           \
      .Rframe_Config = FIRA_RFRAME_CONFIG_SP3,                   \
      .ToF_Report = 1,                                           \
      .AoA_Azimuth_Report = 0,                                   \
      .AoA_Elevation_Report = 0,                                 \
      .AoA_FOM_Report = 0,                                       \
      .nonDeferred_Mode = 0,                                     \
      .STS_Config = 0,                                           \
      .Round_Hopping = 0,                                        \
      .Block_Striding = 0,                                       \
      .Block_Duration_ms = 100,                                  \
      .Round_Duration_RSTU = 18400,                              \
      .Slot_Duration_RSTU = 2400,                                \
      .Channel_Number = 9,                                       \
      .Preamble_Code = 11,                                       \
      .PRF_Mode = 0,                                             \
      .SP0_PHY_Set = FIRA_BPRF_SET_2,                            \
      .SP1_PHY_Set = FIRA_BPRF_SET_3,                            \
      .SP3_PHY_Set = FIRA_BPRF_SET_4,                            \
      .MAX_RR_Retry = 1,                                         \
      .Constraint_Length_Conv_Code_HPRF = 0,                     \
      .UWB_Init_Time_ms = 5,                                     \
      .Block_Timing_Stability = 0,                               \
      .Key_Rotation = 0,                                         \
      .Key_Rotation_Rate = 0,                                    \
      .MAC_FCS_TYPE = 0,                                         \
      .MAC_ADDRESS_MODE = 0,                                     \
      .SRC_ADDR[0] = 0,                                          \
      .SRC_ADDR[1] = 0,                                          \
      .Number_of_Controlee = 1,                                  \
      .DST_ADDR[0] = 1,                                          \
      .DST_ADDR[1] = 0,                                          \
      .Vendor_ID[0] = 0,                                         \
      .Vendor_ID[1] = 0,                                         \
      .Static_STS_IV[0] = 0,                                     \
      .Static_STS_IV[1] = 0,                                     \
      .Static_STS_IV[2] = 0,                                     \
      .Static_STS_IV[3] = 0,                                     \
      .Static_STS_IV[4] = 0,                                     \
      .Static_STS_IV[5] = 0                                      \
  }

extern fira_device_configure_t fira_config;
#endif