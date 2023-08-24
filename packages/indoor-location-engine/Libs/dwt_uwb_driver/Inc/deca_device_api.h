/*! ----------------------------------------------------------------------------
 * @file    deca_device_api.h
 * @brief   DW3000 API Functions
 *
 * @attention
 *
 * Copyright 2013 - 2021(c) Decawave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 */

#ifndef _DECA_DEVICE_API_H_
#define _DECA_DEVICE_API_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "deca_types.h"

#ifndef DWT_NUM_DW_DEV
#define DWT_NUM_DW_DEV (1)
#endif

#define DWT_BIT_MASK(bit_num) (((uint32_t)1) << (bit_num))
#define FCS_LEN               (2)

#define GPIO_MFIO_MODE_MASK 0x7U
#define STS_CONFIG_HI_B0_MASK 0xFF  // STS_CONFIG_HI Byte 0 mask.
#define RX_SFD_HLDOFF 0x20000000UL  //Number of symbols of accumulation to wait before checking for an SFD pattern, when Ipatov len > 64.
#define RX_SFD_HLDOFF_DEF 0x14000000UL   //Default number of symbols of accumulation to wait before checking for an SFD pattern.
#define RX_BUFFER_MAX_LEN (1023)

    /* Default interface implementation functions */

#define DW3000_CHIP_FREQ    499200000
#define DW3000_CHIP_PER_DTU 2
#define DW3000_CHIP_PER_DLY 512
#define DW3000_DTU_FREQ     (DW3000_CHIP_FREQ / DW3000_CHIP_PER_DTU)
#define DTU_TO_US(x)        (int)((uint64_t)(x)*1000000 / DW3000_DTU_FREQ)
#define US_TO_DTU(x)        (int)((uint64_t)(x)*DW3000_DTU_FREQ / 1000000)
#define COEX_TIME_US        1000
#define COEX_TIME_DTU       US_TO_DTU(COEX_TIME_US)
#define COEX_MARGIN_US      20
#define COEX_MARGIN_DTU     US_TO_DTU(COEX_MARGIN_US)

#define DWT_DW3720_PDOA_DEV_ID   DWT_QM33120_PDOA_DEV_ID   /* Backward compatibility definition of the P/N */

// Enable CRC functionality. Disable to save space when CRC not required.
#define DWT_ENABLE_CRC

    /* */
    typedef enum
    {
        DWT_SUCCESS = 0,
        DWT_ERROR = -1,
        DWT_ERR_PLL_LOCK = -2,
        DWT_ERR_RX_CAL_PGF = -3,
        DWT_ERR_RX_CAL_RESI = -4,
        DWT_ERR_RX_CAL_RESQ = -5,
        DWT_ERR_RX_ADC_CAL = -6,
    } dwt_error_e;

#define DWT_TIME_UNITS (1.0 / 499.2e6 / 128.0) //!< = 15.65e-12 s

    typedef enum
    {
        DWT_DW3000_DEV_ID = (int)(0xDECA0302),      //!< DW3000 (non PDOA) silicon device ID
        DWT_QM33110_DEV_ID = (int)(0xDECA0304),     //!< QM33110 (non PDOA) silicon device ID
        DWT_DW3000_PDOA_DEV_ID = (int)(0xDECA0312), //!< DW3000 (with PDOA) silicon device ID
        DWT_DW3700_PDOA_DEV_ID = (int)(0xDECA0313), //!< DW3700 (with PDOA) silicon device ID
        DWT_QM33120_PDOA_DEV_ID = (int)(0xDECA0314) //!< QM33120 (with PDOA) silicon device ID
    } dw_chip_id_e;

#define DELAY_20uUSec       (20) /*Delay of 20uSec(measured 24uSec)*/
#define MAX_RETRIES_FOR_PLL (50) /*The PLL calibration should take less than 400us, typically it is < 100us (however on some corners with ch9 it can take ~900us)*/
#define MAX_RETRIES_FOR_PGF (3)

    typedef enum
    {
        AOA,
        NON_AOA
    } dw3000type_e;

    typedef enum
    {
        DW3000_SPI_RD_BIT = 0x0000U,
        DW3000_SPI_RD_FAST_CMD = 0x0001U,
        DW3000_SPI_WR_FAST_CMD = 0x0002U,
        DW3000_SPI_WR_BIT = 0x8000U,
        DW3000_SPI_AND_OR_8 = 0x8001U,
        DW3000_SPI_AND_OR_16 = 0x8002U,
        DW3000_SPI_AND_OR_32 = 0x8003U,
    } spi_modes_e;

    //! enums for selecting the bit rate for data TX (and RX)
    //! These are defined for write (with just a shift) the TX_FCTRL register
    typedef enum
    {
        DWT_BR_850K = 0,   //!< UWB bit rate 850 kbits/s
        DWT_BR_6M8 = 1,    //!< UWB bit rate 6.8 Mbits/s
        DWT_BR_NODATA = 2, //!< No data (SP3 packet format)
    } dwt_uwb_bit_rate_e;

    //! enums for specifying the (Nominal) mean Pulse Repetition Frequency
    //! These are defined for direct write (with a shift if necessary) to CHAN_CTRL and TX_FCTRL regs
    typedef enum
    {
        DWT_PRF_16M = 1, //!< UWB PRF 16 MHz
        DWT_PRF_64M = 2, //!< UWB PRF 64 MHz
        DWT_PRF_SCP = 3, //!< SCP UWB PRF ~100 MHz
    } dwt_prf_e;

    //! enums for specifying Preamble Acquisition Chunk (PAC) Size in symbols
    typedef enum
    {
        DWT_PAC8 = 0,  //!< PAC  8 (recommended for RX of preamble length  128 and below
        DWT_PAC16 = 1, //!< PAC 16 (recommended for RX of preamble length  256
        DWT_PAC32 = 2, //!< PAC 32 (recommended for RX of preamble length  512
        DWT_PAC4 = 3,  //!< PAC  4 (recommended for RX of preamble length  < 127
    } dwt_pac_size_e;

    //! enums for specifying SFD Types and size
    typedef enum
    {
        DWT_SFD_IEEE_4A = 0, //!< IEEE 8-bit ternary
        DWT_SFD_DW_8 = 1,    //!< DW 8-bit
        DWT_SFD_DW_16 = 2,   //!< DW 16-bit
        DWT_SFD_IEEE_4Z = 3, //!< IEEE 8-bit binary (4z)
        DWT_SFD_LEN8 = 8,    //!< IEEE, and DW 8-bit are length 8
        DWT_SFD_LEN16 = 16,  //!< DW 16-bit is length 16
    } dwt_sfd_type_e;

    //! enums for specifying TX Preamble length in symbols
    //! These are defined to allow them be directly written into byte 2 of the TX_FCTRL register
    //! (i.e. a four bit value destined for bits 20..18 but shifted left by 2 for byte alignment)
    typedef enum
    {
        DWT_PLEN_4096 = 0x03, //! Standard preamble length 4096 symbols
        DWT_PLEN_2048 = 0x0A, //! Non-standard preamble length 2048 symbols
        DWT_PLEN_1536 = 0x06, //! Non-standard preamble length 1536 symbols
        DWT_PLEN_1024 = 0x02, //! Standard preamble length 1024 symbols
        DWT_PLEN_512 = 0x0d,  //! Non-standard preamble length 512 symbols
        DWT_PLEN_256 = 0x09,  //! Non-standard preamble length 256 symbols
        DWT_PLEN_128 = 0x05,  //! Non-standard preamble length 128 symbols
        DWT_PLEN_64 = 0x01,   //! Standard preamble length 64 symbols
        DWT_PLEN_32 = 0x04,   //! Non-standard length 32
        DWT_PLEN_72 = 0x07,   //! Non-standard length 72
    } dwt_tx_plen_e;

#define DWT_SFDTOC_DEF 129 // default SFD timeout value

    //! enums for selecting PHR modes
    typedef enum
    {
        DWT_PHRMODE_STD = 0x0, // standard PHR mode
        DWT_PHRMODE_EXT = 0x1, // DW proprietary extended frames PHR mode
    } dwt_phr_mode_e;

    //! enums for selecting PHR rate
    typedef enum
    {
        DWT_PHRRATE_STD = 0x0, // standard PHR rate
        DWT_PHRRATE_DTA = 0x1, // PHR at data rate (6M81)
    } dwt_phr_rate_e;

    // Define DW3000 PDOA modes
    typedef enum
    {
        DWT_PDOA_M0 = 0x0, // DW PDOA mode is off
        DWT_PDOA_M1 = 0x1, // DW PDOA mode 1
        DWT_PDOA_M3 = 0x3, // DW PDOA mode 3
    } dwt_pdoa_mode_e;

    // Define DW3000 STS modes
    typedef enum
    {
        DWT_STS_MODE_OFF = 0x0, // STS is off
        DWT_STS_MODE_1 = 0x1,   // STS mode 1
        DWT_STS_MODE_2 = 0x2,   // STS mode 2
        DWT_STS_MODE_ND = 0x3,  // STS with no data
        DWT_STS_MODE_SDC = 0x8, // Enable Super Deterministic Codes
        DWT_STS_CONFIG_MASK = 0xB,
        DWT_STS_CONFIG_MASK_NO_SDC = 0x3,
    } dwt_sts_mode_e;

#define DWT_SFD_COUNT_WARN (0x2000 >> 7) // SFD count warning bit (STS quality status bit)

    // Defined constants for "mode" bitmask parameter passed into dwt_starttx() function.
    typedef enum
    {
        DWT_START_TX_IMMEDIATE = 0x00, //! Send the frame immediately
        DWT_START_TX_DELAYED = 0x01,   //! Send the frame at specified time (time must be less that half period away)
        DWT_RESPONSE_EXPECTED = 0x02,  //! Will enable the receiver after TX has completed
        DWT_START_TX_DLY_REF = 0x04,   //! Send the frame at specified time (time in DREF_TIME register + any time in DX_TIME register)
        DWT_START_TX_DLY_RS = 0x08,    //! Send the frame at specified time (time in RX_TIME_0 register + any time in DX_TIME register)
        DWT_START_TX_DLY_TS = 0x10,    //! Send the frame at specified time (time in TX_TIME_LO register + any time in DX_TIME register)
        DWT_START_TX_CCA = 0x20,       //! Send the frame if no preamble detected within PTO time
    } dwt_starttx_mode_e;

    // Defined constants for "mode" bitmask parameter passed into dwt_rxenable() function.
    typedef enum
    {
        DWT_START_RX_IMMEDIATE = 0x00, //! Enable the receiver immediately
        DWT_START_RX_DELAYED = 0x01,   //! Set up delayed RX, if "late" error triggers, then the RX will be enabled immediately
        //! If delay RX fails due to "late" error then if this flag is set, the RX will not be re-enabled immediately, and device will be in IDLE when function
        //! exits.
        DWT_IDLE_ON_DLY_ERR = 0x02,
        DWT_START_RX_DLY_REF = 0x04, //! Enable the receiver at specified time (time in DREF_TIME register + any time in DX_TIME register)
        DWT_START_RX_DLY_RS = 0x08,  //! Enable the receiver at specified time (time in RX_TIME_0 register + any time in DX_TIME register)
        DWT_START_RX_DLY_TS = 0x10,  //! Enable the receiver at specified time (time in TX_TIME_LO register + any time in DX_TIME register)
    } dwt_startrx_mode_e;

    /******************************************************************************
     * @brief Bit definition of the SYS_ENABLE register
     * exported for dwt_setinterrupt() API
     **/
    typedef enum
    {
        DWT_INT_TIMER1_BIT_MASK = (int)(0x80000000), // TIMER1 expiry
        DWT_INT_TIMER0_BIT_MASK = 0x40000000UL,      // TIMER0 expiry
        DWT_INT_ARFE_BIT_MASK = 0x20000000UL,        // Frame filtering error
        DWT_INT_CPERR_BIT_MASK = 0x10000000UL,       // STS quality warning/error
        DWT_INT_HPDWARN_BIT_MASK = 0x8000000UL,      // Half period warning flag when delayed TX/RX is used
        DWT_INT_RXSTO_BIT_MASK = 0x4000000UL,        // SFD timeout
        DWT_INT_PLL_HILO_BIT_MASK = 0x2000000UL,     // PLL calibration flag
        DWT_INT_RCINIT_BIT_MASK = 0x1000000UL,       // Device has entered IDLE_RC
        DWT_INT_SPIRDY_BIT_MASK = 0x800000UL,        // SPI ready flag
        DWT_INT_RXPTO_BIT_MASK = 0x200000UL,         // Preamble timeout
        DWT_INT_RXOVRR_BIT_MASK = 0x100000UL,        // RX overrun event when double RX buffer is used
        DWT_INT_VWARN_BIT_MASK = 0x80000UL,          // Brownout event detected
        DWT_INT_CIAERR_BIT_MASK = 0x40000UL,         // CIA error
        DWT_INT_RXFTO_BIT_MASK = 0x20000UL,          // RX frame wait timeout
        DWT_INT_RXFSL_BIT_MASK = 0x10000UL,          // Reed-Solomon error (RX sync loss)
        DWT_INT_RXFCE_BIT_MASK = 0x8000U,            // RX frame CRC error
        DWT_INT_RXFCG_BIT_MASK = 0x4000U,            // RX frame CRC good
        DWT_INT_RXFR_BIT_MASK = 0x2000U,             // RX ended - frame ready
        DWT_INT_RXPHE_BIT_MASK = 0x1000U,            // PHY header error
        DWT_INT_RXPHD_BIT_MASK = 0x800U,             // PHY header detected
        DWT_INT_CIADONE_BIT_MASK = 0x400U,           // CIA done
        DWT_INT_RXSFDD_BIT_MASK = 0x200U,            // SFD detected
        DWT_INT_RXPRD_BIT_MASK = 0x100U,             // Preamble detected
        DWT_INT_TXFRS_BIT_MASK = 0x80U,              // Frame sent
        DWT_INT_TXPHS_BIT_MASK = 0x40U,              // Frame PHR sent
        DWT_INT_TXPRS_BIT_MASK = 0x20U,              // Frame preamble sent
        DWT_INT_TXFRB_BIT_MASK = 0x10U,              // Frame transmission begins
        DWT_INT_AAT_BIT_MASK = 0x8U,                 // Automatic ACK transmission pending
        DWT_INT_SPICRCE_BIT_MASK = 0x4U,             // SPI CRC error
        DWT_INT_CP_LOCK_BIT_MASK = 0x2U,             // PLL locked
        DWT_INT_IRQS_BIT_MASK = 0x1U,                // Interrupt set
    } dwt_int_conf_e;

    // Bit definition of the double RX buffer status events
    typedef enum
    {
        DWT_RDB_STATUS_CP_ERR1_BIT_MASK = 0x80U,  // STS quality warning/error in RX buffer 1
        DWT_RDB_STATUS_CIADONE1_BIT_MASK = 0x40U, // CIA done for frame in RX buffer 1
        DWT_RDB_STATUS_RXFR1_BIT_MASK = 0x20U,    // Frame ready in RX buffer 1
        DWT_RDB_STATUS_RXFCG1_BIT_MASK = 0x10U,   // Frame CC good in RX buffer 1
        DWT_RDB_STATUS_CP_ERR0_BIT_MASK = 0x8U,   // STS quality warning/error in RX buffer 0
        DWT_RDB_STATUS_CIADONE0_BIT_MASK = 0x4U,  // CIA done for frame in RX buffer 0
        DWT_RDB_STATUS_RXFR0_BIT_MASK = 0x2U,     // Frame ready in RX buffer 0
        DWT_RDB_STATUS_RXFCG0_BIT_MASK = 0x1U,    // Frame CC good in RX buffer 0
    } dwt_rdb_e;

/* RX events mask relating to reception into RX buffer 0,  when double buffer is used */
#define DWT_RDB_STATUS_CLEAR_BUFF0_EVENTS (RDB_STATUS_CP_ERR0_BIT_MASK | RDB_STATUS_CIADONE0_BIT_MASK | RDB_STATUS_RXFR0_BIT_MASK | RDB_STATUS_RXFCG0_BIT_MASK)
/* RX events mask relating to reception into RX buffer 1,  when double buffer is used */
#define DWT_RDB_STATUS_CLEAR_BUFF1_EVENTS (RDB_STATUS_CP_ERR1_BIT_MASK | RDB_STATUS_CIADONE1_BIT_MASK | RDB_STATUS_RXFR1_BIT_MASK | RDB_STATUS_RXFCG1_BIT_MASK)

#define RDB_STATUS_RXOK                                                                                                                                        \
    (DWT_RDB_STATUS_RXFCG0_BIT_MASK | DWT_RDB_STATUS_RXFR0_BIT_MASK | DWT_RDB_STATUS_CIADONE0_BIT_MASK | DWT_RDB_STATUS_CP_ERR0_BIT_MASK                       \
        | DWT_RDB_STATUS_RXFCG1_BIT_MASK | DWT_RDB_STATUS_RXFR1_BIT_MASK | DWT_RDB_STATUS_CIADONE1_BIT_MASK | DWT_RDB_STATUS_CP_ERR1_BIT_MASK)

