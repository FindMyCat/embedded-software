/**
 * @file    util.c
 *
 * @brief   utility functions:
 *          Seconds to/from DW1000 internal time conversions.
 *          1sy = 1us / 1.0256
 *          1dt  = 1s / 499.2e6 / 128.0
 *          sfd timeout calculation
 *
 * @author Decawave Applications
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

#include "util.h"
#include "math.h"

uint64_t util_us_to_dev_time(double microsecu)
{
    uint64_t dt;
    long double dtime;

    dtime = (microsecu / (double)DWT_TIME_UNITS) / 1e6;

    dt = (uint64_t)(dtime);

    return (dt);
}

double util_dev_time_to_sec(uint64_t dt)
{
    double f = 0;

    f = dt * DWT_TIME_UNITS; // seconds #define TIME_UNITS          (1.0/499.2e6/128.0) = 15.65e-12

    return (f);
}

uint64_t util_sec_to_dev_time(double secu)
{
    uint64_t dt;
    double dtime;

    dtime = (secu / (double)DWT_TIME_UNITS);

    dt = 0x0FFFFFFFFFULL & (uint64_t)(dtime);

    return (dt);
}

double util_us_to_sy(double us)
{
    return (double)(us / 1.0256);
}


/* @fn      calc_sfd_to()
 * @param   caclulates SFDTimeout based on given dwt_config_t:
 *          sfdType, txPreambLength , rxPAC
 *          sfdto = {txPreambLength} + 1 + {SFDType} - {rxPAC}
 *
 *          Calculation based on deca_device_api.h driver definition
 *
 * @return  sfdto value
 *
 * */
int16_t calc_sfd_to(void *p)
{
    dwt_config_t *pCfg = p;

    int16_t ret = 1;

    /* + length of SFD based on the SFD type */
    switch (pCfg->sfdType)
    {
    case 0x0: // IEEE standard 8
    case 0x1: // DW non-standard 8
        ret += 0x08;
        break;

    case 0x2: // DW non-standard 16
        ret += 0x10;
        break;

    default :   // IEEE 4z 8
        ret += 0x08;
        break;
    }

    /* + {txPreambLength} */
    switch (pCfg->txPreambLength)
    {
    case DWT_PLEN_64:
    case DWT_PLEN_128:
    case DWT_PLEN_256:
    case DWT_PLEN_512:
        ret += (0x40 << (pCfg->txPreambLength >> 4));
        break;

    case DWT_PLEN_1024:
    case DWT_PLEN_1536:
    case DWT_PLEN_2048:
        ret += (0x200 + (0x200 << (pCfg->txPreambLength >> 4)));
        break;

    default: // Preamble length 4096 and any
        ret += 0x1000;
        break;
    }

    /* - {rxPAC} */
    switch (pCfg->rxPAC)
    {
    case DWT_PAC4:
        ret -= 4;
        break;

    case DWT_PAC16:
        ret -= 16;
        break;

    case DWT_PAC32:
        ret -= 32;
        break;

    default: // PAC8 and any
        ret -= 8;
        break;
    }

    return (ret);
}
