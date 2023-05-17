/**
 * @file   dw3000_mcps_wrapper.h
 *
 * @brief  DW3000 MAC Common Part Sublayer (MCPS)
 *
 * @author Decawave Application
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

#ifndef DW3000_MCPS_WRAPPER_H
#define DW3000_MCPS_WRAPPER_H

#include "deca_dbg.h"

#include "linux/skbuff.h"
#include "linux/ieee802154.h"
#include "_types.h"
#include "linux/errno.h"
#include "deca_interface.h"
#include "dw3000_mcps_mcu.h"
#include "HAL_uwb.h"
#include "uwbmac.h"

#include "HAL_gpio.h"
#include "HAL_timer.h"
#include "HAL_sleep.h"

/**
 * dw3000_rx_disable() - Disable RX
 * @dw: the DW device to put back in IDLE state
 *
 * Return: 0 on success, else a negative error code.
 */
static inline int dw3000_rx_disable(struct dwchip_s *dw)
{
    return dw->dwt_driver->dwt_mcps_ops->rx_disable(dw);
}

/**
 * dw3000_setpdoa() - set device's PDOA mode
 * @dw: the DW device
 * @mode: the PDOA mode
 *
 * Return: zero on success, else a negative error code.
 */
static inline int dw3000_setpdoa(struct dwchip_s *dw, u8 mode)
{
    return 0;
}

/**
 * dw3000_readpdoa - Read the PDOA result.
 * @dw: The DW device.
 *
 * This is used to read the PDOA result, it is the phase difference between
 * either the Ipatov and STS POA, or the two STS POAs, depending on the PDOA
 * mode of operation. (POA - Phase Of Arrival).
 * NOTE: To convert to degrees: float pdoa_deg =
 * ((float)pdoa / (1 << 11)) * 180 / M_PI.
 *
 * Return: The PDOA result (signed in [1:-11] radian units).
 */
static inline s16 dw3000_readpdoa(struct dwchip_s *dw)
{
    int16_t tmp;
    dw->dwt_driver->dwt_mcps_ops->ioctl(dw, DWT_READPDOA, 0, (void *)&tmp);
    return tmp;
}

/**
 * dw3000_setsts() - set device's STS mode
 * @dw: the DW device
 * @mode: the STS mode
 *
 * Return: zero on success, else a negative error code.
 */
static inline int dw3000_set_sts(struct dwchip_s *dw, u8 mode)
{
    uint8_t stsMode = mode;

    return dw->dwt_driver->dwt_mcps_ops->ioctl(dw, DWT_CONFIGURESTSMODE, 0, (void *)&stsMode);
}

/**
 * dw3000_enable_autoack() - Enable the autoack for futures rx.
 * @dw: The DW device.
 * @force: Enable even if it was already in enable state.
 *
 * Return: zero on success, else a negative error code.
 */
static inline int dw3000_enable_autoack(struct dwchip_s *dw, bool force)
{
    dw->dwt_driver->dwt_mcps_ops->mcps_compat.ack_enable(dw, (int)true);
    return 0;
}

/**
 * dw3000_disable_autoack() - Disable the autoack for futures rx.
 * @dw: The DW device.
 * @force: Disable even if it was already in disable state.
 *
 * Return: zero on success, else a negative error code.
 */
static inline int dw3000_disable_autoack(struct dwchip_s *dw, bool force)
{
    dw->dwt_driver->dwt_mcps_ops->mcps_compat.ack_enable(dw, (int)false);
    return 0;
}

/**
 * dw3000_config_antenna_gpios() - Set configuration for all used GPIO
 * @dw: The DW device.
 *
 * This function configure all GPIO found in antenna table.
 * It is called before enabling the DW device in start() MCPS API.
 *
 * Return: zero on success, else a negative error code.
 */
static inline int dw3000_config_antenna_gpios(struct dwchip_s *dw)
{
    return 0;
}

/**
 * dw3000_set_tx_antenna() - Configure device to use selected antenna for TX
 * @dw: The DW device.
 * @antidx: The antenna to use for next TX
 *
 * Prepare the DW device to transmit frame using the specified antenna.
 * The required HW information (port, gpio and gpio value) must be set
 * correctly inside calibration data structure.
 *
 * Return: zero on success, else a negative error code.
 */
static inline int dw3000_set_tx_antenna(struct dwchip_s *dw, int antidx)
{
    return 0;
}

/**
 * dw3000_set_rx_antennas() - Set GPIOs to use selected antennas for RX
 * @dw: The DW device.
 * @ant_pair: The antennas pair to use
 *
 * Return: zero on success, else a negative error code.
 */
