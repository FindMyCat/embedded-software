/**
 * @file      dw3000_pdoa.c
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


/* Current implementation characterized for the ML-1.0 antenna */

#include <math.h>
#include <stdlib.h>

#include "dw3000_pdoa.h"
#include "rf_tuning_config.h"

/* clang-format off */
/* MONALISA CHANNEL 5 */
#if 0 // PDOA_M3
    static const double xs_mon_ch5_m3[] = { -169.52599388379204, -164.55029585798817 , -160.03367003367003  , -155.23640661938535 , 
                                            -150.56637168141592, -145.09003215434083 , -138.81159420289856  , -130.99603174603175 , 
                                            -123.18204488778055, -113.84177215189874 , -102.67299107142857  ,  -91.66666666666667 , 
                                             -78.92921686746988,  -66.17794486215539 ,  -52.489329268292686 ,  -38.47277936962751 , 
                                             -25.19344262295082,  -10.773413897280967,    0.9860681114551083,   15.321236559139784, 
                                              31.16460396039604,   46.739010989010985,   62.48295454545455  ,   79.0              , 
                                              93.02919708029196,  105.17538461538462 ,  116.59448818897638  ,  125.65702479338843 , 
                                             135.40189873417722,  142.97699386503066 ,  151.34635416666666  ,  158.2935656836461  , 
                                             165.31201044386424,  172.39769820971867 ,  180.0               ,  188.23993808049536 ,
                                             198.43213296398892 };
    static const double ys_mon_ch5_m3[] = { -0.02277711059704047  , -0.022690436814620973 , -0.022431075107181914, -0.022000999373923726,
                                            -0.0214034827508634   , -0.020643072700292697 , -0.019725556401844816, -0.018657916708562303,
                                            -0.01744827900316916  , -0.016105849359003235 , -0.014640844476237574, -0.01306441392662416 ,
                                            -0.011388555298520233 , -0.009626022887996853 , -0.007790230630944395, -0.005895150014920516,
                                            -0.003955203747694204 , -0.0019851559917306244,  0.0                 ,  0.001985155991730628, 
                                             0.0039552037476942034,  0.005895150014920523 ,  0.007790230630944404,  0.009626022887996888, 
                                             0.011388555298520233 ,  0.013064413926624103 ,  0.014640844476237608,  0.01610584935900323 ,   
                                             0.017448279003169222 ,  0.018657916708562265 ,  0.019725556401844816,  0.02064307270029269 ,   
                                             0.021403482750863533 ,  0.02200099937392368  ,  0.02243107510718191 ,  0.02269043681462098 ,
                                             0.022777110597040465 };
#else // PDOA_M1
    static const double xs_mon_ch5_m1[] = { -167.451, -166.592, -165.733, -161.641,
                                            -157.549, -150.116, -142.683, -134.165,
                                            -125.647, -116.942, -108.238,  -97.992,
                                             -87.746,  -75.604,  -63.462,  -48.720,
                                             -33.978,  -16.989,    0.0  ,   19.145,
                                              38.290,   55.524,   72.759,   86.546,
                                             100.333,  110.570,  120.808,  129.769,
                                             138.731,  146.481,  154.231,  161.072,
                                             167.913,  173.938,  179.963,  185.898,
                                             191.834 };
    static const double ys_mon_ch5_m1[] = { -0.02277711, -0.02259527, -0.02243108, -0.02183519, 
                                            -0.02140348, -0.02060275, -0.01972556, -0.01864569, 
                                            -0.01744828, -0.01607664, -0.01464084, -0.01298905, 
                                            -0.01138856, -0.00954588, -0.00779023, -0.00581025, 
                                            -0.0039552 , -0.00191495,  0.0       ,  0.00199575,
                                             0.0039552 ,  0.00581256,  0.00779023,  0.00948489,  
                                             0.01138856,  0.01298333,  0.01464084,  0.01606225,  
                                             0.01744828,  0.01861804,  0.01972556,  0.02061772,  
                                             0.02140348,  0.02198524,  0.02243108,  0.02270353,  
                                             0.02277711 };
#endif

