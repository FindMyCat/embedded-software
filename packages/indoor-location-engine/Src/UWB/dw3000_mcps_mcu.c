/**
 * @file    dw3000_mcps_mcu.c
 *
 * @brief   DW3000 MAC Common Part Sublayer (MCPS)
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

#include "_types.h"

#include <stddef.h>

#include "_types.h"

#include "HAL_uwb.h"
#include "HAL_error.h"
#include "HAL_rtc.h"
#include "critical_section.h"

#include "dw3000.h" //this only for a few constant
#include "translate.h"

#include "deca_interface.h"
#include "dw3000_calib_mcu.h"
#include "dw3000_mcps_wrapper.h"
#include "dw3000_mcps_mcu.h"
#include "dw3000_xtal_trim.h"
#include "dw3000_pdoa.h"
#include "dw3000_statistics.h"
#include "task_signal.h"
#include "int_priority.h"

#include "minmax.h"
#include "deca_dbg.h"

#include "dw3000_lp_mcu.h"
#include "create_mcps_Task.h"

extern uint8_t get_local_pavrg_size(void);
extern int get_rx_ctx_size(void);
extern uint8_t update_channel_pcode;

extern struct dwchip_s *dwt_update_dw(struct dwchip_s *new_dw);

static void dw3000_update_timings(struct dwchip_s *dw);
/* Used as a callback to timer expiration */
static void mcps_wakeup_mac_from_idle(void);

#define MCPS_TASK_STACK_SIZE_BYTES 1600

#define MCPS_DIAG_PRINTF(...) {}

#ifndef dev_dbg
#define dev_dbg(a, b, c) diag_printf(b, c)
#endif

#ifndef CLAMP
#define CLAMP(x, min, max) (MIN(max, MAX(x, min)))
#endif

#define sts_to_pdoa(s) ((s) ? 1 : 0)


#ifndef unlikely
#define unlikely(x) (0)
#endif

static task_signal_t mcpsTask;

static void McpsTask(void const *arg)
{
    struct mcps802154_llhw *local_llhw = (struct mcps802154_llhw *)arg;

    while (mcpsTask.Exit == 0)
    {
        osEvent evt = osSignalWait(mcpsTask.SignalMask, osWaitForever);

        if (evt.value.signals & STOP_TASK)
        {
            break;
        }
        else if (evt.value.signals & MCPS_TASK_TX_DONE)
        {
            mcps802154_tx_done(local_llhw);
        }
        else if (evt.value.signals & MCPS_TASK_RX_TIMEOUT)
        {
            mcps802154_rx_timeout(local_llhw);
        }
        else if (evt.value.signals & MCPS_TASK_RX_ERROR)
        {
            mcps802154_rx_error(local_llhw,
                                MCPS802154_RX_ERROR_OTHER);
        }
        else if (evt.value.signals & MCPS_TASK_RX)
        {
            mcps802154_rx_frame(local_llhw);
        }
        else if (evt.value.signals & MCPS_TASK_TIMER_EXPIRED)
        {
            mcps802154_timer_expired(local_llhw);
        }
        else
        {
            // do nothing
        }
    }

    mcpsTask.Exit = 2;
    while (mcpsTask.Exit == 2)
    {
        osThreadYield();
        osDelay(1);
    };
}

static void mcps_txdone_cb(const dwt_cb_data_t *txd)
{
    if (osSignalSet(mcpsTask.Handle, MCPS_TASK_TX_DONE) == 0x80000000)
    {
        error_handler(1, _ERR_Signal_Bad);
    }
}

static void mcps_rxtimeout_cb(const dwt_cb_data_t *rxd)
{
    if (osSignalSet(mcpsTask.Handle, MCPS_TASK_RX_TIMEOUT) == 0x80000000)
    {
        error_handler(1, _ERR_Signal_Bad);
    }
}

static void mcps_rxerror_cb(const dwt_cb_data_t *rxd)
{
    if (osSignalSet(mcpsTask.Handle, MCPS_TASK_RX_ERROR) == 0x80000000)
    {
        error_handler(1, _ERR_Signal_Bad);
    }
}

/* @brief     ISR layer
 *             TWR application Rx callback
 *             to be called from dwt_isr() as an Rx call-back
 * */
static void mcps_rx_cb(const dwt_cb_data_t *rxd)
{
#define MAX_MSG     2
#define MAX_MSG_LEN 128
    static uint8_t message[MAX_MSG][MAX_MSG_LEN];
    static dwt_mcps_rx_t rx[MAX_MSG];
    static int idx = 0;

    struct dwchip_s *dw = rxd->dw;
    const struct dwt_mcps_ops_s *mcps_ops = dw->dwt_driver->dwt_mcps_ops;
    struct dwt_mcps_runtime_s *rt = dw->mcps_runtime;

    struct dwt_mcps_rx_s *pRx = &rx[idx];
    uint64_t ts, timebase64;

    pRx->rtcTimeStamp = Rtc.getTimestamp();

    /* RX TS in RCTU of local timebase */
    ts = mcps_ops->get_timestamp(dw);

    /* convert local timebase in RCTU */
    timebase64 = timestamp_dtu_to_rctu(dw->llhw, get_timebase_dtu(dw));

    pRx->timeStamp = (ts + timebase64 + rt->corr_4ns) & 0xFFFFFFFFFFULL;

    pRx->flags = 0;

    if (rxd->datalength)
    {
        pRx->len = MIN(rxd->datalength, MAX_MSG_LEN);
        pRx->data = (uint8_t *)&message[idx];
        struct dwt_rw_data_s rd = {(uint8_t *)pRx->data, pRx->len, 0};
        mcps_ops->ioctl(dw, DWT_READRXDATA, 0, (void *)&rd);

        int16_t cfo;
        mcps_ops->ioctl(dw, DWT_READCLOCKOFFSET, 0, (void *)&cfo);

        pRx->cfo = cfo;
        /*Below Xtal trimming can be executed in the upper layer: rx[idx].cfo*/
        int cfo_ppm = (int)((float)cfo * (CLOCK_OFFSET_PPM_TO_RATIO * 1e6 * 100));
        dw->mcps_runtime->diag.cfo_ppm = cfo_ppm;
    }
    else
    {
        // most likely an SP3 packet --> no data
        pRx->len = 0;
        pRx->data = NULL;
        pRx->flags |= DW3000_RX_FLAG_ND;
    }

    dw->rx = pRx;

#if 0
    if (data->rx_flags & DW3000_CB_DATA_RX_FLAG_AAT)
            rx->flags |= DW3000_RX_FLAG_AACK;
#endif

    idx = (idx == MAX_MSG - 1) ? 0 : idx + 1;
    if (osSignalSet(mcpsTask.Handle, MCPS_TASK_RX) == 0x80000000)
    {
        error_handler(1, _ERR_Signal_Bad);
    }
}

