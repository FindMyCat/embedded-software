/**
 * @file      rtls_version.h
 *
 * @brief     version number
 * 
 *            Construct the standard version name as "MAJOR.MINOR.PATCH-YYMMDD"
 * 
 *            TAG_RELEASE_VERSION == 1
 *            Construct the version name as "MAJOR.MINOR.PATCH"
 * 
 *            VER_MAJOR 0..999
 *            VER_MINOR 0..999
 *            VER_PATCH 0..999
 * 
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

#ifndef VERSION_H_
#define VERSION_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#define TAG_RELEASE_VERSION (1)

#ifndef TAG_RELEASE_VERSION
#define TAG_RELEASE_VERSION (1)
#endif

#define VER_MAJOR 3
#define VER_MINOR 0
#define VER_PATCH 0


/**
 * construction of the version name as "MAJOR.MINOR.YYMMDD"
 * e.g.    "Oct  7 2020" => "201007"
 **/

#define YEAR_CH2 (__DATE__[9])
#define YEAR_CH3 (__DATE__[10])

#define MONTH_IS_JAN (__DATE__[0] == 'J' && __DATE__[1] == 'a' && __DATE__[2] == 'n')
#define MONTH_IS_FEB (__DATE__[0] == 'F')
#define MONTH_IS_MAR (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'r')
#define MONTH_IS_APR (__DATE__[0] == 'A' && __DATE__[1] == 'p')
#define MONTH_IS_MAY (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'y')
#define MONTH_IS_JUN (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'n')
#define MONTH_IS_JUL (__DATE__[0] == 'J' && __DATE__[2] == 'l')
#define MONTH_IS_AUG (__DATE__[0] == 'A' && __DATE__[1] == 'u')
#define MONTH_IS_SEP (__DATE__[0] == 'S')
#define MONTH_IS_OCT (__DATE__[0] == 'O')
#define MONTH_IS_NOV (__DATE__[0] == 'N')
#define MONTH_IS_DEC (__DATE__[0] == 'D')

#define MONTH_CH0 ((MONTH_IS_OCT || MONTH_IS_NOV || MONTH_IS_DEC) ? '1' : '0')

#define MONTH_CH1 \
    (   (MONTH_IS_JAN) ? '1' : \
        (MONTH_IS_FEB) ? '2' : \
        (MONTH_IS_MAR) ? '3' : \
        (MONTH_IS_APR) ? '4' : \
        (MONTH_IS_MAY) ? '5' : \
        (MONTH_IS_JUN) ? '6' : \
        (MONTH_IS_JUL) ? '7' : \
        (MONTH_IS_AUG) ? '8' : \
        (MONTH_IS_SEP) ? '9' : \
        (MONTH_IS_OCT) ? '0' : \
        (MONTH_IS_NOV) ? '1' : \
        (MONTH_IS_DEC) ? '2' : \
        /* default */    '?' \
    )

#define DAY_CH0 ((__DATE__[4] >= '0') ? (__DATE__[4]) : '0')
#define DAY_CH1 (__DATE__[5])

#if VER_MAJOR >= 100
#define VMAJOR ((VER_MAJOR / 100) + '0'),        \
               (((VER_MAJOR % 100) / 10) + '0'), \
               ((VER_MAJOR % 10) + '0')
#elif VER_MAJOR >= 10
#define VMAJOR ((VER_MAJOR / 10) + '0'), \
               ((VER_MAJOR % 10) + '0')
#else
#define VMAJOR (VER_MAJOR + '0')
#endif

#if VER_MINOR >= 100
#define VMINOR ((VER_MINOR / 100) + '0'),        \
               (((VER_MINOR % 100) / 10) + '0'), \
               ((VER_MINOR % 10) + '0')
#elif VER_MINOR >= 10
#define VMINOR ((VER_MINOR / 10) + '0'), \
               ((VER_MINOR % 10) + '0')
#else
#define VMINOR (VER_MINOR + '0')
#endif

#if VER_PATCH >= 100
#define VPATCH ((VER_PATCH / 100) + '0'),        \
               (((VER_PATCH % 100) / 10) + '0'), \
               ((VER_PATCH % 10) + '0')
#elif VER_PATCH >= 10
#define VPATCH ((VER_PATCH / 10) + '0'), \
               ((VER_PATCH % 10) + '0')
#else
#define VPATCH (VER_PATCH + '0')
#endif

/* VERSION */
#if (TAG_RELEASE_VERSION == 1)
#define FULL_VERSION { VMAJOR, '.', VMINOR, '.', VPATCH, '\0' }
#else
#define FULL_VERSION { VMAJOR, '.', VMINOR, '.', VPATCH, '-', \
                      YEAR_CH2, YEAR_CH3, \
                      MONTH_CH0, MONTH_CH1, \
                      DAY_CH0, DAY_CH1,\
                     '\0' }
#endif

#define DATE_VERSION { YEAR_CH2, YEAR_CH3, MONTH_CH0, MONTH_CH1, DAY_CH0, DAY_CH1, '\0' }


#ifdef __cplusplus
}
#endif

#endif // VERSION_H_
