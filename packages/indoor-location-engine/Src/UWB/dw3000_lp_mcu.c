/**
 * @file    dw3000_lp_mcu.c
 *
 * @brief   DW3000 low-power implementation
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

#include "dw3000_lp_mcu.h"

#include "linux/ieee802154.h"
#include "linux/skbuff.h"


#define LP_DIAG_PRINTF(...) {}
#define LP_DIAG_PRINTF1(...) {}
#define LP_DIAG_PRINTF2(...) {}

extern u32 dw3000_get_dtu_time(struct dwchip_s *dw);

uint8_t update_channel_pcode = 0;
uwbmac_timer_t uwbmac_timer = {.timer_expires_callback = NULL};

const uint32_t RX_RELAX_START_DTU = US_TO_DTU(100); //Relaxed Rx start time

/* QM33120 chip require additional calibration to increase its performance
 */
static inline int calib_required(struct dwchip_s *dw)
{
    return (dw->dwt_driver->devid == DWT_QM33120_PDOA_DEV_ID || dw->dwt_driver->devid == DWT_QM33110_DEV_ID);
}

/* DEEPSLEEP_WAKE_CONSTANT_us is a time required to wakeup safely from the Deep Sleep
 */
static inline int get_DEEPSLEEP_WAKE_CONSTANT_us(struct dwchip_s *dw)
{
    return (calib_required(dw)) ? (DEEPSLEEP_WAKE_CONSTANT_QM33000_us) : (DEEPSLEEP_WAKE_CONSTANT_DW3000_us);
}


/* Continuous timebase (tmbase_dtu) is always in DTU.
 * On the start of the first initialization of the uwb-stack need to clear the timebase.
 * Later need to adjust this timebase every time chip is recovering (from DeepSleep or from IDLE_RC) to the IDLE_PLL.
 */
static void clear_timebase_dtu(struct dwchip_s *dw)
{
    dw->mcps_runtime->tmbase_dtu = 0;
}

static void save_timebase_dtu(struct dwchip_s *dw, uint32_t timebase)
{
    dw->mcps_runtime->tmbase_dtu = timebase & 0xFFFFFFFEUL;
}

uint32_t get_timebase_dtu(struct dwchip_s *dw)
{
    return (uint32_t)(dw->mcps_runtime->tmbase_dtu);
}


/* @brief Callback to be called by the hal_fs_timer
 */