static int dw3000_setcallbacks(struct dwchip_s *dw)
{
    dw->callbacks.cbSPIRDErr = NULL;
    dw->callbacks.cbSPIErr = NULL;
    dw->callbacks.cbSPIRdy = NULL;
    dw->callbacks.cbDualSPIEv = NULL;
    dw->callbacks.cbTxDone = mcps_txdone_cb;
    dw->callbacks.cbRxTo = mcps_rxtimeout_cb;
    dw->callbacks.cbRxErr = mcps_rxerror_cb;
    dw->callbacks.cbRxOk = mcps_rx_cb;
    return 0;
}

void dw3000_coex_stop(struct dwchip_s *dw)
{
      MCPS_DIAG_PRINTF("dw3000_coex_stop\r\n"); //TODO: can be used for WIFI COEX
}

/**
 * dw3000_get_dtu_time() - compute current DTU time
 * @dw: the DW device
 *
 * This function compute the current DTU time;
 * In Linux this is based on ktime, at 15.6MHz rate.
 *
 * In Embedded this is based on switching to a new "timebase" each time after DeepSleep.
 * The timebase adjustement has to be calculated and saved as "save_timebase(newtimebase_dtu)"
 *
 * Return: the current DTU time
 */
u32 dw3000_get_dtu_time(struct dwchip_s *dw)
{
    uint32_t ts;
    dw->dwt_driver->dwt_ops->ioctl(dw, DWT_READSYSTIMESTAMPHI32, 0, &ts);

    ts /= (DW3000_CHIP_PER_DTU / 2);

    ts += get_timebase_dtu(dw);
    return (ts);
}

static inline u32 tx_rmarker_offset(struct dwchip_s *dw, int ant_id)
{
    dwt_config_t *config = dw->config->rxtx_config->pdwCfg;
    // struct dw3000_config *config = &dw->config;
    const struct dw3000_antenna_calib *ant_calib;
    const struct dw3000_antenna_calib_prf *ant_calib_prf;
    int chanidx;
    int prfidx;
#ifdef LATER
    if (ant_id >= ANTMAX || ant_id < 0)
    {
        dev_err(dw->dev, "ant_id %d is out of antenna range, max is %d",
                ant_id, ANTMAX);
        return 0;
    }

    /* Current configured ant_id. */
    if (ant_id == config->ant[0])
        return config->rmarkerOffset;
#endif

    ant_calib = &dw->calib_data->ant[ant_id];

    chanidx = config->chan == 9 ? DW3000_CALIBRATION_CHANNEL_9 : DW3000_CALIBRATION_CHANNEL_5;
    prfidx = config->txCode >= 9 ? DW3000_CALIBRATION_PRF_64MHZ : DW3000_CALIBRATION_PRF_16MHZ;

    ant_calib_prf = &ant_calib->ch[chanidx].prf[prfidx];

    return ant_calib_prf->ant_delay;
}

//////////////////////////////////////////////////////////////////
static int start(struct mcps802154_llhw *llhw)
{
    struct dwchip_s *dw = llhw->priv;
    int ret;

    /* Configure antenna selection GPIO if any */
    ret = dw3000_config_antenna_gpios(dw);
    if (unlikely(ret))
        return ret;

    /* Reset ranging clock requirements */
    dw->mcps_runtime->need_ranging_clock = false;
    dw->mcps_runtime->current_operational_state = DW3000_OP_STATE_IDLE_PLL;

    /* Enable the DW3xxx IRQ */
    ret = dw3000_enable(dw);

    return (ret);
}

static void stop(struct mcps802154_llhw *llhw)
{
    struct dwchip_s *dw = llhw->priv;

    // FIXME: we will come here after broken_safe from fproc, i.e. when stack will get N sequencial rx timeouts
    // we cannot disable IRQ from DW3xxx as MAC is an event-driven system    
    /* Disable IRQ from DW3xxx */
    // dw3000_disable(dw);

    /* Reset ranging clock requirement */
    dw->mcps_runtime->need_ranging_clock = false;
}

static int tx_frame(struct mcps802154_llhw *llhw, struct sk_buff *skb,
                    struct mcps802154_tx_frame_config *info, int frame_idx, int next_delay_dtu)
{
    struct dwchip_s *dw = llhw->priv;
    struct dwt_mcps_runtime_s *rt = dw->mcps_runtime;

    u32 tx_date_dtu = 0;
    int rx_delay_dly = -1;
    u32 rx_timeout_pac = 0;
    int tx_delayed = 1;
    int rc;
    u8 sts_mode;

    info->timestamp_dtu &= 0xFFFFFFFE; /* DTU to actual DTU_TX */

    /* Check data : no data if SP3, must have data otherwise */
    if (((info->flags & MCPS802154_TX_FRAME_CONFIG_STS_MODE_MASK) == MCPS802154_TX_FRAME_CONFIG_SP3) != !skb)
        return -EINVAL;

    /* Enable STS */
    sts_mode = (info->flags & MCPS802154_TX_FRAME_CONFIG_STS_MODE_MASK) >> (__builtin_ffsll(MCPS802154_TX_FRAME_CONFIG_STS_MODE_MASK) - 1);
    rc = dw3000_set_sts(dw, sts_mode);
    if (unlikely(rc))
        return rc;
    rc = dw3000_setpdoa(dw, sts_to_pdoa(sts_mode));
    if (unlikely(rc))
        return rc;
    /* Ensure correct TX antenna is selected. */
    rc = dw3000_set_tx_antenna(dw, info->ant_set_id);
    if (unlikely(rc))
        return rc;

    /* Calculate the transfer date.*/
    if (info->flags & MCPS802154_RX_FRAME_INFO_TIMESTAMP_DTU)
    {
        tx_date_dtu = info->timestamp_dtu + llhw->shr_dtu;
    }
    else
    {
        /* Send immediately. */
        tx_delayed = 0;
    }

    if (info->rx_enable_after_tx_dtu > 0)
    {
        /* Disable auto-ack if it was previously enabled. */
        dw3000_disable_autoack(dw, false);

        /* Calculate the Rx after Tx delay. */
        rx_delay_dly = dtu_to_dly(llhw, info->rx_enable_after_tx_dtu);

        if (rx_delay_dly > DW3000_RX_ENABLE_STARTUP_DLY)
        {
            rx_delay_dly -= DW3000_RX_ENABLE_STARTUP_DLY;
        }
        else
        {
            /* Start Rx immediately after Tx. */
            rx_delay_dly = 0;
        }

        /* Calculate the after tx rx timeout. */
        if (info->rx_enable_after_tx_timeout_dtu == 0)
        {
            rx_timeout_pac = rt->pre_timeout_pac;
        }
        else if (info->rx_enable_after_tx_timeout_dtu != -1)
        {
            rx_timeout_pac = rt->pre_timeout_pac + dtu_to_pac(llhw, info->rx_enable_after_tx_timeout_dtu);
        }
        else
        {
            /* No timeout. */
        }
    }

