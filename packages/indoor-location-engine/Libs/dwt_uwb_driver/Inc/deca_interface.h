/*
 * @file: deca_interface.h
 * @brief Interface to the Decawave driver
 *
 * @author Decawave
 *
 * @attention Copyright 2020 - 2021 (c) Decawave Ltd, Dublin, Ireland.
 *            All rights reserved.
 */

#ifndef DECA_INTERFACE_H_
#define DECA_INTERFACE_H_

#include "deca_device_api.h"
#include <stdint.h>

typedef enum
{
    DWT_READ_REG,
    DWT_WRITE_REG,
    DWT_WAKEUP,
    DWT_FORCETRXOFF,
    DWT_STARTTX,
    DWT_SETDELAYEDTRXTIME,
    DWT_CONFIGURESFDTYPE,
    DWT_CONFIGURELEADDRESS,
    DWT_SETKEYREG128,
    DWT_ENABLEGPIOCLOCKS,
    DWT_OTPREVISION,
    DWT_GETICREFVOLT,
    DWT_GETICREFTEMP,
    DWT_GETPARTID,
    DWT_GETLOTID,
    DWT_SIGNALRXBUFFFREE,
    DWT_SETRXAFTERTXDELAY,
    DWT_ENABLESPICRCCHECK,
    DWT_SETFINEGRAINTXSEQ,
    DWT_SETLNAPAMODE,
    DWT_READPGDELAY,
    DWT_CONFIGURESTSKEY,
    DWT_CONFIGURESTSIV,
    DWT_CONFIGURESTSLOADIV,
    DWT_CONFIGMRXLUT,
    DWT_RESTORECONFIG,
    DWT_CONFIGURESTSMODE,
    DWT_SETRXANTENNADELAY,
    DWT_GETRXANTENNADELAY,
    DWT_SETTXANTENNADELAY,
    DWT_GETTXANTENNADELAY,
    DWT_RXENABLE,
    DWT_WRITETXDATA,
    DWT_READRXDATA,
    DWT_WRITERXSCRATCHDATA,
    DWT_READRXSCRATCHDATA,
    DWT_READCARRIERINTEGRATOR,
    DWT_ENABLEAUTOACK,
    DWT_CHECKDEVID,
    DWT_CONFIGCIADIAG,
    DWT_ENTERSLEEPAFTERTX,
    DWT_ENTERSLEEPAFTER,
    DWT_RUNPGFCAL,
    DWT_PGF_CAL,
    DWT_READCLOCKOFFSET,
    DWT_CLEARAONCONFIG,
    DWT_CALCBANDWIDTHADJ,
    DWT_READDIAGNOSTICS,
    DWT_READTXTIMESTAMPHI32,
    DWT_READTXTIMESTAMPLO32,
    DWT_READTXTIMESTAMP,
    DWT_READPDOA,
    DWT_READTDOA,
    DWT_READWAKEUPTEMP,
    DWT_READWAKEUPVBAT,
    DWT_WRITETXFCTRL,
    DWT_OTPWRITE,
    DWT_OTPWRITEANDVERIFY,
    DWT_ENTERSLEEP,
    DWT_CONFIGURESLEEPCNT,
    DWT_CALIBRATESLEEPCNT,
    DWT_CONFIGURESLEEP,
    DWT_SOFTRESET,
    DWT_SETXTALTRIM,
    DWT_GETXTALTRIM,
    DWT_REPEATEDCW,
    DWT_CONFIGCWMODE,
    DWT_READTEMPVBAT,
    DWT_CONVERTRAWTEMP,
    DWT_CONVERTRAWVBAT,
    DWT_CONFIGCONTINUOUSFRAMEMODE,
    DWT_DISABLECONTINUOUSFRAMEMODE,
    DWT_STOPREPEATEDFRAMES,
    DWT_REPEATEDFRAMES,
    DWT_DOAES,
    DWT_READSTSQUALITY,
    DWT_CONFIGUREAES,
    DWT_READEVENTCOUNTERS,
    DWT_CONFIGEVENTCOUNTERS,
    DWT_SETPREAMBLEDETECTTIMEOUT,
    DWT_SETSNIFFMODE,
    DWT_SETRXTIMEOUT,
    DWT_AONREAD,
    DWT_AONWRITE,
    DWT_READSTSSTATUS,
    DWT_SETLEDS,
    DWT_SETDWSTATE,
    DWT_READSYSTIME,
    DWT_CHECKIDLERC,
    DWT_CHECKIRQ,
    DWT_CONFIGUREFRAMEFILTER,
    DWT_SETEUI,
    DWT_GETEUI,
    DWT_SETPANID,
    DWT_SETADDRESS16,
    DWT_READRXTIMESTAMP,
    DWT_READRXTIMESTAMP_IPATOV,
    DWT_READRXTIMESTAMPUNADJ,
    DWT_READRXTIMESTAMPHI32,
    DWT_READRXTIMESTAMPLO32,
    DWT_READRXTIMESTAMP_STS,
    DWT_READSYSTIMESTAMPHI32,
    DWT_OTPREAD,
    DWT_SETPLENFINE,
    DWT_CALCPGCOUNT,
    DWT_SETGPIOMODE,
    DWT_SETGPIODIR,
    DWT_SETGPIOVALUE,
    DWT_GETDGCDECISION,
    DWT_SETDBLRXBUFFMODE,
    DWT_SETREFERENCETRXTIME,
    DWT_MICSIZEFROMBYTES,
    DWT_PLL_CAL,
    DWT_CONFIGURE_RF_PORT,
    DWT_SETPDOAOFFSET,
    DWT_READPDOAOFFSET,
#ifdef WIN32
    DWT_SPICSWAKEUP,
#endif // WIN32
    DWT_WRITESYSSTATUSLO,
    DWT_WRITESYSSTATUSHI,
    DWT_READSYSSTATUSLO,
    DWT_READSYSSTATUSHI,
    DWT_WRITERDBSTATUS,
    DWT_READRDBSTATUS,
    DWT_GETFRAMELENGTH,
    DWT_READGPIOVALUE,
    DWT_CFGWIFICOEXSET,
    DWT_CFGANTSEL,
    DWT_RSTSYSTEMCNT,
    DWT_CFGOSTRMODE,
    DWT_NLOS_IPDIAG,
    DWT_NLOS_ALLDIAG,
    DWT_ADJ_TXPOWER,
    /* BEGIN: CHIP_SPECIFIC_SECTION DW3700 */
    DWT_SETINTERUPTDB,
    DWT_ENTERSLEEPFCMD,
    DWT_SOFTRESETFCMD,
    DWT_SOFTRESETNOSEMAFCMD,
    DWT_DSSEMAREQUEST,
    DWT_DSSEMARELEASE,
    DWT_DSSEMAFORCE,
    DWT_DSSEMASTATUS,
    DWT_DSENSLEEP,
    DWT_DSSETINT_SPIAVAIL,
    DWT_ENABLEDISABLEEQ,
    /* END: CHIP_SPECIFIC_SECTION DW3700 */
    /* BEGIN: CHIP_SPECIFIC_SECTION DW3720 */
    DWT_TIMERSRST,
    DWT_TIMERSRSTCLR,
    DWT_CONFIGTIMER,
    DWT_TIMEREXPIRATION,
    DWT_TIMERENABLE,
    DWT_CFGWIFICOEXGPIO,
    DWT_SETFIXEDSTS,
    DWT_SET_ALT_PULSE_SHAPE,
    /* END: CHIP_SPECIFIC_SECTION DW3720 */
    /* BEGIN: MCPS SPECIFIC IOCTL */
    DWT_SET_STS_LEN,
    DWT_CFG_STS,
    /* END: MCPS SPECIFIC IOCTL */
    /* BEGIN DEBUG */
    DWT_DBG_REGS,
    DWT_READCTRDBG,
    DWT_READDGCDBG,
    DWT_CIA_VERSION,
    DWT_GET_CIR_REGADD,
    /* END DEBUG */
} dwt_ioctl_e;

