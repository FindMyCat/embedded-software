/*
 * Copyright (c) 2020–2021 Qorvo, Inc
 *
 * All rights reserved.
 *
 * NOTICE: All information contained herein is, and remains the property of
 * Qorvo, Inc. and its suppliers, if any. The intellectual and technical
 * concepts herein are proprietary to Qorvo, Inc. and its suppliers, and may
 * be covered by patents, patent applications, and are protected by trade
 * secret and/or copyright law. Dissemination of this information or
 * reproduction of this material is strictly forbidden unless prior written
 * permission is obtained from Qorvo, Inc.
 */

#ifndef NET_MCPS802154_H
#define NET_MCPS802154_H

#include <net/mac802154.h>
#include <crypto/aes.h>

/* Antennas set id to use for transmission by default. */
#define TX_ANT_SET_ID_DEFAULT 0
/* Antennas set id to use for reception by default. */
#define RX_ANT_SET_ID_DEFAULT 0

/** Maximum number of STS segments. */
#define MCPS802154_STS_N_SEGS_MAX 4

/** Maximum number of RSSI values. */
#define MCPS802154_RSSIS_N_MAX 2

/** Maximum number of angle of arrival measurements. */
#define MCPS802154_RX_AOA_MEASUREMENTS_MAX 3

/**
 * struct mcps802154_channel - Channel parameters.
 */
struct mcps802154_channel {
	/**
	 * @page: Channel page used in conjunction with channel to uniquely
	 * identify the channel.
	 */
	int page;
	/**
	 * @channel: RF channel to use for all transmissions and receptions.
	 */
	int channel;
	/**
	 * @preamble_code: Preamble code index for HRP UWB. Must be zero for
	 * other PHYs.
	 */
	int preamble_code;
};

/**
 * enum mcps802154_llhw_flags - Low-level hardware without MCPS flags.
 * @MCPS802154_LLHW_RDEV:
 *	Support for ranging (RDEV). TODO: move to &ieee802154_hw.
 * @MCPS802154_LLHW_ERDEV:
 *	Support for enhanced ranging (ERDEV). TODO: move to &ieee802154_hw.
 * @MCPS802154_LLHW_BPRF:
 *	Support for BPRF.
 * @MCPS802154_LLHW_HPRF:
 *	Support for HPRF.
 * @MCPS802154_LLHW_DATA_RATE_850K:
 *	Support for data rate 110 kpbs.
 * @MCPS802154_LLHW_DATA_RATE_6M81:
 *	Support for data rate 6.81 Mpbs.
 * @MCPS802154_LLHW_DATA_RATE_7M80:
 *	Support for data rate 7.8 Mpbs.
 * @MCPS802154_LLHW_DATA_RATE_27M2:
 *	Support for data rate 27.2 Mpbs.
 * @MCPS802154_LLHW_DATA_RATE_31M2:
 *	Support for data rate 31.2 Mpbs.
 * @MCPS802154_LLHW_DATA_RATE_CUSTOM:
 *	Support for custom data rate, When presents extra data rate are
 *	possible to set.
 * @MCPS802154_LLHW_PHR_DATA_RATE_850K:
 *	Support PHR data rate 850 kpbs.
 * @MCPS802154_LLHW_PHR_DATA_RATE_6M81:
 *	Support PHR data rate 6.81 Mpbs.
 * @MCPS802154_LLHW_PRF_4:
 *	Support Pulse Repetition Frequency 4 MHz.
 * @MCPS802154_LLHW_PRF_16:
 *	Support Pulse Repetition Frequency 16 MHz.
 * @MCPS802154_LLHW_PRF_64:
 *	Support Pulse Repetition Frequency 64 MHz.
 * @MCPS802154_LLHW_PRF_125:
 *	Support Pulse Repetition Frequency 125 MHz.
 * @MCPS802154_LLHW_PRF_250:
 *	Support Pulse Repetition Frequency 250 MHz.
 * @MCPS802154_LLHW_PSR_16:
 *	Support 16 symbols in preamble symbol repetitions.
 * @MCPS802154_LLHW_PSR_24:
 *	Support 24 symbols in preamble symbol repetitions.
 * @MCPS802154_LLHW_PSR_32:
 *	Support 32 symbols in preamble symbol repetitions.
 * @MCPS802154_LLHW_PSR_48:
 *	Support 48 symbols in preamble symbol repetitions.
 * @MCPS802154_LLHW_PSR_64:
 *	Support 64 symbols in preamble symbol repetitions.
 * @MCPS802154_LLHW_PSR_96:
 *	Support 96 symbols in preamble symbol repetitions.
 * @MCPS802154_LLHW_PSR_128:
 *	Support 128 symbols in preamble symbol repetitions.
 * @MCPS802154_LLHW_PSR_256:
 *	Support 256 symbols in preamble symbol repetitions.
 * @MCPS802154_LLHW_PSR_1024:
 *	Support 1024 symbols in preamble symbol repetitions.
 * @MCPS802154_LLHW_PSR_4096:
 *	Support 4096 symbols in preamble symbol repetitions.
 * @MCPS802154_LLHW_SFD_4A:
 *	Support SFD defined in 4a.
 * @MCPS802154_LLHW_SFD_4Z_4:
 *	Support SFD defined in 4z with length of 4 symbols.
 * @MCPS802154_LLHW_SFD_4Z_8:
 *	Support SFD defined in 4z with length of 8 symbols.
 * @MCPS802154_LLHW_SFD_4Z_16:
 *	Support SFD defined in 4z with length of 16 symbols.
 * @MCPS802154_LLHW_SFD_4Z_32:
 *	Support SFD defined in 4z with length of 32 symbols.
 * @MCPS802154_LLHW_STS_SEGMENT_1:
 *	Support one STS segment.
 * @MCPS802154_LLHW_STS_SEGMENT_2:
 *	Support two STS segments.
 * @MCPS802154_LLHW_STS_SEGMENT_3:
 *	Support three STS segments.
 * @MCPS802154_LLHW_STS_SEGMENT_4:
 *	Support four STS segments.
 * @MCPS802154_LLHW_AOA_AZIMUTH:
 *	Support AOA azimuth [-90°,+90°].
 * @MCPS802154_LLHW_AOA_AZIMUTH_FULL:
 *	Support AOA full azimuth [-180°,+180°].
 * @MCPS802154_LLHW_AOA_ELEVATION:
 *	Support AOA elevation [-90°,+90°].
 * @MCPS802154_LLHW_AOA_FOM:
 *	Support AOA figure of merit.
 */