    /* WRKRND: first Tx in a new ranging round does not require ranging clock before it */
    rt->need_ranging_clock &= !(bool)(info->flags & MCPS802154_TX_FRAME_CONFIG_RANGING_ROUND);

    int nok = dw3000_tx_frame(dw, skb, tx_delayed, tx_date_dtu, rx_delay_dly, rx_timeout_pac);

    /* requirement to keep ranging clock is a pre-requirement for the next Tx/Rx */
    rt->need_ranging_clock = (bool)(info->flags & MCPS802154_TX_FRAME_CONFIG_KEEP_RANGING_CLOCK);

    return (nok) ? (-UWBMAC_ETIME) : (0);
}

static int rx_enable(struct mcps802154_llhw *llhw,
                     const struct mcps802154_rx_frame_config *info, int frame_idx, int next_delay_dtu)
{
    struct dwchip_s *dw = (struct dwchip_s *)llhw->priv;
    struct dwt_mcps_runtime_s *rt = dw->mcps_runtime;

    u32 date_dtu = 0;
    u32 timeout_pac = 0;
    int rx_delayed = 1;
    int rc;
    u8 sts_mode;

    /* Enable STS */
    sts_mode = (info->flags & MCPS802154_RX_FRAME_CONFIG_STS_MODE_MASK) >> (__builtin_ffsll(MCPS802154_RX_FRAME_CONFIG_STS_MODE_MASK) - 1);
    rc = dw3000_set_sts(dw, sts_mode);
    if (unlikely(rc))
        return rc;

    rc = dw3000_setpdoa(dw, sts_to_pdoa(sts_mode));
    if (unlikely(rc))
        return rc;

    /* Ensure correct RX antenna are selected. */
    rc = dw3000_set_rx_antennas(dw, info->ant_set_id);
    if (unlikely(rc))
        return rc;

    /* Calculate the approximate Rx date in DTU units */
    if (info->flags & MCPS802154_RX_FRAME_CONFIG_TIMESTAMP_DTU)
    {
        date_dtu = info->timestamp_dtu - DW3000_RX_ENABLE_STARTUP_DTU;
    }
    else
    {
        /* Receive immediately. */
        rx_delayed = 0;
    }

    if (info->flags & MCPS802154_RX_FRAME_INFO_AACK)
    {
        dw3000_enable_autoack(dw, false);
    }
    else
    {
        dw3000_disable_autoack(dw, false);
    }

    /* Calculate the Rx timeout. */
    if (info->timeout_dtu == 0)
    {
        timeout_pac = rt->pre_timeout_pac;
    }
    else if (info->timeout_dtu != -1)
    {
        timeout_pac = rt->pre_timeout_pac + dtu_to_pac(llhw, info->timeout_dtu);
    }
    else
    {
        /* No timeout. */
    }

    int nok = dw3000_rx_enable(dw, rx_delayed, date_dtu, timeout_pac);

    /* Requirement to keep ranging clock is a pre-requirement for the next Tx/Rx.*/
    rt->need_ranging_clock = (bool)(info->flags & MCPS802154_RX_FRAME_CONFIG_KEEP_RANGING_CLOCK);

    return (nok) ? (-UWBMAC_ETIME) : (0);
}


static int rx_disable(struct mcps802154_llhw *llhw)
{
    struct dwchip_s *dw = (struct dwchip_s *)llhw->priv;

    dw->dwt_driver->dwt_mcps_ops->ioctl(dw, DWT_FORCETRXOFF, 0, 0);

    /* Reset ranging clock requirement */
    dw->mcps_runtime->need_ranging_clock = false;

    /* Release Wifi coexistence. */
    dw3000_coex_stop(dw);

    return 0;
}

/**
 * get_ranging_pdoa_fom() - compute the figure of merit of the PDoA.
 * @sts_fom: STS FoM on a received frame.
 *
 * If the STS FoM is less than sts_fom_threshold, PDoA FoM is 1, the worst.
 * If the STS FoM is greater or equal than sts_fom_threshold,
 * sts_fom_threshold to 255 values are mapped to 2 to 255.
 *
 * Return: the PDoA FoM value.
 */
static u8 get_ranging_pdoa_fom(u8 sts_fom)
{
    /* For a normalized STS FoM in 0 to 255, the STS is not reliable if
     * the STS FoM is less than 60 percents of its maximum value.
     */
    static const int sts_fom_threshold = 153;
    /* sts_fom_threshold .. sts_fom_max values are mapped to pdoa_fom_min .. pdoa_fom_max.
     * The relation is pdoa_fom = a * sts_fom + b, with
     * pdoa_fom_min =  sts_fom_threshold * a + b
     * pdoa_fom_max = sts_fom_max * a + b
     * So:
     * a = (pdoa_fom_max - pdoa_fom_min) / (sts_fom_max - sts_fom_threshold)
     * b = pdoa_fom_min - sts_fom_threshold * a
     */
    const int sts_fom_max = 255;
    const int pdoa_fom_min = 2;
    const int pdoa_fom_max = 255;
    const int a_numerator = pdoa_fom_max - pdoa_fom_min;
    const int a_denominator = sts_fom_max - sts_fom_threshold;
    const int b = pdoa_fom_min - ((sts_fom_threshold * a_numerator) / a_denominator);
    if (sts_fom < sts_fom_threshold)
        return 1;
    return ((a_numerator * sts_fom) / a_denominator) + b;
}

static int get_ranging_sts_fom(struct mcps802154_llhw *llhw,
                               struct mcps802154_rx_frame_info *info)
{
    struct dwchip_s *dw = (struct dwchip_s *)llhw->priv;
    const struct dwt_mcps_ops_s *mcps_ops = dw->dwt_driver->dwt_mcps_ops;
    dwt_config_t *config = dw->config->rxtx_config->pdwCfg;

    /* Max sts_acc_qual value depend on STS length */
    int sts_acc_max = 32 << config->stsLength;
    s16 sts_acc_qual;

    mcps_ops->ioctl(dw, DWT_READSTSQUALITY, 0, (void *)&sts_acc_qual);

    /* DW3000 only support one STS segment. */
    info->ranging_sts_fom[0] = (uint8_t)(CLAMP(1 + sts_acc_qual * 254 / sts_acc_max, 1, 255));

    /* Set FoM of all other segments to maximum value so that they do not
     * cause quality check failure. */
    memset(&info->ranging_sts_fom[1], 0xFF, MCPS802154_STS_N_SEGS_MAX - 1);

    return 0;
}

static int rx_get_frame(struct mcps802154_llhw *llhw, struct sk_buff **skb,
                        struct mcps802154_rx_frame_info *info)
{
    struct dwchip_s *dw = (struct dwchip_s *)llhw->priv;
    struct dwt_mcps_rx_s *rx = dw->rx;
    int ret = 0;

    /* Sanity check parameters */
    if (unlikely(!info || !skb))
    {
        ret = -EINVAL;
        goto error;
    }

    struct sk_buff *local_skb = malloc(sizeof(struct sk_buff));
    *skb = local_skb;

