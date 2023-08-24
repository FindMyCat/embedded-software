/**
 * @file      translate.c
 *
 * @brief     translate DW1000 parameters from Deca to Human and from Human to Deca format
 *
 *            return translated value or (-1) if out of allowed range
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

#include "deca_device_api.h"

/* Channel */
int chan_to_deca(int i)
{
    int ret = -1;

    if (i == 5 || i == 9)
    {
        ret = i;
    }

    return (ret);
}

int deca_to_chan(int i)
{
    return (chan_to_deca(i));
}


/* Bitrate */
int bitrate_to_deca(int i)
{
    switch (i)
    {
    case 850:
        return DWT_BR_850K;
    case 6810:
        return DWT_BR_6M8;
    default:
        return -1;
    }
}

int deca_to_bitrate(int i)
{
    switch (i)
    {
    case DWT_BR_850K:
        return 850;
    case DWT_BR_6M8:
        return 6810;
    default:
        return -1;
    }
}


/* PAC */
int pac_to_deca(int i)
{
    switch (i)
    {
    case 8:
        return DWT_PAC8;
    case 16:
        return DWT_PAC16;
    case 32:
        return DWT_PAC32;
    case 4:
        return DWT_PAC4;
    default:
        return -1;
    }
}

int deca_to_pac(int i)
{
    switch (i)
    {
    case DWT_PAC8:
        return 8;
    case DWT_PAC16:
        return 16;
    case DWT_PAC32:
        return 32;
    case DWT_PAC4:
        return 4;
    default:
        return -1;
    }
}


/* PLEN */
int plen_to_deca(int i)
{
    switch (i)
    {
    // case 4096:
    //     return DWT_PLEN_4096;
    case 2048:
        return DWT_PLEN_2048;
    case 1536:
        return DWT_PLEN_1536;
    case 1024:
        return DWT_PLEN_1024;
    case 512:
        return DWT_PLEN_512;
    case 256:
        return DWT_PLEN_256;
    case 128:
        return DWT_PLEN_128;
    case 64:
        return DWT_PLEN_64;
    default:
        return -1;
    }
}

int deca_to_plen(int i)
{
    switch (i)
    {
    // case DWT_PLEN_4096:
    //     return 4096;
    case DWT_PLEN_2048:
        return 2048;
    case DWT_PLEN_1536:
        return 1536;
    case DWT_PLEN_1024:
        return 1024;
    case DWT_PLEN_512:
        return 512;
    case DWT_PLEN_256:
        return 256;
    case DWT_PLEN_128:
        return 128;
    case DWT_PLEN_64:
        return 64;
    default:
        return -1;
    }
}


/*STS Length*/
int sts_length_to_deca(int i)
{
    switch (i)
    {
    case 2048:
        return DWT_STS_LEN_2048;
    case 1024:
        return DWT_STS_LEN_1024;
    case 512:
        return DWT_STS_LEN_512;
    case 256:
        return DWT_STS_LEN_256;
    case 128:
        return DWT_STS_LEN_128;
    case 64:
        return DWT_STS_LEN_64;
    case 32:
        return DWT_STS_LEN_32;
    default:
        return -1;
    }
}

int deca_to_sts_length(int i)
{
    switch (i)
    {
    case DWT_STS_LEN_2048:
        return 2048;
    case DWT_STS_LEN_1024:
        return 1024;
    case DWT_STS_LEN_512:
        return 512;
    case DWT_STS_LEN_256:
        return 256;
    case DWT_STS_LEN_128:
        return 128;
    case DWT_STS_LEN_64:
        return 64;
    case DWT_STS_LEN_32:
        return 32;
    default:
        return -1;
    }
}

/* END of translate */