struct dwt_rw_data_s
{
    uint8_t *buffer;
    uint16_t length;
    uint16_t offset;
};

struct dwt_otp_read_s
{
    uint16_t address;
    uint32_t *array;
    uint8_t length;
};

struct dwt_enable_spi_crc_check_s
{
    dwt_spi_crc_mode_e crc_mode;
    dwt_spierrcb_t spireaderr_cb;
};

struct dwt_set_gpio_mode_s
{
    uint32_t mask;
    uint32_t mode;
};

struct dwt_set_gpio_value_s
{
    uint16_t gpio; // GPIO to set, can be single or multiple:
    int value;     // 0 or 1
};

struct dwt_configure_ff_s
{
    uint16_t enabletype;
    uint16_t filtermode;
};

struct dwt_aon_read_s
{
    uint8_t ret_val;
    uint16_t aon_address;
};

struct dwt_aon_write_s
{
    uint16_t aon_address;
    uint8_t aon_write_data;
};

struct dwt_opt_write_and_verify_s
{
    uint32_t value;
    uint16_t address;
};

struct dwt_configure_sleep_s
{
    uint16_t mode;
    uint8_t wake;
};

#ifdef WIN32
struct dwt_spi_cs_wakeup_s
{
    uint8_t *buff;
    uint16_t length;
};
#endif // WIN32