    /* Check buffer available */
    if (unlikely(!*skb && !(rx->flags & DW3000_RX_FLAG_ND)))
    {
            ret = UWBMAC_EAGAIN;
            goto error;
    }
    local_skb->data = dw->rx->data;
    local_skb->len = dw->rx->len;
    if (local_skb->len)
        local_skb->len -= IEEE802154_FCS_LEN;

    if (info->flags & (MCPS802154_RX_FRAME_INFO_TIMESTAMP_RCTU | MCPS802154_RX_FRAME_INFO_TIMESTAMP_DTU))
    {
        info->timestamp_rctu = rx->timeStamp - dw->config->rxtx_config->rxAntDelay;
        info->timestamp_dtu = timestamp_rctu_to_dtu(dw, rx->timeStamp) - llhw->shr_dtu;

        if (dw->mcps_runtime->diag.enable)
        {
            calculateStats(dw, &dw->mcps_runtime->diag);
        }
    }

    if (!(rx->flags & DW3000_RX_FLAG_ND))
    {
        /* Adjust Clock offset after RX of SP0/SP1 packets only */
        trim_XTAL_proc(dw, &dw->config->xtalTrim, dw->mcps_runtime->diag.cfo_ppm);
    }

    /* In case of auto-ack send. */
    if (rx->flags & DW3000_RX_FLAG_AACK)
    {
        info->flags |= MCPS802154_RX_FRAME_INFO_AACK;
    }

    /* In case of STS */
    if (info->flags & MCPS802154_RX_FRAME_INFO_RANGING_STS_TIMESTAMP_RCTU)
    {
        u64 sts_ts_rctu;

        sts_ts_rctu = rx->timeStamp;

        /* DW3000 only support one STS segment. */
        info->ranging_sts_timestamp_diffs_rctu[0] = 0;
        info->ranging_sts_timestamp_diffs_rctu[1] = sts_ts_rctu - rx->timeStamp;
    }

    info->ranging_sts_fom[0] = 0;
    if (info->flags & MCPS802154_RX_FRAME_INFO_RANGING_STS_FOM)
    {
        ret = get_ranging_sts_fom(llhw, info);
        if (ret)
        {
            goto error;
        }
    }

    if (info->flags & MCPS802154_RX_FRAME_INFO_RANGING_PDOA_FOM)
    {
        if (!(info->flags & MCPS802154_RX_FRAME_INFO_RANGING_STS_FOM))
        {
            ret = get_ranging_sts_fom(llhw, info);
            if (ret)
            {
                goto error;
            }
        }
        if (info->ranging_sts_fom[0])
        {
            info->ranging_pdoa_fom = get_ranging_pdoa_fom(info->ranging_sts_fom[0]);
        }
        else
        {
            info->ranging_pdoa_fom = 0;
        }
    }

    if (info->flags & MCPS802154_RX_FRAME_INFO_RANGING_OFFSET)
    {
        int16_t cfo;
        dw->dwt_driver->dwt_mcps_ops->ioctl(dw, DWT_READCLOCKOFFSET, 0, (void *)&cfo);
        info->ranging_offset_rctu = cfo;
        /* DW3000 provide directly the ratio (as Q26), so set arbitrarily the ranging
         * interval (denominator) to 1 */
        info->ranging_tracking_interval_rctu = 1 << 26;
    }

    /* Keep only implemented. */
    info->flags &= (MCPS802154_RX_FRAME_INFO_TIMESTAMP_RCTU |
                    MCPS802154_RX_FRAME_INFO_TIMESTAMP_DTU |
                    MCPS802154_RX_FRAME_INFO_AACK |
                    MCPS802154_RX_FRAME_INFO_RANGING_PDOA |
                    MCPS802154_RX_FRAME_INFO_RANGING_PDOA_FOM |
                    MCPS802154_RX_FRAME_INFO_RANGING_STS_FOM |
                    MCPS802154_RX_FRAME_INFO_RANGING_STS_TIMESTAMP_RCTU |
                    MCPS802154_RX_FRAME_INFO_RANGING_OFFSET);

    if ((*skb)->len == 0)
    {
        free(*skb);
        *skb = NULL;
    }

error:
    return ret;
}

/* uwb-stack guarantee the unique rx_ctx per ranging device.
 * rx_ctx ssize shall be requested over llhw->
 * uwb-stakc allocs & deallocs of rx_ctx.
 */
static int rx_get_measurement(struct mcps802154_llhw *llhw, void *rx_ctx,
                              struct mcps802154_rx_measurement_info *info)
{
    struct dwchip_s *dw = (struct dwchip_s *)llhw->priv;
    dwt_config_t *config = dw->config->rxtx_config->pdwCfg;

    // diag_printf("info->flags: 0x%08x\r\n", info->flags);

    if (hal_uwb.is_aoa() == AOA_ENABLED && info->flags & MCPS802154_RX_MEASUREMENTS_AOAS)
    {
        info->aoas[0].pdoa_rad_q11 = dw3000_readpdoa(dw);
        info->n_aoas = 1;

        /* rx_ctx is the allocated buffer used for AoA calculation */
        if (rx_ctx)
        {
            struct fpdoa_in_s in;
            struct pdoa_aoa_s out;

            in.chan = config->chan;
            in.corr_en = 1;
            in.max_avrg = get_local_pavrg_size();
            in.pdoa_q11 = info->aoas[0].pdoa_rad_q11;
            in.p_deg100 = 100 * (int32_t)((float)(in.pdoa_q11 / 2048.f) * 180.f / M_PI);

            fpdoa2aoa(&in, &out, rx_ctx); // current calculations based on in.p_deg100

            info->aoas[0].pdoa_rad_q11 = out.pdoa_q11;
            info->aoas[0].aoa_rad_q11 = out.aoa_q11;
        }
        else
        {
            info->aoas[0].aoa_rad_q11 = 0; // call direct .pdoa_to_aoa_lut(uwb, info->aoas[0].pdoa_rad_q11);
        }
    }
    else
    {
        info->aoas[0].pdoa_rad_q11 = 0;
        info->aoas[0].aoa_rad_q11 = 0;
        info->n_aoas = 0;
    }

    /* keep only implemented */
    info->flags = MCPS802154_RX_MEASUREMENTS_AOAS;

    return 0;
}


static int rx_get_error_frame(struct mcps802154_llhw *llhw,
                              struct mcps802154_rx_frame_info *info)
{
    int ret = 0;

    /* Sanity check */
    if (info == NULL)
    {
        ret = -1;
        goto error;
    }
    if (info->flags & MCPS802154_RX_FRAME_INFO_TIMESTAMP_RCTU)
    {
#ifdef LATER
        if (dw3000_read_rx_timestamp(dw, &info->timestamp_rctu))
            info->flags &= ~MCPS802154_RX_FRAME_INFO_TIMESTAMP_RCTU;
#endif
    }
    else
    {
        /* Not implemented */
        info->flags = 0;
    }
error:
    return ret;
}