enum mcps802154_llhw_flags {
	MCPS802154_LLHW_RDEV = BIT(0),
	MCPS802154_LLHW_ERDEV = BIT(1),
	MCPS802154_LLHW_BPRF = BIT(2),
	MCPS802154_LLHW_HPRF = BIT(3),
	MCPS802154_LLHW_DATA_RATE_850K = BIT(4),
	MCPS802154_LLHW_DATA_RATE_6M81 = BIT(5),
	MCPS802154_LLHW_DATA_RATE_7M80 = BIT(6),
	MCPS802154_LLHW_DATA_RATE_27M2 = BIT(7),
	MCPS802154_LLHW_DATA_RATE_31M2 = BIT(8),
	MCPS802154_LLHW_DATA_RATE_CUSTOM = BIT(9),
	MCPS802154_LLHW_PHR_DATA_RATE_850K = BIT(10),
	MCPS802154_LLHW_PHR_DATA_RATE_6M81 = BIT(11),
	MCPS802154_LLHW_PRF_4 = BIT(12),
	MCPS802154_LLHW_PRF_16 = BIT(13),
	MCPS802154_LLHW_PRF_64 = BIT(14),
	MCPS802154_LLHW_PRF_125 = BIT(15),
	MCPS802154_LLHW_PRF_250 = BIT(16),
	MCPS802154_LLHW_PSR_16 = BIT(17),
	MCPS802154_LLHW_PSR_24 = BIT(18),
	MCPS802154_LLHW_PSR_32 = BIT(19),
	MCPS802154_LLHW_PSR_48 = BIT(20),
	MCPS802154_LLHW_PSR_64 = BIT(21),
	MCPS802154_LLHW_PSR_96 = BIT(22),
	MCPS802154_LLHW_PSR_128 = BIT(23),
	MCPS802154_LLHW_PSR_256 = BIT(24),
	MCPS802154_LLHW_PSR_1024 = BIT(25),
	MCPS802154_LLHW_PSR_4096 = BIT(26),
	MCPS802154_LLHW_SFD_4A = BIT(27),
	MCPS802154_LLHW_SFD_4Z_4 = BIT(28),
	MCPS802154_LLHW_SFD_4Z_8 = BIT(29),
	MCPS802154_LLHW_SFD_4Z_16 = BIT(30),
	MCPS802154_LLHW_SFD_4Z_32 = BIT(31),
	MCPS802154_LLHW_STS_SEGMENT_1 = BIT_ULL(32),
	MCPS802154_LLHW_STS_SEGMENT_2 = BIT_ULL(33),
	MCPS802154_LLHW_STS_SEGMENT_3 = BIT_ULL(34),
	MCPS802154_LLHW_STS_SEGMENT_4 = BIT_ULL(35),
	MCPS802154_LLHW_AOA_AZIMUTH = BIT_ULL(36),
	MCPS802154_LLHW_AOA_AZIMUTH_FULL = BIT_ULL(37),
	MCPS802154_LLHW_AOA_ELEVATION = BIT_ULL(38),
	MCPS802154_LLHW_AOA_FOM = BIT_ULL(39),
};

/**
 * struct mcps802154_llhw - Low-level hardware without MCPS.
 *
 * This must be allocated with mcps802154_alloc_llhw(). The low-level driver
 * should then initialize it.
 */
struct mcps802154_llhw {
	/**
	 * @dtu_freq_hz: Inverse of device time unit duration, in Hz.
	 */
	int dtu_freq_hz;
	/**
	 * @symbol_dtu: Symbol duration in device time unit, can change if radio
	 * parameters are changed. Can be set to 1 if device time unit is the
	 * symbol.
	 */
	int symbol_dtu;
	/**
	 * @cca_dtu: CCA duration in device time unit, can change if radio
	 * parameters or CCA modes are changed.
	 */
	int cca_dtu;
	/**
	 * @shr_dtu: Synchronisation header duration in device time unit, can
	 * change if radio parameters are changed. If ranging is supported, this
	 * is the difference between the RMARKER and the first frame symbol.
	 */
	int shr_dtu;
	/**
	 * @dtu_rctu: Duration of one device time unit in ranging counter time
	 * unit (RDEV only).
	 */
	int dtu_rctu;
	/**
	 * @rstu_dtu: Duration of ranging slot time unit in device time unit
	 * (ERDEV only).
	 */
	int rstu_dtu;
	/**
	 * @anticip_dtu: Reasonable delay between reading the current timestamp
	 * and doing an operation in device time unit.
	 */
	int anticip_dtu;
	/**
	 * @idle_dtu: Duration long enough to prefer entering the idle mode
	 * rather than trying to find a valid access.
	 */
	int idle_dtu;
	/**
	 * @current_preamble_code: Current value of preamble code index for HRP
	 * UWB. Must be zero for other PHYs.
	 */
	int current_preamble_code;
	/**
	 * @rx_antenna_pairs: Number of antenna pairs for RX.
	 */
	u32 rx_antenna_pairs;
	/**
	 * @tx_antennas: Number of antennas for TX.
	 */
	u32 tx_antennas;
	/**
	 * @flags: Low-level hardware flags, see &enum mcps802154_llhw_flags.
	 */
	u64 flags;
	/**
	 * @hw: Pointer to IEEE802154 hardware exposed by MCPS. The low-level
	 * driver needs to update this and hw->phy according to supported
	 * hardware features and radio parameters. More specifically:
	 *
	 * * &ieee802154_hw.extra_tx_headroom
	 * * in &ieee802154_hw.flags:
	 *
	 *     * IEEE802154_HW_TX_OMIT_CKSUM
	 *     * IEEE802154_HW_RX_OMIT_CKSUM
	 *     * IEEE802154_HW_RX_DROP_BAD_CKSUM
	 *
	 * * &wpan_phy.flags
	 * * &wpan_phy_supported
	 * * &wpan_phy.symbol_duration
	 */
	struct ieee802154_hw *hw;
	/**
	 * @priv: Driver private data.
	 */
	void *priv;
	/**
	 * @rx_ctx_size: size of the context.
	 */
	u32 rx_ctx_size;
};

/**
 * enum mcps802154_tx_frame_config_flags - Flags for transmitting a frame.
 * @MCPS802154_TX_FRAME_CONFIG_TIMESTAMP_DTU:
 *	Start transmission at given timestamp in device time unit.
 * @MCPS802154_TX_FRAME_CONFIG_CCA:
 *	Use CCA before transmission using the programmed mode.
 * @MCPS802154_TX_FRAME_CONFIG_RANGING:
 *	Enable precise timestamping for the transmitted frame and its response
 *	(RDEV only).
 * @MCPS802154_TX_FRAME_CONFIG_KEEP_RANGING_CLOCK:
 *      Request that the ranging clock be kept valid after the transmission of
 *      this frame (RDEV only).
 * @MCPS802154_TX_FRAME_CONFIG_RANGING_PDOA:
 *	Enable phase difference of arrival measurement for the response frame
 *	(RDEV only).
 * @MCPS802154_TX_FRAME_CONFIG_SP1:
 *	Enable STS for the transmitted frame and its response, mode 1 (STS after
 *	SFD and before PHR, ERDEV only).
 * @MCPS802154_TX_FRAME_CONFIG_SP2:
 *	Enable STS for the transmitted frame and its response, mode 2 (STS after
 *	the payload, ERDEV only).
 * @MCPS802154_TX_FRAME_CONFIG_SP3:
 *	Enable STS for the transmitted frame and its response, mode 3 (STS after
 *	SFD, no PHR, no payload, ERDEV only).
 * @MCPS802154_TX_FRAME_CONFIG_STS_MODE_MASK:
 *      Mask covering all the STS mode configuration values.
 * @MCPS802154_TX_FRAME_CONFIG_RANGING_ROUND:
 *	Inform low-level driver the transmitted frame is the start of a ranging
 *	round (RDEV only).
 *
 * If no timestamp flag is given, transmit as soon as possible.
 */
enum mcps802154_tx_frame_config_flags {
	MCPS802154_TX_FRAME_CONFIG_TIMESTAMP_DTU = BIT(0),
	MCPS802154_TX_FRAME_CONFIG_CCA = BIT(1),
	MCPS802154_TX_FRAME_CONFIG_RANGING = BIT(2),
	MCPS802154_TX_FRAME_CONFIG_KEEP_RANGING_CLOCK = BIT(3),
	MCPS802154_TX_FRAME_CONFIG_RANGING_PDOA = BIT(4),
	MCPS802154_TX_FRAME_CONFIG_SP1 = BIT(5),
	MCPS802154_TX_FRAME_CONFIG_SP2 = BIT(6),
	MCPS802154_TX_FRAME_CONFIG_SP3 = BIT(5) | BIT(6),
	MCPS802154_TX_FRAME_CONFIG_STS_MODE_MASK = BIT(5) | BIT(6),
	MCPS802154_TX_FRAME_CONFIG_RANGING_ROUND = BIT(7),
};

/**
 * struct mcps802154_tx_frame_config - Information for transmitting a frame.
 */
