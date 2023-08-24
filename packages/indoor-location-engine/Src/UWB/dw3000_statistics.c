/**
 * @file      dw3000_statistics.c
 *
 * @brief     Compute various statistics of a signal
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

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include "deca_device_api.h"
#include "dw3000_statistics.h"

#define SIG_LVL_FACTOR     0.4     // Factor between 0 and 1; default 0.4 from experiments and simulations.
#define SIG_LVL_THRESHOLD  12      // Threshold unit is dB; default 12dB from experiments and simulations.
#define ALPHA_PRF_16       113.8   // Constant A for PRF of 16 MHz. See User Manual for more information.
#define ALPHA_PRF_64       120.7   // Constant A for PRF of 64 MHz. See User Manual for more information.
#define RX_CODE_THRESHOLD  8       // For 64 MHz PRF the RX code is 9.
#define LOG_CONSTANT_C0    63.2    // 10log10(2^21) = 63.2    // See User Manual for more information.
#define LOG_CONSTANT_D0_E0 51.175  // 10log10(2^17) = 51.175  // See User Manual for more information.
#define IP_MIN_THRESHOLD   3.3     // Minimum Signal Level in dB. Please see App Notes "APS006 PART 3"
#define IP_MAX_THRESHOLD   6.0     // Minimum Signal Level in dB. Please see App Notes "APS006 PART 3"
#define CONSTANT_PR_IP_A   0.39178 // Constant from simulations on DW device accumulator, please see App Notes "APS006 PART 3"
#define CONSTANT_PR_IP_B   1.31719 // Constant from simulations on DW device accumulator, please see App Notes "APS006 PART 3"

void calculateStats(struct dwchip_s *dw, struct mcps_diag_s *diag)
{
    /* Line-of-sight / Non-line-of-sight Variables */
    uint32_t dev_id, D;
    dwt_nlos_alldiag_t all_diag;
    dwt_nlos_ipdiag_t index;

    // All float variables used for recording different diagnostic results and probability.
    float ip_f1, ip_f2, ip_f3, sts1_f1, sts1_f2, sts1_f3, sts2_f1, sts2_f2, sts2_f3 = 0;
    float ip_n, sts1_n, sts2_n, ip_cp, sts1_cp, sts2_cp, ip_rsl, ip_fsl, sts1_rsl, sts1_fsl, sts2_rsl, sts2_fsl = 0;
    float pr_nlos, sl_diff_ip, sl_diff_sts1, sl_diff_sts2, sl_diff, index_diff = 0;
    float alpha, ip_alpha, log_constant = 0;

    dev_id = dw->dwt_driver->devid;

    if ((dev_id == (uint32_t)DWT_DW3000_DEV_ID) || (dev_id == (uint32_t)DWT_DW3000_PDOA_DEV_ID))
    {
        log_constant = LOG_CONSTANT_C0;
    }
    else
    {
        log_constant = LOG_CONSTANT_D0_E0;
    }
    // Select IPATOV to read Ipatov diagnostic registers from API function dwt_nlos_alldiag()
    all_diag.diag_type = IPATOV;
    dw->dwt_driver->dwt_ops->ioctl(dw, DWT_NLOS_ALLDIAG, 0, (void *)&all_diag);
    ip_alpha = (dw->config->rxtx_config->pdwCfg->rxCode > RX_CODE_THRESHOLD) ? (-(ALPHA_PRF_64 + 1)) : -(ALPHA_PRF_16);
    ip_n = all_diag.accumCount; // The number of preamble symbols accumulated
    ip_f1 = all_diag.F1 / 4;    // The First Path Amplitude (point 1) magnitude value (it has 2 fractional bits),
    ip_f2 = all_diag.F2 / 4;    // The First Path Amplitude (point 2) magnitude value (it has 2 fractional bits),
    ip_f3 = all_diag.F3 / 4;    // The First Path Amplitude (point 3) magnitude value (it has 2 fractional bits),
    ip_cp = all_diag.cir_power;

    // Select STS1 to read STS1 diagnostic registers from API function dwt_nlos_alldiag()
    all_diag.diag_type = STS1;
    dw->dwt_driver->dwt_ops->ioctl(dw, DWT_NLOS_ALLDIAG, 0, (void *)&all_diag);
    alpha = -(ALPHA_PRF_64 + 1);
    sts1_n = all_diag.accumCount; // The number of preamble symbols accumulated
    sts1_f1 = all_diag.F1 / 4;    // The First Path Amplitude (point 1) magnitude value (it has 2 fractional bits),
    sts1_f2 = all_diag.F2 / 4;    // The First Path Amplitude (point 2) magnitude value (it has 2 fractional bits),
    sts1_f3 = all_diag.F3 / 4;    // The First Path Amplitude (point 3) magnitude value (it has 2 fractional bits),
    sts1_cp = all_diag.cir_power;

    // Select STS2 to read STS2 diagnostic registers from API function dwt_nlos_alldiag()
    all_diag.diag_type = STS2;
    dw->dwt_driver->dwt_ops->ioctl(dw, DWT_NLOS_ALLDIAG, 0, (void *)&all_diag);
    sts2_n = all_diag.accumCount; // The number of preamble symbols accumulated
    sts2_f1 = all_diag.F1 / 4;    // The First Path Amplitude (point 1) magnitude value (it has 2 fractional bits),
    sts2_f2 = all_diag.F2 / 4;    // The First Path Amplitude (point 2) magnitude value (it has 2 fractional bits),
    sts2_f3 = all_diag.F3 / 4;    // The First Path Amplitude (point 3) magnitude value (it has 2 fractional bits),
    sts2_cp = all_diag.cir_power;

    // The calculation of First Path Power Level(FSL) and Receive Signal Power Level(RSL) is taken from
    // DW3000 User Manual section 4.7.1 & 4.7.2

    // For IPATOV
    ip_n *= ip_n;
    ip_f1 *= ip_f1;
    ip_f2 *= ip_f2;
    ip_f3 *= ip_f3;

    // FOR STS1
    sts1_n *= sts1_n;
    sts1_f1 *= sts1_f1;
    sts1_f2 *= sts1_f2;
    sts1_f3 *= sts1_f3;

    // For STS2
    sts2_n *= sts2_n;
    sts2_f1 *= sts2_f1;
    sts2_f2 *= sts2_f2;
    sts2_f3 *= sts2_f3;

    //    D = all_diag.D * 6;
    uint8_t tmp = 0;
    dw->dwt_driver->dwt_ops->ioctl(dw, DWT_GETDGCDECISION, 0, (void *)&tmp);
    D = tmp * 6;

    // Calculate the First Signal Level(FSL) and Receive Signal Level(RSL) then subtract FSL from RSL
    // to find out Signal Level Difference which is compared to defined Signal Threshold.

    // For the CIR Ipatov.
    ip_rsl = 10 * log10((float)ip_cp / ip_n) + ip_alpha + log_constant + D;
    ip_fsl = 10 * log10(((ip_f1 + ip_f2 + ip_f3) / ip_n)) + ip_alpha + D;

    // Signal Level Difference value for IPATOV.
    sl_diff_ip = ip_rsl - ip_fsl;

    // For the CIR STS1.
    sts1_rsl = 10 * log10((float)sts1_cp / sts1_n) + alpha + log_constant + D;
    sts1_fsl = 10 * log10(((sts1_f1 + sts1_f2 + sts1_f3) / sts1_n)) + alpha + D;

    // For the CIR STS2.
    sts2_rsl = 10 * log10((float)sts2_cp / sts2_n) + alpha + log_constant + D;
    sts2_fsl = 10 * log10(((sts2_f1 + sts2_f2 + sts2_f3) / sts2_n)) + alpha + D;

    // STS Mode OFF, Signal Level Difference of STS1 and STS2 is zero.
    if (dw->config->rxtx_config->pdwCfg->stsMode == DWT_STS_MODE_OFF)
    {
        sl_diff_sts1 = 0;
        sl_diff_sts2 = 0;
    }
    else // If PDOA MODE 3 is enabled then there's Signal Level Difference value for all IPATOV, STS1 and STS2.
    {
        sl_diff_sts1 = sts1_rsl - sts1_fsl;

        // IF PDOA MODE 3 is not enabled then Signal Level Difference of STS2 is zero.
        if (dw->config->rxtx_config->pdwCfg->pdoaMode != DWT_PDOA_M3)
        {
            sl_diff_sts2 = 0;
        }
        else
        {
            sl_diff_sts2 = sts2_rsl - sts2_fsl;
        }
    }

    /* Check for Line-of-sight or Non-line-of-sight */
    // The Signal Level Threshold is 12 dB, based on the experiments and simulations if the received signal power is above
    // 12 dB then the Signal is Non Line of Sight.

    // Calculating the probability of NLOS.
    // 1. If the signal level difference of IPATOV, STS1 or STS2 is greater than 12 dB then the signal is Non Line of sight.

    if ((sl_diff_ip > SIG_LVL_THRESHOLD) || (sl_diff_sts1 > SIG_LVL_THRESHOLD) || (sl_diff_sts2 > SIG_LVL_THRESHOLD))
    {
        pr_nlos = 100;
        // test_run_info((unsigned char *)"Non-Line of sight");
    }

    // If the received signal power is in between 4.8 dB and 12 dB then there's a possibility that the signal is
    // Non Line of Sight and probabilty of it being Non Line of Sight signal is calculated.

    // 2. If the signal level difference of IPATOV, STS1 or STS2 is greater than
    //    (Signal Level Threshold(12) * Signal Level Factor(0.4)) = 4.8 dB but less than 12 dB, then calculate the
    //    probability of Non Line of sight based on the signal has greater strength(IPATOV, STS1 or STS2).

    else if ((sl_diff_ip > SIG_LVL_THRESHOLD * SIG_LVL_FACTOR) || (sl_diff_sts1 > SIG_LVL_THRESHOLD * SIG_LVL_FACTOR) || (sl_diff_sts2 > SIG_LVL_THRESHOLD * SIG_LVL_FACTOR))
    {
        if (sl_diff_ip > SIG_LVL_THRESHOLD * SIG_LVL_FACTOR)
        {
            sl_diff = sl_diff_ip;
        }
        else if (sl_diff_sts1 > SIG_LVL_THRESHOLD * SIG_LVL_FACTOR)
        {
            sl_diff = sl_diff_sts1;
        }
        else
        {
            sl_diff = sl_diff_sts2;
        }

        pr_nlos = 100 * ((sl_diff / SIG_LVL_THRESHOLD - SIG_LVL_FACTOR) / (1 - SIG_LVL_FACTOR));
        // snprintf(prob_str, sizeof(prob_str), "Probability of NLOS: %3.2f", fabsf(pr_nlos));
        // test_run_info((unsigned char *)prob_str);
    }

    // 3. If the signal is less than the Combined Threshold (Signal Level Threshold(12) * Signal Level Factor(0.4)) for all three
    //    IPATOV, STS1 and STS2 reported through dwt_nlos_alldiag() then check the IPATOV Diagnostic First Path and Peak Path Index
    //    through dwt_nlos_ipdiag().

    //    3.a. If the Index difference is less than 3.3 dB then it's a Line of Sight signal.
    //    3.b. If the Index difference is greater than 3.3 dB and less than 6 dB then the probability of Non Line of Sight
    //         is calculated.
    //    3.c. If the Index level is greater than 6 dB then it's a Non Line of Sight signal.
    else
    {
        dwt_nlos_ipdiag(&index);
        index_diff = ((float)index.index_pp_u32 - (float)index.index_fp_u32) / 32;

        if (index_diff <= IP_MIN_THRESHOLD)
        {
            pr_nlos = 0;
            // test_run_info((unsigned char *)"Line of Sight");
        }
        else if ((index_diff > IP_MIN_THRESHOLD) && (index_diff < IP_MAX_THRESHOLD))
        {
            pr_nlos = 100 * ((CONSTANT_PR_IP_A * index_diff) - CONSTANT_PR_IP_B);
            // snprintf(prob_str, sizeof(prob_str), "**Probability of NLOS: %3.2f", fabsf(pr_nlos));
            // test_run_info((unsigned char *)prob_str);
        }
        else
        {
            pr_nlos = 100;
            // test_run_info((unsigned char *)"Non-Line of Sight");
        }
    }
    diag->rssi = ip_rsl;
    diag->non_line_of_sight = fabsf(pr_nlos);
}