static int idle(struct mcps802154_llhw *llhw, bool timestamp, u32 idle_duration_dtu)
{
    uint32_t cur_time_dtu = 0;
    uint32_t tmr_tick = 0;

    struct dwchip_s *dw = llhw->priv;

    struct dw3000_deep_sleep_state *dss = &dw->mcps_runtime->deep_sleep_state;
    hal_fs_timer_t *htimer = (hal_fs_timer_t *)dw->mcps_runtime->deep_sleep_timer;

    MCPS_DIAG_PRINTF("calling idle() from stack: bool:%d\r\n", timestamp);

    /* Reset ranging clock requirement. */
    dw->mcps_runtime->need_ranging_clock = false;

    dss->next_operational_state = DW3000_OP_STATE_MAC_EXIT_IDLE;

    if (timestamp)
    {

        idle_duration_dtu = idle_duration_dtu - llhw->anticip_dtu;
        if (idle_duration_dtu < 0)
            return -UWBMAC_ETIME;
        /* Get current dtu time*/
        cur_time_dtu = dw3000_get_dtu_time(dw);
        /* Get timer tick should be close to get_dtu_time*/
        tmr_tick = htimer->get_tick(htimer);
        /* Check if idle duration has expired*/
        if ((idle_duration_dtu - cur_time_dtu) < 0)
            return -UWBMAC_ETIME;

        /* This is the delay that is going to be used for the htimer*/
        uint32_t effective_delay_us = DTU_TO_US(idle_duration_dtu - cur_time_dtu);

        htimer->start(htimer, true, effective_delay_us, tmr_tick);

        /* Set the callback for timer expiring */
        uwbmac_timer.timer_expires_callback = mcps_wakeup_mac_from_idle;
    }
    return 0;
}


static int reset(struct mcps802154_llhw *llhw)
{
    struct dwchip_s *dw = llhw->priv;
    const struct dwt_mcps_ops_s *ops = dw->dwt_driver->dwt_mcps_ops;

    int ret = 0;

    enter_critical_section();

    hal_uwb.disableIRQ();
    hal_uwb.reset();
    hal_uwb.uwbs->spi->fast_rate(hal_uwb.uwbs->spi->handler);

    dw3000_setcallbacks(dw);

    /* Set time related field that are configuration dependent */
    dw3000_update_timings(dw);

    /* Initializing and configuring DW IC for MCPS */
    /* Will set callbacks to NULL inside ops->init */
    if (ops->init(dw))
    {
        MCPS_DIAG_PRINTF("Init failed.\r\n");
        ret = _ERR_INIT; // device initialise has failed
    }

    int tmp;
    { // MCPS applies antenna delays to TS manually
        tmp = 0;
        dw->dwt_driver->dwt_ops->ioctl(dw, DWT_SETTXANTENNADELAY, 0, (void *)&tmp);
        dw->dwt_driver->dwt_ops->ioctl(dw, DWT_SETRXANTENNADELAY, 0, (void *)&tmp);
    }

    { // PDoA offset
        if (dw->mcps_runtime->pdoa_offset < 0)
        {
            tmp = (int)(-1 * (dw->mcps_runtime->pdoa_offset) * M_PI) * (1 << 11) / 180;
        }
        else if (dw->mcps_runtime->pdoa_offset > 0)
        {
            tmp = (int)((360 - dw->mcps_runtime->pdoa_offset) * M_PI) * (1 << 11) / 180;
        }
        else
        {
            tmp = 0;
        }

        dw->dwt_driver->dwt_ops->ioctl(dw, DWT_SETPDOAOFFSET, 0, (void *)&tmp);
    }

    leave_critical_section(); /**< all RTOS tasks can be scheduled */

    return ret;
}

static int get_current_timestamp_dtu(struct mcps802154_llhw *llhw,
                                     u32 *timestamp_dtu)
{
    struct dwchip_s *dw = (struct dwchip_s *)llhw->priv;

#define DW3000_SPI_COLLISION_STATUS_BIT_MASK 0x1fU
    uint32_t data = DW3000_SPI_COLLISION_STATUS_BIT_MASK;
    /* WRKRND DW chip bug:
     * Reading of two systemtimes sequentially is possible only
     * if have any SPI transaction in between them
     */
    dw->dwt_driver->dwt_ops->ioctl(dw, DWT_WRITE_REG, 0x10020, (void *)data);

    *timestamp_dtu = dw3000_get_dtu_time(dw);

    return 0;
}


static uint64_t tx_timestamp_dtu_to_rmarker_rctu(struct mcps802154_llhw *llhw,
                                                 uint32_t tx_timestamp_dtu,
                                                 const struct mcps802154_hrp_uwb_params *hrp_uwb_params,
                                                 const struct mcps802154_channel *channel_params,
                                                 int ant_id)
{
    struct dwchip_s *dw = (struct dwchip_s *)llhw->priv;

    /* LSB is ignored. */
    uint32_t tmp;
    uint64_t rctu;

    tmp = (tx_timestamp_dtu + llhw->shr_dtu) & 0xFFFFFFFEUL;

    rctu = timestamp_dtu_to_rctu(llhw, tmp);

    uint64_t rctu_antdel = (rctu + dw->config->rxtx_config->txAntDelay) & 0xFFFFFFFFFFULL;

    return (rctu_antdel);
}

static s64 difference_timestamp_rctu(struct mcps802154_llhw *llhw,
                                     u64 timestamp_a_rctu,
                                     u64 timestamp_b_rctu)
{
    const u64 rctu_rollover = 1ll << 40;
    const u64 rctu_mask = rctu_rollover - 1;
    s64 diff_rctu = (timestamp_a_rctu - timestamp_b_rctu) & rctu_mask;
    if (diff_rctu & (rctu_rollover >> 1))
        diff_rctu = diff_rctu | ~rctu_mask;
    return diff_rctu;
}

static int compute_frame_duration_dtu(struct mcps802154_llhw *llhw,
                                      int payload_bytes)
{
    struct dwchip_s *dw = llhw->priv;
    dwt_config_t *config = dw->config->rxtx_config->pdwCfg;
    int chip_per_symb, phr_chip_per_symb, data_chip_per_symb;

    if (config->txCode >= 9)
        chip_per_symb = 508; //@64MHz
    else
        chip_per_symb = 496; //@16MHz

    if (config->dataRate == DWT_BR_850K)
    {
        phr_chip_per_symb = 512;
        data_chip_per_symb = 512;
    }
    else
    {
        phr_chip_per_symb = 512;
        data_chip_per_symb = 64;
    }

    /* STS part */
    const int sts_symb = config->stsMode == DWT_STS_MODE_OFF ? 0 : 32 << config->stsLength;
    const int sts_chips = sts_symb * chip_per_symb;

    /* PHR part. */
    static const int phr_tail_bits = 19 + 2;
    const int phr_chips = phr_tail_bits * phr_chip_per_symb; // 1 bit/symbol

    // Data part, 48 Reed-Solomon bits per 330 bits.
    const int data_bits = payload_bytes * 8;
    const int data_rs_bits = data_bits + (data_bits + 329) / 330 * 48;
    const int data_chips = data_rs_bits * data_chip_per_symb; // 1 bit/symbol

    // Done, convert to dtu.
    return llhw->shr_dtu + (sts_chips + phr_chips + data_chips) / DW3000_CHIP_PER_DTU;
}