struct mcps802154_tx_frame_config {
	/**
	 * @timestamp_dtu: If timestamped, date of transmission start.
	 */
	u32 timestamp_dtu;
	/**
	 * @rx_enable_after_tx_dtu: If positive, enable receiver this number of
	 * device time unit after the end of the transmitted frame.
	 */
	int rx_enable_after_tx_dtu;
	/**
	 * @rx_enable_after_tx_timeout_dtu: When receiver is enabled after the
	 * end of the transmitted frame: if negative, no timeout, if zero, use
	 * a default timeout value, else this is the timeout value in device
	 * time unit.
	 */
	int rx_enable_after_tx_timeout_dtu;
	/**
	 * @flags: See &enum mcps802154_tx_frame_config_flags.
	 */
	u8 flags;
	/**
	 * @ant_set_id : antenna set index to use for transmit.
	 */
	int ant_set_id;
};

/**
 * enum mcps802154_rx_frame_config_flags - Flags for enabling the receiver.
 * @MCPS802154_RX_FRAME_CONFIG_TIMESTAMP_DTU:
 *	Enable receiver at given timestamp in device time unit.
 * @MCPS802154_RX_FRAME_CONFIG_AACK:
 *	Enable automatic acknowledgment.
 * @MCPS802154_RX_FRAME_CONFIG_RANGING:
 *	Enable precise timestamping for the received frame (RDEV only).
 * @MCPS802154_RX_FRAME_CONFIG_KEEP_RANGING_CLOCK:
 *      Request that the ranging clock be kept valid after the reception of the
 *      frame (RDEV only).
 * @MCPS802154_RX_FRAME_CONFIG_RANGING_PDOA:
 *	Enable phase difference of arrival measurement (RDEV only).
 * @MCPS802154_RX_FRAME_CONFIG_SP1:
 *	Enable STS for the received frame, mode 1 (STS after SFD and before PHR,
 *	ERDEV only).
 * @MCPS802154_RX_FRAME_CONFIG_SP2:
 *	Enable STS for the received frame, mode 2 (STS after the payload, ERDEV
 *	only).
 * @MCPS802154_RX_FRAME_CONFIG_SP3:
 *	Enable STS for the received frame, mode 3 (STS after SFD, no PHR, no
 *	payload, ERDEV only).
 * @MCPS802154_RX_FRAME_CONFIG_STS_MODE_MASK:
 *      Mask covering all the STS mode configuration values.
 * @MCPS802154_RX_FRAME_CONFIG_RANGING_ROUND:
 *	Inform low-level driver the expected received frame is the start of a
 *	ranging round (RDEV only).
 *
 * If no timestamp flag is given, enable receiver as soon as possible.
 */
enum mcps802154_rx_frame_config_flags {
	MCPS802154_RX_FRAME_CONFIG_TIMESTAMP_DTU = BIT(0),
	MCPS802154_RX_FRAME_CONFIG_AACK = BIT(1),
	MCPS802154_RX_FRAME_CONFIG_RANGING = BIT(2),
	MCPS802154_RX_FRAME_CONFIG_KEEP_RANGING_CLOCK = BIT(3),
	MCPS802154_RX_FRAME_CONFIG_RANGING_PDOA = BIT(4),
	MCPS802154_RX_FRAME_CONFIG_SP1 = BIT(5),
	MCPS802154_RX_FRAME_CONFIG_SP2 = BIT(6),
	MCPS802154_RX_FRAME_CONFIG_SP3 = BIT(5) | BIT(6),
	MCPS802154_RX_FRAME_CONFIG_STS_MODE_MASK = BIT(5) | BIT(6),
	MCPS802154_RX_FRAME_CONFIG_RANGING_ROUND = BIT(7),
};

/**
 * struct mcps802154_rx_frame_config - Information for enabling the receiver.
 */
struct mcps802154_rx_frame_config {
	/**
	 * @timestamp_dtu: If timestamped, date to enable the receiver.
	 */
	u32 timestamp_dtu;
	/**
	 * @timeout_dtu: If negative, no timeout, if zero, use a default timeout
	 * value, else this is the timeout value in device time unit.
	 */
	int timeout_dtu;
	/**
	 * @frame_timeout_dtu: If no zero, timeout value for the full frame
	 * reception. This allow limiting the length of accepted frame. The
	 * timeout starts after &mcps802154_rx_frame_config.timeout_dtu value.
	 */
	int frame_timeout_dtu;
	/**
	 * @flags: See &enum mcps802154_rx_frame_config_flags.
	 */
	u8 flags;
	/**
	 * @ant_set_id: Antenna set index to use for reception.
	 */
	int ant_set_id;
};

/**
 * enum mcps802154_rx_frame_info_flags - Flags for a received frame.
 * @MCPS802154_RX_FRAME_INFO_TIMESTAMP_DTU:
 *	Set by MCPS to request timestamp in device time unit.
 * @MCPS802154_RX_FRAME_INFO_TIMESTAMP_RCTU:
 *	Set by MCPS to request RMARKER timestamp in ranging counter time unit
 *	(RDEV only).
 * @MCPS802154_RX_FRAME_INFO_LQI:
 *	Set by MCPS to request link quality indicator (LQI).
 * @MCPS802154_RX_FRAME_INFO_RSSI:
 *	Set by MCPS to request RSSI.
 * @MCPS802154_RX_FRAME_INFO_RANGING_FOM:
 *	Set by MCPS to request ranging figure of merit (FoM, RDEV only).
 * @MCPS802154_RX_FRAME_INFO_RANGING_OFFSET:
 *	Set by MCPS to request clock characterization data (RDEV only).
 * @MCPS802154_RX_FRAME_INFO_RANGING_PDOA:
 *	Set by MCPS to request phase difference of arrival (RDEV only).
 * @MCPS802154_RX_FRAME_INFO_RANGING_PDOA_FOM:
 *	Set by MCPS to request phase difference of arrival figure of merit (FoM,
 *	RDEV only).
 * @MCPS802154_RX_FRAME_INFO_RANGING_STS_TIMESTAMP_RCTU:
 *	Set by MCPS to request SRMARKERx timestamps for each STS segments in
 *	ranging counter time unit (ERDEV only).
 * @MCPS802154_RX_FRAME_INFO_RANGING_STS_FOM:
 *	Set by MCPS to request STS segments figure of merit measuring the
 *	correlation strength between the received STS segment and the expected
 *	one (FoM, ERDEV only).
 * @MCPS802154_RX_FRAME_INFO_AACK:
 *	Set by low-level driver if an automatic acknowledgment was sent or is
 *	being sent.
 *
 * The low-level driver must clear the corresponding flag if the information is
 * not available.
 */
enum mcps802154_rx_frame_info_flags {
	MCPS802154_RX_FRAME_INFO_TIMESTAMP_DTU = BIT(0),
	MCPS802154_RX_FRAME_INFO_TIMESTAMP_RCTU = BIT(1),
	MCPS802154_RX_FRAME_INFO_LQI = BIT(2),
	MCPS802154_RX_FRAME_INFO_RSSI = BIT(3),
	MCPS802154_RX_FRAME_INFO_RANGING_FOM = BIT(4),
	MCPS802154_RX_FRAME_INFO_RANGING_OFFSET = BIT(5),
	MCPS802154_RX_FRAME_INFO_RANGING_PDOA = BIT(6),
	MCPS802154_RX_FRAME_INFO_RANGING_PDOA_FOM = BIT(7),
	MCPS802154_RX_FRAME_INFO_RANGING_STS_TIMESTAMP_RCTU = BIT(8),
	MCPS802154_RX_FRAME_INFO_RANGING_STS_FOM = BIT(9),
	MCPS802154_RX_FRAME_INFO_AACK = BIT(10),
};

/**
 * struct mcps802154_rx_frame_info - Information on a received frame.
 */