static void lp_timer_fira(void *p_context)
{
    uint32_t rx_relax_to_pac; //relaxed Rx Timeout, calculated based on RX_RELAX_START_DTU

    dwchip_t *dw = (dwchip_t *)p_context;
    hal_fs_timer_t *htimer = (hal_fs_timer_t *)dw->mcps_runtime->deep_sleep_timer;

    const struct dwt_mcps_ops_s *ops = dw->dwt_driver->dwt_mcps_ops;
    struct dwt_mcps_runtime_s *rt = dw->mcps_runtime;
    struct dw3000_deep_sleep_state *dss = &dw->mcps_runtime->deep_sleep_state;
    uint32_t tmr_tick;

    static uint32_t cnt;

    tmr_tick = htimer->get_tick(htimer);

    LP_DIAG_PRINTF2("lp:");

    if (rt->current_operational_state == DW3000_OP_STATE_DEEP_SLEEP)
    {
        hal_uwb.wakeup_start();

        htimer->start(htimer, true, TMR_WAKE_TIME_US, tmr_tick);

        cnt = tmr_tick;

        rt->current_operational_state = DW3000_OP_STATE_WAKE_UP;

        struct spi_s *spi = hal_uwb.uwbs->spi;
        spi->slow_rate(spi->handler);

        LP_DIAG_PRINTF2("1:");
        LP_DEBUG_D1();
    }
    else if (rt->current_operational_state == DW3000_OP_STATE_WAKE_UP)
    {
        hal_uwb.wakeup_end();

        htimer->start(htimer, true, TMR_IDLE_RC_US, tmr_tick);

        cnt += tmr_tick;

        rt->current_operational_state = DW3000_OP_STATE_INIT_RC;

        /* fast SPI will be allowed after TMR_IDLE_RC_US pause */
        struct spi_s *spi = hal_uwb.uwbs->spi;
        spi->fast_rate(spi->handler);

        LP_DIAG_PRINTF2("2");
        LP_DEBUG_D1();
    }
    else if (rt->current_operational_state == DW3000_OP_STATE_INIT_RC)
    {
        /* Technically this is IDLE_RC. Using the _INIT_ name for DWT_RESTORECONFIG step */

        htimer->start(htimer, true, TMR_IDLE_PLL_US, tmr_tick);

        cnt += tmr_tick;

        /* verifying chip has woken up */
        uint32_t devid;
        ops->ioctl(dw, DWT_READ_REG, 0, (void *)&devid);
        if(!devid)
        {
            LP_DIAG_PRINTF1("Panic: devid is 0x0\r\n");
        }

        /* configure chip to IDLE_PLL */
        ops->ioctl(dw, DWT_SETDWSTATE, DWT_DW_IDLE, NULL);

        /* need to re-load the pre-set IV after deep-sleep for SP1/SP3 */
        ops->ioctl(dw, DWT_CONFIGURESTSLOADIV, 0, NULL);

        rt->current_operational_state = DW3000_OP_STATE_IDLE_RC;

        LP_DIAG_PRINTF2("3:0x%08x", devid);
        LP_DEBUG_D1();
    }
    else if (rt->current_operational_state == DW3000_OP_STATE_IDLE_RC)
    {
        uint32_t tmp = (calib_required(dw)) ? (TMR_CALIB_QM33_US) : (TMR_CALIB_DW3_US);

        htimer->start(htimer, true, tmp, tmr_tick);

        cnt += tmr_tick;

        uint32_t bitmask = 0;

        hal_uwb.disableIRQ();

        /* calibration of QM33xxx */
        ops->ioctl(dw, DWT_RESTORECONFIG, 0, NULL);

        /* workaround to drop the unwanted IRQ */
        ops->ioctl(dw, DWT_READSYSSTATUSLO, 0, (void *)&bitmask);
        ops->ioctl(dw, DWT_WRITESYSSTATUSLO, 0, (void *)&bitmask);

        hal_uwb.enableIRQ();

        rt->current_operational_state = DW3000_OP_STATE_IDLE_PLL;
        LP_DIAG_PRINTF2("4:");
        LP_DEBUG_D1();

    }
    else
    {
        int ret = 0;
        uint32_t new_timebase, new_local_txrx_date_dtu, dw_sysclock_at_C, tmp;

        ops->ioctl(dw, DWT_READSYSTIMESTAMPHI32, 0, (void *)&dw_sysclock_at_C);

        cnt += htimer->get_tick(htimer);

        dw_sysclock_at_C &= 0xFFFFFFFEUL;

        tmp = (calib_required(dw)) ? (TMR_ABC_QM33_US) : (TMR_ABC_DW3000_US);

        new_local_txrx_date_dtu = US_TO_DTU(get_DEEPSLEEP_WAKE_CONSTANT_us(dw) - tmp - cnt) + dw_sysclock_at_C;

        new_local_txrx_date_dtu &= 0xFFFFFFFE;

        /* keep htimer running during TWR to avoid jitter on start/stop */
        htimer->start(htimer, false, UINT32_MAX, 0);

        LP_DIAG_PRINTF2("5: c=0x%08x, cnt=0x%08x, nltrxd=0x%08x\r\n", dw_sysclock_at_C, cnt, new_local_txrx_date_dtu);

        if (dss->next_operational_state == DW3000_OP_STATE_TX)
        {
            new_timebase = dss->txops.tx_date_dtu - new_local_txrx_date_dtu;
            new_timebase &= 0xFFFFFFFE;
            save_timebase_dtu(dw, new_timebase);

            dss->txops.tx_date_dtu = new_local_txrx_date_dtu;
            dss->next_operational_state = DW3000_OP_STATE_IDLE_PLL;
            rt->current_operational_state = DW3000_OP_STATE_TX;

            if (dss->tx_skb)
            {
                ret = ops->tx_frame(dw, dss->tx_skb->data, dss->tx_skb->len + 2 /*IEEE802154_FCS_LEN*/, &dss->txops);

                free(dss->tx_skb->data);
                free(dss->tx_skb);
                dss->tx_skb = NULL;
            }
            else
            {
                ret = ops->tx_frame(dw, NULL, 0, &dss->txops);
            }

            LP_DIAG_PRINTF1("Tx: new_tb=0x%08x, nltdd=0x%08x, txdd=0x%08x", new_timebase, new_local_txrx_date_dtu, dss->txops.tx_date_dtu);
            LP_DIAG_PRINTF2("Tx:");
            LP_DEBUG_D1();
        }
        else if (dss->next_operational_state == DW3000_OP_STATE_RX)
        {
            new_timebase = dss->rxops.rx_date_dtu - new_local_txrx_date_dtu;
            new_timebase &= 0xFFFFFFFE;
            save_timebase_dtu(dw, new_timebase);

            dss->rxops.rx_date_dtu = new_local_txrx_date_dtu - RX_RELAX_START_DTU;

            rt->corr_4ns = 0; /* Reset adjusting of RX RCTU timestamp to 4ns */

            if (dss->rxops.rx_timeout_pac > 0)
            {
                rx_relax_to_pac = 2*(RX_RELAX_START_DTU * DW3000_CHIP_PER_DTU + rt->chips_per_pac - 1) / rt->chips_per_pac;
                dss->rxops.rx_timeout_pac += rx_relax_to_pac;
            }

            if(dss->rxops.rx_timeout_pac > UINT16_MAX)
            {
                dss->rxops.rx_timeout_pac = 0;
                
                uint32_t rx_timeout_dtu;
                rx_timeout_dtu = (dss->rxops.rx_timeout_pac * dw->mcps_runtime->chips_per_pac + 1 
                                                    - dw->mcps_runtime->chips_per_pac ) / DW3000_CHIP_PER_DTU;
                
                /* Timeout detect preamble is limited to UINT16, for Infrequent this does not work
                 * replacing with RX_Timeout */
                ops->ioctl(dw, DWT_SETRXTIMEOUT, 0, (void*)&rx_timeout_dtu);
            }

            dss->next_operational_state = DW3000_OP_STATE_IDLE_PLL;
            rt->current_operational_state = DW3000_OP_STATE_RX;

            ret = ops->rx_enable(dw, &dw->mcps_runtime->deep_sleep_state.rxops);

            LP_DIAG_PRINTF1("Rx: new_tb=0x%08x, nltdd=0x%08x, rxdd=0x%08x", new_timebase, new_local_txrx_date_dtu, dss->rxops.rx_date_dtu);
            LP_DIAG_PRINTF2("Rx:");
            LP_DEBUG_D1();
        }
        else if (dss->next_operational_state == DW3000_OP_STATE_MAC_EXIT_IDLE)
        {
            /* No low power is used for now so no need to set up a new timebase*/
            dss->next_operational_state = DW3000_OP_STATE_IDLE_PLL;
            rt->current_operational_state = DW3000_OP_STATE_MAC_EXIT_IDLE;
            if (uwbmac_timer.timer_expires_callback)
            {
                uwbmac_timer.timer_expires_callback();
            }
        }

        else
        {
            LP_DIAG_PRINTF2("Panic:");
            LP_DIAG_PRINTF1("Panic: Should not come here.\r\n");
        }

        if (ret)
        {
            LP_DIAG_PRINTF2("Late:");
            LP_DIAG_PRINTF1("Panic: TxRx late\r\n");
        }        
    }
    
    LP_DIAG_PRINTF2("\r\n");
}