static inline int dw3000_compute_shr_dtu(struct dwchip_s *dw)
{
    dwt_config_t *config = dw->config->rxtx_config->pdwCfg;

    const int symb = deca_to_plen(config->txPreambLength);
    const int sfd_symb = (config->dataRate == 0) && (config->sfdType == 1) ? 16 : 8;
    const int shr_symb = symb + sfd_symb;

    const int chip_per_symb = config->txCode >= 9 ? 508 : 496;

    return shr_symb * chip_per_symb / DW3000_CHIP_PER_DTU;
}

static inline int dw3000_compute_symbol_dtu(struct dwchip_s *dw)
{
    dwt_config_t *config = dw->config->rxtx_config->pdwCfg;

    const int chip_per_symb = config->txCode >= 9 ? 508 : 496;
    return chip_per_symb / DW3000_CHIP_PER_DTU;
}

static inline int dw3000_compute_chips_per_pac(struct dwchip_s *dw)
{
    // PAC means preamble accumulation count
    dwt_config_t *config = dw->config->rxtx_config->pdwCfg;
    int symb = deca_to_plen(config->txPreambLength);
    int pac_symb;

    if (symb <= 128)
    {
        pac_symb = 8;
    }
    else if (symb <= 512)
    {
        pac_symb = 16;
    }
    else if (symb <= 1024)
    {
        pac_symb = 32;
    }
    else
    {
        pac_symb = 64;
    }
    const int chip_per_symb = config->txCode >= 9 ? 508 : 496;

    return chip_per_symb * pac_symb;
}

static inline int dw3000_compute_pre_timeout_pac(struct dwchip_s *dw)
{
    /* Must be called AFTER dw->chips_per_pac initialisation */
    dwt_config_t *config = dw->config->rxtx_config->pdwCfg;
    int symb = deca_to_plen(config->txPreambLength);
    int pac_symb;

    if (symb <= 128)
    {
        pac_symb = 8;
    }
    else if (symb <= 512)
    {
        pac_symb = 16;
    }
    else if (symb <= 1024)
    {
        pac_symb = 32;
    }
    else
    {
        pac_symb = 64;
    }

    return (DW3000_RX_ENABLE_STARTUP_DLY * DW3000_CHIP_PER_DLY + dw->mcps_runtime->chips_per_pac - 1) / dw->mcps_runtime->chips_per_pac + symb / pac_symb + 2;
}

static void dw3000_update_timings(struct dwchip_s *dw)
{
    struct mcps802154_llhw *llhw = dw->llhw;
    /* Update configuration dependant timings */
    llhw->shr_dtu = dw3000_compute_shr_dtu(dw);
    llhw->symbol_dtu = dw3000_compute_symbol_dtu(dw);
    /* The CCA detection time shall be equivalent to 40 data symbol periods,
       Tdsym, for a nominal 850 kb/s, or equivalently, at least 8 (multiplexed)
       preamble symbols should be captured in the CCA detection time. */
    llhw->cca_dtu = 8 * llhw->symbol_dtu;
    dw->mcps_runtime->chips_per_pac = dw3000_compute_chips_per_pac(dw);
    dw->mcps_runtime->pre_timeout_pac = dw3000_compute_pre_timeout_pac(dw);
}

static int set_channel(struct mcps802154_llhw *llhw, u8 page, u8 channel,
                       u8 preamble_code)
{
    struct dwchip_s *dw = llhw->priv;
    dwt_config_t *config = dw->config->rxtx_config->pdwCfg;

    int error = 0;
    /* Check parameters early */
    if (page != 4 || (channel != 5 && channel != 9))
    {
        error = -1;
    }
    if (!error)
    {
        switch (preamble_code)
        {
        /* Set default value if MCPS don't give one to driver */
        case 0:
            preamble_code = 9;
            break;
        /* DW3000_PRF_16M */
        case 3:
        case 4:
        /* DW3000_PRF_64M */
        case 9:
        case 10:
        case 11:
        case 12:
            break;
        default:
            error = -1;
        }
    }
    if (!error)
    {
        config->chan = channel;
        config->txCode = preamble_code;
        config->rxCode = preamble_code;
    }
    if (!error)
    {
        if (config->chan != channel)
        {
            config->chan = channel;
            update_channel_pcode = 1;
        }

        if (config->txCode != preamble_code)
        {
            config->txCode = preamble_code;
            update_channel_pcode = 1;
        }

        if (config->rxCode != preamble_code)
        {
            config->rxCode = preamble_code;
            update_channel_pcode = 1;
        }
    }

    // Temporary implementation
    // Configuration field are updated in this function, and device will be reconfigured
    // after wake-up in dw3000_lp_mcu.c if required.

    return error;
}

static int set_hrp_uwb_params(struct mcps802154_llhw *llhw,
                              const struct mcps802154_hrp_uwb_params *params)
{
    return 0;
}

/**
 * @brief src(array) -> dst(uint32_t)
 * @src: *array16 bytes
 * @dst: *uint32_t[4]
 */
static inline void swap16(uint32_t *dst, const uint8_t *src)
{
    dst[3] = src[0] << 24 | src[1] << 16 | src[2] << 8 | src[3];
    dst[2] = src[4] << 24 | src[5] << 16 | src[6] << 8 | src[7];
    dst[1] = src[8] << 24 | src[9] << 16 | src[10] << 8 | src[11];
    dst[0] = src[12] << 24 | src[13] << 16 | src[14] << 8 | src[15];
}

static int set_sts_params(struct mcps802154_llhw *llhw,
                          const struct mcps802154_sts_params *arg)
{
    struct dwchip_s *dw = llhw->priv;
    const struct dwt_mcps_ops_s *ops = dw->dwt_driver->dwt_mcps_ops;

    int tmp, rc;
    uint32_t tmp32[4];
    dwt_sts_lengths_e len;

    tmp = (arg->seg_len == 32) ? (DWT_STS_LEN_32) : (arg->seg_len == 64) ? (DWT_STS_LEN_64)
                                                : (arg->seg_len == 128)  ? (DWT_STS_LEN_128)
                                                : (arg->seg_len == 256)  ? (DWT_STS_LEN_256)
                                                : (arg->seg_len == 512)  ? (DWT_STS_LEN_512)
                                                : (arg->seg_len == 1024) ? (DWT_STS_LEN_1024)
                                                : (arg->seg_len == 2048) ? (DWT_STS_LEN_2048)
                                                                         : (-1);

    if (tmp < 0)
    {
        return -EINVAL;
    }

    len = (dwt_sts_lengths_e)tmp;

    /* Check parameters */
    if (arg->n_segs > 1)
    {
        return -EINVAL;
    }
    /* Set STS segment(s) length */
    rc = dw3000_set_sts_length(dw, len);

    if (rc)
    {
        return rc;
    }
    /* Send KEY & IV */
    swap16(tmp32, arg->key);
    rc = ops->ioctl(dw, DWT_CONFIGURESTSKEY, 0, (void *)tmp32);
    if (rc)
    {
        return rc;
    }

    swap16(tmp32, arg->v);
    rc = ops->ioctl(dw, DWT_CONFIGURESTSIV, 0, (void *)tmp32);
    if (rc)
    {
        return rc;
    }
    return ops->ioctl(dw, DWT_CONFIGURESTSLOADIV, 0, NULL);
}