struct mcps802154_rx_frame_info {
	/**
	 * @timestamp_dtu: Timestamp of start of frame in device time unit.
	 */
	u32 timestamp_dtu;
	/**
	 * @timestamp_rctu: Timestamp of RMARKER in ranging count time unit
	 * (RDEV only).
	 */
	u64 timestamp_rctu;
	/**
	 * @frame_duration_dtu: Duration of the whole frame in device time unit
	 * or 0 if unknown.
	 */
	int frame_duration_dtu;
	/**
	 * @rssi: Received signal strength indication (RSSI),
	 * absolute value in Q1 fixed point format.
	 */
	int rssi;
	/**
	 * @ranging_tracking_interval_rctu: Interval on which tracking offset
	 * was measured (RDEV only).
	 */
	int ranging_tracking_interval_rctu;
	/**
	 * @ranging_offset_rctu: Difference between the transmitter and the
	 * receiver clock measure over the tracking interval, if positive, the
	 * transmitter operates at a higher frequency (RDEV only).
	 */
	int ranging_offset_rctu;
	/**
	 * @ranging_sts_timestamp_diffs_rctu: For each SRMARKERx, difference
	 * between the measured timestamp and the expected timestamp relative to
	 * RMARKER in ranging count time unit (ERDEV only). When STS mode is
	 * 1 or 3, SRMARKER0 is the same as RMARKER and difference is always 0.
	 */
	s16 ranging_sts_timestamp_diffs_rctu[MCPS802154_STS_N_SEGS_MAX + 1];
	/**
	 * @lqi: Link quality indicator (LQI).
	 */
	u8 lqi;
	/**
	 * @ranging_fom: Ranging figure of merit (FoM, RDEV only). Should be
	 * formatted according to 802.15.4.
	 */
	u8 ranging_fom;
	/**
	 * @ranging_pdoa_fom: Phase difference of arrival figure of merit (FoM,
	 * RDEV only). Range is 0 to 255, with 0 being an invalid measure and
	 * 255 being a 100% confidence.
	 */
	u8 ranging_pdoa_fom;
	/**
	 * @ranging_sts_fom: Table of figures of merit measuring the correlation
	 * strength between the received STS segment and the expected one (FoM,
	 * ERDEV only). Range is 0 to 255, with 0 being an invalid measure and
	 * 255 being a 100% confidence.
	 */
	u8 ranging_sts_fom[MCPS802154_STS_N_SEGS_MAX];
	/**
	 * @flags: See &enum mcps802154_rx_frame_info_flags.
	 */
	u16 flags;
};

/**
 * enum mcps802154_rx_measurement_info_flags - Flags for measurements on a received
 * frame.
 * @MCPS802154_RX_MEASUREMENTS_TIMESTAMP:
 *	Set by MCPS to request time of arrival measurement and associated figure
 *	of merit (RDEV only).
 * @MCPS802154_RX_MEASUREMENTS_CLOCK_OFFSET:
 *	Set by MCPS to request clock characterization data (RDEV only).
 * @MCPS802154_RX_MEASUREMENTS_STS_SEGS_TIMESTAMPS:
 *	Set by MCPS to request time of arrival measurement on STS segments and
 *	associated figure of merit (ERDEV only).
 * @MCPS802154_RX_MEASUREMENTS_RSSIS:
 *	Set by MCPS to request RSSI values.
 * @MCPS802154_RX_MEASUREMENTS_AOAS:
 *	Set by MCPS to request angle of arrival measurements, time difference of
 *	arrival, phase difference of arrival and associated figure of merit
 *	(RDEV only).
 * @MCPS802154_RX_MEASUREMENTS_CIRS:
 *	Set by MCPS to request CIR samples (RDEV only).
 * @MCPS802154_RX_MEASUREMENTS_VENDOR0:
 *	Set by MCPS to request first set of vendor specific measurements.
 * @MCPS802154_RX_MEASUREMENTS_VENDOR1:
 *	Set by MCPS to request second set of vendor specific measurements.
 * @MCPS802154_RX_MEASUREMENTS_VENDOR2:
 *	Set by MCPS to request third set of vendor specific measurements.
 * @MCPS802154_RX_MEASUREMENTS_VENDOR3:
 *	Set by MCPS to request fourth set of vendor specific measurements.
 *
 * The low-level driver must clear the corresponding flag if the information is
 * not available.
 */
enum mcps802154_rx_measurement_info_flags {
	MCPS802154_RX_MEASUREMENTS_TIMESTAMP = BIT(0),
	MCPS802154_RX_MEASUREMENTS_CLOCK_OFFSET = BIT(1),
	MCPS802154_RX_MEASUREMENTS_STS_SEGS_TIMESTAMPS = BIT(2),
	MCPS802154_RX_MEASUREMENTS_RSSIS = BIT(3),
	MCPS802154_RX_MEASUREMENTS_AOAS = BIT(4),
	MCPS802154_RX_MEASUREMENTS_CIRS = BIT(5),
	MCPS802154_RX_MEASUREMENTS_VENDOR0 = BIT(12),
	MCPS802154_RX_MEASUREMENTS_VENDOR1 = BIT(13),
	MCPS802154_RX_MEASUREMENTS_VENDOR2 = BIT(14),
	MCPS802154_RX_MEASUREMENTS_VENDOR3 = BIT(15),
};

/**
 * enum mcps802154_aoa_type - Type of Angle of Arrival measurement (RDEV only).
 *
 * @MCPS802154_AOA_TYPE_X_AXIS:
 *	AoA measured on X axis.
 * @MCPS802154_AOA_TYPE_Y_AXIS:
 *	AoA measured on Y axis.
 * @MCPS802154_AOA_TYPE_Z_AXIS:
 *	AoA measured on Z axis.
 *
 * The axes are defined on a right hand orthonormal base.
 */
enum __attribute__((__packed__)) mcps802154_aoa_type {
	MCPS802154_AOA_TYPE_X_AXIS = 0,
	MCPS802154_AOA_TYPE_Y_AXIS = 1,
	MCPS802154_AOA_TYPE_Z_AXIS = 2,
};

/**
 * struct mcps802154_rx_aoa_measurements - Angle of arrival measurements on a
 * received frame (RDEV only).
 */
struct mcps802154_rx_aoa_measurements {
	/**
	 * @tdoa_rctu: Time difference of arrival, in ranging count time unit.
	 */
	s16 tdoa_rctu;
	/**
	 * @pdoa_rad_q11: Phase difference of arrival, unit is radian multiplied
	 * by 2048.
	 */
	s16 pdoa_rad_q11;
	/**
	 * @aoa_rad_q11: Angle of arrival, unit is radian multiplied by 2048.
	 */
	s16 aoa_rad_q11;
	/**
	 * @fom: Measurements figure of merit (FoM). Range is 0 to 255, with 0
	 * being an invalid measure and 255 being a 100% confidence.
	 */
	u8 fom;
	/**
	 * @type: Measurement type.
	 */
	enum mcps802154_aoa_type type;
};

/**
 * struct mcps802154_rx_cir_sample_window - Window of CIR samples.
 */
struct mcps802154_rx_cir_sample_window {
	/**
	 * @n_samples: The number of samples contained in the window.
	 */
	u16 n_samples;
	/**
	 * @sizeof_sample: The size of a single sample.
	 */
	u16 sizeof_sample;
	/**
	 * @samples: CIR samples values.
	 *
	 * Each sample is composed of the real and imaginary part which are
	 * signed numbers. Each sample is encoded using the platform endianness
	 * with @mcps802154_rx_cir_sample_window.sizeof_sample bytes, first half
	 * is the real part, second half is the imaginary part.
	 *
	 * Must be kept valid until next received frame
	 */
	void *samples;
};