/* MONALISA CHANNEL 9 */
#if 0 //PDOA_M3
    static const double xs_mon_ch9_m3[] = { -182.178     , -172.91577143, -163.65354286, -154.39131429,
                                            -145.12908571, -135.86685714, -126.60462857, -117.3424    ,
                                            -108.08017143,  -98.81794286,  -89.55571429,  -80.29348571,
                                             -71.03125714,  -61.76902857,  -52.5068    ,  -43.24457143,
                                             -33.98234286,  -24.72011429,  -15.45788571,   -6.19565714,
                                               3.06657143,   12.3288    ,   21.59102857,   30.85325714,
                                              40.11548571,   49.37771429,   58.63994286,   67.90217143,
                                              77.1644    ,   86.42662857,   95.68885714,  104.95108571,
                                             114.21331429,  123.47554286,  132.73777143,  142.         };
    static const double ys_mon_ch9_m3[] = { -0.0178831 , -0.01753701, -0.01681944, -0.01530936, 
                                            -0.0133346 , -0.01188188, -0.01064578, -0.0094633 , 
                                            -0.008383  , -0.00741476, -0.00653003, -0.00569933, 
                                            -0.00490387, -0.00413532, -0.00338581, -0.00264728, 
                                            -0.00190919, -0.00115921, -0.00038495,  0.00042524,
                                             0.0012678 ,  0.00212467,  0.00297723,  0.00381115,  
                                             0.00463958,  0.00548639,  0.00637547,  0.00732847,  
                                             0.00836314,  0.00949486,  0.0106894 ,  0.01186556,  
                                             0.01331466,  0.01548235,  0.0166216 ,  0.01761142 };
#else //PDOA_M1
    static const double xs_mon_ch9_m1[] = { -169.84426         , -167.11048, -158.87248000000002 , -149.57176          , 
                                            -137.69556         , -119.09592,  -92.6935           ,  -62.443799999999996,
                                             -33.2075          ,    0.0    ,   32.961839999999995,   64.67824          , 
                                              88.73612         ,  108.79534,  124.74589999999999 ,  137.60603999999998 ,
                                             147.50220000000002,  161.44412,  176.59868 };
    static const double ys_mon_ch9_m1[] = { -0.0178831 , -0.01761142, -0.01680462, -0.01548722, 
                                            -0.01369925, -0.01149504, -0.00894155, -0.00611638,
                                            -0.00310537,  0.0       ,  0.00310537,  0.00611638,  
                                             0.00894155,  0.01149504,  0.01369925,  0.01548722, 
                                             0.01680462,  0.01761142,  0.0178831 };
#endif

/* JOLIE CHANNEL 5 */
static const double xs_jol_ch5_m1[] = { -182.178     , -172.91577143, -163.65354286, -154.39131429,
                                        -145.12908571, -135.86685714, -126.60462857, -117.3424    ,
                                        -108.08017143,  -98.81794286,  -89.55571429,  -80.29348571,
                                         -71.03125714,  -61.76902857,  -52.5068    ,  -43.24457143,
                                         -33.98234286,  -24.72011429,  -15.45788571,   -6.19565714,
                                           3.06657143,   12.3288    ,   21.59102857,   30.85325714,
                                          40.11548571,   49.37771429,   58.63994286,   67.90217143,
                                          77.1644    ,   86.42662857,   95.68885714,  104.95108571,
                                         114.21331429,  123.47554286,  132.73777143,  142.         };
static const double ys_jol_ch5_m1[] = { -0.0178831 , -0.01753701, -0.01681944, -0.01530936, 
                                        -0.0133346 , -0.01188188, -0.01064578, -0.0094633 , 
                                        -0.008383  , -0.00741476, -0.00653003, -0.00569933, 
                                        -0.00490387, -0.00413532, -0.00338581, -0.00264728, 
                                        -0.00190919, -0.00115921, -0.00038495,  0.00042524,
                                         0.0012678 ,  0.00212467,  0.00297723,  0.00381115,  
                                         0.00463958,  0.00548639,  0.00637547,  0.00732847,  
                                         0.00836314,  0.00949486,  0.0106894 ,  0.01186556,  
                                         0.01331466,  0.01548235,  0.0166216 ,  0.01761142 };

/* JOLIE CHANNEL 9 */
static const double xs_jol_ch9_m1[] = { -182.178     , -172.91577143, -163.65354286, -154.39131429,
                                        -145.12908571, -135.86685714, -126.60462857, -117.3424    ,
                                        -108.08017143,  -98.81794286,  -89.55571429,  -80.29348571,
                                         -71.03125714,  -61.76902857,  -52.5068    ,  -43.24457143,
                                         -33.98234286,  -24.72011429,  -15.45788571,   -6.19565714,
                                           3.06657143,   12.3288    ,   21.59102857,   30.85325714,
                                          40.11548571,   49.37771429,   58.63994286,   67.90217143,
                                          77.1644    ,   86.42662857,   95.68885714,  104.95108571,
                                         114.21331429,  123.47554286,  132.73777143,  142.         };