struct dwt_enable_auto_ack_s
{
    uint8_t responseDelayTime;
    int enable;
};

struct dwt_set_dbl_rx_buff_mode_s
{
    dwt_dbl_buff_state_e dbl_buff_state;
    dwt_dbl_buff_mode_e dbl_buff_mode;
};

struct dwt_set_sniff_mode_s
{
    int enable;
    uint8_t timeOn;
    uint8_t timeOff;
};

struct dwt_repeated_cw_s
{
    int cw_enable;
    int cw_mode_config;
};

struct dwt_convert_raw_temp_s
{
    float result;
    uint8_t raw_temp;
};

struct dwt_convert_raw_volt_s
{
    float result;
    uint8_t raw_voltage;
};

struct dwt_calc_bandwidth_adj_s
{
    uint8_t result;
    uint16_t target_count;
};

struct dwt_calc_pg_count_s
{
    uint16_t result;
    uint8_t pgdly;
};

struct dwt_mic_size_from_bytes_s
{
    dwt_mic_size_e result;
    uint8_t mic_size_in_bytes;
};

struct dwt_do_aes_s
{
    int8_t result;
    dwt_aes_job_t *job;
    dwt_aes_core_type_e core_type;
};

struct dwt_configure_le_address_s
{
    uint16_t addr;
    int leIndex;
};

struct dwt_set_interrupt_db_s
{
    uint8_t bitmask;
    dwt_INT_options_e INT_options;
};

struct dwt_tx_fctrl_s
{
    uint16_t txFrameLength;
    uint16_t txBufferOffset;
    uint8_t ranging;
};

struct dwt_config_rf_port_s
{
    dwt_rf_port_selection_e port;
    dwt_rf_port_ctrl_e enable;
};

struct dwt_ostr_mode_s
{
    uint8_t enable;
    uint16_t wait_time;
};

struct dwt_cfg_wifi_coex_set_s
{
    dwt_wifi_coex_e enable;
    int coex_io_swap;
};

struct dwt_cfg_wifi_coex_s
{
    uint8_t timer_coexout;
    uint8_t coex_swap;
};

struct dwt_timer_exp_s
{
    dwt_timers_e timer_name;
    uint32_t exp;
};

struct dwt_adj_tx_power_s
{
    int result;
    uint16_t boost;
    uint32_t ref_tx_power;
    uint8_t channel;
    uint32_t* adj_tx_power;
    uint16_t* applied_boost;
};

struct dwchip_s;
struct dw_rx_s;

struct dwt_callbacks_s
{
    dwt_spierrcb_t cbSPIRDErr; // Callback for SPI read error events
    dwt_cb_t cbTxDone;         // Callback for TX confirmation event
    dwt_cb_t cbRxOk;           // Callback for RX good frame event
    dwt_cb_t cbRxTo;           // Callback for RX timeout events
    dwt_cb_t cbRxErr;          // Callback for RX error events
    dwt_cb_t cbSPIErr;         // Callback for SPI error events
    dwt_cb_t cbSPIRdy;         // Callback for SPI ready events
    dwt_cb_t cbDualSPIEv;      // Callback for dual SPI events

    int (*rx_frame)(struct dwchip_s *, struct dw_rx_s *);
    int (*rx_error)(struct dwchip_s *, int);
};