//DW37xx double RX buffer interrupt events
#define DWT_DB_INT_RX           (RDB_STATUS_RXOK)

// DW3000 interrupt events
#define DWT_INT_RX                                                                                                                                             \
    (DWT_INT_CIADONE_BIT_MASK | DWT_INT_RXFCG_BIT_MASK | DWT_INT_RXPHE_BIT_MASK | DWT_INT_RXFR_BIT_MASK | DWT_INT_RXFCE_BIT_MASK | DWT_INT_RXFSL_BIT_MASK      \
        | DWT_INT_RXFTO_BIT_MASK | DWT_INT_CIAERR_BIT_MASK | DWT_INT_RXPTO_BIT_MASK | DWT_INT_RXSTO_BIT_MASK | DWT_INT_ARFE_BIT_MASK)
#define DWT_INT_ALL_LO (0xffffffff)
#define DWT_INT_ALL_HI (0xffffffff)

/* User defined RX timeouts (frame wait timeout and preamble detect timeout) mask. */
#define SYS_STATUS_ALL_RX_TO (DWT_INT_RXFTO_BIT_MASK | DWT_INT_RXPTO_BIT_MASK | DWT_INT_CPERR_BIT_MASK)

/* All RX errors mask. */
#define SYS_STATUS_ALL_RX_ERR                                                                                                                                  \
    (DWT_INT_RXPHE_BIT_MASK | DWT_INT_RXFCE_BIT_MASK | DWT_INT_RXFSL_BIT_MASK | DWT_INT_RXSTO_BIT_MASK | DWT_INT_ARFE_BIT_MASK | DWT_INT_CIAERR_BIT_MASK       \
        | DWT_INT_CPERR_BIT_MASK)

/* All RX events after a correct packet reception mask. */
#define SYS_STATUS_ALL_RX_GOOD                                                                                                                                 \
    (DWT_INT_RXFR_BIT_MASK | DWT_INT_RXFCG_BIT_MASK | DWT_INT_RXPRD_BIT_MASK | DWT_INT_RXSFDD_BIT_MASK | DWT_INT_RXPHD_BIT_MASK | DWT_INT_CIADONE_BIT_MASK)

/* All STS Mode 3 RX errors mask. */
#define SYS_STATUS_ALL_ND_RX_ERR     (DWT_INT_CIAERR_BIT_MASK | DWT_INT_RXSTO_BIT_MASK)
#define DWT_INT_HI_CCA_FAIL_BIT_MASK 0x1000U

#define DWT_INT_AES_STS_MEM_CONF_BIT_MASK  0x8U
#define DWT_INT_AES_STS_TRANS_ERR_BIT_MASK 0x4U
#define DWT_INT_AES_STS_AUTH_ERR_BIT_MASK  0x2U

/* Entry point to AES IV 0 */
#define DWT_AES_IV_ENTRY (0x10034)

#define DWT_AES_ERRORS (DWT_INT_AES_STS_AUTH_ERR_BIT_MASK | DWT_INT_AES_STS_TRANS_ERR_BIT_MASK | DWT_INT_AES_STS_MEM_CONF_BIT_MASK)
    // Defined constants when SPI CRC mode is used:
    typedef enum
    {
        DWT_SPI_CRC_MODE_NO = 0, /* No CRC */
        DWT_SPI_CRC_MODE_WR, /* This is used to enable SPI CRC check (the SPI CRC check will be enabled on DW3000 and CRC-8 added for SPI write transactions) */
        DWT_SPI_CRC_MODE_WRRD /* This is used to optionally enable additional CRC check on the SPI read operations, while the CRC check on the SPI write
                                 operations is also enabled */
    } dwt_spi_crc_mode_e;

    // Defined constants for "mode" bit field parameter passed to dwt_setleds() function.
    typedef enum
    {
        DWT_LEDS_DISABLE = 0x00,
        DWT_LEDS_ENABLE = 0x01,
        DWT_LEDS_INIT_BLINK = 0x02,
        // Default blink time. Blink time is expressed in multiples of 14 ms. The value defined here is ~225 ms.
        DWT_LEDS_BLINK_TIME_DEF = 0x10,
    } dwt_setleds_mode_e;

    typedef enum
    {
        // Common for all devices
        GPIO_PIN0_EXTTXE = 0x2 << 0,     /* Deprecated, only works for DW3000. The pin operates as the EXTTXE output (output TX state) */
        GPIO_PIN1_EXTRXE = 0x2 << (1*3), /* Deprecated, only works for DW3000. The pin operates as the EXTRXE output (output RX state) */
        GPIO_PIN2_RXLED  = 0x1 << (2*3), /* The pin operates as the RXLED output */
        GPIO_PIN3_TXLED  = 0x1 << (3*3), /* The pin operates as the TXLED output */
        GPIO_PIN4_EXTDA  = 0x1 << (4*3), /* Deprecated, only works for DW3000. The pin operates to support external DA/PA */
        GPIO_PIN4_EXTTXE = 0x2 << (4*3), /* Deprecated, only works for DW37xx. The pin operates as the EXTTXE output (output TX state) */
        GPIO_PIN5_EXTTX  = 0x1 << (5*3), /* Deprecated, only works for DW3000. The pin operates to support external PA / TX enable */
        GPIO_PIN5_EXTRXE = 0x2 << (5*3), /* Deprecated, only works for DW37xx. The pin operates as the EXTRXE output (output RX state) */
        GPIO_PIN6_EXTRX  = 0x1 << (6*3), /* Deprecated, only works for DW3000. The pin operates to support external LNA */
        // DW3000
        DW3000_GPIO_PIN0_GPIO        = 0x0,
        DW3000_GPIO_PIN0_RXOKLED     = 0x1,
        DW3000_GPIO_PIN0_PDOA_SW_TX  = 0x2,
        DW3000_GPIO_PIN1_GPIO        = 0x0 << (1*3),
        DW3000_GPIO_PIN1_SFDLED      = 0x1 << (1*3),
        DW3000_GPIO_PIN1_PDOA_SW_RX  = 0x2 << (1*3),
        DW3000_GPIO_PIN2_GPIO        = 0x0 << (2*3),
        DW3000_GPIO_PIN2_RXLED       = 0x1 << (2*3),
        DW3000_GPIO_PIN2_PDOA_SW_RF1 = 0x2 << (2*3),
        DW3000_GPIO_PIN3_GPIO        = 0x0 << (3*3),
        DW3000_GPIO_PIN3_TXLED       = 0x1 << (3*3),
        DW3000_GPIO_PIN3_PDOA_SW_RF2 = 0x2 << (3*3),
        DW3000_GPIO_PIN4_GPIO        = 0x0 << (4*3),
        DW3000_GPIO_PIN4_EXTPA       = 0x1 << (4*3),
        DW3000_GPIO_PIN4_IRQ         = 0x2 << (4*3),
        DW3000_GPIO_PIN5_GPIO        = 0x0 << (5*3),
        DW3000_GPIO_PIN5_EXTTXE      = 0x1 << (5*3),
        DW3000_GPIO_PIN6_GPIO        = 0x0 << (6*3),
        DW3000_GPIO_PIN6_EXTRXE      = 0x1 << (6*3),
        DW3000_GPIO_PIN7_SYNC        = 0x0 << (7*3),
        DW3000_GPIO_PIN7_GPIO        = 0x1 << (7*3),
        DW3000_GPIO_PIN8_IRQ         = 0x0 << (8*3),
        DW3000_GPIO_PIN8_GPIO        = 0x1 << (8*3),
        // DW3700 and DW3720
        DW37XX_GPIO_PIN0_SPI2_CLK    = 0x0,
        DW37XX_GPIO_PIN0_RXOKLED     = 0x1,
        DW37XX_GPIO_PIN0_GPIO        = 0x2,
        DW37XX_GPIO_PIN1_SPI2_MISO   = 0x0 << (1*3),
        DW37XX_GPIO_PIN1_SFDLED      = 0x1 << (1*3),
        DW37XX_GPIO_PIN1_GPIO        = 0x2 << (1*3),
        DW37XX_GPIO_PIN2_IRQ2        = 0x0 << (2*3),
        DW37XX_GPIO_PIN2_RXLED       = 0x1 << (2*3),
        DW37XX_GPIO_PIN2_GPIO        = 0x2 << (2*3),
        DW37XX_GPIO_PIN3_SPI2_MOSI   = 0x0 << (3*3),
        DW37XX_GPIO_PIN3_TXLED       = 0x1 << (3*3),
        DW37XX_GPIO_PIN3_GPIO        = 0x2 << (3*3),
        DW37XX_GPIO_PIN4_GPIO        = 0x0 << (4*3),
        DW37XX_GPIO_PIN4_COEX_IN     = 0x1 << (4*3),
        DW37XX_GPIO_PIN4_PDOA_SW_TX  = 0x2 << (4*3),
        DW37XX_GPIO_PIN5_GPIO        = 0x0 << (5*3),
        DW37XX_GPIO_PIN5_COEX_OUT    = 0x1 << (5*3),
        DW37XX_GPIO_PIN5_PDOA_SW_RX  = 0x2 << (5*3),
        DW37XX_GPIO_PIN6_GPIO        = 0x0 << (6*3),
        DW37XX_GPIO_PIN6_EXT_SW_RX   = 0x1 << (6*3),
        DW37XX_GPIO_PIN6_PDOA_SW_RF1 = 0x2 << (6*3),
        DW37XX_GPIO_PIN7_SYNC        = 0x0 << (7*3),
        DW37XX_GPIO_PIN7_GPIO        = 0x1 << (7*3),
        DW37XX_GPIO_PIN7_PDOA_SW_RF2 = 0x2 << (7*3),
        DW37XX_GPIO_PIN8_IRQ         = 0x0 << (8*3),
        DW37XX_GPIO_PIN8_GPIO        = 0x1 << (8*3)
    } dwt_gpio_pin_e;

    /* Mask that can be used in e.g ull_setgpiomode to use the GPIO mode of all GPIO pins on a DW3000 IC */
    #define DW3000_ENABLE_ALL_GPIOS_MASK  0x1200000
    /* Mask that can be used in e.g ull_setgpiomode to use the GPIO mode of all GPIO pins on a DW3700 or DW3720 IC*/
    #define DW37XX_ENABLE_ALL_GPIOS_MASK  0x1200492

    typedef enum
    {
        GPIO0_FUNC_MASK = 0x0000007,
        GPIO1_FUNC_MASK = 0x0000038,
        GPIO2_FUNC_MASK = 0x00001c0,
        GPIO3_FUNC_MASK = 0x0000e00,
        GPIO4_FUNC_MASK = 0x0007000,
        GPIO5_FUNC_MASK = 0x0038000,
        GPIO6_FUNC_MASK = 0x01c0000,
        GPIO7_FUNC_MASK = 0x0e00000,
        GPIO8_FUNC_MASK = 0x7000000,
    } dwt_gpio_func_mask_e;

    /* Mask values for GPIO pins on DW3xxx */
    typedef enum
    {
        GPIO0_BIT_MASK = 0x001,
        GPIO1_BIT_MASK = 0x002,
        GPIO2_BIT_MASK = 0x004,
        GPIO3_BIT_MASK = 0x008,
        GPIO4_BIT_MASK = 0x010,
        GPIO5_BIT_MASK = 0x020,
        GPIO6_BIT_MASK = 0x040,
        GPIO7_BIT_MASK = 0x080,
        GPIO8_BIT_MASK = 0x100,
    } dwt_gpio_mask_e;

    #define GPIO_BIT_MASK_ALL 0x1FF

    /* Deprecated. Use dwt_gpio_mask_e instead */
    typedef enum
    {
        GPIO_0 = GPIO0_BIT_MASK,
        GPIO_1 = GPIO1_BIT_MASK,
        GPIO_2 = GPIO2_BIT_MASK,
        GPIO_3 = GPIO3_BIT_MASK,
        GPIO_4 = GPIO4_BIT_MASK,
        GPIO_5 = GPIO5_BIT_MASK,
        GPIO_6 = GPIO6_BIT_MASK,
        GPIO_7 = GPIO7_BIT_MASK,
        GPIO_8 = GPIO8_BIT_MASK,
        GPIO_ALL = GPIO_BIT_MASK_ALL,
    } gpio_num_e;

    #define GPIO_MFIO_MODE_MASK 0x7U

    /* Enable/disable WiFi co-existence */
    typedef enum
    {
        DWT_EN_WIFI_COEX = 0, /* Configure GPIO for WiFi co-ex - GPIO high*/
        DWT_DIS_WIFI_COEX     /* Configure GPIO for WiFi co-ex - GPIO low */
    } dwt_wifi_coex_e;

    // Defined constants for "lna_pa" bit field parameter passed to dwt_setlnapamode() function
    typedef enum
    {
        DWT_LNA_PA_DISABLE = 0x00,
        DWT_LNA_ENABLE = 0x01,
        DWT_PA_ENABLE = 0x02,
        DWT_TXRX_EN = 0x04,
    } dwt_setlnapmodes_e;

    typedef enum
    {
        DWT_TIMER0 = 0,
        DWT_TIMER1
    } dwt_timers_e;

    typedef enum
    {
        DWT_TIM_SINGLE = 0,
        DWT_TIM_REPEAT
    } dwt_timer_mode_e;

    typedef enum
    {
        DWT_XTAL = 0,       // 38.4 MHz
        DWT_XTAL_DIV2 = 1,  // 19.2 MHz
        DWT_XTAL_DIV4 = 2,  // 9.6 MHz
        DWT_XTAL_DIV8 = 3,  // 4.8 MHz
        DWT_XTAL_DIV16 = 4, // 2.4 MHz
        DWT_XTAL_DIV32 = 5, // 1.2 MHz
        DWT_XTAL_DIV64 = 6, // 0.6 MHz
        DWT_XTAL_DIV128 = 7 // 0.3 MHz
    } dwt_timer_period_e;

    typedef struct
    {
        dwt_timers_e timer;           // Select the timer to use.
        dwt_timer_period_e timer_div; // Select the timer frequency (divider).
        dwt_timer_mode_e timer_mode;  // Select the timer mode.
        uint8_t timer_gpio_stop;      // Set to '1' to halt GPIO on interrupt.
        uint8_t timer_coexout;        // Configure GPIO for WiFi co-ex.
    } dwt_timer_cfg_t;

    // Reset options
    typedef enum
    {
        DWT_RESET_ALL = 0x00,
        DWT_RESET_CTRX = 0x0F,
        DWT_RESET_RX = 0xEF,
        DWT_RESET_CLEAR = 0xFF,
    } dwt_reset_options_e;

    // frame filtering configuration options
    typedef enum
    {
        DWT_FF_ENABLE_802_15_4 = 0x2, // enable FF for 802.15.4
        DWT_FF_DISABLE = 0x0,         // disable FF
        DWT_FF_BEACON_EN = 0x001,     // beacon frames allowed
        DWT_FF_DATA_EN = 0x002,       // data frames allowed
        DWT_FF_ACK_EN = 0x004,        // ack frames allowed
        DWT_FF_MAC_EN = 0x008,        // mac control frames allowed
        DWT_FF_RSVD_EN = 0x010,       // reserved frame types allowed
        DWT_FF_MULTI_EN = 0x020,      // multipurpose frames allowed
        DWT_FF_FRAG_EN = 0x040,       // fragmented frame types allowed
        DWT_FF_EXTEND_EN = 0x080,     // extended frame types allowed
        DWT_FF_COORD_EN = 0x100,      // behave as coordinator (can receive frames with no dest address (PAN ID has to match))
        DWT_FF_IMPBRCAST_EN = 0x200,  // allow MAC implicit broadcast
        DWT_FF_MAC_LE0_EN = 0x408,    // mac frames allowed if address in LE0_PEND matches source address
        DWT_FF_MAC_LE1_EN = 0x808,    // mac frames allowed if address in LE1_PEND matches source address
        DWT_FF_MAC_LE2_EN = 0x1008,   // mac frames allowed if address in LE2_PEND matches source address
        DWT_FF_MAC_LE3_EN = 0x2008,   // mac frames allowed if address in LE3_PEND matches source address
    } dwt_ff_conf_options_e;

    // DW3000 SLEEP and WAKEUP configuration parameters
    typedef enum
    {
        DWT_PGFCAL = 0x0800,
        DWT_GOTORX = 0x0200,
        DWT_GOTOIDLE = 0x0100,
        DWT_SEL_OPS3 = 0x00C0,
        DWT_SEL_OPS2 = 0x0080, // Short OPS table
        DWT_SEL_OPS1 = 0x0040, // SCP
        DWT_SEL_OPS0 = 0x0000, // Long OPS table
        DWT_ALT_OPS = 0x0020,
        DWT_LOADLDO = 0x0010,
        DWT_LOADDGC = 0x0008,
        DWT_LOADBIAS = 0x0004,
        DWT_RUNSAR = 0x0002,
        DWT_CONFIG = 0x0001, // download the AON array into the HIF (configuration download)
    } dwt_on_wake_param_e;

    typedef enum
    {
        DWT_PRES_SLEEP = 0x20, // allows for SLEEP_EN bit to be "preserved", although it will self - clear on wake up
        DWT_WAKE_WUP = 0x10,   // wake up on WAKEUP PIN
        DWT_WAKE_CSN = 0x8,    // wake up on chip select
        DWT_BROUT_EN = 0x4,    // enable brownout detector during sleep/deep sleep
        DWT_SLEEP = 0x2,       // enable sleep (if this bit is clear the device will enter deep sleep)
        DWT_SLP_EN = 0x1,      // enable sleep/deep sleep functionality
    } dwt_wkup_param_e;

    // Events that can be used to automatically transition to SLEEP or DEEPSLEEP
    typedef enum
    {
        DWT_TX_COMPLETE = 0x01,
        DWT_RX_COMPLETE = 0x02
    } dwt_sleep_after_param_e;

    // DW3000 IDLE/INIT mode definitions
    typedef enum
    {
        DWT_DW_INIT = 0x0,
        DWT_DW_IDLE = 0x1,
        DWT_DW_IDLE_RC = 0x2,
    } dwt_idle_init_modes_e;

    typedef enum
    {
        DWT_READ_OTP_PID = 0x10, // read part ID from OTP
        DWT_READ_OTP_LID = 0x20, // read lot ID from OTP
        DWT_READ_OTP_BAT = 0x40, // read ref voltage from OTP
        DWT_READ_OTP_TMP = 0x80, // read ref temperature from OTP
    } dwt_read_otp_modes_e;

    // DW3xxx RF Port configuration set selection
    typedef enum
    {
        DWT_RF_PORT_DEF = 0, // Configure RF port to default value 0 (device will internally switch depending on the PDoA mode)
        DWT_RF_PORT_1 = 1,   // select RF port 1
        DWT_RF_PORT_2 = 2,   // select RF port 2
    } dwt_rf_port_selection_e;

    // DW3xxx enabling manual control of antenna selection
    typedef enum
    {
        DWT_RF_PORT_MAN_CTRL_EN = 1,    // enabling manual control of antenna selection bits
        DWT_RF_PORT_MAN_CTRL_DISEN = 0, // disabling manual control of antenna selection bits
    } dwt_rf_port_ctrl_e;