/**
 * struct mcps802154_rx_cir - CIR measurements.
 */
struct mcps802154_rx_cir {
	/**
	 * @fp_index: The absolute index of the sample considered as first path.
	 */
	u16 fp_index;
	/**
	 * @fp_snr: The SNR of the sample considered as first path.
	 */
	s16 fp_snr;
	/**
	 * @fp_ns_q6: (Q10.6) Time in nanosecond of the first path index
	 */
	u16 fp_ns_q6;
	/**
	 * @pp_index: The absolute index of the sample considered as peak path.
	 */
	u16 pp_index;
	/**
	 * @pp_snr: The SNR of the sample considered as peak path.
	 */
	s16 pp_snr;
	/**
	 * @pp_ns_q6: (Q10.6) Time in nanosecond of the peak path index
	 */
	u16 pp_ns_q6;
	/**
	 * @fp_sample_offset: The offset of the first path in the sample window.
	 */
	u16 fp_sample_offset;
	/**
	 * @sample_window: CIR samples.
	 */
	struct mcps802154_rx_cir_sample_window sample_window;
};

/**
 * struct mcps802154_rx_measurement_info - Measurements on a received frame.
 */
struct mcps802154_rx_measurement_info {
	/**
	 * @n_rssis: The number of RSSI computed for this frame. Depends on the
	 * antenna set used to receive.
	 *
	 * Set by low-level driver.
	 */
	int n_rssis;
	/**
	 * @rssis_q1: Received signal strength indication (RSSI), array of
	 * absolute values in Q7.1 fixed point format, unit is dBm.
	 */
	u8 rssis_q1[MCPS802154_RSSIS_N_MAX];
	/**
	 * @n_aoas: Number of angle of arrival measurements.
	 *
	 * Set by low-level driver.
	 */
	int n_aoas;
	/**
	 * @aoas: Angle of arrival measurements, ordered by increasing
	 * measurement type.
	 */
	struct mcps802154_rx_aoa_measurements
		aoas[MCPS802154_RX_AOA_MEASUREMENTS_MAX];
	/**
	 * @n_cirs: Number of parts of CIR measurements.
	 *
	 * Set by low-level driver.
	 */
	int n_cirs;
	/**
	 * @cirs: CIR measurements for different parts of the frame.
	 *
	 * Set by low-level driver, must be kept valid until next received
	 * frame.
	 */
	struct mcps802154_rx_cir *cirs;
	/**
	 * @flags: See &enum mcps802154_rx_measurement_info_flags.
	 */
	int flags;
};

/**
 * struct mcps802154_sts_params - STS parameters for HRP UWB.
 */
struct mcps802154_sts_params {
	/**
	 * @v: Value V used in DRBG for generating the STS. The 32 LSB are the
	 * VCounter which is incremented every 128 generated pulse.
	 */
	u8 v[AES_BLOCK_SIZE];
	/**
	 * @key: STS AES key used in DRBG for generating the STS.
	 */
	u8 key[AES_KEYSIZE_128];
	/**
	 * @n_segs: Number of STS segments.
	 */
	int n_segs;
	/**
	 * @seg_len: Length of STS segments.
	 */
	int seg_len;
	/**
	 * @sp2_tx_gap_4chips: For SP2 frame format, additional gap in unit of
	 * 4 chips between the end of the payload and the start of the STS, used
	 * for TX.
	 */
	int sp2_tx_gap_4chips;
	/**
	 * @sp2_rx_gap_4chips: For SP2 frame format, additional gap in unit of
	 * 4 chips between the end of the payload and the start of the STS, used
	 * for RX. A0 and A1 bits in PHR are used to index the array.
	 */
	int sp2_rx_gap_4chips[MCPS802154_STS_N_SEGS_MAX];
};

/**
 * enum mcps802154_prf - Pulse repetition frequency.
 * @MCPS802154_PRF_16:
 *	16 MHz, only used in 4a.
 * @MCPS802154_PRF_64:
 *	64 MHz, used for 4a and 4z BPRF.
 * @MCPS802154_PRF_125:
 *	125 MHz, used for 4z HPRF.
 * @MCPS802154_PRF_250:
 *	250 MHz, used for 4z HPRF.
 */
enum mcps802154_prf {
	MCPS802154_PRF_16 = 16,
	MCPS802154_PRF_64 = 64,
	MCPS802154_PRF_125 = 125,
	MCPS802154_PRF_250 = 250,
};

/**
 * enum mcps802154_psr - Number of preamble symbol repetitions in the SYNC
 * sequence.
 * @MCPS802154_PSR_16:
 *	16 symbols, used in 4a and 4z HPRF.
 * @MCPS802154_PSR_24:
 *	24 symbols, used only in 4z HPRF.
 * @MCPS802154_PSR_32:
 *	32 symbols, used only in 4z HPRF.
 * @MCPS802154_PSR_48:
 *	48 symbols, used only in 4z HPRF.
 * @MCPS802154_PSR_64:
 *	64 symbols, used 4a and 4z BPRF and HPRF.
 * @MCPS802154_PSR_96:
 *	96 symbols, used only in 4z HPRF.
 * @MCPS802154_PSR_128:
 *	128 symbols, used only in 4z HPRF.
 * @MCPS802154_PSR_256:
 *	256 symbols, used only in 4z HPRF.
 * @MCPS802154_PSR_1024:
 *	1024 symbols, used only in 4a.
 * @MCPS802154_PSR_2048:
 *	2048 symbols, used only in 4a.
 * @MCPS802154_PSR_4096:
 *	4096 symbols, used only in 4a.
 */
enum mcps802154_psr {
	MCPS802154_PSR_16 = 16,
	MCPS802154_PSR_24 = 24,
	MCPS802154_PSR_32 = 32,
	MCPS802154_PSR_48 = 48,
	MCPS802154_PSR_64 = 64,
	MCPS802154_PSR_96 = 96,
	MCPS802154_PSR_128 = 128,
	MCPS802154_PSR_256 = 256,
	MCPS802154_PSR_1024 = 1024,
	MCPS802154_PSR_2048 = 2048,
	MCPS802154_PSR_4096 = 4096,
};

/**
 * enum mcps802154_sfd - sfd type selector.
 * @MCPS802154_SFD_4A:
 *	SFD defined in 4a, length of 8 symbols.
 * @MCPS802154_SFD_4Z_4:
 *	SFD defined in 4z, length of 4 symbols.
 * @MCPS802154_SFD_4Z_8:
 *	SFD defined in 4z, length of 8 symbols.
 * @MCPS802154_SFD_4Z_16:
 *	SFD defined in 4z, length of 16 symbols.
 * @MCPS802154_SFD_4Z_32:
 *	SFD defined in 4z, length of 32 symbols.
 */
enum mcps802154_sfd {
	MCPS802154_SFD_4A,
	MCPS802154_SFD_4Z_4,
	MCPS802154_SFD_4Z_8,
	MCPS802154_SFD_4Z_16,
	MCPS802154_SFD_4Z_32,
};

/**
 * enum mcps802154_data_rate - Data rate.
 * @MCPS802154_DATA_RATE_850K:
 *	850 kbps, used only for 4a.
 * @MCPS802154_DATA_RATE_6M81:
 *	6.81 Mbps, used for 4a and 4z (PRF must be 125MHz).
 * @MCPS802154_DATA_RATE_7M80:
 *	7.80 Mbps, only used for 4z (PRF must be 125MHz).
 * @MCPS802154_DATA_RATE_27M2:
 *	27.2 Mbps, used for 4a and 4z (PRF must be 250MHz).
 * @MCPS802154_DATA_RATE_31M2:
 *	31.2 Mbps, used for 4z (PRF must be 250MHz).
 * NOTE: device specific values can be set to use a custom data rate.
 */