void lp_timer_fira_init(struct dwchip_s *dw)
{
    /* if hal_fs_timer is NULL, then low-power sleep is disabled */
    dw->mcps_runtime->deep_sleep_timer = (void *)&hal_fs_timer;

    hal_fs_timer_t *htimer = (hal_fs_timer_t *)dw->mcps_runtime->deep_sleep_timer;

    /* When starting new process the time is counted from the base chip clock */
    clear_timebase_dtu(dw);

    /* timer expiration callback set to NULL*/
    uwbmac_timer.timer_expires_callback = NULL;


    if (htimer && htimer->init)
    {
        htimer->init(htimer, dw, lp_timer_fira);
    }
}


void lp_timer_fira_deinit(struct dwchip_s *dw)
{
    if (dw->mcps_runtime->deep_sleep_timer)
    {
        hal_fs_timer_t *htimer = (hal_fs_timer_t *)dw->mcps_runtime->deep_sleep_timer;
        if (htimer && htimer->init)
        {
            htimer->init(htimer, dw, NULL);
        }

        dw->mcps_runtime->deep_sleep_timer = NULL;
    }

    if (dw->mcps_runtime->deep_sleep_state.tx_skb)
    {
        free(dw->mcps_runtime->deep_sleep_state.tx_skb->data);
        free(dw->mcps_runtime->deep_sleep_state.tx_skb);
        dw->mcps_runtime->deep_sleep_state.tx_skb = NULL;
    }
}