static const double ys_jol_ch9_m1[] = { -0.0178831 , -0.01753701, -0.01681944, -0.01530936, 
                                        -0.0133346 , -0.01188188, -0.01064578, -0.0094633 , 
                                        -0.008383  , -0.00741476, -0.00653003, -0.00569933, 
                                        -0.00490387, -0.00413532, -0.00338581, -0.00264728, 
                                        -0.00190919, -0.00115921, -0.00038495,  0.00042524,
                                         0.0012678 ,  0.00212467,  0.00297723,  0.00381115,  
                                         0.00463958,  0.00548639,  0.00637547,  0.00732847,  
                                         0.00836314,  0.00949486,  0.0106894 ,  0.01186556,  
                                         0.01331466,  0.01548235,  0.0166216 ,  0.01761142 };

/* CUSTOM */
static const double xs_cst_chn_m1[] = { -182.178     , -172.91577143, -163.65354286, -154.39131429,
                                        -145.12908571, -135.86685714, -126.60462857, -117.3424    ,
                                        -108.08017143,  -98.81794286,  -89.55571429,  -80.29348571,
                                         -71.03125714,  -61.76902857,  -52.5068    ,  -43.24457143,
                                         -33.98234286,  -24.72011429,  -15.45788571,   -6.19565714,
                                           3.06657143,   12.3288    ,   21.59102857,   30.85325714,
                                          40.11548571,   49.37771429,   58.63994286,   67.90217143,
                                          77.1644    ,   86.42662857,   95.68885714,  104.95108571,
                                         114.21331429,  123.47554286,  132.73777143,  142.         };
static const double ys_cst_chn_m1[] = { -0.0178831 , -0.01753701, -0.01681944, -0.01530936, 
                                        -0.0133346 , -0.01188188, -0.01064578, -0.0094633 , 
                                        -0.008383  , -0.00741476, -0.00653003, -0.00569933, 
                                        -0.00490387, -0.00413532, -0.00338581, -0.00264728, 
                                        -0.00190919, -0.00115921, -0.00038495,  0.00042524,
                                         0.0012678 ,  0.00212467,  0.00297723,  0.00381115,  
                                         0.00463958,  0.00548639,  0.00637547,  0.00732847,  
                                         0.00836314,  0.00949486,  0.0106894 ,  0.01186556,  
                                         0.01331466,  0.01548235,  0.0166216 ,  0.01761142 };
/* clang-format on */

const static double *xs;
const static double *ys;
static int local_lut_size = -1;

/* Set XS and YS pointers to the correct LUT based on the antenna type */
void pdoaupdate_lut(void)
{
    rf_tuning_t *rf_tuning = get_rf_tuning_config();

    switch (rf_tuning->antenna.port1)
    {
    case ANT_TYPE_NONE:
    case ANT_TYPE_MAN5:
    case ANT_TYPE_CPWING5:
    case ANT_TYPE_CPWING9:
        local_lut_size = -1;
        break;
    case ANT_TYPE_MONALISA5:
        xs = xs_mon_ch5_m1;
        ys = ys_mon_ch5_m1;
        local_lut_size = sizeof(xs_mon_ch5_m1) / sizeof(xs_mon_ch5_m1[0]);
        break;
    case ANT_TYPE_MONALISA9:
        xs = xs_mon_ch9_m1;
        ys = ys_mon_ch9_m1;
        local_lut_size = sizeof(xs_mon_ch9_m1) / sizeof(xs_mon_ch9_m1[0]);
        break;
    case ANT_TYPE_JOLIE5:
        xs = xs_jol_ch5_m1;
        ys = ys_jol_ch5_m1;
        local_lut_size = sizeof(xs_jol_ch5_m1) / sizeof(xs_jol_ch5_m1[0]);
        break;
    case ANT_TYPE_JOLIE9:
        xs = xs_jol_ch9_m1;
        ys = ys_jol_ch9_m1;
        local_lut_size = sizeof(xs_jol_ch9_m1) / sizeof(xs_jol_ch9_m1[0]);
        break;
    case ANT_TYPE_CUSTOM:
        xs = xs_cst_chn_m1;
        ys = ys_cst_chn_m1;
        local_lut_size = sizeof(xs_cst_chn_m1) / sizeof(xs_cst_chn_m1[0]);
        break;
    default:
        break;
    }
}