enum mcps802154_data_rate {
	MCPS802154_DATA_RATE_850K = 0,
	MCPS802154_DATA_RATE_6M81 = 6,
	MCPS802154_DATA_RATE_7M80 = 7,
	MCPS802154_DATA_RATE_27M2 = 27,
	MCPS802154_DATA_RATE_31M2 = 31,
};

/**
 * enum mcps802154_hrp_uwb_psdu_size - PSDU size in HPRF.
 * @MCPS802154_HRP_UWB_PSDU_SIZE_1023:
 *	1023-bytes PSDU.
 * @MCPS802154_HRP_UWB_PSDU_SIZE_2047:
 *	2047-bytes PSDU.
 * @MCPS802154_HRP_UWB_PSDU_SIZE_4095:
 *	4095-bytes PSDU.
 */
enum mcps802154_hrp_uwb_psdu_size {
	MCPS802154_HRP_UWB_PSDU_SIZE_1023 = 0,
	MCPS802154_HRP_UWB_PSDU_SIZE_2047 = 1,
	MCPS802154_HRP_UWB_PSDU_SIZE_4095 = 2,
};

/**
 * struct mcps802154_hrp_uwb_params - Parameters for HRP UWB.
 *
 * Parameters are given directly to driver without checking. The driver needs to
 * check the parameters for supported values, but it can accept non-standard
 * values.
 */
struct mcps802154_hrp_uwb_params {
	/**
	 * @prf: Nominal mean Pulse Repetition Frequency.
	 *
	 * For 4a, one of MCPS802154_PRF_16 or MCPS802154_PRF_64.
	 *
	 * For 4z BPRF, must be MCPS802154_PRF_64.
	 *
	 * For 4z HPRF, one of MCPS802154_PRF_125 or MCPS802154_PRF_250.
	 */
	enum mcps802154_prf prf;
	/**
	 * @psr: Number of preamble symbol repetitions in the SYNC sequence, or
	 * preamble length.
	 *
	 * For 4a, one of 16, 64, 1024 or 4096.
	 *
	 * For 4z BPRF, must be 64.
	 *
	 * For 4z HPRF, one of 16, 24, 32, 48, 64, 96, 128 or 256.
	 */
	enum mcps802154_psr psr;
	/**
	 * @sfd_selector: SFD type selector.
	 *
	 * When MCPS802154_SFD_4A, use short SFD defined in 802.15.4a.
	 *
	 * When MCPS802154_SFD_4Z_*, use SFD defined in 802.15.4z, with length
	 * 4, 8, 16 or 32.
	 *
	 * For 4a, must be MCPS802154_SFD_4A.
	 *
	 * For 4z BPRF, one of MCPS802154_SFD_4A or MCPS802154_SFD_4Z_8.
	 *
	 * For 4z HPRF, one of MCPS802154_SFD_4Z_{4,8,16,32}.
	 */
	enum mcps802154_sfd sfd_selector;
	/**
	 * @data_rate: Data rate.
	 *
	 * For 4a, one of 850 kbps, 6.81 Mbps or 27.2 Mbps.
	 *
	 * For 4z BPRF, must be 6.81 Mbps.
	 *
	 * For 4z HPRF at 125 MHz, use 6.81 Mbps or 7.8 Mbps.
	 *
	 * For 4z HPRF at 250 MHz, use 27.2 Mbps or 31.2 Mbps.
	 */
	int data_rate;
	/**
	 * @phr_hi_rate: Use high PHR data rate, for 4z BPRF only.
	 *
	 * For 4a and 4z HPRF, this parameter is ignored.
	 *
	 * For 4z BPRF, when enabled use 6.81 Mbps, otherwise use 850 kbps.
	 */
	bool phr_hi_rate;
	/**
	 * @psdu_size: PSDU size in HPRF.
	 */
	enum mcps802154_hrp_uwb_psdu_size psdu_size;
};

/**
 * enum mcps802154_antenna_caps - Antenna set capabilities
 * @MCPS802154_AOA_X_AXIS:
 *   Antenna can report azimuth
 * @MCPS802154_AOA_Y_AXIS:
 *   Antenna can report elevation
 */
enum mcps802154_antenna_caps {
	MCPS802154_AOA_X_AXIS = BIT(0),
	MCPS802154_AOA_Y_AXIS = BIT(1),
};

/**
 * enum mcps802154_power_state - Power states
 * @MCPS802154_PWR_STATE_OFF:
 *	Power off state.
 * @MCPS802154_PWR_STATE_SLEEP:
 * 	Deep sleep state.
 * @MCPS802154_PWR_STATE_IDLE:
 * 	Idle state, ready to transmit or receive.
 * @MCPS802154_PWR_STATE_RX:
 * 	Receive state.
 * @MCPS802154_PWR_STATE_TX:
 * 	Transmit state.
 * @MCPS802154_PWR_STATE_MAX:
 * 	Total power states count.
 */
enum mcps802154_power_state {
	MCPS802154_PWR_STATE_OFF,
	MCPS802154_PWR_STATE_SLEEP,
	MCPS802154_PWR_STATE_IDLE,
	MCPS802154_PWR_STATE_RX,
	MCPS802154_PWR_STATE_TX,
	MCPS802154_PWR_STATE_MAX
};

/**
 * struct mcps802154_power_state_stats - Statistics for a power state.
 * @dur: Duration in this power state in ns.
 * @count: Count of transitions in this power state.
 */
struct mcps802154_power_state_stats {
	u64 dur;
	u64 count;
};

/**
 * struct mcps802154_power_stats - Global power statistics.
 * @power_state_stats: Array of power statistics for each power state.
 * @interrupts: Hardware interrupts count on the device.
 */
struct mcps802154_power_stats {
	struct mcps802154_power_state_stats
		power_state_stats[MCPS802154_PWR_STATE_MAX];
	u64 interrupts;
};

/**
 * struct mcps802154_ops - Callback from MCPS to the driver.
 */