static void update_channel_pcode_fn(struct dwchip_s *dw, uint32_t delay_us)
{
    dwt_config_t *config = dw->config->rxtx_config->pdwCfg;
    const struct dwt_ops_s *dwt_ops = dw->dwt_driver->dwt_ops;

    if(delay_us> MIN_TIME_FOR_CONFIG_US)
    {
        LP_DIAG_PRINTF2("Time > MIN: can update_channel_pcode_fn\r\n");
      
        if(update_channel_pcode)
        {
            hal_uwb.disableIRQ();
            dwt_ops->configure(dw, config);
            update_channel_pcode = 0;
            hal_uwb.enableIRQ();
        }
    }
}

/**
 * dw3000_rx_enable() - Enable RX
 * @dw: the DW device to put in RX mode
 * @rx_delayed:  true if RX delayed must be use
 * @date_dtu: the date at which RX must be enabled if @rx_delayed is true
 * @timeout_pac: the preamble detect timeout
 *
 * This function enable RX on DW3000, delayed or not, with a configurable
 * preamble detection timeout.
 *
 * Return: 0 on success, else a negative error code.
 */
int dw3000_rx_enable(struct dwchip_s *dw, int rx_delayed, uint32_t date_dtu, uint32_t timeout_pac)
{
    int ret = 0;
    uint32_t ts_dtu, delay_dtu, min_sleep_dtu, tmr_tick, wake_const_us;
    const struct dwt_ops_s *ops = dw->dwt_driver->dwt_ops;
    const struct dwt_mcps_ops_s *mcps_ops = dw->dwt_driver->dwt_mcps_ops;
    struct dwt_mcps_runtime_s *rt = dw->mcps_runtime;
    struct dw3000_deep_sleep_state *ddss = &dw->mcps_runtime->deep_sleep_state;
    hal_fs_timer_t *htimer = (hal_fs_timer_t *)dw->mcps_runtime->deep_sleep_timer;

    struct dw_rx_frame_info_s rxops = {
        .rx_date_dtu /* 4ns */ = date_dtu /* DTU in 4ns */,
        .rx_timeout_pac = timeout_pac,
        .rx_delayed = rx_delayed};

    wake_const_us = get_DEEPSLEEP_WAKE_CONSTANT_us(dw);
    min_sleep_dtu = US_TO_DTU(wake_const_us);

    if (htimer && htimer->get_tick)
    {
        ts_dtu = dw3000_get_dtu_time(dw);    /* this time is always in a "common" timebase */
        tmr_tick = htimer->get_tick(htimer); /* needs to be called as soon to dw3000_get_dtu_time as possible */
    }
    else
    {
        ts_dtu = dw3000_get_dtu_time(dw); /* this time is always in a "common" timebase */
        tmr_tick = 0;
    }

    delay_dtu = (date_dtu - ts_dtu);

    if (rx_delayed && htimer->start && !rt->need_ranging_clock && (delay_dtu - dw->llhw->shr_dtu) > min_sleep_dtu)
    {
        /* implementing only deep sleep power save */
        LP_DEBUG_D1();

        uint32_t timer_time_us = DTU_TO_US(delay_dtu);

        /* set timer to trigger slightly earlier to wake up the DW chip */
        htimer->start(htimer, true, timer_time_us - wake_const_us, tmr_tick);

        ddss->next_operational_state = DW3000_OP_STATE_RX;
        ddss->rxops.rx_date_dtu = rxops.rx_date_dtu;
        ddss->rxops.rx_timeout_pac = rxops.rx_timeout_pac;
        ddss->rxops.rx_delayed = rxops.rx_delayed;
        ddss->tx_skb = NULL;

        if (rt->current_operational_state > DW3000_OP_STATE_DEEP_SLEEP)
        {
            /* switching off the rx timeout such it would not trigger until programmed after wakeup */
            uint32_t rx_to_nill = 0;
            ops->ioctl(dw, DWT_SETRXTIMEOUT, 0, (void *)&rx_to_nill);

            /* can update channel and other configuration as there are plenty of time and can loose the timebase */
            update_channel_pcode_fn(dw, timer_time_us - wake_const_us);

            /* deep sleep now */
            ops->ioctl(dw, DWT_ENTERSLEEP, 0, NULL);
            rt->current_operational_state = DW3000_OP_STATE_DEEP_SLEEP;
        }

        LP_DIAG_PRINTF("RX LP: tmr_tick: %d 0x%08x\r\n", tmr_tick, timer_time_us - wake_const_us);
        LP_DEBUG_D1();
    }
    else
    {
        rxops.rx_date_dtu -= get_timebase_dtu(dw);
        
        /* Relaxing RX start time (a bit) to tolerate mutual ppm offset plus standard allowance */
        rxops.rx_date_dtu -= (RX_RELAX_START_DTU);
        if (rxops.rx_timeout_pac > 0)
        {
            rxops.rx_timeout_pac += 2*(RX_RELAX_START_DTU * DW3000_CHIP_PER_DTU + rt->chips_per_pac - 1) / rt->chips_per_pac;
        }

        ret = mcps_ops->rx_enable(dw, &rxops);
    }

    return (ret);
}