static int set_hw_addr_filt(struct mcps802154_llhw *llhw,
                            struct ieee802154_hw_addr_filt *filt,
                            unsigned long changed)
{
    struct dwchip_s *dw = llhw->priv;
    const struct dwt_mcps_ops_s *ops = dw->dwt_driver->dwt_mcps_ops;
    struct dw3000_deep_sleep_state *dss = &dw->mcps_runtime->deep_sleep_state;
    int ret = 0;


    if (dw->mcps_runtime->current_operational_state < DW3000_OP_STATE_IDLE_RC)
    {
        /* Cannot configure device, save info and ensure it will be
        configured on wakeup */
        dss->config_changed |= changed;
        return ret;
    }

    if (changed & IEEE802154_AFILT_SADDR_CHANGED)
    {
        ret = ops->ioctl(dw, DWT_SETADDRESS16, 0,
                         (void *)&filt->short_addr);
    }
    if (!ret && (changed & IEEE802154_AFILT_IEEEADDR_CHANGED))
    {
        ret = ops->ioctl(dw, DWT_SETEUI, 0, (void *)&filt->ieee_addr);
    }
    if (!ret && (changed & IEEE802154_AFILT_PANID_CHANGED))
    {
        ret = ops->ioctl(dw, DWT_SETPANID, 0, (void *)&filt->pan_id);
    }
    if (!ret && (changed & IEEE802154_AFILT_PANC_CHANGED))
    {
        /*if (filt->pan_coord)
        {
            struct dwt_configure_ff_s tmp = {1, DWT_FF_COORD_EN};
            ret = ops->ioctl(dw, DWT_CONFIGUREFRAMEFILTER, 0,
                             (void *)&tmp);
        }
        else*/
        {
            struct dwt_configure_ff_s tmp = {0, 0};
            ret = ops->ioctl(dw, DWT_CONFIGUREFRAMEFILTER, 0,
                             (void *)&tmp);
        }
    }
    return ret;
}

static int set_txpower(struct mcps802154_llhw *llhw, s32 mbm)
{
    // struct dw3000 *dw = llhw->priv;
    // dev_dbg(dw->dev, "%s called\n", __func__);
    return 0;
}

static int set_cca_mode(struct mcps802154_llhw *llhw,
                        const struct wpan_phy_cca *cca)
{
    // struct dw3000 *dw = llhw->priv;
    // dev_dbg(dw->dev, "%s called\n", __func__);
    return 0;
}

static int set_cca_ed_level(struct mcps802154_llhw *llhw, s32 mbm)
{
    // struct dw3000 *dw = llhw->priv;
    // dev_dbg(dw->dev, "%s called\n", __func__);
    return 0;
}

static int set_promiscuous_mode(struct mcps802154_llhw *llhw, bool on)
{
    struct dwchip_s *dw = llhw->priv;
    rxtx_configure_t *p = dw->config->rxtx_config;

    p->frameFilter = on ? DWT_FF_DISABLE : DWT_FF_ENABLE_802_15_4;
#if 0
    // Not needed, application dependent and set by the application
    p->frameFilterMode = DWT_FF_BEACON_EN | DWT_FF_DATA_EN | DWT_FF_ACK_EN |
                         DWT_FF_COORD_EN;
#endif

    struct dwt_configure_ff_s tmp = {p->frameFilter, p->frameFilterMode};

    dw->dwt_driver->dwt_mcps_ops->ioctl(dw, DWT_CONFIGUREFRAMEFILTER, 0,
                                        (void *)&tmp);
    return 0;
}

static int set_calibration(struct mcps802154_llhw *llhw, const char *key,
                           void *value, size_t length)
{
    // struct dwchip_s *dw = llhw->priv;
    int len = 0;
    /* Sanity checks */
    if (!key || !value || !length)
        return -1;
    /* The individual calibration data can be storred in the OTP of the DW3000 chip during mass production.
     * With a good approximation a mean default value can be used instead.
     * OTP calibration is not used in this example
     */
    return len;
}

static int get_calibration(struct mcps802154_llhw *llhw, const char *key,
                           void *value, size_t length)
{
    int len = 0;
    /* Sanity checks */
    if (!key)
        return -1;

    /* The individual calibration data can be storred in the OTP of the DW3000 chip during mass production.
     * With a good approximation a mean default value can be used instead.
     * OTP calibration is not used in this example
     */

    return len;
}

static const char *const *list_calibration(struct mcps802154_llhw *llhw)
{
    /* The individual calibration data can be storred in the OTP of the DW3000 chip during mass production.
     * With a good approximation a mean default value can be used instead.
     * OTP calibration is not used in this example
     */

    return NULL;
}

/**
 * vendor_cmd() - Forward vendor commands processing to dw3000 function.
 *
 * @llhw: Low-level hardware without MCPS.
 * @vendor_id: Vendor Identifier on 3 bytes.
 * @subcmd: Sub-command identifier.
 * @data: Null or data related with the sub-command.
 * @data_len: Length of the data
 *
 * Return: 0 on success, 1 to request a stop, error on other value.
 */
static int vendor_cmd(struct mcps802154_llhw *llhw, u32 vendor_id, u32 subcmd,
                      void *data, size_t data_len)
{
#ifdef LATER
    struct dwchip_s *dw = llhw->priv;
    struct do_vendor_params
    {
        u32 vendor_id;
        u32 subcmd;
        void *data;
        size_t data_len;
    };
    static int do_vendor_cmd(struct dw3000 * dw, const void *in, void *out)
    {
        const struct do_vendor_params *params = in;
        /* There is only NFCC coexitence vendor command for now. */
        return dw3000_nfcc_coex_vendor_cmd(dw, params->vendor_id,
                                           params->subcmd, params->data,
                                           params->data_len);
    }
    struct do_vendor_params params = {
        .vendor_id = vendor_id,
        .subcmd = subcmd,
        .data = data,
        .data_len = data_len,
    };
    struct dw3000_stm_command cmd = {do_vendor_cmd, &params, NULL};
    return dw3000_enqueue_generic(dw, &cmd);
#else
    return 0;
#endif
    return -1;
}