static inline int dw3000_set_rx_antennas(struct dwchip_s *dw, int ant_pair)
{
    return 0;
}

/**
 * dw3000_enable() - Enable the device
 * @dw: the DW device
 *
 * This function masks the device's internal interruptions (fixed
 * configuration) then enables the IRQ linked to the device interruption line.
 *
 * Return: zero on success, else a negative error code.
 */
static inline int dw3000_enable(struct dwchip_s *dw)
{
    /* enable MCU IRQ from DW3000 */
    hal_uwb.enableIRQ(); /**< enable MCU IRQ from DW3xxx */
    return 0;
}

/**
 * dw3000_disable() - Disable the device
 * @dw: the DW device
 *
 * This function disables the IRQ linked to the device interruption line,
 * unmasks the device's internal interruptions then clears its pending
 * interruptions.
 *
 * Return: zero on success, else a negative error code.
 */
static inline int dw3000_disable(struct dwchip_s *dw)
{
    const struct dwt_mcps_ops_s *ops = dw->dwt_driver->dwt_mcps_ops;

    /* Disable receiver and transmitter */
    ops->ioctl(dw, DWT_FORCETRXOFF, 0, 0);
    /* Disable further interrupt generation */
    ops->mcps_compat.set_interrupt(dw, 0xFFFFFFFFUL, 0xFFFFFFFFUL, DWT_DISABLE_INT);

    hal_uwb.disableIRQ(); /**< disable MCU IRQ from DW3xxx */

    return 0;
}

/**
 * dw3000_set_sts_length() - set device's STS length
 * @dw: the DW device
 * @len: length of the STS in blocks of 8 symbols
 *
 * Return: zero on success, else a negative error code.
 */
static int dw3000_set_sts_length(struct dwchip_s *dw, dwt_sts_lengths_e len)
{
    const struct dwt_mcps_ops_s *ops = dw->dwt_driver->dwt_mcps_ops;

    dwt_config_t *config = dw->config->rxtx_config->pdwCfg;

    int ret = 0;

    if (config->stsLength != len)
    {
        ret = ops->ioctl(dw, DWT_SET_STS_LEN, 0, (void *)(&len));

        if (ret < 0)
        {
            return UWBMAC_ENOENT;
        }
        config->stsLength = len;

        ret = ops->ioctl(dw, DWT_CFG_STS, 0, config);
    }

    return (ret);
}


static inline int dtu_to_pac(struct mcps802154_llhw *llhw, int timeout_dtu)
{
    struct dwchip_s *dw = llhw->priv;
    return (timeout_dtu * DW3000_CHIP_PER_DTU + dw->mcps_runtime->chips_per_pac - 1) / dw->mcps_runtime->chips_per_pac;
}

static inline int dtu_to_dly(struct mcps802154_llhw *llhw, int dtu)
{
    return (dtu * DW3000_CHIP_PER_DTU / DW3000_CHIP_PER_DLY);
}

static inline u32 timestamp_rctu_to_dtu(struct dwchip_s *dw, u64 timestamp_rctu)
{
    uint32_t dtu;
    dtu = (u32)(timestamp_rctu / DW3000_RCTU_PER_DTU);
    return (dtu);
}

static inline u64 timestamp_dtu_to_rctu(struct mcps802154_llhw *llhw,
                                        u32 timestamp_dtu)
{
    uint64_t rctu;
    rctu = ((u64)timestamp_dtu) * DW3000_RCTU_PER_DTU;
    return (rctu);
}

/**
 * dw3000_wakeup_timer_start() - Program wakeup timer
 * @dw: the DW device on which the SPI transfer will occurs
 * @delay_us: the delay after the DW device is woken up
 *
 * The timer is configured to wake-up the device after the specified delay.
 * Caller should take care of wake-up latency.
 *
 * Return: zero on success, else a negative error code.
 */
void dw3000_wakeup_timer_start(struct dwchip_s *dw, int delay_us)
{
    //	hrtimer_start(&dw->deep_sleep_timer, ns_to_ktime(delay_us * 1000),
    //		      HRTIMER_MODE_REL);
    //	dw->call_timer_expired = true;
    //	trace_dw3000_wakeup_timer_start(dw, delay_us);
    usleep(delay_us);
    /* Entered DEEP SLEEP from mcps802154_ops.idle() */
    mcps802154_timer_expired(dw->llhw);
}

#endif // DW3000_MCPS_WRAPPER_H