/*! ------------------------------------------------------------------------------------------------------------------
    * @brief The dwt_spi_s structure is a structure assembling all the SPI functions that must be defined externally
    * NB: In porting this to a particular microprocessor, the implementer needs to define the low
    * level abstract functions matching the selected hardware.
*/
struct dwt_spi_s
{
/*! ------------------------------------------------------------------------------------------------------------------
    * @brief readfromspi
    * Low level abstract function to read from the SPI
    * Takes two separate byte buffers for write header and read data
    * input parameters:
    * @param headerLength  - number of bytes header to write
    * @param headerBuffer  - pointer to buffer containing the 'headerLength' bytes of header to write
    * @param readlength    - number of bytes data being read
    * @param readBuffer    - pointer to buffer containing to return the data (NB: size required = headerLength + readlength)
    *
    * output parameters:
    * returns DWT_SUCCESS for success, or DWT_ERROR for error
    */
    int (*readfromspi)(uint16_t headerLength, /*const*/ uint8_t *headerBuffer, uint16_t readlength, uint8_t *readBuffer);

/*! ------------------------------------------------------------------------------------------------------------------
    * @brief writetospi
    * Low level abstract function to write to the SPI
    * Takes two separate byte buffers for write header and write data
    * input parameters:
    * @param headerLength  - number of bytes header being written
    * @param headerBuffer  - pointer to buffer containing the 'headerLength' bytes of header to be written
    * @param bodylength    - number of bytes data being written
    * @param bodyBuffer    - pointer to buffer containing the 'bodylength' bytes od data to be written
    *
    * output parameters:
    * returns DWT_SUCCESS for success, or DWT_ERROR for error
    */
    int (*writetospi)(uint16_t headerLength, const uint8_t *headerBuffer, uint16_t bodyLength, const uint8_t *bodyBuffer);

/*! ------------------------------------------------------------------------------------------------------------------
    * @brief writetospiwithcrc
    * Low level abstract function to write to the SPI, this should be used when DW3000 SPI CRC mode is used
    * Takes two separate byte buffers for write header and write data
    * input parameters:
    * @param headerLength  - number of bytes header being written
    * @param headerBuffer  - pointer to buffer containing the 'headerLength' bytes of header to be written
    * @param bodylength    - number of bytes data being written
    * @param bodyBuffer    - pointer to buffer containing the 'bodylength' bytes od data to be written
    * @param crc8          - 8-bit crc, calculated on the header and data bytes
    *
    * output parameters:
    * returns DWT_SUCCESS for success, or DWT_ERROR for error
    */
    int (*writetospiwithcrc)(uint16_t headerLength, const uint8_t *headerBuffer, uint16_t bodyLength, const uint8_t *bodyBuffer, uint8_t crc8);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief setslowrate
     * Low level abstract function to switch the SPI into slow rate mode
     *
     * input parameters:
     *
     * output parameters
     *
     */
    void (*setslowrate)(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief setspifastrate
     * Low level abstract function to switch the SPI into fast rate mode
     *
     * input parameters:
     *
     * output parameters
     *
     */
    void (*setfastrate)(void);
};

struct rxtx_configure_s
{
    dwt_config_t *pdwCfg;
    dwt_txconfig_t *txConfig;
    uint16_t frameFilter;
    uint16_t frameFilterMode;
    uint16_t txAntDelay;
    uint16_t rxAntDelay;
    uint16_t panId;
    uint16_t shortadd;
};

typedef struct rxtx_configure_s rxtx_configure_t;

struct dwt_mcps_config_s
{
    int mode;
    int do_reset;
    int led_mode;
    int lnapamode;
    int bitmask_lo;
    int bitmask_hi;
    int int_options;
    struct dwt_configure_sleep_s sleep_config;
    dwt_sts_cp_key_t *stsKey; /**< AES Key to be used to set the STS */
    dwt_sts_cp_iv_t *stsIv;   /**< AES IV to be used to set the initial IV */
    rxtx_configure_t *rxtx_config;
    uint8_t xtalTrim;
    uint8_t cia_enable_mask;
    uint8_t loadIv;
    uint8_t event_counter;
};
typedef struct dwt_mcps_config_s dwt_mcps_config_t;

/*The contract is that we have the SPI interface to the DW chip*/
struct dwchip_s
{   
    /*HAL*/
    struct dwt_spi_s *SPI; // first
    void(*wakeup_device_with_io)(void);

    /*Driver*/
    struct dwt_driver_s *dwt_driver;
    struct dwt_callbacks_s callbacks;

    /* driver configuration */
    struct dwt_mcps_config_s *config;

    /* MCPS */
    struct mcps802154_llhw *llhw;
    struct mcps802154_ops *mcps_ops;
    struct dw3000_calibration_data *calib_data;
    struct dwt_mcps_runtime_s *mcps_runtime;
    struct dwt_mcps_rx_s *rx;

    /* GPIO used to switch off WIFI while transmitting, for example */
    int8_t coex_gpio_pin;
    int8_t coex_gpio_active_state;