// Conversion factor to convert clock offset from PPM to ratio
#define CLOCK_OFFSET_PPM_TO_RATIO (1.0 / (1 << 26))

    typedef enum
    {
        AON_SLPCNT_LO = (0x102),       // address of SLEEP counter bits [19:12] in AON memory
        AON_SLPCNT_HI = (0x103),       // address of SLEEP counter bits [27:20] in AON memory
        AON_SLPCNT_CAL_CTRL = (0x104), // address of SLEEP counter cal control
        AON_LPOSC_TRIM = (0x10B),      // address of LP OSC trim code
        AON_SLPCNT_CAL_LO = (0x10E),   // address of SLEEP counter cal value low byte
        AON_SLPCNT_CAL_HI = (0x10F),   // address of SLEEP counter cal value high byte
    } dwt_aon_sleep_conf_e;

    typedef enum
    {
        DW_CIA_DIAG_LOG_MAX = 0x8, // CIA to copy to swinging set a maximum set of diagnostic registers in Double Buffer mode
        DW_CIA_DIAG_LOG_MID = 0x4, // CIA to copy to swinging set a medium set of diagnostic registers in Double Buffer mode
        DW_CIA_DIAG_LOG_MIN = 0x2, // CIA to copy to swinging set a minimal set of diagnostic registers in Double Buffer mode
        DW_CIA_DIAG_LOG_ALL = 0x1, // CIA to log all diagnostic registers
        DW_CIA_DIAG_LOG_OFF = 0x0, // CIA to log reduced set of diagnostic registers
    } dwt_cia_diag_log_conf_e;

    // Call-back data RX frames flags
    typedef enum
    {
        DWT_CB_DATA_RX_FLAG_RNG = 0x01,  // Ranging bit
        DWT_CB_DATA_RX_FLAG_ND = 0x02,   // No data mode
        DWT_CB_DATA_RX_FLAG_CIA = 0x04,  // CIA done
        DWT_CB_DATA_RX_FLAG_CER = 0x08,  // CIA error
        DWT_CB_DATA_RX_FLAG_CPER = 0x10, // STS error
    } dwt_cb_data_rx_flags_e;

// Multiplication factors to convert carrier integrator value to a frequency offset in Hertz
#define FREQ_OFFSET_MULTIPLIER (998.4e6 / 2.0 / 1024.0 / 131072.0)

// Multiplication factors to convert frequency offset in Hertz to PPM crystal offset
// NB: also changes sign so a positive value means the local RX clock is running slower than the remote TX device.
#define HERTZ_TO_PPM_MULTIPLIER_CHAN_5 (-1.0e6 / 6489.6e6)
#define HERTZ_TO_PPM_MULTIPLIER_CHAN_9 (-1.0e6 / 7987.2e6)

#define DWT_VALID_TDOA_LIMIT (100) // If the abs(TDoA) value is larger than this constant, this means the PDoA will not be valid, and should not be used.

    // Low Energy (LE) device addresses
    typedef enum
    {
        LE0 = 0, // LE0_PEND address
        LE1 = 1, // LE1_PEND address
        LE2 = 2, // LE2_PEND address
        LE3 = 3, // LE3_PEND address
    } dwt_le_addresses_e;

    // TX/RX call-back data
    typedef struct
    {
        uint32_t status;     // initial value of register as ISR is entered
        uint16_t status_hi;  // initial value of register as ISR is entered, if relevant for that event type
        uint16_t datalength; // length of frame
        uint8_t  rx_flags;   // RX frame flags, see above
        uint8_t  dss_stat;   // Dual SPI status reg 11:38, 2 LSbits relevant : bit0 (DWT_CB_DSS_SPI1_AVAIL) and bit1 (DWT_CB_DSS_SPI2_AVAIL)
        struct dwchip_s *dw;
    } dwt_cb_data_t;

    // Call-back type for SPI read error event (if the DW3000 generated CRC does not match the one calculated by the dwt_generatecrc8 function)
    typedef void (*dwt_spierrcb_t)(void);

    // Call-back type for all interrupt events
    typedef void (*dwt_cb_t)(const dwt_cb_data_t *);

#define SQRT_FACTOR       181 /*Factor of sqrt(2) for calculation*/
#define STS_LEN_SUPPORTED 7 /*The supported STS length options*/
#define SQRT_SHIFT_VAL    7
#define SHIFT_VALUE       11
#define MOD_VALUE         2048
#define HALF_MOD          (MOD_VALUE >> 1)

    /*This Enum holds INT working options.*/
    typedef enum
    {
        DWT_DISABLE_INT = 0,          /* Disable these INT    */
        DWT_ENABLE_INT,               /* Enable these INT     */
        DWT_ENABLE_INT_ONLY,          /* Enable only these INT*/
        DWT_ENABLE_INT_DUAL_SPI,      /* Enable these INT, dual SPI mode */
        DWT_ENABLE_INT_ONLY_DUAL_SPI, /* Enable only these INT, dual SPI mode */

    } dwt_INT_options_e;

    /*This Enum holds the index for factor calculation.*/
    typedef enum
    {
        DWT_STS_LEN_32 = 0,
        DWT_STS_LEN_64 = 1,
        DWT_STS_LEN_128 = 2,
        DWT_STS_LEN_256 = 3,
        DWT_STS_LEN_512 = 4,
        DWT_STS_LEN_1024 = 5,
        DWT_STS_LEN_2048 = 6
    } dwt_sts_lengths_e;

    /* Returns the value to set in CP_CFG0_ID for STS length. The x is the enum value from dwt_sts_lengths_e */
#define GET_STS_REG_SET_VALUE(x) ((uint16_t)1 << ((x) + 2))

    /* Enum used for selecting channel for DGC on-wake kick. */
    typedef enum
    {
        DWT_DGC_SEL_CH5 = 0,
        DWT_DGC_SEL_CH9
    } dwt_dgc_chan_sel;

    /* Enum used for selecting location to load DGC data from */
    typedef enum
    {
        DWT_DGC_LOAD_FROM_SW = 0,
        DWT_DGC_LOAD_FROM_OTP
    } dwt_dgc_load_location;

    /*! ------------------------------------------------------------------------------------------------------------------
     * Structure typedef: dwt_config_t
     *
     * Structure for setting device configuration via dwt_configure() function
     *
     */
    typedef struct
    {
        uint8_t chan;                 //!< Channel number (5 or 9)
        dwt_tx_plen_e txPreambLength; //!< DWT_PLEN_64..DWT_PLEN_4096
        dwt_pac_size_e rxPAC;         //!< Acquisition Chunk Size (Relates to RX preamble length)
        uint8_t txCode;               //!< TX preamble code (the code configures the PRF, e.g. 9 -> PRF of 64 MHz)
        uint8_t rxCode;               //!< RX preamble code (the code configures the PRF, e.g. 9 -> PRF of 64 MHz)
        dwt_sfd_type_e sfdType;       //!< SFD type (0 for short IEEE 8-bit standard, 1 for DW 8-bit, 2 for DW 16-bit, 3 for 4z BPRF)
        dwt_uwb_bit_rate_e dataRate;  //!< Data rate {DWT_BR_850K or DWT_BR_6M8}
        dwt_phr_mode_e phrMode;       //!< PHR mode {0x0 - standard DWT_PHRMODE_STD, 0x3 - extended frames DWT_PHRMODE_EXT}
        dwt_phr_rate_e phrRate;       //!< PHR rate {0x0 - standard DWT_PHRRATE_STD, 0x1 - at datarate DWT_PHRRATE_DTA}
        uint16_t sfdTO;               //!< SFD timeout value (in symbols)
        dwt_sts_mode_e stsMode;       //!< STS mode (no STS, STS before PHR or STS after data)
        dwt_sts_lengths_e stsLength;  //!< STS length (the allowed values are listed in dwt_sts_lengths_e
        dwt_pdoa_mode_e pdoaMode;     //!< PDOA mode
#ifndef WIN32
    } __attribute__((packed)) dwt_config_t;
#else
} dwt_config_t;
#endif // WIN32

    typedef struct
    {
        uint8_t PGdly;
        // TX POWER
        // 31:24     TX_CP_PWR
        // 23:16     TX_SHR_PWR
        // 15:8      TX_PHR_PWR
        // 7:0       TX_DATA_PWR
        uint32_t power;
        uint16_t PGcount;
#ifndef WIN32
    } __attribute__((packed)) dwt_txconfig_t;
#else
} dwt_txconfig_t;
#endif // WIN32

    typedef struct
    {
        uint8_t ipatovRxTime[5]; // RX timestamp from Ipatov sequence
        uint8_t ipatovRxStatus;  // RX status info for Ipatov sequence
        uint16_t ipatovPOA;      // POA of Ipatov

        uint8_t stsRxTime[5];  // RX timestamp from STS
        uint16_t stsRxStatus;  // RX status info for STS
        uint16_t stsPOA;       // POA of STS block 1
        uint8_t sts2RxTime[5]; // RX timestamp from STS
        uint16_t sts2RxStatus; // RX status info for STS
        uint16_t sts2POA;      // POA of STS block 2

        uint8_t tdoa[6]; // TDOA from two STS RX timestamps
        int16_t pdoa;    // PDOA from two STS POAs signed int [1:-11] in radians

        int16_t xtalOffset; // estimated xtal offset of remote device
        uint32_t ciaDiag1;  // Diagnostics common to both sequences

        uint32_t ipatovPeak;       // index and amplitude of peak sample in Ipatov sequence CIR
        uint32_t ipatovPower;      // channel area allows estimation of channel power for the Ipatov sequence
        uint32_t ipatovF1;         // F1 for Ipatov sequence
        uint32_t ipatovF2;         // F2 for Ipatov sequence
        uint32_t ipatovF3;         // F3 for Ipatov sequence
        uint16_t ipatovFpIndex;    // First path index for Ipatov sequence
        uint16_t ipatovAccumCount; // Number accumulated symbols for Ipatov sequence

        uint32_t stsPeak;       // index and amplitude of peak sample in STS CIR
        uint32_t stsPower;      // channel area allows estimation of channel power for the STS
        uint32_t stsF1;         // F1 for STS
        uint32_t stsF2;         // F2 for STS
        uint32_t stsF3;         // F3 for STS
        uint16_t stsFpIndex;    // First path index for STS
        uint16_t stsAccumCount; // Number accumulated symbols for STS

        uint32_t sts2Peak;       // index and amplitude of peak sample in STS CIR
        uint32_t sts2Power;      // channel area allows estimation of channel power for the STS
        uint32_t sts2F1;         // F1 for STS
        uint32_t sts2F2;         // F2 for STS
        uint32_t sts2F3;         // F3 for STS
        uint16_t sts2FpIndex;    // First path index for STS
        uint16_t sts2AccumCount; // Number accumulated symbols for STS

#ifndef WIN32
    } __attribute__((packed)) dwt_rxdiag_t;
#else
} dwt_rxdiag_t;
#endif // WIN32

    typedef struct
    {
        // all of the below are mapped to a register in DW3000
        uint16_t PHE;   // 12-bit number of received header error events
        uint16_t RSL;   // 12-bit number of received frame sync loss event events
        uint16_t CRCG;  // 12-bit number of good CRC received frame events
        uint16_t CRCB;  // 12-bit number of bad CRC (CRC error) received frame events
        uint8_t ARFE;   // 8-bit number of address filter error events
        uint8_t OVER;   // 8-bit number of receive buffer overrun events (used in double buffer mode)
        uint16_t SFDTO; // 12-bit number of SFD timeout events
        uint16_t PTO;   // 12-bit number of Preamble timeout events
        uint8_t RTO;    // 8-bit number of RX frame wait timeout events
        uint16_t TXF;   // 12-bit number of transmitted frame events
        uint8_t HPW;    // 8-bit half period warning events (when delayed RX/TX enable is used)
        uint8_t CRCE;   // 8-bit SPI CRC error events
        uint16_t PREJ;  // 12-bit number of Preamble rejection events
        uint16_t SFDD;  // 12-bit SFD detection events ... only DW3720
        uint8_t STSE;   // 8-bit STS error/warning events
#ifndef WIN32
    } __attribute__((packed)) dwt_deviceentcnts_t;
#else
} dwt_deviceentcnts_t;
#endif // WIN32

    /* BEGIN: CHIP_SPECIFIC_SECTION DW37xx */

    /* Hosts for the SPI bus */
    typedef enum
    {
        DWT_HOST_SPI1 = 0, /* Host using SPI1 interface */
        DWT_HOST_SPI2      /* Host using SPI2 interface */
    } dwt_spi_host_e;

