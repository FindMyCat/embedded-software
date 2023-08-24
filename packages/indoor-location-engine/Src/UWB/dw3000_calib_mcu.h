/**
 * @file      dw3000_calib_mcu.h
 *
 * @brief     Header for calibration for dw3000
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

#ifndef __DW3000_CALIB_H
#define __DW3000_CALIB_H

/**
 * DW3000_CALIBRATION_ANTENNA_MAX - number of antenna
 */
#define DW3000_CALIBRATION_ANTENNA_MAX 4

/**
 * enum dw3000_calibration_channels - calibration channel number.
 */
enum dw3000_calibration_channel
{
    DW3000_CALIBRATION_CHANNEL_5 = 0,
    DW3000_CALIBRATION_CHANNEL_9 = 1,

    DW3000_CALIBRATION_CHANNEL_MAX
};

/**
 * enum dw3000_calibration_prfs - calibration Pulse Repetition Frequency.
 */
enum dw3000_calibration_prf
{
    DW3000_CALIBRATION_PRF_16MHZ = 0,
    DW3000_CALIBRATION_PRF_64MHZ = 1,

    DW3000_CALIBRATION_PRF_MAX
};

/**
 * DW3000_CALIBRATION_PDOA_LUT_MAX - number of value in PDOA LUT table
 */
#define DW3000_CALIBRATION_PDOA_LUT_MAX 7

/**
 * DW3000_DEFAULT_ANT_DELAY - antenna delay default value
 */
#define DW3000_DEFAULT_ANT_DELAY 16450

/**
 * DW3000_DEFAULT_ANT_PAIR_SPACING - antpair spacing default value
 *
 * Value is for Monalisa antennas pair where antennas are spaced
 * 20.8mm.
 * So default spacing_mm_q11 = 20.8 * 1<<11 = 42598.4
 */
#define DW3000_DEFAULT_ANTPAIR_SPACING 42598

/**
 * struct dw3000_channel_calib - per-channel dependent calibration parameters
 * @pll_locking_code: PLL locking code
 */
struct dw3000_channel_calib
{
    /* chY.pll_locking_code */
    u32 pll_locking_code;
};

/**
 * struct dw3000_antenna_calib_prf - antenna calibration parameters
 * @ant_delay: antenna delay
 * @tx_power: tx power
 * @pg_count: PG count
 * @pg_delay: PG delay
 */
struct dw3000_antenna_calib_prf
{
    u32 ant_delay;
    u32 tx_power;
    u8 pg_count;
    u8 pg_delay;
};

/**
 * struct dw3000_antenna_calib - per-antenna dependent calibration parameters
 * @ch: table of channels and PRF dependent calibration values
 * @ant: antenna pair specific calibration values
 * @port: port value this antenna belong to (0 for RF1, 1 for RF2)
 * @selector_gpio: GPIO number to select this antenna
 * @selector_gpio_value: GPIO value to select this antenna
 */
struct dw3000_antenna_calib
{
    /* antX.chY.prfZ.* */
    struct
    {
        struct dw3000_antenna_calib_prf prf[DW3000_CALIBRATION_PRF_MAX];
    } ch[DW3000_CALIBRATION_CHANNEL_MAX];
    /* antX.* */
    u8 port, selector_gpio, selector_gpio_value;
};

/**
 * struct dw3000_antenna_pair_calib - antenna pair dependent calibration values
 * @ch: table of channels dependent calibration values
 */
struct dw3000_antenna_pair_calib
{
    /* antX.antW.chY.* */
    struct
    {
        s16 pdoa_offset;
        u32 pdoa_lut[DW3000_CALIBRATION_PDOA_LUT_MAX];
    } ch[DW3000_CALIBRATION_CHANNEL_MAX];
};

/* Just to ease reading of the following formulas. */
#define ANTMAX DW3000_CALIBRATION_ANTENNA_MAX

/**
 * ANTPAIR_MAX - calculated antpair table size
 */
#define ANTPAIR_MAX ((ANTMAX * (ANTMAX - 1)) / 2)

/**
 * ANTPAIR_OFFSET - calculate antpair table indexes row offset
 * @x: first antenna index
 *
 * Return: An index for the first element in antpair table for the given value.
 */
#define ANTPAIR_OFFSET(x) ((((2 * (ANTMAX - 1)) + 1 - (x)) * (x)) / 2)

/**
 * ANTPAIR_IDX - calculate antpair table indexes
 * @x: first antenna index
 * @w: second antenna index, must be > @x
 *
 * Return: An index for the antpair table in [0;ANTPAIR_MAX-1] interval.
 */
#define ANTPAIR_IDX(x, w) (ANTPAIR_OFFSET(x) + ((w) - (x)-1))

/**
 * dw3000_calib_antpair_to_ant - convert antenna pair to corresponding antennas
 * @ant_pair: antenna pair
 * @antidx1: first antenna
 * @antidx2: second antenna
 */
static inline void dw3000_calib_antpair_to_ant(int ant_pair, u8 *antidx1, u8 *antidx2)
{
    static const u8 pair_to_ant[ANTPAIR_MAX][2] = {{0, 1}, {0, 2}, {0, 3}, {1, 2}, {1, 3}, {2, 3}};
    *antidx1                                    = pair_to_ant[ant_pair][0];
    *antidx2                                    = pair_to_ant[ant_pair][1];
}

/**
 * struct dw3000_calibration_data - all per-antenna and per-channel calibration
 * parameters
 * @ant: table of antenna dependant calibration values
 * @antpair: table of antenna pair dependant calibration values
 * @ch: table of channel dependant calibration values
 */
struct dw3000_calibration_data
{
    struct dw3000_antenna_calib ant[ANTMAX];
    struct dw3000_antenna_pair_calib antpair[ANTPAIR_MAX];
    struct dw3000_channel_calib ch[DW3000_CALIBRATION_CHANNEL_MAX];
};

struct dwchip_s;

int dw3000_calib_parse_key(struct dwchip_s *dw, const char *key, void **param);
const char *const *dw3000_calib_list_keys(struct dwchip_s *dw);
int dw3000_calib_update_config(struct dwchip_s *dw);

#endif /* __DW3000_CALIB_H */