const struct mcps802154_ops mcps_mcu_ops = {
    .start = start,
    .stop = stop,
    .tx_frame = (int (*)(struct mcps802154_llhw *llhw, struct sk_buff *skb, const struct mcps802154_tx_frame_config *info, int frame_idx, int next_delay_dtu))tx_frame,
    .rx_enable = rx_enable,
    .rx_disable = rx_disable,
    .rx_get_frame = rx_get_frame,
    .rx_get_measurement = rx_get_measurement,
    .rx_get_error_frame = rx_get_error_frame,
    .idle = idle,
    .reset = reset,
    .get_current_timestamp_dtu = get_current_timestamp_dtu,
    .tx_timestamp_dtu_to_rmarker_rctu = tx_timestamp_dtu_to_rmarker_rctu,
    .difference_timestamp_rctu = difference_timestamp_rctu,
    .compute_frame_duration_dtu = compute_frame_duration_dtu,
    .set_channel = set_channel,
    .set_hrp_uwb_params = set_hrp_uwb_params,
    .set_sts_params = set_sts_params,
    .set_hw_addr_filt = set_hw_addr_filt,
    .set_txpower = set_txpower,
    .set_cca_mode = set_cca_mode,
    .set_cca_ed_level = set_cca_ed_level,
    .set_promiscuous_mode = set_promiscuous_mode,
    .set_calibration = set_calibration,
    .get_calibration = get_calibration,
    .list_calibration = list_calibration,
    .vendor_cmd = vendor_cmd,
    MCPS802154_TESTMODE_CMD(dw3000_tm_cmd)};

/*
 * probe of the DW chip driver
 *  input:
 *  dw
 *  dw->llhw
 *  dw->mcps_ops
 *
 *  output:
 *  +
 *  dw->SPI
 *  dw->wakeup_device_with_io
 *  dw->dwt_driver
 *  dw->calib_data
 */
static bool dw_chip_alloc(struct dwchip_s *dw)
{
    if (!(dw->calib_data = malloc(sizeof(struct dw3000_calibration_data))))
    {
        return false;
    }

    if (!(dw->mcps_runtime = calloc(1, sizeof(struct dwt_mcps_runtime_s))))
    {
        free(dw->calib_data);
        return false;
    }

    /* copy instances to the hal_uwb.uwbs */
    dw->SPI = hal_uwb.uwbs->dw->SPI;
    dw->wakeup_device_with_io = hal_uwb.uwbs->dw->wakeup_device_with_io;
    dw->dwt_driver = hal_uwb.uwbs->dw->dwt_driver;

    dw->priv = NULL;
    dw->coex_gpio_pin = -1;

    if (!dw->dwt_driver)
    {
        free(dw->mcps_runtime);
        free(dw->calib_data);
        return false;
    }

    return true;
}

static void dw_chip_free(struct dwchip_s *dw)
{
    if (!dw)
        return;
    free(dw->priv);
    free(dw->mcps_runtime);
    free(dw->calib_data);
}

void dw3000_mcps_free(struct dwchip_s *dw)
{
    if (dw)
    {
        if (dw->mcps_runtime && (dw->mcps_runtime->current_operational_state <= DW3000_OP_STATE_SLEEP))
        {
            hal_uwb.sleep_status_set(UWB_IS_SLEEPING);
        }
        else
        {
            hal_uwb.sleep_status_set(UWB_IS_NOT_SLEEPING);
        }

        dw_chip_free(dw);               // this will free priv, SPI and Queue, NOT dw
        mcps802154_free_llhw(dw->llhw); // will also free dw since it has been allocated by mcps802154_alloc_llhw()
        dw = NULL;
    }
}

struct dwchip_s *dw3000_mcps_alloc(void)
{
    struct mcps802154_llhw *llhw;
    struct dwchip_s *dw;

    hal_uwb.wakeup_with_io();

    llhw = mcps802154_alloc_llhw(sizeof(struct dwchip_s), &mcps_mcu_ops);
    if (llhw == NULL)
        return NULL;

    dw = llhw->priv;
    dw->llhw = llhw;
    dw->mcps_ops = (struct mcps802154_ops *)&mcps_mcu_ops;
    dw->mcps_runtime = NULL;

    if (!dw_chip_alloc(dw))
    {
        dw3000_mcps_free(dw);
        return NULL;
    }

    /* Configure IEEE802154 HW capabilities */
    llhw->hw->flags = (IEEE802154_HW_TX_OMIT_CKSUM | IEEE802154_HW_AFILT | IEEE802154_HW_PROMISCUOUS | IEEE802154_HW_RX_OMIT_CKSUM);
    llhw->flags = llhw->hw->flags;

    /* UWB High band 802.15.4a-2007. Only channels 5 & 9 for DW3000. */
    llhw->hw->phy->supported.channels[4] = (1 << 5) | (1 << 9);

    /* Set time related fields */
    llhw->dtu_freq_hz = DW3000_DTU_FREQ;
    llhw->dtu_rctu = DW3000_RCTU_PER_DTU;
    llhw->rstu_dtu = DW3000_DTU_PER_RSTU;
    llhw->anticip_dtu = 4 * (DW3000_DTU_FREQ / 1000);
    llhw->idle_dtu = DW3000_DTU_FREQ;

    llhw->rx_ctx_size = get_rx_ctx_size(); // stack will allocate this size per each "controlee"

    /* Set extended address. */
    llhw->hw->phy->perm_extended_addr = 0x11; // 0xd6552cd6e41ceb57;

    llhw->hw->phy->current_page = 4;

    return dw;
}

static struct dwchip_s *old_dw = NULL;

int dw3000_mcps_register(struct dwchip_s *dw)
{
    //      dev_dbg(dw->dev, "%s called\n", __FUNCTION__);
    int r = mcps802154_register_llhw(dw->llhw);

    mcpsTask.Exit = 0;

    mcpsTask.task_stack = NULL;

    error_e ret = create_mcps_task((void *)McpsTask, &mcpsTask, (uint16_t)MCPS_TASK_STACK_SIZE_BYTES, dw->llhw);
    if (ret != _NO_ERR)
    {
        error_handler(1, _ERR_Create_Task_Bad);
    }

    old_dw = dwt_update_dw(dw);
    reset(dw->llhw);

    /* init low-power timer */
    lp_timer_fira_init(dw);

    return r;
}

void dw3000_mcps_unregister(struct dwchip_s *dw)
{
    terminate_task(&mcpsTask);

    /* deinit low-power timer */
    lp_timer_fira_deinit(dw);

    mcps802154_unregister_llhw(dw->llhw);
    dwt_update_dw(old_dw);
}

void mcps_wakeup_mac_from_idle(void)
{
    if (osSignalSet(mcpsTask.Handle, MCPS_TASK_TIMER_EXPIRED) == 0x80000000)
    {
        error_handler(1, _ERR_Signal_Bad);
    }
}