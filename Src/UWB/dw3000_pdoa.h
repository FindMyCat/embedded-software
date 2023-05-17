/**
 * @file      dw3000_pdoa.h
 *
 * @brief     Characterization of a giving UWB RF system design.
 *            Required to get from raw chip PDoA to human readable format.
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
 */

#ifndef __DW3000_PDOA__H__
#define __DW3000_PDOA__H__ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#ifndef M_PI
#define M_PI (3.141592654f)
#endif

#ifndef SPEED_OF_LIGHT
#define SPEED_OF_LIGHT (299702547.0) // in m/s in the air
#endif

#ifndef FB
#define FB    (499.2e6f)                    /* Basis frequency */
#define L_M_5 (SPEED_OF_LIGHT / FB / 13.0f) /* Lambda, m, CH5 */
#define L_M_9 (SPEED_OF_LIGHT / FB / 16.0f) /* Lambda, m, CH9 */
#define D_M_5 (0.022777110597040736f)       /* Distance between centers of antennas, ~(L_M/2), m, CH5 */
#define D_M_9 (0.017883104683497245f)       /* Distance between centers of antennas, ~(L_M/2), m, CH9 */

#define PDOA_INTERVAL_SHIFT_CH5 (15.0f)
#define PDOA_INTERVAL_SHIFT_CH9 (-22.0f)
#endif

/* on allocation of rx_ctx, allocate (avrg_s + size_of(float)*avrg_max) */
struct avrg_s
{
    uint8_t lcnt;     // local counter, loop over 0..max_cnt
    uint8_t max_cnt;  // increase from 0 to avrg_max
    uint8_t avrg_max; // size of avrg[] buff, 0 = no averaging
    float prev_avrg;  // prev avrg result
    float *avrg;      // must be the last element, the &avrg[0] will be the next address
};

struct pdoa_aoa_s
{
    int16_t pdoa_q11; // normalized pdoa - can be used for antenna calibration
    int16_t aoa_q11;
};

struct fpdoa_in_s
{
    int32_t p_deg100;
    int16_t pdoa_q11;
    uint8_t chan;
    uint8_t corr_en;
    uint8_t max_avrg;
};

void pdoaupdate_lut(void);
void fpdoa2aoa(struct fpdoa_in_s *in, struct pdoa_aoa_s *out, void *rx_ctx);

#ifdef __cplusplus
}
#endif

#endif /* __DW3000_PDOA__H__ */