#define SPI2MAVAIL_BIT_MASK 0x2 // bit 1 of 1a:01
#define SPI1MAVAIL_BIT_MASK 0x4 // bit 2 of 1a:01

    typedef enum
    {
        HOST_EN_SLEEP = 0x00, /* Host enable Sleep/Deepsleep */
        HOST_DIS_SLEEP = 0x60 /* Host disable Sleep/Deepsleep */
    } dwt_host_sleep_en_e;
    /* END: CHIP_SPECIFIC_SECTION DW37xx */

    /********************************************************************************************************************/
    /*                                                AES BLOCK                                                         */
    /********************************************************************************************************************/

    // enums below are defined in such a way as to allow a simple write to DW3000 AES configuration registers

    /* For MIC size definition */
    typedef enum
    {
        MIC_0 = 0,
        MIC_4,
        MIC_6,
        MIC_8,
        MIC_10,
        MIC_12,
        MIC_14,
        MIC_16
    } dwt_mic_size_e;

    /* Key size definition */
    typedef enum
    {
        AES_KEY_128bit = 0,
        AES_KEY_192bit = 1,
        AES_KEY_256bit = 2
    } dwt_aes_key_size_e;

    /* Load key from RAM selection */
    typedef enum
    {
        AES_KEY_No_Load = 0,
        AES_KEY_Load
    } dwt_aes_key_load_e;

    /* Key source - RAM or registers */
    typedef enum
    {
        AES_KEY_Src_Register = 0, /* Use AES KEY from registers */
        AES_KEY_Src_RAMorOTP      /* Use AES KEY from RAM or OTP (depending if AES_key_OTP set),
                                         AES_KEY_Load needs to be set as well */
    } dwt_aes_key_src_e;

    /* Operation selection */
    typedef enum
    {
        AES_Encrypt = 0,
        AES_Decrypt
    } dwt_aes_mode_e;

    /* This defines the source port for encrypted/unencrypted data */
    typedef enum
    {
        AES_Src_Scratch = 0,
        AES_Src_Rx_buf_0,
        AES_Src_Rx_buf_1,
        AES_Src_Tx_buf
    } dwt_aes_src_port_e;

    /* This defines the dest port for encrypted/unencrypted data */
    typedef enum
    {
        AES_Dst_Scratch = 0,
        AES_Dst_Rx_buf_0,
        AES_Dst_Rx_buf_1,
        AES_Dst_Tx_buf,
        AES_Dst_STS_key
    } dwt_aes_dst_port_e;

    /* storage for 128/192/256-bit key */
    typedef struct
    {
        uint32_t key0;
        uint32_t key1;
        uint32_t key2;
        uint32_t key3;
        uint32_t key4;
        uint32_t key5;
        uint32_t key6;
        uint32_t key7;
    } dwt_aes_key_t;

    typedef enum
    {
        AES_core_type_GCM = 0, /* Core type GCM */
        AES_core_type_CCM      /* Core type CCM */
    } dwt_aes_core_type_e;

    typedef enum
    {
        AES_key_RAM = 0, /* Use the AES KEY from RAM */
        AES_key_OTP      /* Use the AES KEY from OTP, key_load needs to match -> needs to be set to AES_KEY_Src_Ram */
    } dwt_aes_key_otp_type_e;

    typedef enum
    {
        AES_key_otp_sel_1st_128 = 0, /* Key first 128bits */
        AES_key_otp_sel_2nd_128      /* Key second 128bits */
    } dwt_aes_otp_sel_key_block_e;

    typedef struct
    {
        dwt_aes_otp_sel_key_block_e aes_otp_sel_key_block; //!< Select OTP key, first 128 or 2nd 128 bits
        dwt_aes_key_otp_type_e aes_key_otp_type;           //!< Using KEY from OTP or RAM, if this is set to AES_key_OTP, KEY from OTP is used
        dwt_aes_core_type_e aes_core_type;                 //!< Core type GCM or CCM*
        dwt_mic_size_e mic;                                //!< Message integrity code size
        dwt_aes_key_src_e key_src;                         //!< Location of the key: either as programmed in registers(128 bit) or in the RAM or in the OTP
        dwt_aes_key_load_e key_load;                       //!< Loads key from RAM or uses KEY from the registers
        uint8_t key_addr;                                  //!< Address offset of AES key when using AES key in RAM
        dwt_aes_key_size_e key_size;                       //!< AES key length configuration corresponding to AES_KEY_128/192/256bit
        dwt_aes_mode_e mode;                               //!< Operation type encrypt/decrypt
    } dwt_aes_config_t;

    typedef struct
    {
        uint8_t *nonce;              //!< Pointer to the nonce
        uint8_t *header;             //!< Pointer to header (this is not encrypted/decrypted)
        uint8_t *payload;            //!< Pointer to payload (this is encrypted/decrypted)
        uint8_t header_len;          //!< Header size
        uint16_t payload_len;        //!< Payload size
        dwt_aes_src_port_e src_port; //!< Source port
        dwt_aes_dst_port_e dst_port; //!< Dest port
        dwt_aes_mode_e mode;         //!< Encryption or decryption
        uint8_t mic_size;            //!< tag_size;
    } dwt_aes_job_t;

    /* storage for 128-bit STS CP key */
    typedef struct
    {
        uint32_t key0;
        uint32_t key1;
        uint32_t key2;
        uint32_t key3;
#ifndef WIN32
    } __attribute__((packed)) dwt_sts_cp_key_t;
#else
} dwt_sts_cp_key_t;
#endif // WIN32

    /* storage for 128-bit STS CP IV (nonce) */
    typedef struct
    {
        uint32_t iv0;
        uint32_t iv1;
        uint32_t iv2;
        uint32_t iv3;
#ifndef WIN32
    } __attribute__((packed)) dwt_sts_cp_iv_t;
#else
} dwt_sts_cp_iv_t;
#endif // WIN32

#define ERROR_DATA_SIZE      (-1)
#define ERROR_WRONG_MODE     (-2)
#define ERROR_WRONG_MIC_SIZE (-3)
#define ERROR_PAYLOAD_SIZE   (-4)
#define MIC_ERROR            (0xff)
#define STS_LEN_128BIT       (16)

    typedef enum
    {
        DBL_BUF_STATE_EN = 0, /*Double buffer enabled*/
        DBL_BUF_STATE_DIS     /*Double buffer disabled*/
    } dwt_dbl_buff_state_e;

    typedef enum
    {
        DBL_BUF_MODE_AUTO = 0, /*Automatic*/
        DBL_BUF_MODE_MAN       /*Manual*/
    } dwt_dbl_buff_mode_e;

    typedef enum
    {
        DBL_BUFF_OFF = 0x0,
        DBL_BUFF_ACCESS_BUFFER_0 = 0x1,
        DBL_BUFF_ACCESS_BUFFER_1 = 0x3,
    } dwt_dbl_buff_conf_e;

    // DW3000 NLOS DIAGNOSTIC TYPE
    typedef enum
    {
        IPATOV = 0x0, // Select Ipatov Diagnostic
        STS1 = 0x1,   // Select STS1 Diagnostic
        STS2 = 0x2,   // Select STS2 Diagnostic
    } dwt_diag_type_e;

    //NLOS structs
    typedef struct
    {
        uint32_t accumCount;          // the number of preamble symbols accumulated, or accumulated STS length.
        uint32_t F1;                  // the First Path Amplitude (point 1) magnitude value.
        uint32_t F2;                  // the First Path Amplitude (point 2) magnitude value.
        uint32_t F3;                  // the First Path Amplitude (point 3) magnitude value.
        uint32_t cir_power;           // the Channel Impulse Response Power value.
        uint8_t D;                    // the DGC_DECISION, treated as an unsigned integer in range 0 to 7.
        dwt_diag_type_e diag_type;
        uint8_t  result;
    } dwt_nlos_alldiag_t;

    typedef struct
    {
        uint32_t index_fp_u32;  // the First Path Index.
        uint32_t index_pp_u32;  // the Peak Path Index
    } dwt_nlos_ipdiag_t;

/*
 * The default XTAL TRIM value for load capacitors of 2pF.
 * During the initialization the XTAL TRIM value can be read from the OTP and in case it is not present, the default would be used instead
 * */
#define DEFAULT_XTAL_TRIM 0x2E

/*
 * Max allowed value for XTAL trim
 * */
#define XTAL_TRIM_BIT_MASK 0x3F

/*
 * Antenna configuration
 */
#define ANT_GPIO6_POS_MASK   0x1U
#define ANT_GPIO6_POS_OFFSET (0U)
#define ANT_GPIO6_VAL_MASK   0x2U
#define ANT_GPIO6_VAL_OFFSET (1U)
#define ANT_GPIO7_POS_MASK   0x4U
#define ANT_GPIO7_POS_OFFSET (2U)
#define ANT_GPIO7_VAL_MASK   0x8U
#define ANT_GPIO7_VAL_OFFSET (3U)


 /*
  * DW37xx - enable/disable equaliser in the CIA
  */
    typedef enum
    {
        DWT_EQ_DISABLED = 0x0,
        DWT_EQ_ENABLED = 0x1,
    } dwt_eq_config_e;