    /** driver data*/
    void *priv; // last
};
typedef struct dwchip_s dwchip_t;

struct dw_rx_frame_info_s
{
    uint32_t rx_date_dtu;
    uint32_t rx_timeout_pac;
    int rx_delayed;
};

struct dw_tx_frame_info_s
{
    uint32_t tx_date_dtu;    /* 4ns time units: */
    int rx_delay_dly;        /* sy: 1.0256us */
    uint32_t rx_timeout_pac; /* SFD Timeout ? */
    int flag;                /* flag */

#ifndef MCPS_RANGING_BIT
#define MCPS_RANGING_BIT 0x40
#endif
};
struct dw_addr_filt_s;

struct dwt_ops_s
{
    int (*configure)(struct dwchip_s *dw, dwt_config_t *config);
    int (*write_tx_data)(struct dwchip_s *dw, uint16_t txDataLength, uint8_t *txDataBytes, uint16_t txBufferOffset);
    void (*write_tx_fctrl)(struct dwchip_s *dw, uint16_t txFrameLength, uint16_t txBufferOffset, uint8_t ranging);
    void (*read_rx_data)(struct dwchip_s *dw, uint8_t *buffer, uint16_t length, uint16_t rxBufferOffset);
    void (*read_acc_data)(struct dwchip_s *dw, uint8_t *buffer, uint16_t length, uint16_t accOffset);
    void (*read_rx_timestamp)(struct dwchip_s *dw, uint8_t *timestamp);
    void (*configure_tx_rf)(struct dwchip_s *dw, dwt_txconfig_t *config);
    void (*set_interrupt)(struct dwchip_s *dw, uint32_t bitmask_lo, uint32_t bitmask_hi, dwt_INT_options_e INT_options);
    int (*rx_enable)(struct dwchip_s *dw, int mode);
    int (*initialize)(struct dwchip_s *dw, int mode);
    void (*xfer)(struct dwchip_s *dw, uint32_t regFileID, uint16_t index, uint16_t length, uint8_t *buffer, const spi_modes_e mode);

    int (*ioctl)(struct dwchip_s *, dwt_ioctl_e, int parm, void *ptr);

    void (*isr)(struct dwchip_s *);

    void* (*dbg_fn)(struct dwchip_s*, dwt_ioctl_e, int parm, void* ptr);
};

struct dwt_mcps_ops_s
{
    int (*init)(struct dwchip_s *);
    void (*deinit)(struct dwchip_s *);
    int (*tx_frame)(struct dwchip_s *, uint8_t *data, size_t len, struct dw_tx_frame_info_s *info);
    int (*rx_enable)(struct dwchip_s *, struct dw_rx_frame_info_s *info);
    int (*rx_disable)(struct dwchip_s *);
    uint64_t (*get_timestamp)(struct dwchip_s *);
    void (*get_rx_frame)(struct dwchip_s *, uint8_t *ptr, size_t len);
    int (*set_hrp_uwb_params)(struct dwchip_s *, int prf, int fsr, int sfd_selector, int phr_rate, int data_rate);
    int (*set_channel)(struct dwchip_s *, int page, int channel, int preamble_code);
    int (*set_hw_addr_filt)(struct dwchip_s *, struct dw_addr_filt_s *file, int changed);

    // Compat direct-access fns - Used in dw3000_mcps_mcu.c . This is required for compatibility purpose with some MCPS functions
    struct mcps_compat_
    {
        int (*sys_status_and_or)(struct dwchip_s *dw, uint32_t _and, uint32_t _or);
        void (*ack_enable)(struct dwchip_s *dw, int enable);
        void (*set_interrupt)(struct dwchip_s *dw, uint32_t bitmask_lo, uint32_t bitmask_hi, dwt_INT_options_e INT_options);
    } mcps_compat;

    int (*ioctl)(struct dwchip_s *, dwt_ioctl_e, int parm, void *ptr);

    void (*isr)(struct dwchip_s *);
};
struct dwt_driver_s
{
    uint32_t devid;
    uint32_t devmatch;
    const char *name;
    const char *version;
    const struct dwt_ops_s *dwt_ops;
    const struct dwt_mcps_ops_s *dwt_mcps_ops;
    uint32_t vernum;
};

/* STD interface fn() */
int interface_init(struct dwchip_s *p);
void interface_deinit(struct dwchip_s *p);
int interface_tx_frame(struct dwchip_s *dw, uint8_t *data, size_t len, struct dw_tx_frame_info_s *info);
int interface_rx_enable(struct dwchip_s *dw, struct dw_rx_frame_info_s *info);
int interface_rx_disable(struct dwchip_s *dw);
uint64_t interface_get_timestamp(struct dwchip_s *dw);
void interface_read_rx_frame(struct dwchip_s *dw, uint8_t *ptr, size_t len);

#endif /* DECA_INTERFACE_H_ */