struct mcps802154_ops {
	/**
	 * @start: Initialize device. Reception should not be activated.
	 *
	 * Return: 0 or error.
	 */
	int (*start)(struct mcps802154_llhw *llhw);
	/**
	 * @stop: Stop device. Should stop any transmission or reception and put
	 * the device in a low power mode.
	 */
	void (*stop)(struct mcps802154_llhw *llhw);
	/**
	 * @tx_frame: Transmit a frame. skb contains the buffer starting from
	 * the IEEE 802.15.4 header. The low-level driver should send the frame
	 * as specified in config. Receiver should be disabled automatically
	 * unless a frame is being received.
	 *
	 * The &frame_idx parameter gives the index of the frame in a "block".
	 * Frames from the same block (aka frame_idx > 0) should maintain the
	 * same synchronization.
	 *
	 * The &next_delay_dtu parameter gives the expected delay between the
	 * start of the transmitted frame and the next action.
	 *
	 * Return: 0, -ETIME if frame can not be sent at specified timestamp,
	 * -EBUSY if a reception is happening right now, or any other error.
	 */
	int (*tx_frame)(struct mcps802154_llhw *llhw, struct sk_buff *skb,
			const struct mcps802154_tx_frame_config *config,
			int frame_idx, int next_delay_dtu);
	/**
	 * @rx_enable: Enable receiver.
	 *
	 * The &frame_idx parameter gives the index of the frame in a "block".
	 * Frames from the same block (aka frame_idx > 0) should maintain the
	 * same synchronization.
	 *
	 * The &next_delay_dtu parameter gives the expected delay between the
	 * start of the received frame or timeout event and the next action.
	 *
	 * Return: 0, -ETIME if receiver can not be enabled at specified
	 * timestamp, or any other error.
	 */
	int (*rx_enable)(struct mcps802154_llhw *llhw,
			 const struct mcps802154_rx_frame_config *config,
			 int frame_idx, int next_delay_dtu);
	/**
	 * @rx_disable: Disable receiver, or a programmed receiver enabling,
	 * unless a frame reception is happening right now.
	 *
	 * Return: 0, -EBUSY if a reception is happening right now, or any other
	 * error.
	 */
	int (*rx_disable)(struct mcps802154_llhw *llhw);
	/**
	 * @rx_get_frame: Get previously received frame. MCPS calls this handler
	 * after a frame reception has been signaled by the low-level driver.
	 *
	 * The received buffer is owned by MCPS after this call. Only the
	 * requested information need to be filled in the information structure.
	 *
	 * Return: 0, -EBUSY if no longer available, or any other error.
	 */
	int (*rx_get_frame)(struct mcps802154_llhw *llhw, struct sk_buff **skb,
			    struct mcps802154_rx_frame_info *info);
	/**
	 * @rx_get_error_frame: Get information on rejected frame. MCPS can call
	 * this handler after a frame rejection has been signaled by the
	 * low-level driver.
	 *
	 * In case of error, info flags must be cleared by this callback.
	 *
	 * Return: 0, -EBUSY if no longer available, or any other error.
	 */
	int (*rx_get_error_frame)(struct mcps802154_llhw *llhw,
				  struct mcps802154_rx_frame_info *info);
	/**
	 * @rx_get_measurement: Get measurement associated with a received
	 * frame.
	 *
	 * Return: 0, -EBUSY if no longer available, or any other error.
	 */
	int (*rx_get_measurement)(struct mcps802154_llhw *llhw, void *rx_ctx,
				  struct mcps802154_rx_measurement_info *info);
	/**
	 * @idle: Put the device into idle mode without time limit or until the
	 * given timestamp.  The driver should call &mcps802154_timer_expired()
	 * before the given timestamp so that an action can be programmed at the
	 * given timestamp.
	 *
	 * The &mcps802154_timer_expired() function must not be called
	 * immediately from this callback, but should be scheduled to be called
	 * later.
	 *
	 * If the driver is late, the regular handling of late actions will take
	 * care of the situation.
	 *
	 * Return: 0 or error.
	 */
	int (*idle)(struct mcps802154_llhw *llhw, bool timestamp,
		    u32 timestamp_dtu);
	/**
	 * @reset: Reset device after an unrecoverable error.
	 *
	 * Return: 0 or error.
	 */
	int (*reset)(struct mcps802154_llhw *llhw);
	/**
	 * @get_current_timestamp_dtu: Get current timestamp in device time
	 * unit.
	 *
	 * If the device is currently in a low power state, the eventual wake up
	 * delay should be added to the returned timestamp.
	 *
	 * If the current timestamp can not be determined precisely, it should
	 * return a pessimistic value, i.e. rounded up.
	 *
	 * Return: 0 or error.
	 */
	int (*get_current_timestamp_dtu)(struct mcps802154_llhw *llhw,
					 u32 *timestamp_dtu);
	/**
	 * @tx_timestamp_dtu_to_rmarker_rctu: Compute the RMARKER timestamp in
	 * ranging counter time unit for a frame transmitted at given timestamp
	 * in device time unit (RDEV only).
	 *
	 * Return: The RMARKER timestamp.
	 */
	u64 (*tx_timestamp_dtu_to_rmarker_rctu)(
		struct mcps802154_llhw *llhw, u32 tx_timestamp_dtu,
		const struct mcps802154_hrp_uwb_params *hrp_uwb_params,
		const struct mcps802154_channel *channel_params,
		int ant_set_id);
	/**
	 * @difference_timestamp_rctu: Compute the difference between two
	 * timestamp values.
	 *
	 * Return: The difference between A and B.
	 */
	s64 (*difference_timestamp_rctu)(struct mcps802154_llhw *llhw,
					 u64 timestamp_a_rctu,
					 u64 timestamp_b_rctu);
	/**
	 * @compute_frame_duration_dtu: Compute the duration of a frame with
	 * given payload length (header and checksum included) using the current
	 * radio parameters.
	 *
	 * Return: The duration in device time unit.
	 */
	int (*compute_frame_duration_dtu)(struct mcps802154_llhw *llhw,
					  int payload_bytes);
	/**
	 * @set_channel: Set channel parameters.
	 *
	 * Return: 0 or error.
	 */
	int (*set_channel)(struct mcps802154_llhw *llhw, u8 page, u8 channel,
			   u8 preamble_code);
	/**
	 * @set_hrp_uwb_params: Set radio parameters for HRP UWB.
	 *
	 * The parameters in &mcps802154_llhw can change according to radio
	 * parameters.
	 *
	 * Return: 0 or error.
	 */
	int (*set_hrp_uwb_params)(
		struct mcps802154_llhw *llhw,
		const struct mcps802154_hrp_uwb_params *params);
	/**
	 * @check_hrp_uwb_params: Check that the HRP parameters are compatible
	 * with the hardware capabilities.
	 *
	 * Return: 0 or error.
	 */
	int (*check_hrp_uwb_params)(
		struct mcps802154_llhw *llhw,
		const struct mcps802154_hrp_uwb_params *params);
	/**
	 * @set_sts_params: Set STS parameters (ERDEV only).
	 *
	 * Return: 0 or error.
	 */
	int (*set_sts_params)(struct mcps802154_llhw *llhw,
			      const struct mcps802154_sts_params *params);
	/**
	 * @set_hw_addr_filt: Set hardware filter parameters.
	 *
	 * Return: 0 or error.
	 */
	int (*set_hw_addr_filt)(struct mcps802154_llhw *llhw,
				struct ieee802154_hw_addr_filt *filt,
				unsigned long changed);
	/**
	 * @set_txpower: Set transmission power.
	 *
	 * Return: 0 or error.
	 */
	int (*set_txpower)(struct mcps802154_llhw *llhw, s32 mbm);
	/**
	 * @set_cca_mode: Set CCA mode.
	 *
	 * The CCA duration in &mcps802154_llhw can change according to CCA
	 * mode.
	 *
	 * Return: 0 or error.
	 */
	int (*set_cca_mode)(struct mcps802154_llhw *llhw,
			    const struct wpan_phy_cca *cca);
	/**
	 * @set_cca_ed_level: Set CCA energy detection threshold.
	 *
	 * Return: 0 or error.
	 */
	int (*set_cca_ed_level)(struct mcps802154_llhw *llhw, s32 mbm);
	/**
	 * @set_promiscuous_mode: Set promiscuous mode.
	 *
	 * Return: 0 or error.
	 */
	int (*set_promiscuous_mode)(struct mcps802154_llhw *llhw, bool on);
	/**
	 * @set_scanning_mode: Set SW scanning mode.
	 *
	 * Return: 0 or error.
	 */
	int (*set_scanning_mode)(struct mcps802154_llhw *llhw, bool on);
	/**
	 * @set_calibration: Set calibration value.
	 *
	 * Set the calibration parameter specified by the key string with the
	 * value specified in the provided buffer. The provided length must
	 * match the length returned by the @get_calibration() callback.
	 *
	 * Return: 0 or error.
	 */
	int (*set_calibration)(struct mcps802154_llhw *llhw, const char *key,
			       void *value, size_t length);
	/**
	 * @get_calibration: Get calibration value.
	 *
	 * Get the calibration parameter specified by the key string into the
	 * provided buffer.
	 *
	 * Return: size of parameter written in buffer or error.
	 */
	int (*get_calibration)(struct mcps802154_llhw *llhw, const char *key,
			       void *value, size_t length);
	/**
	 * @list_calibration: Returns list of accepted calibration key strings
	 *
	 * Return: NULL terminated strings pointer array.
	 */
	const char *const *(*list_calibration)(struct mcps802154_llhw *llhw);
	/**
	 * @vendor_cmd: Run a vendor specific command.
	 *
	 * Do not (ab)use this feature to implement features that could be
	 * openly shared across drivers.
	 *
	 * Return: 0 or error.
	 */
	int (*vendor_cmd)(struct mcps802154_llhw *llhw, u32 vendor_id,
			  u32 subcmd, void *data, size_t data_len);
	/**
	 * @get_antenna_caps: Return antenna set capabilites.
	 *
	 * Return: 0 or error.
	 */
	int (*get_antenna_caps)(struct mcps802154_llhw *llhw, int ant_idx,
				u32 *caps);
	/**
	 * @get_power_stats: Get the power statistics.
	 *
	 * Return: 0 or error.
	 */
	int (*get_power_stats)(struct mcps802154_llhw *llhw,
			       struct mcps802154_power_stats *pwr_stats);
#ifdef CONFIG_MCPS802154_TESTMODE
	/**
	 * @testmode_cmd: Run a testmode command.
	 *
	 * Return: 0 or error.
	 */
	int (*testmode_cmd)(struct mcps802154_llhw *llhw, void *data, int len);
#endif
};

