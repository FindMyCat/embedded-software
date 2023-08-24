/**
 *  @file     common_fira.h
 *
 *  @brief    Header file for common_fira
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

#ifndef CORE_SRC_APPS_COMMON_FIRA_H_
#define CORE_SRC_APPS_COMMON_FIRA_H_

#ifdef __cplusplus
extern "C" {
#endif

#if CONFIG_PEG_UWB == 0
#include "deca_interface.h"
#else
#include "uwb_config.h"
#endif
#include "uwb_frames.h"
#include "fira_helper.h"
#include "app.h"
#include "appConfig.h"
#include "fira_app_config.h"

#define BPRF_SET_1 (1) // SP0 IEEE SFD
#define BPRF_SET_2 (2) // SP0 4z SFD
#define BPRF_SET_3 (3) // SP1 4z SFD
#define BPRF_SET_4 (4) // SP3 4z SFD
#define BPRF_SET_5 (5) // SP1 IEEE SFD
#define BPRF_SET_6 (6) // SP3 IEEE SFD

#define DATA_TRANSFER 2

// Comment below to see the debug information in a real-time, diag_printf is a buffered I/O
//#define DEBUG_SP3_MSG(...)

#ifndef DEBUG_SP3_MSG
#include "deca_dbg.h"
#define DEBUG_SP3_MSG diag_printf
#endif

#ifndef TWR_ENTER_CRITICAL
#define TWR_ENTER_CRITICAL taskENTER_CRITICAL
#endif

#ifndef TWR_EXIT_CRITICAL
#define TWR_EXIT_CRITICAL taskEXIT_CRITICAL
#endif

struct mini_diag_s
{ // temporary diagnostics for PDoA debugging
    uint8_t DTUNE5;
    uint32_t CIA_TDOA_0;
    uint32_t CIA_TDOA_1_PDOA;
    uint16_t IP_DIAG_10;
    uint16_t CY0_DIAG_10;
    uint16_t CY0_TOA_HI;
    uint16_t CY1_TOA_HI;
};

typedef struct mini_diag_s mini_diag_t;

struct pdoa_s
{
    int16_t pdoa; /* DW3000 PDOA */
    mini_diag_t mDiag;
};

typedef struct pdoa_s pdoa_t;

struct result_s
{
    uint16_t addr16;
    uint16_t rangeNum;      // number from Tag Poll and Final messages, which indicates the current range number
    uint32_t resTime_us;    // reception time of the end of the Final from the Tag wrt node's SuperFrame start, microseconds
    float pdoa_raw_deg;     // pdoa_raw: phase differences in degrees without any correction [-180 .. 180]
    float pdoa_raw_degP;    // pdoa_raw: phase differences in degrees from Poll message
    float dist_cm;          // distance to the tag in cm, corrected to a rngOffset_mm
    float x_cm;             // X of the tag wrt to the node, cm
    float y_cm;             // Y of the tag wrt to the node, cm
    float clockOffset_pphm; // clock offset in hundredths of ppm (i.e. 1ppm is 100)
    uint16_t flag;          // service message data from the tag (low byte) and node (high byte), bitmask (defined as "RES_FLAG_")
    int16_t acc_x;          // Normalized accel data X from the Tag, mg: acc_x
    int16_t acc_y;          // Normalized accel data Y from the Tag, mg: acc_y
    int16_t acc_z;          // Normalized accel data Z from the Tag, mg: acc_z

    pdoa_t pollPDOA;
    pdoa_t finalPDOA;

    int8_t tMaster_C; // temperature of Master in degree centigrade
};

typedef struct result_s result_t;

/**
 * struct data_measurements - Data measurement.
 */
struct string_measurement
{
    /**
     * @str: buffer of string measurement to allocate.
     */
    char *str;
    /**
     * @len: Length of measuremeent string.
     */
    uint16_t len;
};

// ----------------------------------------------------------------------------
//
void show_fira_params();
void scan_fira_params(const char *text, bool controller);
uwbmac_error fira_set_session_parameters(struct fira_context *fira_context, uint32_t session_id, struct session_parameters *session);

void fira_uwb_mcps_init(fira_param_t *fira_param);
void fira_uwb_mcps_deinit(void);
int32_t fira_uwb_mcps_get_cfo_ppm(void);
bool fira_uwb_is_diag_enabled(void);
int fira_uwb_add_diag(char *str, int len, int max_len);

void set_local_pavrg_size(void);
uint8_t get_local_pavrg_size(void);

#ifdef __cplusplus
}
#endif

#endif /* CORE_SRC_APPS_COMMON_FIRA_H_ */