/**
 * dw3000_tx_frame() - prepare, execute or program TX
 * @dw: the DW device
 * @skb: TX socket buffer
 * @tx_delayed: true if TX delayed must be use
 * @tx_date_dtu: the date at which TX must be executed
 * @rx_delay_dly: positive if needed to active RX after TX otherwise null
 * @rx_timeout_pac: the preamble detect timeout
 *
 * This function prepares, executes or programs TX according to a given socket
 * buffer pointer provided by the MCPS.
 *
 * Return: zero on success, else a negative error code.
 */
int dw3000_tx_frame(struct dwchip_s *dw, struct sk_buff *skb, int tx_delayed, uint32_t tx_date_dtu,
                    int rx_delay_dly, uint32_t rx_timeout_pac)
{
    int ret = 0;
    uint32_t ts_dtu, delay_dtu, min_sleep_dtu, tmr_tick, wake_const_us;
    const struct dwt_ops_s *ops = dw->dwt_driver->dwt_ops;
    const struct dwt_mcps_ops_s *mcps_ops = dw->dwt_driver->dwt_mcps_ops;
    struct dwt_mcps_runtime_s *rt = dw->mcps_runtime;
    struct dw3000_deep_sleep_state *ddss = &dw->mcps_runtime->deep_sleep_state;
    hal_fs_timer_t *htimer = (hal_fs_timer_t *)dw->mcps_runtime->deep_sleep_timer;

    struct dw_tx_frame_info_s txops = {.tx_date_dtu /*4ns resolution*/ = tx_date_dtu /* DTU in 4ns resolution */,
                                       .rx_delay_dly = rx_delay_dly,
                                       .rx_timeout_pac = rx_timeout_pac};

    txops.flag = (tx_delayed) ? (DWT_START_TX_DELAYED) : (DWT_START_TX_IMMEDIATE);
    txops.flag |= (rx_delay_dly > 0) ? (DWT_RESPONSE_EXPECTED) : (0);

    wake_const_us = get_DEEPSLEEP_WAKE_CONSTANT_us(dw);
    min_sleep_dtu = US_TO_DTU(wake_const_us);

    if (htimer && htimer->get_tick)
    {
        ts_dtu = dw3000_get_dtu_time(dw);    /* this time is always in a "virtual" timebase */
        tmr_tick = htimer->get_tick(htimer); /* needs to be called as soon to dw3000_get_dtu_time as possible */
    }
    else
    {
        ts_dtu = dw3000_get_dtu_time(dw); /* this time is always in a "virtual" timebase */
        tmr_tick = 0;
    }

    delay_dtu = (tx_date_dtu - ts_dtu);

    rt->corr_4ns = tx_date_dtu & 0x01; // Tx delayed set in 4ns resolution, but actual Tx is happening on the even time of 8ns

    if (tx_delayed && htimer->start && !rt->need_ranging_clock && (delay_dtu - dw->llhw->shr_dtu) > min_sleep_dtu)
    {
        { /* implementing only deep sleep power save */
            LP_DEBUG_D0();

            uint32_t timer_time_us = DTU_TO_US(delay_dtu);

            /* set timer to trigger slightly earlier to wake up the DW chip */
            htimer->start(htimer, true, timer_time_us - wake_const_us, tmr_tick);

            rt->deep_sleep_state.next_operational_state = DW3000_OP_STATE_TX;

            memcpy(&ddss->txops, &txops, sizeof(ddss->txops));

            if(skb)
            {

                struct sk_buff *tx_skb;

                tx_skb = malloc(sizeof(struct sk_buff));
                tx_skb->data = malloc(skb->len);
                tx_skb->len = skb->len;
                memcpy(tx_skb->data, skb->data, skb->len);

                ddss->tx_skb = tx_skb;
            }
            else
            {
                ddss->tx_skb = NULL;
            }

            if (rt->current_operational_state > DW3000_OP_STATE_DEEP_SLEEP)
            {
                /* switching off the rx timeout such it would not trigger until programmed after wakeup */
                uint32_t rx_to_nill = 0;
                ops->ioctl(dw, DWT_SETRXTIMEOUT, 0, (void *)&rx_to_nill);

                /* can update channel and other configuration as there are plenty of time and can loose the timebase */
                update_channel_pcode_fn(dw, timer_time_us - wake_const_us);

                /* deep sleep now */
                ops->ioctl(dw, DWT_ENTERSLEEP, 0, NULL);
                rt->current_operational_state = DW3000_OP_STATE_DEEP_SLEEP;
            }

            LP_DIAG_PRINTF("will Tx after sleep: stx_date_dtu: 0x%08x00 tmr_tick:%d\r\n", txops.tx_date_dtu, tmr_tick);
            LP_DEBUG_D0();
        }
    }
    else
    {
        if (skb == NULL)
        {
            txops.tx_date_dtu -= get_timebase_dtu(dw);
            ret = mcps_ops->tx_frame(dw, NULL, 0, &txops);
        }
        else
        {
            txops.tx_date_dtu -= get_timebase_dtu(dw);
            ret = mcps_ops->tx_frame(dw, skb->data, skb->len + IEEE802154_FCS_LEN, &txops);
        }
    }

    return (ret);
}

/* END LOW POWER
 ************************************************************************/