/*
 * Debug register name/value structure
 */
    typedef struct {
        char* name;
        uint32_t address;
    } register_name_add_t;


    /*! ------------------------------------------------------------------------------------------------------------------
        * @brief The dwt_probe_s structure is a structure assembling all the external structures and function
        * that must be defined externally
        * NB: In porting this to a particular microprocessor, the implementer needs to define the low
        * level abstract functions matching the selected hardware.
    */
    struct dwt_probe_s
    {
        /*! ------------------------------------------------------------------------------------------------------------------
            * @brief dw pointer to an externally defined dwchip_s.
            * if set to NULL then an internal structure will be used
            * the typical use case is to support multiple DW chip connection on the same board
            * NB: see dwchip_s structure definition for details in deca_interface.h
        */
        void *dw;
        /*! ------------------------------------------------------------------------------------------------------------------
            * @brief dw pointer to an externally defined dwt_spi_s structure
            * NB: see dwt_spi_s structure definition for details in deca_interface.h
        */
        void *spi;
        /*! ------------------------------------------------------------------------------------------------------------------
        * @brief  This function wakeup device by an IO pin. DW3000 SPI_CS or WAKEUP pins can be used for this.
        *         wakeup_device_with_io() which is external to this file and is platform dependant and it should be modified to
        *         toggle the correct pin depending on the HW/MCU connections with DW3000.
        */
        void(*wakeup_device_with_io)(void);
    };

    /********************************************************************************************************************/
    /*                                                     API LIST                                                     */
    /********************************************************************************************************************/

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function selects the correct DecaDriver from the list
     *
     * input parameters
     * @param probe_interf pointer to a dwt_probe_s structure. See above description
     *
     * output parameters
     *
     * returns ret - DWT_ERROR  if no driver found or DWT_SUCCESS if driver is found.
     */
    int dwt_probe(struct dwt_probe_s *probe_interf);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function returns the version of the API
     *
     * input parameters
     *
     * output parameters
     *
     * returns version (DW3xxx_DRIVER_VERSION)
     */
    int32_t dwt_apiversion(void);
    
    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function will update dw pointer used by interrupt
     *
     * input parameters
     * @param new_dw - dw instatiated by MCPS layer
     *
     * return parameters
     * old_dw pointer. This pointer can be restored when deleting MCPS instance
     *
     */
    struct dwchip_s* dwt_update_dw(struct dwchip_s* new_dw);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function returns the driver version of the API
     *
     * input parameters
     *
     * output parameters
     *
     * returns version string
     */
    char *dwt_version_string(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function sets the local data structure pointer to point to the element in the local array as given by the index.
     *
     * input parameters
     * @param index    - selects the array element to point to. Must be within the array bounds, i.e. < DWT_NUM_DW_DEV
     *
     * output parameters
     *
     * returns DWT_SUCCESS for success, or DWT_ERROR for error
     */
    int dwt_setlocaldataptr(unsigned int index);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to return the read V measured @ 3.0 V value recorded in OTP address 0x8 (VBAT_ADDRESS)
     *
     * NOTE: dwt_initialise() must be called prior to this function so that it can return a relevant value.
     *
     * input parameters
     *
     * output parameters
     *
     * returns the 8 bit V bat value as programmed in the factory
     */
    uint8_t dwt_geticrefvolt(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to return the read T measured @ 22 C value recorded in OTP address 0x9 (VTEMP_ADDRESS)
     *
     * NOTE: dwt_initialise() must be called prior to this function so that it can return a relevant value.
     *
     * input parameters
     *
     * output parameters
     *
     * returns the 8 bit V temp value as programmed in the factory
     */
    uint8_t dwt_geticreftemp(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to return the read part ID of the device
     *
     * NOTE: dwt_initialise() must be called prior to this function so that it can return a relevant value.
     *
     * input parameters
     *
     * output parameters
     *
     * returns the 32 bit part ID value as programmed in the factory
     */
    uint32_t dwt_getpartid(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to return the read lot ID of the device
     *
     * NOTE: dwt_initialise() must be called prior to this function so that it can return a relevant value.
     *
     * input parameters
     *
     * output parameters
     *
     * returns the 32 bit lot ID value as programmed in the factory
     */
    uint32_t dwt_getlotid(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to return the read device type and revision information of the DW UWB chip
     *
     * input parameters
     *
     * output parameters
     *
     * returns the silicon DevID
     */
    uint32_t dwt_readdevid(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to return the read OTP revision
     *
     * NOTE: dwt_initialise() must be called prior to this function so that it can return a relevant value.
     *
     * input parameters
     *
     * output parameters
     *
     * returns the read OTP revision value
     */
    uint8_t dwt_otprevision(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function enables/disables the fine grain TX sequencing (enabled by default).
     *
     * input parameters
     * @param enable - 1 to enable fine grain TX sequencing, 0 to disable it.
     *
     * output parameters none
     *
     * no return value
     */
    void dwt_setfinegraintxseq(int enable);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to enable GPIO for external LNA or PA functionality - HW dependent, consult the DW3000 User Manual.
     *        This can also be used for debug as enabling TX and RX GPIOs is quite handy to monitor DW3000's activity.
     *
     * NOTE: Enabling PA functionality requires that fine grain TX sequencing is deactivated. This can be done using
     *       dwt_setfinegraintxseq().
     *
     * input parameters
     * @param lna_pa - bit field: bit 0 if set will enable LNA functionality,
     *                          : bit 1 if set will enable PA functionality,
     *                          : to disable LNA/PA set the bits to 0 (
     * output parameters
     *
     * no return value
     */
    void dwt_setlnapamode(int lna_pa);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to configure GPIO function
     *
     *
     * input parameters
     * @param gpio_mask - the mask of the GPIOs to change the mode of. Typically built from dwt_gpio_mask_e values.
     * @param gpio_modes - the GPIO modes to set. Typically built from dwt_gpio_pin_e values.
     *
     * output parameters
     *
     * no return value
     */
    void dwt_setgpiomode(uint32_t gpio_mask, uint32_t gpio_modes);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to configure the GPIOs as inputs or outputs, default is input == 1
     *
     * input parameters
     * @param in_out - if corresponding GPIO bit is set to 1 then it is input, otherwise it is output
     *               - GPIO 0 = bit 0, GPIO 1 = bit 1 etc...
     *
     * output parameters
     *
     * no return value
     */
    void dwt_setgpiodir(uint16_t in_out);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to set output value on GPIOs that have been configured for output via dwt_setgpiodir() API
     *
     * input parameters
     * @param gpio - should be one or multiple of dwt_gpio_mask_e values
     * @param value - Logic value for GPIO or GPIOs if multiple set at same time.
     *
     * output parameters
     *
     * no return value
     */
    void dwt_setgpiovalue(uint16_t gpio, int value);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to read the raw value of the GPIO pins.
     *        It is presumed that functions such as dwt_setgpiomode(), dwt_setgpiovalue() and dwt_setgpiodir() are called before this function.
     *
     * input parameters
     *
     * returns a uint16_t value that holds the value read on the GPIO pins.
     */
    uint16_t dwt_readgpiovalue(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function initialises the DW3xxx transceiver:
     * It performs the initially required device configurations and initializes
     * a static data belonging to the low-level driver.
     *
     * NOTES:
     * 1.this function needs to be run before dwt_configuresleep, also the SPI frequency has to be < 7MHz
     * 2.it also reads and applies LDO and BIAS tune and crystal trim values from OTP memory
     * 3.it is assumed this function is called after a reset or on power up of the DW3xxx transceiver
     *
     * input parameters
     * @param mode - mask which defines which OTP values to read.
     *
     * output parameters
     *
     * returns DWT_SUCCESS for success, or DWT_ERROR for error
     */
    int dwt_initialise(int mode);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function can place DW3000 into IDLE/IDLE_PLL or IDLE_RC mode when it is not actively in TX or RX.
     *
     * input parameters
     * @param state - DWT_DW_IDLE (1) to put DW3000 into IDLE/IDLE_PLL state; DWT_DW_INIT (0) to put DW3000 into INIT_RC state;
     *                DWT_DE_IDLE_RC (2) to put DW3000 into IDLE_RC state.
     *
     * output parameters none
     *
     * no return value
     */
    void dwt_setdwstate(int state);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to enable GPIO clocks. The clocks are needed to ensure correct GPIO operation
     *
     * input parameters
     *
     * output parameters
     *
     * no return value
     */
    void dwt_enablegpioclocks(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function needs to be called after device is woken up from DEEPSLEEP/SLEEP state, to restore the
     * configuration which has not been automatically restored from AON
     *
     * input parameters
     *
     * return DWT_SUCCESS
     *
     */
    void dwt_restoreconfig(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function configures STS mode: e.g. DWT_STS_MODE_OFF, DWT_STS_MODE_1 etc
     * The dwt_configure should be called prior to this to configure other parameters
     *
     * input parameters
     * @param stsMode    -   e.g. DWT_STS_MODE_OFF, DWT_STS_MODE_1 etc.
     *
     * return DWT_SUCCESS
     *
     */
    void dwt_configurestsmode(uint8_t stsMode);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function provides the main API for the configuration of the
     * DW3000 and this low-level driver.  The input is a pointer to the data structure
     * of type dwt_config_t that holds all the configurable items.
     * The dwt_config_t structure shows which ones are supported
     *
     * input parameters
     * @param config    -   pointer to the configuration structure, which contains the device configuration data.
     *
     * output parameters
     *
     * return DWT_SUCCESS or DWT_ERROR (e.g. when PLL CAL fails / PLL fails to lock)
     */
    int dwt_configure(dwt_config_t *config);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function provides the API for the configuration of the TX spectrum
     * including the power and pulse generator delay. The input is a pointer to the data structure
     * of type dwt_txconfig_t that holds all the configurable items.
     *
     * input parameters
     * @param config    -   pointer to the txrf configuration structure, which contains the tx rf config data
     *
     * output parameters
     *
     * no return value
     */
    void dwt_configuretxrf(dwt_txconfig_t *config);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function re-loads the STS AES initial value
     *
     * input parameters
     *
     * output parameters
     *
     * no return value
     */
    void dwt_configurestsloadiv(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function sets the default values of the lookup tables depending on the channel selected.
     *
     * input parameters
     * @param[in] channel - Channel that the device will be transmitting/receiving on.
     *
     * no return value
     */
    void dwt_configmrxlut(int channel);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function configures the STS AES 128 bit key value.
     * the default value is [31:00]c9a375fa,
     *                      [63:32]8df43a20,
     *                      [95:64]b5e5a4ed,
     *                     [127:96]0738123b
     *
     * input parameters
     * @param pStsKey - the pointer to the structure of dwt_sts_cp_key_t type, which holds the AES128 key value to generate STS
     *
     * output parameters
     *
     * no return value
     */
    void dwt_configurestskey(dwt_sts_cp_key_t *pStsKey);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function configures the STS AES 128 bit initial value, the default value is 1, i.e. DW3000 reset value is 1.
     *
     * input parameters
     * @param pStsIv - the pointer to the structure of dwt_sts_cp_iv_t type, which holds the IV value to generate STS
     *
     * output parameters
     *
     * no return value
     */
    void dwt_configurestsiv(dwt_sts_cp_iv_t *pStsIv);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This API function writes the antenna delay (in time units) to RX registers
     *
     * input parameters:
     * @param rxDelay - this is the total (RX) antenna delay value, which
     *                          will be programmed into the RX register
     *
     * output parameters
     *
     * no return value
     */
    void dwt_setrxantennadelay(uint16_t antennaDly);
    
    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This API function reads the antenna delay (in time units) from the RX antenna delay register
     *
     * input parameters:
     * @param dw - DW3xxx chip descriptor handler.
     *
     * output parameters
     *
     * returns 16-bit RX antenna delay value which is currently programmed in CIA_CONF_ID register
     */
    uint16_t dwt_getrxantennadelay(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This API function writes the antenna delay (in time units) to TX registers
     *
     * input parameters:
     * @param txDelay - this is the total (TX) antenna delay value, which
     *                          will be programmed into the TX delay register
     *
     * output parameters
     *
     * no return value
     */
    void dwt_settxantennadelay(uint16_t antennaDly);

    /*! ------------------------------------------------------------------------------------------------------------------
    * @brief This API function reads the antenna delay (in time units) from the TX antenna delay register
    *
    * input parameters:
    * @param dw - DW3xxx chip descriptor handler.
    *
    * output parameters
    *
    * returns 16-bit TX antenna delay value which is currently programmed in TX_ANTD_ID register
    */
    uint16_t dwt_gettxantennadelay(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This API function writes the supplied TX data into the DW3000's
     * TX buffer.  The input parameters are the data length in bytes and a pointer
     * to those data bytes.
     *
     * input parameters
     * @param txDataLength   - This is the total length of data (in bytes) to write to the tx buffer.
     *                         Note: the size of tx buffer is 1024 bytes.
     *                         The standard PHR mode allows to transmit frames of up to 127 bytes (including 2 byte CRC)
     *                         The extended PHR mode allows to transmit frames of up to 1023 bytes (including 2 byte CRC)
     *                         if > 127 is programmed, DWT_PHRMODE_EXT needs to be set in the phrMode configuration
     *                         see dwt_configure function
     * @param txDataBytes    - Pointer to the user's buffer containing the data to send.
     * @param txBufferOffset - This specifies an offset in the DW IC's TX Buffer at which to start writing data.
     *
     * output parameters
     *
     * returns DWT_SUCCESS for success, or DWT_ERROR for error
     */
    int dwt_writetxdata(uint16_t txDataLength, uint8_t *txDataBytes, uint16_t txBufferOffset);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This API function configures the TX frame control register before the transmission of a frame
     *
     * input parameters:
     * @param txFrameLength - this is the length of TX message (including the 2 byte CRC) - max is 1023
     *                              NOTE: standard PHR mode allows up to 127 bytes
     *                              if > 127 is programmed, DWT_PHRMODE_EXT needs to be set in the phrMode configuration
     *                              see dwt_configure function
     * @param txBufferOffset - the offset in the tx buffer to start writing the data
     * @param ranging - 1 if this is a ranging frame, else 0
     *
     * output parameters
     *
     * no return value
     */
    void dwt_writetxfctrl(uint16_t txFrameLength, uint16_t txBufferOffset, uint8_t ranging);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This API function is used to configure frame preamble length, the frame premable length can be
     * configured in steps of 8, from 16 to 2048 symbols. If a non-zero value is configured, then the TXPSR_PE setting is ignored.
     *
     * input parameters:
     * @param preambleLength - sets the length of the preamble, value of 0 disables this setting and the length of the
     *                         frame will be dependent on the TXPSR_PE setting as configured by dwt_configure function
     *
     * output parameters
     *
     * no return value
     */
    void dwt_setplenfine(uint8_t preambleLength);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This call initiates the transmission, input parameter indicates which TX mode is used see below
     *
     * input parameters:
     * @param mode - if mode = DWT_START_TX_IMMEDIATE - immediate TX (no response expected)
     *               if mode = DWT_START_TX_DELAYED   - delayed TX (no response expected) at specified time (time in DX_TIME register)
     *               if mode = DWT_START_TX_DLY_REF   - delayed TX (no response expected) at specified time
     *                                                  (time in DREF_TIME register + any time in DX_TIME register)
     *               if mode = DWT_START_TX_DLY_RS    - delayed TX (no response expected) at specified time
     *                                                  (time in RX_TIME_0 register + any time in DX_TIME register)
     *               if mode = DWT_START_TX_DLY_TS    - delayed TX (no response expected) at specified time
     *                                                  (time in TX_TIME_LO register + any time in DX_TIME register)
     *               if mode = DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED - immediate TX (response expected,
     *                                                                          so the receiver will be automatically
     *                                                                          turned on after TX is done)
     *               if mode = DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED - delayed TX (response expected,
     *                                                                        so the receiver will be automatically
     *                                                                        turned on after TX is done)
     *               if mode = DWT_START_TX_CCA        - Send the frame if no preamble detected within PTO time
     *               if mode = DWT_START_TX_CCA  | DWT_RESPONSE_EXPECTED - Send the frame if no preamble detected
     *                                                                     within PTO time and then enable RX output parameters
     *
     * returns DWT_SUCCESS for success, or DWT_ERROR for error (e.g. a delayed transmission will be cancelled if the delayed time has passed)
     */
    int dwt_starttx(uint8_t mode);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This API function configures the reference time used for relative timing of delayed sending and reception.
     * The value is at a 8ns resolution.
     *
     * input parameters
     * @param reftime - the reference time (which together with DX_TIME or TX timestamp or RX timestamp time is used to define a
     * transmission time or delayed RX on time)
     *
     * output parameters none
     *
     * no return value
     */
    void dwt_setreferencetrxtime(uint32_t reftime);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This API function configures the delayed transmit time or the delayed RX on time
     *
     * input parameters
     * @param starttime - the TX/RX start time (the 32 bits should be the high 32 bits of the system time at which to send the message,
     * or at which to turn on the receiver)
     *
     * output parameters none
     *
     * no return value
     */
    void dwt_setdelayedtrxtime(uint32_t starttime);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to read the DGC_DECISION index when RX_TUNING is enabled, this value is used to adjust the
     *        RX level and FP level estimation
     *
     * input parameters
     *
     * output parameters - the index value to be used in RX level and FP level formulas
     *
     * no return value
     */
    uint8_t dwt_get_dgcdecision(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to read the TX timestamp (adjusted with the programmed antenna delay)
     *
     * input parameters
     * @param timestamp - a pointer to a 5-byte buffer which will store the read TX timestamp time
     *
     * output parameters - the timestamp buffer will contain the value after the function call
     *
     * no return value
     */
    void dwt_readtxtimestamp(uint8_t *timestamp);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to read the high 32-bits of the TX timestamp (adjusted with the programmed antenna delay)
     *
     * input parameters
     *
     * output parameters
     *
     * returns high 32-bits of TX timestamp
     */
    uint32_t dwt_readtxtimestamphi32(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to read the low 32-bits of the TX timestamp (adjusted with the programmed antenna delay)
     *
     * input parameters
     *
     * output parameters
     *
     * returns low 32-bits of TX timestamp
     */
    uint32_t dwt_readtxtimestamplo32(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to read the PDOA result, it is the phase difference between either the Ipatov and STS POA, or
     * the two STS POAs, depending on the PDOA mode of operation. (POA - Phase Of Arrival)
     *
     * NOTE: To convert to degrees: float pdoa_deg = ((float)pdoa / (1 << 11)) * 180 / M_PI
     *
     * input parameters
     *
     * output parameters - the PDOA result (signed in [1:-11] radian units)
     *
     * no return value
     */
    int16_t dwt_readpdoa(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function is used to read the TDOA (Time Difference On Arrival). The TDOA value that is read from the
     * register is 41-bits in length. However, 6 bytes (or 48 bits) are read from the register. The remaining 7 bits at
     * the 'top' of the 6 bytes that are not part of the TDOA value should be set to zero and should not interfere with
     * rest of the 41-bit value. However, there is no harm in masking the returned value.
     *
     * input parameters
     *
     * output parameters
     * @param tdoa: time difference on arrival - buffer of 6 bytes that will be filled with TDOA value by calling this function
     *
     * no return value
     */
    void dwt_readtdoa(uint8_t *tdoa);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to read the RX timestamp (adjusted time of arrival)
     *
     * input parameters
     * @param timestamp - a pointer to a 5-byte buffer which will store the read RX timestamp time
     *
     * output parameters - the timestamp buffer will contain the value after the function call
     *
     * no return value
     */
    void dwt_readrxtimestamp(uint8_t *timestamp);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to read the RX timestamp (unadjusted time of arrival)
     *
     * input parameters
     * @param timestamp - a pointer to a 5-byte buffer which will store the read RX timestamp time
     *
     * output parameters - the timestamp buffer will contain the value after the function call
     *
     * no return value
     */
    void dwt_readrxtimestampunadj(uint8_t *timestamp);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to read the RX timestamp (adjusted time of arrival) w.r.t. Ipatov CIR
     *
     * input parameters
     * @param timestamp - a pointer to a 5-byte buffer which will store the read RX timestamp time
     *
     * output parameters - the timestamp buffer will contain the value after the function call
     *
     * no return value
     */
    void dwt_readrxtimestamp_ipatov(uint8_t *timestamp);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to read the RX timestamp (adjusted time of arrival) w.r.t. STS CIR
     *
     * input parameters
     * @param timestamp - a pointer to a 5-byte buffer which will store the read RX timestamp time
     *
     * output parameters - the timestamp buffer will contain the value after the function call
     *
     * no return value
     */
    void dwt_readrxtimestamp_sts(uint8_t *timestamp);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to read the high 32-bits of the RX timestamp (adjusted with the programmed antenna delay)
     *
     * NOTE: This should not be used when RX double buffer mode is enabled. Following APIs to read RX timestamp should be
     * used:  dwt_readrxtimestamp_ipatov or dwt_readrxtimestamp_sts or dwt_readrxtimestamp
     *
     * input parameters
     *
     * output parameters
     *
     * returns high 32-bits of RX timestamp
     */
    uint32_t dwt_readrxtimestamphi32(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to read the low 32-bits of the RX timestamp (adjusted with the programmed antenna delay)
     *
     * NOTE: This should not be used when RX double buffer mode is enabled. Following APIs to read RX timestamp should be
     * used:  dwt_readrxtimestamp_ipatov or dwt_readrxtimestamp_sts or dwt_readrxtimestamp
     *
     * input parameters
     *
     * output parameters
     *
     * returns low 32-bits of RX timestamp
     */
    uint32_t dwt_readrxtimestamplo32(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to read the high 32-bits of the system time
     *
     * input parameters
     *
     * output parameters
     *
     * returns high 32-bits of system time timestamp
     */
    uint32_t dwt_readsystimestamphi32(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to read the system time
     *
     * input parameters
     * @param timestamp - a pointer to a 4-byte buffer which will store the read system time
     *
     * output parameters
     * @param timestamp - the timestamp buffer will contain the value after the function call
     *
     * no return value
     */
    void dwt_readsystime(uint8_t *timestamp);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to turn off the transceiver
     *
     * input parameters
     *
     * output parameters
     *
     * no return value
     */
    void dwt_forcetrxoff(void);

    /*! ------------------------------------------------------------------------------------------------------------------
    * @brief This call turns on the receiver, can be immediate or delayed (depending on the mode parameter). In the case of a
    * "late" error the receiver will only be turned on if the DWT_IDLE_ON_DLY_ERR is not set.
    * The receiver will stay turned on, listening to any messages until
    * it either receives a good frame, an error (CRC, PHY header, Reed Solomon) or  it times out (SFD, Preamble or Frame).
    *
    * input parameters
    * @param mode - this can be one of the following allowed values:
    *
    * DWT_START_RX_IMMEDIATE      0x00    Enable the receiver immediately
    * DWT_START_RX_DELAYED        0x01    Set up delayed RX, if "late" error triggers, then the RX will be enabled immediately
    * DWT_IDLE_ON_DLY_ERR         0x02    If delayed RX failed due to "late" error then if this
                                       flag is set the RX will not be re-enabled immediately, and device will be in IDLE when function exits
    * DWT_START_RX_DLY_REF        0x04    Enable the receiver at specified time (time in DREF_TIME register + any time in DX_TIME register)
    * DWT_START_RX_DLY_RS         0x08    Enable the receiver at specified time (time in RX_TIME_0 register + any time in DX_TIME register)
    * DWT_START_RX_DLY_TS         0x10    Enable the receiver at specified time (time in TX_TIME_LO register + any time in DX_TIME register)

    * e.g.
    * (DWT_START_RX_DELAYED | DWT_IDLE_ON_DLY_ERR) 0x03 used to disable re-enabling of receiver if delayed RX failed due to "late" error
    *
    * returns DWT_SUCCESS for success, or DWT_ERROR for error (e.g. a delayed receive enable will be too far in the future if delayed time has passed)
    */
    int dwt_rxenable(int mode);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief enable/disable and configure SNIFF mode.
     *
     * SNIFF mode is a low-power reception mode where the receiver is sequenced on and off instead of being on all the time.
     * The time spent in each state (on/off) is specified through the parameters below.
     * See DW3000 User Manual section 4.5 "Low-Power SNIFF mode" for more details.
     *
     * input parameters:
     * @param enable - 1 to enable SNIFF mode, 0 to disable. When 0, all other parameters are not taken into account.
     * @param timeOn - duration of receiver ON phase, expressed in multiples of PAC size. The counter automatically adds 1 PAC
     *                 size to the value set. Min value that can be set is 1 (i.e. an ON time of 2 PAC size), max value is 15.
     * @param timeOff - duration of receiver OFF phase, expressed in multiples of 128/125 us (~1 us). Max value is 255.
     *
     * output parameters
     *
     * no return value
     */
    void dwt_setsniffmode(int enable, uint8_t timeOn, uint8_t timeOff);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This call enables the double receive buffer mode
     *
     * input parameters
     * @param dbl_buff_state - enum variable for enabling/disabling double buffering mode
     * @param dbl_buff_mode - enum variable for Receiver Auto-Re-enable
     *
     * output parameters
     *
     * no return value
     */
    void dwt_setdblrxbuffmode(dwt_dbl_buff_state_e dbl_buff_state, dwt_dbl_buff_mode_e dbl_buff_mode);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This call signal to the chip that the specific RX buff is free for fill
     *
     * input parameters
     * @param None
     *
     * output parameters
     *
     * no return value
     */
    void dwt_signal_rx_buff_free(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This call enables RX timeout (SY_STAT_RFTO event)
     *
     * input parameters
     * @param time - how long the receiver remains on from the RX enable command
     *               The time parameter used here is in 1.0256 us (512/499.2MHz) units
     *               If set to 0 the timeout is disabled.
     *
     * output parameters
     *
     * no return value
     */
    void dwt_setrxtimeout(uint32_t time);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This call enables preamble timeout (SY_STAT_RXPTO event)
     *
     * input parameters
     * @param  timeout - Preamble detection timeout, expressed in multiples of PAC size. The counter automatically adds 1 PAC
     *                   size to the value set. Min value that can be set is 1 (i.e. a timeout of 2 PAC size).
     *
     * output parameters
     *
     * no return value
     */
    void dwt_setpreambledetecttimeout(uint16_t timeout);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief calibrates the local oscillator as its frequency can vary between 15 and 34kHz depending on temp and voltage
     *
     * NOTE: this function needs to be run before dwt_configuresleepcnt, so that we know what the counter units are
     *
     * input parameters
     *
     * output parameters
     *
     * returns the number of XTAL cycles per low-power oscillator cycle. LP OSC frequency = 38.4 MHz/return value
     */
    uint16_t dwt_calibratesleepcnt(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief sets the sleep counter to new value, this function programs the high 16-bits of the 28-bit counter
     *
     * NOTE: this function needs to be run before dwt_configuresleep, also the SPI frequency has to be < 3MHz
     *
     * input parameters
     * @param sleepcnt - this it value of the sleep counter to program
     *
     * output parameters
     *
     * no return value
     */
    void dwt_configuresleepcnt(uint16_t sleepcnt);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief configures the device for both DEEP_SLEEP and SLEEP modes, and on-wake mode
     * i.e. before entering the sleep, the device should be programmed for TX or RX, then upon "waking up" the TX/RX settings
     * will be preserved and the device can immediately perform the desired action TX/RX
     *
     * NOTE: e.g. Tag operation - after deep sleep, the device needs to just load the TX buffer and send the frame
     *
     *
     *      mode:
     *      DWT_PGFCAL       0x0800 - Re-enable receiver on wake-up. !!!! on DW3720 - this should be cleared
     *      DWT_GOTORX       0x0200
     *      DWT_GOTOIDLE     0x0100
     *      DWT_SEL_OPS      0x0040 | 0x0080
     *      DWT_LOADOPS      0x0020
     *      DWT_LOADLDO      0x0010
     *      DWT_LOADDGC      0x0008
     *      DWT_LOADBIAS     0x0004
     *      DWT_RUNSAR       0x0002
     *      DWT_CONFIG       0x0001 - download the AON array into the HIF (configuration download)
     *
     *      wake: wake up parameters
     *      DWT_SLP_CNT_RPT  0x40 - sleep counter loop after expiration
     *      DWT_PRESRVE_SLP  0x20 - allows for SLEEP_EN bit to be "preserved", although it will self-clear on wake up
     *      DWT_WAKE_WK      0x10 - wake up on WAKEUP PIN
     *      DWT_WAKE_CS      0x8 - wake up on chip select
     *      DWT_BR_DET       0x4 - enable brownout detector during sleep/deep sleep
     *      DWT_SLEEP        0x2 - enable sleep
     *      DWT_SLP_EN       0x1 - enable sleep/deep sleep functionality
     *
     * input parameters
     * @param mode - config on-wake parameters
     * @param wake - config wake up parameters
     *
     * output parameters
     *
     * no return value
     */
    void dwt_configuresleep(uint16_t mode, uint8_t wake);

    /*! ------------------------------------------------------------------------------------------------------------------
     *
     * @brief this function clears the AON configuration in DW3000
     *
     * input parameters:
     *
     * output parameters
     *
     * no return value
     */
    void dwt_clearaonconfig(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function puts the device into deep sleep or sleep. dwt_configuresleep() should be called first
     * to configure the sleep and on-wake/wake-up parameters.
     *
     * input parameters
     * @param idle_rc - if this is set to DWT_DW_IDLE_RC, the auto INIT2IDLE bit will be cleared prior to going to sleep
     *                  thus after wakeup device will stay in IDLE_RC state
     *
     * output parameters
     *
     * no return value
     */
    void dwt_entersleep(int idle_rc);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief sets or clears the auto TX to sleep bit. This means that after a frame
     * transmission the device will enter sleep or deep sleep mode. The dwt_configuresleep() function
     * needs to be called before this to configure the on-wake settings
     *
     * NOTE: the IRQ line has to be low/inactive (i.e. no pending events)
     *
     * Deprecated, might be removed in a future mayor release. Consider using the dwt_entersleepafter() function instead.
     *
     * input parameters
     * @param enable - 1 to configure the device to enter sleep or deep sleep after TX, 0 - disables the configuration
     *
     * no return value
     */
    void dwt_entersleepaftertx(int enable);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief Sets or clears the auto TX and/or RX to sleep bits.
     *
     * This makes the device automatically enter deep sleep or sleep mode after a frame transmission and/or reception.
     * dwt_configuresleep() needs to be called before this to configure the sleep and on-wake/wake-up parameters.
     *
     * NOTE: the IRQ line has to be low/inactive (i.e. no pending events)
     *
     * input parameters
     * @param event_mask: bitmask to go to sleep after:
     *  - DWT_TX_COMPLETE to configure the device to enter sleep or deep sleep after TX
     *  - DWT_RX_COMPLETE to configure the device to enter sleep or deep sleep after RX
     *
     * output parameters
     *
     * no return value
     */
    void dwt_entersleepafter(int event_mask);

#ifdef WIN32
    /*! ------------------------------------------------------------------------------------------------------------------
     * @fn dwt_spicswakeup()   ---------  ********** NOTE: in decatest only ****************
     *
     * @brief wake up the device from sleep mode using the SPI read,
     * the device will wake up on chip select line going low if the line is held low for at least 500us.
     * To define the length depending on the time one wants to hold
     * the chip select line low, use the following formula:
     *
     *      length (bytes) = time (s) * byte_rate (Hz)
     *
     * where fastest byte_rate is spi_rate (Hz) / 8 if the SPI is sending the bytes back-to-back.
     * To save time and power, a system designer could determine byte_rate value more precisely.
     *
     * NOTE: Alternatively the device can be waken up with WAKE_UP pin if configured for that operation
     *
     * input parameters
     * @param buff   - this is a pointer to the dummy buffer which will be used in the SPI read transaction used for the WAKE UP of the device
     * @param length - this is the length of the dummy buffer
     *
     * output parameters
     *
     * returns DWT_SUCCESS for success, or DWT_ERROR for error
     */
    int dwt_spicswakeup(uint8_t *buff, uint16_t length);
#endif

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function is used to register the different callbacks called when one of the corresponding event occurs.
     *
     * NOTE: Callbacks can be undefined (set to NULL). In this case, dwt_isr() will process the event as usual but the 'null'
     * callback will not be called.
     *
     * input parameters
     * @param cbTxDone - the pointer to the TX confirmation event callback function
     * @param cbRxOk - the pointer to the RX good frame event callback function
     * @param cbRxTo - the pointer to the RX timeout events callback function
     * @param cbRxErr - the pointer to the RX error events callback function
     * @param cbSPIErr - the pointer to the SPI error events callback function
     * @param cbSPIRdy - the pointer to the SPI ready events callback function
    * @param cbDualSPIEv - the pointer to the Dual SPI events callback function (SPI1 Avail, SPI2 Avail)
    *
    * output parameters
    *
    * no return value
    */
    void dwt_setcallbacks(dwt_cb_t cbTxDone, dwt_cb_t cbRxOk, dwt_cb_t cbRxTo, dwt_cb_t cbRxErr, dwt_cb_t cbSPIErr, dwt_cb_t cbSPIRdy, dwt_cb_t cbDualSPIEv);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function checks if the IRQ line is active - this is used instead of interrupt handler
     *
     * input parameters
     *
     * output parameters
     *
     * return value is 1 if the IRQS bit is set and 0 otherwise
     */
    uint8_t dwt_checkirq(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function checks if the DW3000 is in IDLE_RC state
     *
     * The DW3XXX states are described in the User Manual. On power up, or following a reset the device will progress from INIT_RC to IDLE_RC. 
     * Once the device is in IDLE_RC SPI rate ca be increased to more than 7 MHz. The device will automatically proceed from INIT_RC to IDLE_RC
     * and both INIT_RC and SPI_RDY event flags will be set, once device is in IDLE_RC. 
     *
     * It is recommended that host waits for SPI_RDY event, which will also generate interrupt once device is ready after reset/power on. 
     * If the host cannot use interrupt as a way to check device is ready for SPI comms, then we recommend the host waits for 2 ms and reads this function, 
     * which checks if the device is in IDLE_RC state by reading the SYS_STATUS register and checking for the IDLE_RC event to be set. 
     * If host initiates SPI transaction with the device prior to it being ready, the SPI transaction may be incorrectly decoded by the device and 
     * the device may be misconfigured. Reading registers over SPI prior to device being ready may return garbage on the MISO, which may confuse the host application. 
     * 
     * input parameters
     *
     * output parameters
     *
     * return value is 1 if the IDLE_RC bit is set and 0 otherwise
     */
    uint8_t dwt_checkidlerc(void);

    /*! ------------------------------------------------------------------------------------------------------------------
    * @brief This is the DW3000's general Interrupt Service Routine. It will process/report the following events:
    *          - RXFCG (through cbRxOk callback)
    *          - TXFRS (through cbTxDone callback)
    *          - RXRFTO/RXPTO (through cbRxTo callback)
    *          - RXPHE/RXFCE/RXRFSL/RXSFDTO/AFFREJ/LDEERR (through cbRxErr callback)
    *        For all events, corresponding interrupts are cleared and necessary resets are performed. In addition, in the RXFCG case,
    *        received frame information and frame control are read before calling the callback. If double buffering is activated, it
    *        will also toggle between reception buffers once the reception callback processing has ended.
    *
    *        /!\ This version of the ISR supports double buffering but does not support automatic RX re-enabling!
    *
    * NOTE:  In PC based system using (Cheetah or ARM) USB to SPI converter there can be no interrupts, however we still need something
    *        to take the place of it and operate in a polled way. In an embedded system this function should be configured to be triggered
    *        on any of the interrupts described above.

    * input parameters
    *
    * output parameters
    *
    * no return value
    */
    void dwt_isr(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function enables the specified events to trigger an interrupt.
     * The following events can be found in SYS_ENABLE_LO and SYS_ENABLE_HI registers.
     *
     *
     * input parameters:
     * @param bitmask_lo - sets the events in SYS_ENABLE_LO_ID register which will generate interrupt
     * @param bitmask_hi - sets the events in SYS_ENABLE_HI_ID register which will generate interrupt
     * @param operation  - if set to DWT_ENABLE_INT additional interrupts as selected in the bitmask are enabled
     *                   - if set to DWT_ENABLE_INT_ONLY the interrupts in the bitmask are forced to selected state -
     *                      i.e. the mask is written to the register directly.
     *                   - otherwise (if set to DWT_DISABLE_INT) clear the interrupts as selected in the bitmask
     * output parameters
     *
     * no return value
     */
    void dwt_setinterrupt(uint32_t bitmask_lo, uint32_t bitmask_hi, dwt_INT_options_e INT_options);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to set the PAN ID
     *
     * input parameters
     * @param panID - this is the PAN ID
     *
     * output parameters
     *
     * no return value
     */
    void dwt_setpanid(uint16_t panID);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to set 16-bit (short) address
     *
     * input parameters
     * @param shortAddress - this sets the 16 bit short address
     *
     * output parameters
     *
     * no return value
     */
    void dwt_setaddress16(uint16_t shortAddress);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to set the EUI 64-bit (long) address
     *
     * input parameters
     * @param eui64 - this is the pointer to a buffer that contains the 64bit address
     *
     * output parameters
     *
     * no return value
     */
    void dwt_seteui(uint8_t *eui64);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to get the EUI 64-bit from the DW3000
     *
     * input parameters
     * @param eui64 - this is the pointer to a buffer that will contain the read 64-bit EUI value
     *
     * output parameters
     *
     * no return value
     */
    void dwt_geteui(uint8_t *eui64);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to read from AON memory
     *
     * input parameters
     * @param aon_address - this is the address of the memory location to read
     *
     * output parameters - None
     *
     * returns 8-bits read from given AON memory address
     */
    uint8_t dwt_aon_read(uint16_t aon_address);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to write to AON memory
     *
     * @param aon_address - this is the address of the memory location to write
     * @param aon_write_data - this is the data to write
     *
     * output parameters - None
     *
     * no return value
     *
     */
    void dwt_aon_write(uint16_t aon_address, uint8_t aon_write_data);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to enable the frame filtering - (the default option is to
     * accept any data and ACK frames with correct destination address
     *
     * input parameters
     * @param enabletype (bitmask) - enables/disables the frame filtering and configures 802.15.4 type
     *       DWT_FF_ENABLE_802_15_4      0x2             // use 802.15.4 filtering rules
     *       DWT_FF_DISABLE              0x0             // disable FF
     * @param filtermode (bitmask) - configures the frame filtering options according to
     *       DWT_FF_BEACON_EN            0x001           // beacon frames allowed
     *       DWT_FF_DATA_EN              0x002           // data frames allowed
     *       DWT_FF_ACK_EN               0x004           // ack frames allowed
     *       DWT_FF_MAC_EN               0x008           // mac control frames allowed
     *       DWT_FF_RSVD_EN              0x010           // reserved frame types allowed
     *       DWT_FF_MULTI_EN             0x020           // multipurpose frames allowed
     *       DWT_FF_FRAG_EN              0x040           // fragmented frame types allowed
     *       DWT_FF_EXTEND_EN            0x080           // extended frame types allowed
     *       DWT_FF_COORD_EN             0x100           // behave as coordinator (can receive frames with no dest address (PAN ID has to match))
     *       DWT_FF_IMPBRCAST_EN         0x200           // allow MAC implicit broadcast
     *
     * output parameters
     *
     * no return value
     */
    void dwt_configureframefilter(uint16_t enabletype, uint16_t filtermode);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief  this function is used to calculate 8-bit CRC, it uses 100000111 polynomial (i.e. P(x) = x^8+ x^2+ x^1+ x^0)
     *
     * input parameters:
     * @param byteArray     - data to calculate CRC for
     * @param flen          - length of byteArray
     * @param crcInit       - initialisation value for CRC calculation
     *
     * output parameters
     *
     * returns 8-bit calculate CRC value
     */
    uint8_t dwt_generatecrc8(const uint8_t *byteArray, int flen, uint8_t crcInit);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to enable SPI CRC check in DW3000
     *
     * input parameters
     * @param crc_mode - if set to DWT_SPI_CRC_MODE_WR then SPI CRC checking will be performed in DW3000 on each SPI write
     *                   last byte of the SPI write transaction needs to be the 8-bit CRC, if it does not match
     *                   the one calculated by DW3000 SPI CRC ERROR event will be set in the status register (SYS_STATUS_SPICRC)
     *
     * @param spireaderr_cb - this needs to contain the callback function pointer which will be called when SPI read error
     *                        is detected (when the DW3000 generated CRC does not match the one calculated by  dwt_generatecrc8
     *                        following the SPI read transaction)
     *
     * output parameters
     *
     * no return value
     */
    void dwt_enablespicrccheck(dwt_spi_crc_mode_e crc_mode, dwt_spierrcb_t spireaderr_cb);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This call enables the auto-ACK feature. If the responseDelayTime (parameter) is 0, the ACK will be sent a.s.a.p.
     * otherwise it will be sent with a programmed delay (in symbols), max is 255.
     * NOTE: needs to have frame filtering enabled as well
     *
     * input parameters
     * @param responseDelayTime - if non-zero the ACK is sent after this delay, max is 255.
     * @param enable - enables or disables the auto-ACK feature
     *
     * output parameters
     *
     * no return value
     */
    void dwt_enableautoack(uint8_t responseDelayTime, int enable);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This sets the receiver turn on delay time after a transmission of a frame
     *
     * input parameters
     * @param rxDelayTime - (20 bits) - the delay is in UWB microseconds
     *
     * output parameters
     *
     * no return value
     */
    void dwt_setrxaftertxdelay(uint32_t rxDelayTime);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief this function resets the DW3000
     *
     * NOTE: SPI rate must be <= 7MHz before a call to this function as the device will use FOSC/4 as part of internal reset
     *
     * input parameters:
     * @param reset_semaphore - if set to 1 the semaphore will be also reset. (only valid for DW3720 device)
     *
     * output parameters
     *
     * no return value
     */
    void dwt_softreset(int reset_semaphore);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to read the data from the RX buffer, from an offset location give by offset parameter
     *
     * input parameters
     * @param buffer - the buffer into which the data will be read
     * @param length - the length of data to read (in bytes)
     * @param rxBufferOffset - the offset in the rx buffer from which to read the data
     *
     * output parameters
     *
     * no return value
     */
    void dwt_readrxdata(uint8_t *buffer, uint16_t length, uint16_t rxBufferOffset);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to write the data from the RX scratch buffer, from an offset location given by offset parameter.
     *
     * input parameters
     * @param buffer - the buffer which to write to the device
     * @param length - the length of data to read (in bytes)
     * @param bufferOffset - the offset in the scratch buffer to which to write the data
     *
     * output parameters
     *
     * no return value
     */
    void dwt_write_rx_scratch_data(uint8_t *buffer, uint16_t length, uint16_t bufferOffset);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to read the data from the RX scratch buffer, from an offset location given by offset parameter.
     *
     * input parameters
     * @param buffer - the buffer into which the data will be read
     * @param length - the length of data to read (in bytes)
     * @param bufferOffset - the offset in the scratch buffer from which to read the data
     *
     * output parameters
     *
     * no return value
     */
    void dwt_read_rx_scratch_data(uint8_t *buffer, uint16_t length, uint16_t bufferOffset);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to read the 18 bit data from the Accumulator buffer, from an offset location give by offset parameter
     *        for 18 bit complex samples, each sample is 6 bytes (3 real and 3 imaginary)
     *
     *
     * NOTE: Because of an internal memory access delay when reading the accumulator the first octet output is a dummy octet
     *       that should be discarded. This is true no matter what sub-index the read begins at.
     *
     * input parameters
     * @param buffer - the buffer into which the data will be read
     * @param length - the length of data to read (in bytes)
     * @param accOffset - the offset in the acc buffer from which to read the data, this is a complex sample index
     *                    e.g. to read 10 samples starting at sample 100
     *                    buffer would need to be >= 10*6 + 1, length is 61 (1 is for dummy), accOffset is 100
     *
     * output parameters
     *
     * no return value
     */
    void dwt_readaccdata(uint8_t *buffer, uint16_t len, uint16_t accOffset);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to read the crystal offset (relating to the frequency offset of the far DW3000 device compared to this one)
     *        Note: the returned signed 16-bit number shoudl be divided by 16 to get ppm offset.
     *
     * input parameters - NONE
     *
     * return value - the (int12) signed offset value. (s[6:-4])
     *                A positive value means the local RX clock is running faster than the remote TX device.
     */
    int16_t dwt_readclockoffset(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to read the RX carrier integrator value (relating to the frequency offset of the TX node)
     *
     * input parameters - NONE
     *
     * return value - the (int32_t) signed carrier integrator value.
     *                A positive value means the local RX clock is running faster than the remote TX device.
     */
    int32_t dwt_readcarrierintegrator(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief this function enables CIA diagnostic data. When turned on the following registers will be logged:
     * IP_TOA_LO, IP_TOA_HI, STS_TOA_LO, STS_TOA_HI, STS1_TOA_LO, STS1_TOA_HI, CIA_TDOA_0, CIA_TDOA_1_PDOA, CIA_DIAG_0, CIA_DIAG_1
     *
     * input parameters
     * @param enable_mask :     DW_CIA_DIAG_LOG_MAX (0x8)   //CIA to copy to swinging set a maximum set of diagnostic registers in Double Buffer mode
     *                          DW_CIA_DIAG_LOG_MID (0x4)   //CIA to copy to swinging set a medium set of diagnostic registers in Double Buffer mode
     *                          DW_CIA_DIAG_LOG_MIN (0x2)   //CIA to copy to swinging set a minimal set of diagnostic registers in Double Buffer mode
     *                          DW_CIA_DIAG_LOG_ALL (0x1)   //CIA to log all diagnostic registers
     *                          DW_CIA_DIAG_LOG_OFF (0x0)   //CIA to log reduced set of diagnostic registers
     *
     * output parameters
     *
     * no return value
     */
    void dwt_configciadiag(uint8_t enable_mask);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief this function reads the STS signal quality index
     *
     * input parameters
     * @param rxStsQualityIndex - the (int16_t) signed STS quality index value.
     *
     * output parameters
     * return value - >=0 for good and < 0 if bad STS quality.
     *
     * Note: For the 64 MHz PRF if value is >= 90% of the STS length then we can assume good STS reception.
     *       Otherwise the STS timestamp may not be accurate.
     */
    int dwt_readstsquality(int16_t *rxStsQualityIndex);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief this function reads the STS status
     *
     * input parameters
     * @param stsstatus - the (uint8_t) STS status value.
     * @param sts_num   - 0 for 1st STS, 1 for 2nd STS (2nd only valid when PDOA Mode 3 is used)
     *
     * output parameters
     * return value 0 for good/valid STS < 0 if bad STS quality.
     */
    int dwt_readstsstatus(uint16_t *stsStatus, int sts_num);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief this function reads the RX signal quality diagnostic data
     *
     * input parameters
     * @param diagnostics - diagnostic structure pointer, this will contain the diagnostic data read from the DW3000
     *
     * output parameters
     *
     * no return value
     */
    void dwt_readdiagnostics(dwt_rxdiag_t *diagnostics);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to enable/disable the event counter in the IC
     *
     * input parameters
     * @param - enable - 1 enables (and reset), 0 disables the event counters
     * output parameters
     *
     * no return value
     */
    void dwt_configeventcounters(int enable);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to read the event counters in the IC
     *
     * input parameters
     * @param counters - pointer to the dwt_deviceentcnts_t structure which will hold the read data
     *
     * output parameters
     *
     * no return value
     */
    void dwt_readeventcounters(dwt_deviceentcnts_t *counters);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to read the OTP data from given address into provided array
     *
     * input parameters
     * @param address - this is the OTP address to read from
     * @param array - this is the pointer to the array into which to read the data
     * @param length - this is the number of 32 bit words to read (array needs to be at least this length)
     *
     * output parameters
     *
     * no return value
     */
    void dwt_otpread(uint16_t address, uint32_t* array, uint8_t length);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to program 32-bit value into the DW3000 OTP memory.
     *
     * input parameters
     * @param value - this is the 32-bit value to be programmed into OTP
     * @param address - this is the 16-bit OTP address into which the 32-bit value is programmed
     *
     * output parameters
     *
     * returns DWT_SUCCESS for success, or DWT_ERROR for error
     */
    int dwt_otpwriteandverify(uint32_t value, uint16_t address);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to program 32-bit value into the DW3700 OTP memory, it will not validate the word was written correctly
     *
     * input parameters
     * @param value - this is the 32-bit value to be programmed into OTP
     * @param address - this is the 16-bit OTP address into which the 32-bit value is programmed
     *
     * output parameters
     *
     * returns DWT_SUCCESS
     */
    int dwt_otpwrite(uint32_t value, uint16_t address);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to set up Tx/Rx GPIOs which could be used to control LEDs
     * Note: not completely IC dependent, also needs board with LEDS fitted on right I/O lines
     *       this function enables GPIOs 2 and 3 which are connected to LED3 and LED4 on EVB1000
     *
     * input parameters
     * @param mode - this is a bit field interpreted as follows:
     *          - bit 0: 1 to enable LEDs, 0 to disable them
     *          - bit 1: 1 to make LEDs blink once on init. Only valid if bit 0 is set (enable LEDs)
     *          - bit 2 to 7: reserved
     *
     * output parameters none
     *
     * no return value
     */
    void dwt_setleds(uint8_t mode);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to adjust the crystal frequency
     *
     * input parameters:
     * @param   value - crystal trim value (in range 0x0 to 0x3F) 64 steps (~1.65ppm per step)
     *
     * output parameters
     *
     * no return value
     */
    void dwt_setxtaltrim(uint8_t value);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function returns the value of XTAL trim that has been applied during initialisation (dwt_init). This can
     *        be either the value read in OTP memory or a default value.
     *
     * NOTE: The value returned by this function is the initial value only! It is not updated on dwt_setxtaltrim calls.
     *
     * input parameters
     *
     * output parameters
     *
     * returns the XTAL trim value set upon initialisation
     */
    uint8_t dwt_getxtaltrim(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function disables repeated frames from being generated.
     *
     * input parameters:
     * None
     *
     * output parameters:
     * None
     *
     * No return value
     */
    void dwt_stop_repeated_frames(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function enables repeated frames to be generated given a frame repetition rate.
     *
     * input parameters:
     * @param framerepetitionrate - Value specifying the rate at which frames will be repeated.
     *                            If the value is less than the frame duration, the frames are sent
     *                            back-to-back.
     *
     * output parameters:
     * None
     *
     * No return value
     */
    void dwt_repeated_frames(uint32_t framerepetitionrate);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function will enable a repeated continuous waveform on the device
     *
     * input parameters:
     * @param cw_enable: CW mode enable
     * @param cw_mode_config: CW configuration mode.
     *
     * output parameters:
     *
     */
    void dwt_repeated_cw(int cw_enable, int cw_mode_config);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief this function sets the DW3xxx to transmit continuous wave (CW) signal at specific channel frequency
     *
     * input parameters
     *
     * output parameters
     *
     * no return value
     */
    void dwt_configcwmode(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function sets the DW3xxx to continuous tx frame mode for regulatory approvals testing.
     *
     * input parameters:
     * @param framerepetitionrate - This is a 32-bit value that is used to set the interval between transmissions.
     *  The minimum value is 2. The units are approximately 4 ns. (or more precisely 512/(499.2e6*256) seconds)).
     *
     * output parameters
     *
     * no return value
     */
    void dwt_configcontinuousframemode(uint32_t framerepetitionrate);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function stops the continuous tx frame mode.
     *
     * input parameters:
     *
     * output parameters
     *
     * no return value
     */
    void dwt_disablecontinuousframemode(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief this function reads the raw battery voltage and temperature values of the DW IC.
     * The values read here will be the current values sampled by DW IC AtoD converters.
     *
     *
     * input parameters:
     *
     * output parameters
     *
     * returns  (temp_raw<<8)|(vbat_raw)
     */
    uint16_t dwt_readtempvbat(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief  this function takes in a raw temperature value and applies the conversion factor
     * to give true temperature. The dwt_initialise needs to be called before call to this to
     * ensure pdw3000local->tempP contains the SAR_LTEMP value from OTP.
     *
     * input parameters:
     * @param raw_temp - this is the 8-bit raw temperature value as read by dwt_readtempvbat
     *
     * output parameters:
     *
     * returns: temperature sensor value
     */
    float dwt_convertrawtemperature(uint8_t raw_temp);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief this function takes in a raw voltage value and applies the conversion factor
     * to give true voltage. The dwt_initialise needs to be called before call to this to
     * ensure pdw3000local->vBatP contains the SAR_LVBAT value from OTP
     *
     * input parameters:
     * @param raw_voltage - this is the 8-bit raw voltage value as read by dwt_readtempvbat
     *
     * output parameters:
     *
     * returns: voltage sensor value
     */
    float dwt_convertrawvoltage(uint8_t raw_voltage);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief this function reads the temperature of the DW3000 that was sampled
     * on waking from Sleep/Deepsleep. They are not current values, but read on last
     * wakeup if DWT_TANDV bit is set in mode parameter of dwt_configuresleep
     *
     * input parameters:
     *
     * output parameters:
     *
     * returns: 8-bit raw temperature sensor value
     */
    uint8_t dwt_readwakeuptemp(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief this function reads the battery voltage of the DW3000 that was sampled
     * on waking from Sleep/Deepsleep. They are not current values, but read on last
     * wakeup if DWT_TANDV bit is set in mode parameter of dwt_configuresleep
     *
     * input parameters:
     *
     * output parameters:
     *
     * returns: 8-bit raw battery voltage sensor value
     */
    uint8_t dwt_readwakeupvbat(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief Returns the PG delay value of the TX
     *
     * input parameters
     *
     * output parameters
     *
     * returns uint8_t
     */
    uint8_t dwt_readpgdelay(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief this function determines the adjusted bandwidth setting (PG_DELAY bitfield setting)
     * of the DW3000. The adjustemnt is a result of DW3000 internal PG cal routine, given a target count value it will try to
     * find the PG delay which gives the closest count value.
     * Manual sequencing of TX blocks and TX clocks need to be enabled for either channel 5 or 9.
     * This function presumes that the PLL is already in the IDLE state. Please configure the PLL to IDLE
     * state before calling this function, by calling dwt_configure.
     *
     * input parameters:
     * @param target_count - uint16_t - the PG count target to reach in order to correct the bandwidth
     *
     * output parameters:
     * returns: (uint8_t) The setting that was written to the PG_DELAY register (when calibration completed)
     */
    uint8_t dwt_calcbandwidthadj(uint16_t target_count);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief this function calculates the value in the pulse generator counter register (PGC_STATUS) for a given PG_DELAY
     * This is used to take a reference measurement, and the value recorded as the reference is used to adjust the
     * bandwidth of the device when the temperature changes. This function presumes that the PLL is already in the IDLE
     * state.
     *
     * input parameters:
     * @param pgdly - uint8_t - the PG_DELAY (max value 63) to set (to control bandwidth), and to find the corresponding count value for
     *
     * output parameters:
     * returns (uint16_t) - The count value calculated from the provided PG_DELAY value (from PGC_STATUS) - used as reference
     * for later bandwidth adjustments
     */
    uint16_t dwt_calcpgcount(uint8_t pgdly);

    /********************************************************************************************************************/
    /*                                                AES BLOCK                                                         */
    /********************************************************************************************************************/

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief   This function provides the API for the configuration of the AES key before first usage.
     * @param   key - pointer to the key which will be programmed to the Key register
     *          Note, key register supports only 128-bit keys.
     *
     * output parameters
     *
     * no return value
     */
    void dwt_set_keyreg_128(const dwt_aes_key_t *key);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief   This function provides the API for the configuration of the AES block before its first usage.
     *
     * input parameters
     * @param   pCfg - pointer to the configuration structure, which contains the AES configuration data.
     *
     * output parameters
     *
     * no return value
     */
    void dwt_configure_aes(const dwt_aes_config_t *pCfg);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief   This function provides the API for the job of encript/decript the data block
     *
     *          128 bit key shall be pre-loaded with dwt_set_aes_key()
     *          dwt_configure_aes
     *
     *          supports AES_KEY_Src_Register mode only
     *          packet sizes < 127
     *          note, the "nonce" shall be unique for every transaction
     * @param job - pointer to AES job, contains data info and encryption info.
     * @param core_type - Core type
     *
     * @return  AES_STS_ID status bits
     *
     *
     */
    int8_t dwt_do_aes(dwt_aes_job_t *job, dwt_aes_core_type_e core_type);

    /****************************************************************************************************************************************************
     *
     * Declaration of platform-dependent lower level functions.
     *
     ****************************************************************************************************************************************************/

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief  This function wakeup device by an IO pin
     *
     * @param None
     *
     * output parameters
     *
     * no return value
     */
    void dwt_wakeup_ic(void);

    // ---------------------------------------------------------------------------
    //
    // NB: The purpose of the deca_mutex.c file is to provide for microprocessor interrupt enable/disable, this is used for
    //     controlling mutual exclusion from critical sections in the code where interrupts and background
    //     processing may interact.  The code using this is kept to a minimum and the disabling time is also
    //     kept to a minimum, so blanket interrupt disable may be the easiest way to provide this.  But at a
    //     minimum those interrupts coming from the Decawave device should be disabled/re-enabled by this activity.
    //
    //     In porting this to a particular microprocessor, the implementer may choose to use #defines here
    //     to map these calls transparently to the target system.  Alternatively the appropriate code may
    //     be embedded in the functions provided in the deca_irq.c file.
    //
    // ---------------------------------------------------------------------------

    typedef int decaIrqStatus_t; // Type for remembering IRQ status

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function should disable interrupts. This is called at the start of a critical section
     * It returns the IRQ state before disable, this value is used to re-enable in decamutexoff call
     *
     * Note: The body of this function is defined in deca_mutex.c and is platform specific
     *
     * input parameters:
     *
     * output parameters
     *
     * returns the state of the DW3000 interrupt
     */
    decaIrqStatus_t decamutexon(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function should re-enable interrupts, or at least restore their state as returned(&saved) by decamutexon
     * This is called at the end of a critical section
     *
     * Note: The body of this function is defined in deca_mutex.c and is platform specific
     *
     * input parameters:
     * @param s - the state of the DW3000 interrupt as returned by decamutexon
     *
     * output parameters
     *
     * returns the state of the DW3000 interrupt
     */
    void decamutexoff(decaIrqStatus_t s);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief Wait for a given amount of time.
     * NB: The body of this function is defined in deca_sleep.c and is platform specific
     *
     * input parameters:
     * @param time_ms - time to wait in milliseconds
     *
     * output parameters
     *
     * no return value
     */
    void deca_sleep(unsigned int time_ms);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief Wait for a given amount of time.
     * NB: The body of this function is defined in deca_sleep.c and is platform specific
     *
     * input parameters:
     * @param time_us - time to wait in microseconds
     *
     * output parameters
     *
     * no return value
     */
    void deca_usleep(unsigned long time_us);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief this reads the device ID and checks if it is the right one
     *
     * input parameters
     * None
     *
     * output parameters
     *
     * returns DWT_SUCCESS for success, or DWT_ERROR for error
     */
    int dwt_check_dev_id(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     *
     * @brief This function runs the PGF calibration. This is needed prior to reception.
     * Note: If the RX calibration routine fails the device receiver performance will be severely affected, the application should reset and try again
     *
     * input parameters
     * @param ldoen    -   if set to 1 the function will enable LDOs prior to calibration and disable afterwards.
     *
     * return result of PGF calibration (DWT_ERROR/-1 = error)
     *
     */
    int dwt_run_pgfcal(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function runs the PGF calibration. This is needed prior to reception.
     *
     * input parameters
     * @param ldoen    -   if set to 1 the function will enable LDOs prior to calibration and disable afterwards.
     *
     * return result of PGF calibration (0 = error)
     *
     */
    int dwt_pgf_cal(int ldoen);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief
     * This function will re-calibrate and re-lock the PLL. If the cal/lock is successful DWT_SUCCESS
     * will be returned otherwise DWT_ERROR will be returned
     *
     * input parameters:None
     *
     * output parameters:
     * returns DWT_SUCCESS for success or DWT_ERROR for error.
     */
    int dwt_pll_cal(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function is used to control what rf port to use for TX/RX.
     *
     * @param rfPort - enum value for selecting desired port
     * @param enable - enum value for enabling or disabling manual control of antenna selction
     *
     * No return value
     */
    void dwt_configure_rf_port(dwt_rf_port_selection_e rfPort, dwt_rf_port_ctrl_e enable);

    /*! ------------------------------------------------------------------------------------------------------------------
     *
     * @brief   This function is used to write a 16 bit address to a desired Low-Energy device (LE) address. For frame pending to function when
     * the correct bits are set in the frame filtering configuration via the dwt_configureframefilter. See dwt_configureframefilter for more details.
     *
     * @param addr - the address value to be written to the selected LE register
     * @param leIndex - Low-Energy device (LE) address to write to
     *
     * no return value
     *
     */
    void dwt_configure_le_address(uint16_t addr, int leIndex);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function configures SFD type only: e.g. IEEE 4a - 8, DW-8, DW-16, or IEEE 4z -8 (binary)
     * The dwt_configure should be called prior to this to configure other parameters
     *
     * input parameters
     * @param sfdType    -   e.g. DWT_SFD_IEEE_4A, DWT_SFD_DW_8, DWT_SFD_DW_16, DWT_SFD_IEEE_4Z
     *
     * return none
     *
     */
    void dwt_configuresfdtype(uint8_t sfdType);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function writes a value to the system status register (lower).
     *
     * input parameters
     * @param mask - mask value to send to the system status register (lower 32-bits).
     *               e.g. "SYS_STATUS_TXFRS_BIT_MASK" to clear the TX frame sent event.
     *
     * return none
     */
    void dwt_writesysstatuslo(uint32_t mask);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function writes a value to the system status register (higher).
     *
     * input parameters
     * @param mask - mask value to send to the system status register (higher bits).
     *               NOTE: Be aware that the size of this register varies per device.
     *               DW3000 devices only require a 16-bit mask value typecast to 32-bit.
     *               DW3700 devices only require a 16-bit mask value typecast to 32-bit.
     *               DW3720 devices require a 32-bit mask value.
     *
     * return none
     */
    void dwt_writesysstatushi(uint32_t mask);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function reads the current value of the system status register (lower 32 bits)
     *
     * input parameters
     *
     * return A uint32_t value containing the value of the system status register (lower 32 bits)
     */
    uint32_t dwt_readsysstatuslo(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function reads the current value of the system status register (higher bits)
     *
     * input parameters
     *
     * return A uint32_t value containing the value of the system status register (higher bits)
     *        NOTE: Be aware that the size of this register varies per device.
     *        DW3000 devices will return a 16-bit value of the register that is typecast to a 32-bit value.
     *        DW3700 devices will return a 16-bit value of the register that is typecast to a 32-bit value.
     *        DW3720 devices will return a 'true' 32-bit value of the register.
     */
    uint32_t dwt_readsysstatushi(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function writes a value to the Receiver Double Buffer status register.
     *
     * input parameters
     * @param mask - mask value to send to the register.
     *               e.g. "RDB_STATUS_CLEAR_BUFF0_EVENTS" to clear the clear buffer 0 events.
     *
     * return none
     */
    void dwt_writerdbstatus(uint8_t mask);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function reads the current value of the Receiver Double Buffer status register.
     *
     * input parameters
     *
     * return A uint8_t value containing the value of the Receiver Double Buffer status register.
     */
    uint8_t dwt_readrdbstatus(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function will read the frame length of the last received frame.
     *        This function presumes that a good frame or packet has been received.
     *
     * input parameters
     *
     * return frame_len - A uint16_t with the number of octets in the received frame.
     */
    uint16_t dwt_getframelength(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This is used to read the value stored in CIA_ADJUST_ID register
     *
     * input parameters
     *
     * output parameters
     *
     * returns value stored in CIA_ADJUST_ID register
     */
    uint32_t dwt_readpdoaoffset(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function will set the value to the CIA_ADJUST_ID register.
     *
     * input parameters
     * @param offset - the offset value to be written into the CIA_ADJUST_ID register.
     *
     * return None.
     */
    void dwt_setpdoaoffset(uint16_t offset);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function can set GPIO output to high (1) or low (0) which can then be used to signal e.g. WiFi chip to
     * turn off or on. This can be used in devices with multiple radios to minimise co-existence interference.
     *
     * input parameters
     * @param enable       - specifies if to enable or disable WiFi co-ex functionality on GPIO5 (or GPIO4)
     *                       depending if coex_io_swap param is set to 1 or 0
     * @param coex_io_swap -  when set to 0, GPIO5 is used as co-ex out, otherwise GPIO4 is used
     *
     * return event counts from both timers: TIMER0 events in bits [7:0], TIMER1 events in bits [15:8]
     *
     */
    void dwt_wifi_coex_set(dwt_wifi_coex_e enable, int coex_io_swap);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function will reset the internal system time counter. The counter will be momentarily reset to 0,
     * and then will continue counting as normal. The system time/counter is only available when device is in
     * IDLE or TX/RX states.
     *
     * input parameters
     * none
     *
     * return
     * none
     *
     */
    void dwt_reset_system_counter(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function is used to configure the device for OSTR mode (One Shot Timebase Reset mode), this will
     * prime the device to reset the internal system time counter on SYNC pulse / SYNC pin toggle.
     * For more information on this operation please consult the device User Manual.
     *
     * input parameters
     * @param  enable       - Set to 1 to enable OSTR mode and 0 to disable
     * @param  wait_time    - When a counter running on the 38.4 MHz external clock and initiated on the rising edge
     *                        of the SYNC signal equals the WAIT programmed value, the DW3700 timebase counter will be reset.
     *
     * NOTE: At the time the SYNC signal is asserted, the clock PLL dividers generating the DW3700 125 MHz system clock are reset,
     * to ensure that a deterministic phase relationship exists between the system clock and the asynchronous 38.4 MHz external clock.
     * For this reason, the WAIT value programmed will dictate the phase relationship and should be chosen to give the
     * desired phase relationship, as given by WAIT modulo 4. A WAIT value of 33 decimal is recommended,
     * but if a different value is chosen it should be chosen so that WAIT modulo 4 is equal to 1, i.e. 29, 37, and so on.
     *
     * return
     * none
     *
     */
    void dwt_config_ostr_mode(uint8_t enable, uint16_t wait_time);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function drive the antenna configuration GPIO6/7
     *
     * input parameters
     * @param antenna_config - configure GPIO 6 and or 7 to use for Antenna selection with expected value.
     *      bitfield configuration:
     *      Bit 0: Use GPIO 6
     *      Bit 1: Value to apply (0/1)
     *      Bit 2: Use GPIO 7
     *      Bit 3: Value to apply (0/1)
     *
     * return none
     *
     */
    void dwt_configure_and_set_antenna_selection_gpio(uint8_t antenna_config);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function will read the Diagnostics Registers - IPATOV, STS1, STS2 from the device, which can help in 
     *        determining if packet has been received in LOS (line-of-sight) or NLOS (non-line-of-sight) condition.
     *        To help determine/estimate NLOS condition either Ipatov, STS1 or STS2 can be used, (or all three).
     * 
     * NOTE:  CIA Diagnostics need to be enabled to "DW_CIA_DIAG_LOG_ALL" else the diagnostic registers read will be 0.
     * input parameters:
     * @param dw        - DW3xxx chip descriptor handler.
     * 
     * @param all_diag  - this is the pointer to the Structure into which to read the data.
     *
     * @return a uint8_t value indicating if success or failure.
     */
    uint8_t dwt_nlos_alldiag(dwt_nlos_alldiag_t *all_diag);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function will read the IPATOV Diagnostic Registers to get the First Path and Peak Path Index value.
     *        This function is used when signal power is low to determine the signal type (LOS or NLOS). Hence only 
     *        Ipatov diagnostic registers are used to determine the signal type.
     *
     * input parameters:
     * @param dw        - DW3xxx chip descriptor handler.
     * 
     * @param index - this is the pointer to the Structure into which to read the data.
     *
     */
    void dwt_nlos_ipdiag(dwt_nlos_ipdiag_t *index);

    /*! ------------------------------------------------------------------------------------------------------------------
    * @brief This is used to read the value stored in CTR_DBG_ID register, these are the low 32-bits of the STS IV counter.
    *
    * input parameters
    *
    * output parameters
    *
    * returns value stored in CTR_DBG_ID register
    */
    uint32_t dwt_readctrdbg(void);
    
    /*! ------------------------------------------------------------------------------------------------------------------
    * @brief This is used to read the value stored in DGC_DBG_ID register.
    *
    * input parameters
    *
    * output parameters
    *
    * returns value stored in DGC_DBG_ID register
    */
    uint32_t dwt_readdgcdbg(void);

    /*! ------------------------------------------------------------------------------------------------------------------
    * @brief This function read CIA version in this device.
    *
    * input parameters
    * @param dw - DW3xxx chip descriptor handler.
    *
    * return
    * none
    *
    */
    uint32_t dwt_readCIAversion(void);

    /*! ------------------------------------------------------------------------------------------------------------------
    * @brief This is used to return base address of ACC_MEM_ID register (CIR base address)
    *
    * input parameters
    *
    * output parameters
    *
    * returns address of ACC_MEM_ID register
    */
    uint32_t dwt_getcirregaddress(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This API enables returns a list of register name/value pairs, to enable debug output / logging in external applications
     * e.g. DecaRanging
     *
     * input parameters
     * @param regs - Pointer to registers structure register_name_add_t, will be returned
     *
     * return
     * none
     *
     */
    register_name_add_t* dwt_get_reg_names(void);
    
    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function calculates the adjusted TxPower setting by applying a boost over a reference TxPower setting.
     * The reference TxPower setting should correspond to a 1ms frame (or 0dB) boost.
     * The boost to be applied should be provided in unit of 0.1dB boost.
     * For example, for a 125us frame, a theoretical boost of 9dB can be applied. A boost of 90 should be provided as input
     * parameter and will be applied over the reference TxPower setting for a 1ms frame.
     *
     * input parameters
     * @param boost - the boost to apply in 0.1dB units.
     * DW3XXX maximum boost is 354 in channel 5, 305 in channel 9. If the input value is greater than the maximum boost,
     * then it will be discarded and maximum boost will be targeted.
     * @param ref_tx_power - the tx_power_setting corresponding to a frame of 1ms (0dB boost)
     * @param channel - the current RF channel used for transmission of UWB frames
     * @param adj_tx_power - if successful, the adjusted tx power setting will be returned through this pointer
     * @param applied_boost - if successful, the exact amount of boost applied will be returned through this pointer
     *
     * return
     * int: DWT_SUCCESS: if an adjusted tx power setting could be calculated. In this case, the actual amount of boost that was
     * applied and the adjusted tx power setting will be respectively returned through the parameters adj_tx_power and boost
     *      DWT_ERROR: if the API could not calculate a valid adjusted TxPower setting
     *
     *
     */
    int dwt_adjust_tx_power(uint16_t boost, uint32_t ref_tx_power, uint8_t channel, uint32_t* adj_tx_power, uint16_t* applied_boost);

    /* BEGIN: CHIP_SPECIFIC_SECTION DW37xx */

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function enables the specified double RX buffer to trigger an interrupt.
     * The following events can be found in RDB_STAT_EN_ID registers.
     *
     *
     * input parameters:
     * @param bitmask    - sets the events in RDB_STAT_EN_ID register which will generate interrupt
     * @param operation  - if set to DWT_ENABLE_INT additional interrupts as selected in the bitmask are enabled
     *                   - if set to DWT_ENABLE_INT_ONLY the interrupts in the bitmask are forced to selected state -
     *                      i.e. the mask is written to the register directly.
     *                   - otherwise (if set to DWT_DISABLE_INT) clear the interrupts as selected in the bitmask
     * output parameters
     *
     * no return value
     */
    void dwt_setinterrupt_db(uint8_t bitmask, dwt_INT_options_e INT_options);

    /*! ------------------------------------------------------------------------------------------------------------------
     *
     * @brief This resets the DW3700 device including the semaphore.
     *
     * input parameters None
     *
     * output parameters None
     *
     * return None
     */
    void dwt_softreset_fcmd(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     *
     * @brief This resets the DW3700 device without the semaphore.
     *
     * input parameters None
     *
     * output parameters None
     *
     * return None
     */
    void dwt_softreset_no_sema_fcmd(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     *
     * @brief Request access to the device registers, using the dual SPI semaphore request command. If the semaphore is available,
     * the semaphore will be given, this will be shown by the returned status value.
     *
     * input parameters
     * @param
     *
     * @return semaphore value
     *
     */
    void dwt_ds_sema_request(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     *
     * @brief Release the semaphore that was taken by this host
     *
     * input parameters
     *
     * output parameters
     *
     * return None
     */
    void dwt_ds_sema_release(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     *
     * @brief This can be used by host on the SPI2 to force taking of the semaphore. Take semaphore even if it is not available.
     *        This does not apply to host on SPI1, only host on SPI2 can force taking of the semaphore.
     *
     * input parameters
     *
     * output parameters
     *
     * return None
     */
    void dwt_ds_sema_force(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     *
     * @brief Reports the semaphore status low byte.
     *
     * input parameters
     *
     * output parameters
     *
     * @return semaphore value
     */
    uint8_t dwt_ds_sema_status(void);
    
    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief Reports the semaphore status high byte.
     *
     * input parameters
     *
     * output parameters
     *
     * @return semaphore value
     */
    uint8_t dwt_ds_sema_status_hi(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     *
     * @brief With this API each host can prevent the device going into Sleep/Deepsleep state.
     * By default it is possible for either host to place the device into Sleep/Deepsleep. This may not be desirable,
     * thus a host once it is granted access can set a SLEEP_DISABLE bit in the register
     * to prevent the other host from putting the device to sleep once it gives up its access.
     *
     * @param host_sleep_en - HOST_EN_SLEEP: clears the bit - allowing the the device to go to sleep.
     *                        HOST_DIS_SLEEP: sets the bit to prevent the device from going to sleep
     *
     * return None
     */
    void dwt_ds_en_sleep(dwt_host_sleep_en_e host_sleep_en);

    /*! ------------------------------------------------------------------------------------------------------------------
    *
    * @brief With this API the host on either SPI1 or SPI2 can enable/disable whether the interrupt is raised upon
    * SPI1MAVAIL or SPI2MAVAIL event.
    *
    * @param dwt_spi_host_e spi_num - should be set to either DWT_HOST_SPI1 or DWT_HOST_SPI2
    * @param dwt_INT_options_e int_set    - should be set to either DWT_ENABLE_INT or DWT_DISABLE_INT
    *
    * return DWT_SUCCESS or DWT_ERROR (if input parameters not consistent)
    */
    int dwt_ds_setinterrupt_SPIxavailable(dwt_spi_host_e spi_num, dwt_INT_options_e int_set);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function enables or disables the equaliser block within in the CIA. The equaliser should be used when
     * receiving from devices which transmit using a Symmetric Root Raised Cosine pulse shape. The equaliser will adjust
     * the CIR to give improved receive timestamp results. Normally, this is left disabled (the default value), which
     * gives the best receive timestamp performance when interworking with devices (like this IC) that use the
     * IEEE 802.15.4z recommended minimum precursor pulse shape.
     *
     * @param en - DWT_EQ_ENABLED or DWT_EQ_DISABLED, enables/disables the equaliser block
     */
    void dwt_enable_disable_eq(uint8_t en);
    /* END: CHIP_SPECIFIC_SECTION DW37xx */

    /* BEGIN: CHIP_SPECIFIC_SECTION DW3720 */
    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function will reset the timers block. It will reset both timers. It can be used to stop a timer running
     * in repeat mode.
     *
     * input parameters
     * none
     *
     * return none
     *
     */
    void dwt_timers_reset(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function will read the timers' event counts. When reading from this register the values will be reset/cleared,
     * thus the host needs to read both timers' event counts the events relating to TIMER0 are in bits [7:0] and events
     * relating to TIMER1 in bits [15:8].
     *
     * input parameters
     * none
     *
     * return event counts from both timers: TIMER0 events in bits [7:0], TIMER1 events in bits [15:8]
     *
     */
    uint16_t dwt_timers_read_and_clear_events(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function configures selected timer (TIMER0 or TIMER1) as per configuration structure
     *
     * input parameters
     * @param tim_cfg       - pointer to timer configuration structure
     *
     * return none
     *
     */
    void dwt_configure_timer(dwt_timer_cfg_t *tim_cfg);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function configures the GPIOs (4 and 5) for COEX_OUT
     *
     * input parameters
     * @param timer_coexout - configure if timer controls the COEX_OUT
     * @param coex_swap     - configures if the COEX_OUT is on GPIO4 or GPIO5, when set to 1 the GPIO4 will be COEX_OUT
     *
     * return none
     *
     */
    void dwt_configure_wificoex_gpio(uint8_t timer_coexout, uint8_t coex_swap);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function sets timer expiration period, it is a 22-bit number
     *
     * input parameters
     * @param timer_name - specify which timer period to set: TIMER0 or TIMER1
     * @param exp        - expiry count - e.g. if units are XTAL/64 (1.66 us) then setting 1024 ~= 1.7 ms period
     *
     * return none
     *
     */
    void dwt_set_timer_expiration(dwt_timers_e timer_name, uint32_t exp);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This function enables the timer. In order to enable, the timer enable bit [0] for TIMER0 or [1] for TIMER1
     * needs to transition from 0->1.
     *
     * input parameters
     * @param enable       - specifies which timer to enable
     *
     * return none
     *
     */
    void dwt_timer_enable(dwt_timers_e timer_name);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This API enables "Fixed STS" function. The fixed STS function means that the same STS will be sent in each packet. 
     * And also in the receiver, when the receiver is enabled the STS will be reset. Thus transmitter and the receiver will be in sync.
     * 
     * input parameters
     * @param set - Set to 1 to set FIXED STS and 0 to disable
     *
     * return
     * none
     *
     */
    void dwt_set_fixedsts(uint8_t set);

    /*! ------------------------------------------------------------------------------------------------------------------
     * @brief This API sets the Alternative Pulse Shape according to ARIB.
     * 
     * input parameters
     * @param set_alternate - Set to 1 to enable the alternate pulse shape and 0 to restore default shape.
     *
     * return
     * none
     *
     */
    void dwt_set_alternative_pulse_shape(uint8_t set_alternate);


    /* END: CHIP_SPECIFIC_SECTION DW3720 */
#ifdef __cplusplus
}
#endif

#endif /* _DECA_DEVICE_API_H_ */