#ifdef CONFIG_MCPS802154_TESTMODE
#define MCPS802154_TESTMODE_CMD(cmd) .testmode_cmd = (cmd),
#else
#define MCPS802154_TESTMODE_CMD(cmd)
#endif

/**
 * enum mcps802154_rx_error_type - Type of reception errors.
 * @MCPS802154_RX_ERROR_NONE:
 *      RX successful.
 * @MCPS802154_RX_ERROR_TIMEOUT:
 *      RX timeout.
 * @MCPS802154_RX_ERROR_BAD_CKSUM:
 *	Checksum is not correct.
 * @MCPS802154_RX_ERROR_UNCORRECTABLE:
 *	During reception, the error correction code detected an uncorrectable
 *	error.
 * @MCPS802154_RX_ERROR_FILTERED:
 *	A received frame was rejected due to frame filter.
 * @MCPS802154_RX_ERROR_SFD_TIMEOUT:
 *	A preamble has been detected but without SFD.
 * @MCPS802154_RX_ERROR_OTHER:
 *	Other error, frame reception is aborted.
 * @MCPS802154_RX_ERROR_PHR_DECODE:
 *	the preamble and SFD have been detected but without PHR.
 */
enum mcps802154_rx_error_type {
	MCPS802154_RX_ERROR_NONE = 0,
	MCPS802154_RX_ERROR_TIMEOUT = 1,
	MCPS802154_RX_ERROR_BAD_CKSUM = 2,
	MCPS802154_RX_ERROR_UNCORRECTABLE = 3,
	MCPS802154_RX_ERROR_FILTERED = 4,
	MCPS802154_RX_ERROR_SFD_TIMEOUT = 5,
	MCPS802154_RX_ERROR_OTHER = 6,
	MCPS802154_RX_ERROR_PHR_DECODE = 7,
};

/**
 * mcps802154_alloc_llhw() - Allocate a new low-level hardware device.
 * @priv_data_len: Length of private data.
 * @ops: Callbacks for this device.
 *
 * Return: A pointer to the new low-level hardware device, or %NULL on error.
 */
struct mcps802154_llhw *mcps802154_alloc_llhw(size_t priv_data_len,
					      const struct mcps802154_ops *ops);

/**
 * mcps802154_free_llhw() - Free low-level hardware descriptor.
 * @llhw: Low-level device pointer.
 *
 * You must call mcps802154_unregister_hw() before calling this function.
 */
void mcps802154_free_llhw(struct mcps802154_llhw *llhw);

/**
 * mcps802154_register_llhw() - Register low-level hardware device.
 * @llhw: Low-level device pointer.
 *
 * Return: 0 or error.
 */
int mcps802154_register_llhw(struct mcps802154_llhw *llhw);

/**
 * mcps802154_unregister_llhw() - Unregister low-level hardware device.
 * @llhw: Low-level device pointer.
 */
void mcps802154_unregister_llhw(struct mcps802154_llhw *llhw);

/**
 * mcps802154_rx_frame() - Signal a frame reception.
 * @llhw: Low-level device this frame came in on.
 *
 * The MCPS will call the &mcps802154_ops.rx_get_frame() handler to retrieve
 * frame.
 */
void mcps802154_rx_frame(struct mcps802154_llhw *llhw);

/**
 * mcps802154_rx_timeout() - Signal a reception timeout.
 * @llhw: Low-level device pointer.
 */
void mcps802154_rx_timeout(struct mcps802154_llhw *llhw);

/**
 * mcps802154_rx_error() - Signal a reception error.
 * @llhw: Low-level device pointer.
 * @error: Type of detected error.
 *
 * In case of filtered frame, the MCPS can call the
 * &mcps802154_ops.rx_get_error_frame() handler to retrieve frame information.
 */
void mcps802154_rx_error(struct mcps802154_llhw *llhw,
			 enum mcps802154_rx_error_type error);

/**
 * mcps802154_tx_done() - Signal the end of an MCPS transmission.
 * @llhw: Low-level device pointer.
 */
void mcps802154_tx_done(struct mcps802154_llhw *llhw);

/**
 * mcps802154_broken() - Signal an unrecoverable error, device needs to be
 * reset.
 * @llhw: Low-level device pointer.
 */
void mcps802154_broken(struct mcps802154_llhw *llhw);

/**
 * mcps802154_timer_expired() - Signal that a programmed timer expired.
 * @llhw: Low-level device pointer.
 *
 * To be called before the timestamp given to &mcps802154_ops.idle() callback.
 */
void mcps802154_timer_expired(struct mcps802154_llhw *llhw);

/**
 * is_before_dtu() - Check if timestamp A is before timestamp B.
 * @a_dtu: A timestamp in device time unit.
 * @b_dtu: B timestamp in device time unit.
 *
 * Return: true if A timestamp is before B timestamp.
 */
static inline bool is_before_dtu(u32 a_dtu, u32 b_dtu)
{
	return (s32)(a_dtu - b_dtu) < 0;
}

#ifdef CONFIG_MCPS802154_TESTMODE
/**
 * mcps802154_testmode_alloc_reply_skb() - Allocate testmode reply.
 * @llhw: Low-level device pointer.
 * @approxlen: an upper bound of the length of the data that will
 * be put into the skb.
 *
 * This function allocates and pre-fills an skb for a reply to
 * the testmode command. Since it is intended for a reply, calling
 * it outside of the @testmode_cmd operation is invalid.
 *
 * The returned skb is pre-filled with the netlink message's header
 * and attribute's data and set up in a way that any data that is
 * put into the skb (with skb_put(), nla_put() or similar) will end up
 * being within the %MCPS802154_ATTR_TESTDATA attribute, so all
 * that needs to be done with the skb is adding data for
 * the corresponding userspace tool which can then read that data
 * out of the testdata attribute. You must not modify the skb
 * in any other way.
 *
 * When done, call mcps802154_testmode_reply() with the skb and return
 * its error code as the result of the @testmode_cmd operation.
 *
 * Return: An allocated and pre-filled skb. %NULL if any errors happen.
 */
struct sk_buff *
mcps802154_testmode_alloc_reply_skb(struct mcps802154_llhw *llhw,
				    int approxlen);

/**
 * mcps802154_testmode_reply() - Send the reply skb.
 * @llhw: Low-level device pointer.
 * @skb: The skb, must have been allocated with
 * mcps802154_testmode_alloc_reply_skb().
 *
 * Since calling this function will usually be the last thing
 * before returning from the @testmode_cmd you should return
 * the error code.  Note that this function consumes the skb
 * regardless of the return value.
 *
 * Return: 0 or error.
 */
int mcps802154_testmode_reply(struct mcps802154_llhw *llhw,
			      struct sk_buff *skb);
#endif

#endif /* NET_MCPS802154_H */