static double pdoa2path_diff(float x)
{
    int i;
    double dx, dy;

    /* Check if the pointers are initialised*/
    if (local_lut_size == -1)
    {
        return 0;
    }

    if (x < xs[0])
    {
        return ys[0]; /* return minimum element */
    }

    if (x > xs[local_lut_size - 1])
    {
        return ys[local_lut_size - 1]; /* return maximum */
    }

    /* find i, such that xs[i] <= x < xs[i+1] */
    for (i = 0; i < local_lut_size - 1; i++)
    {
        if (xs[i + 1] > x)
        {
            break;
        }
    }

    /* interpolate */
    dx = xs[i + 1] - xs[i];
    dy = ys[i + 1] - ys[i];
    return ys[i] + (x - xs[i]) * dy / dx;
}

//-----------------------------------------------------------------------------
static void fpdoaaverage(float *inout, struct avrg_s *p)
{
    float accum = 0.f;
    int tmp_cnt = 0;

    if (p->lcnt >= p->avrg_max)
    {
        p->lcnt = 0;
    }

    p->max_cnt = (p->max_cnt < p->avrg_max) ? (p->max_cnt + 1) : (p->avrg_max);

    if (p->max_cnt > 0)
    {
        p->avrg[p->lcnt] = *inout;
        for (int i = 0; i < p->max_cnt; i++)
        {
            if (abs(p->avrg[p->lcnt]) > 130)
            { // average in the array of the same sign as avrg[lcnt]
                if ((p->avrg[p->lcnt] < 0 && p->avrg[i] < 0) || (p->avrg[p->lcnt] >= 0 && p->avrg[i] >= 0))
                {
                    accum += p->avrg[i];
                    tmp_cnt++;
                }
            }
            else
            {
                accum += p->avrg[i];
                tmp_cnt++;
            }
        }
        accum /= tmp_cnt;
        *inout = accum;
    }
    p->lcnt++;
    p->prev_avrg = *inout;
}


/* @brief Calculates the path_diff and AOA from PDoA.
 */
void fpdoa2aoa(struct fpdoa_in_s *pIn, struct pdoa_aoa_s *pRes, void *rx_ctx)
{
    float x_m, y_m, l_m, d_m;
    float p_diff_m; /* Path difference between the ports (m). */

    struct avrg_s *pavrg = (struct avrg_s *)rx_ctx;
    float pdoa_deg = (float)pIn->p_deg100 / 100.f;

    if (pavrg->avrg_max == 0)
    { /* initializing of pavrg struct */
        pavrg->avrg_max = pIn->max_avrg;
        /* avrg buff allocated on the next address after pavrg->avrg */
        pavrg->avrg = (float *)((int)pavrg + (int)sizeof(struct avrg_s));
    }

    if (pIn->chan == 5)
    {
        l_m = L_M_5;
        d_m = D_M_5;

        /* Shift the range of PDOAs out of the board */
        pdoa_deg = fmodf(pdoa_deg - PDOA_INTERVAL_SHIFT_CH5 + 540.0f, 360.0f) + PDOA_INTERVAL_SHIFT_CH5 - 180.0f;
    }
    else
    {
        l_m = L_M_9;
        d_m = D_M_9;

        /* Shift the range of PDOAs out of the board */
        pdoa_deg = fmodf(pdoa_deg - PDOA_INTERVAL_SHIFT_CH9 + 540.0f, 360.0f) + PDOA_INTERVAL_SHIFT_CH9 - 180.0f;
    }

    if ((abs(pdoa_deg) > 130.0f) && (pdoa_deg * pavrg->prev_avrg < 0.0f))
    { // if current value has different sign from prev average, use the original value;
        pdoa_deg = (float)pIn->p_deg100 / 100.f;
        // the same as
        // pdoa_deg = fmod(pdoa_deg - prev_avrg + 540.0f, 360.0f) + prev_avrg - 180.0f;
    }

    fpdoaaverage(&pdoa_deg, pavrg);

    if (pIn->corr_en)
    { /* Path difference (either LUT or just wave propagation theory). */
        p_diff_m = pdoa2path_diff(pdoa_deg);
    }
    else
    {
        p_diff_m = pdoa_deg / 360.0f * l_m;
    }

    /* x and y from path difference and range */
    /* X and y for the purpose of AoA: imagine the dist is 2m */
    float r_m = 2.0f;
    x_m = p_diff_m / d_m * r_m;
    float tmp = (x_m >= 0) ? (x_m) : (-1.f * x_m); /**< Do not optimize*/
    y_m = (tmp < r_m) ? sqrt(r_m * r_m - x_m * x_m) : 0.0f;

    /* results */
    tmp = atan2f(x_m, y_m);

    pRes->aoa_q11 = (int16_t)(tmp * 2048.0);
    pRes->pdoa_q11 = (int16_t)(pdoa_deg * 2048 * M_PI / 180.0); // normalized updated pdoa of the current input
}
