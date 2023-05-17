/*
 * Copyright (c) 2021 Qorvo US, Inc. All rights reserved.
 * This software is proprietary to Qorvo; you may not use it for any purpose
 * without first obtaining a license from Qorvo.
 * Please contact Qorvo to inquire about licensing terms.
 */

#ifndef NEARBY_INTERACTION_QORVO_H
#define NEARBY_INTERACTION_QORVO_H    1

#include "stddef.h"

// UWB "NIQ" level access methods

/* @brief Initialization of Nearby Interaction Qorvo module
 * @parm  start_uwb - reference to implementation to start UWB TWR session
 *        stop_uwb  - reference to implementation to stop UWB TWR session
 *        crypto_init - reference to the crypto module initialization
 *        crypto_deinit - reference to the crypto module deinitialization
 *        crypto_range_vector_geneator - reference to the crypto module generator
 * NOTE: all parameters are mandatory
 *
 * @return 0 on success or
 *        -EINPVAL - incorrect input parameters
 */
int  niq_init(void (*start_uwb)(void), 
              void (*stop_uwb)(void), 
              void (*crypto_init)(void), 
              void (*crypto_deinit)(void),
              void (*crypto_range_vector_geneator)(uint8_t * const data, size_t data_len));

/* @brief Restart of the Nearby Interaction Qorvo module
 *        Must be called every time UWB TWR session is stopped.
 *        This function is also called internally during niq_init().
 */
void niq_reinit(void);

/* @brief Deinitializes the Nearby Interaction Qorvo module.
 *        Calls previously set crypto_deinit().
 */
void niq_deinit(void);

/* @brief Stops UWB TWR session
 *        Calls previously set stop_uwb().
 */
void niq_stop_uwb(void);

/* @brief Set the ranging role of the Accessory 
 * @parm  role: 0 - Controlee+Responder
 *              1 - Controller+Initiator
 */
void niq_set_ranging_role(uint8_t role);

/* @brief Provides Accessory Configuration Data for Out Of Band handshake.
 */
void niq_populate_accessory_uwb_config_data(void * out_buffer, uint8_t * out_buffer_len);

/* @brief Starts the UWB TWR session.
 *        Sets the configuration for UWB session and calls start_uwb().
 * @parm  payload - input data from NI service
 *        length  - length of data from NI service
 *        arg     - pointer to the struct fira_device_configure_s
 * @return 0 on success or
 *        -EINPVAL - incorrect input data
 *        -EVERSIONNOTSUPPORTED - version of NI protocol not supported
 */
int  niq_configure_and_start_uwb(uint8_t *payload, uint8_t length, void *arg);

/// ---------------------------------------------------------------------------------------------------------------
/// The following definitions exposed to the embedded application
/// ---------------------------------------------------------------------------------------------------------------

#define E_NIQ_INPVAL              1
#define E_NIQ_VERSIONNOTSUPPORTED 2

#define NI_ACCESSORY_PROTOCOL_SPEC_MAJOR_VERSION 1
#define NI_ACCESSORY_PROTOCOL_SPEC_MINOR_VERSION 0

#define MAX_UWB_CONFIG_SIZE (64)
#define ACCESSORY_CONFIGURATION_DATA_FIX_LEN (16)

///  Section 3.4.3.1 in Nearby Interaction Accessory Protocol Specification, Developer Preview.
enum PreferredUpdateRate {
    PreferredUpdateRate_Automatic = 0, // iOS will choose a value
    PreferredUpdateRate_Infrequent = 10, // ~1.3Hz
    PreferredUpdateRate_UserInteractive = 20, // ~5.5Hz
};

struct AccessoryConfigurationData {
    uint16_t            majorVersion; // NI Accessory Protocol major version
    uint16_t            minorVersion; // NI Accessory Protocol minor version
    uint8_t             preferredUpdateRate;
    uint8_t             rfu[10];
    uint8_t             uwbConfigDataLength;
    // ACCESSORY_CONFIGURATION_DATA_FIX_LEN
    uint8_t             uwbConfigData[MAX_UWB_CONFIG_SIZE];
} __attribute__((packed));

/// ---------------------------------------------------------------------------------------------------------------
/// The following definitions are needed for successful interoperation with the iOS sample app
/// ---------------------------------------------------------------------------------------------------------------

#define NI_MAX_PACKET_SIZE 64

/// An minimal messaging protocol between the embedded application and the iOS app
/// The same Mesasge Id values are reflected in the iOS sample app
///
/// A Reference iOS Application, which is using the same messaging protocol Message ID values: 
/// https://developer.apple.com/documentation/nearbyinteraction/implementing_spatial_interactions_with_third-party_accessories
///
/// A general documentation of the Newarby Interaction:
/// https://developer.apple.com/nearby-interaction/
///
enum SampleAppMessageId {
    // From accessory
    MessageId_accessoryConfigurationData = 0x1,
    MessageId_accessoryUwbDidStart = 0x2,
    MessageId_accessoryUwbDidStop = 0x3,
    
    // To accessory
    MessageId_init = 0xA,
    MessageId_configure_and_start = 0xB,
    MessageId_stop = 0xC,
   
   // Developer can extend SampleAppMessageId with other MessageId
   UserId_getDeviceStruct = 0x20,
   UserId_setDeviceStruct = 0x21,

   UserId_iOSNotify = 0x2F
};


typedef struct __attribute__((__packed__)) {
    uint8_t message_id;
    uint8_t payload[NI_MAX_PACKET_SIZE];
} ni_packet_t;


/// ---------------------------------------------------------------------------------------------------------------
/// The following definitions are needed for configuration of the accessory sample app
/// ---------------------------------------------------------------------------------------------------------------

/* UWB Device unicast configuration */
struct fira_device_configure_s {
    uint8_t role;
    uint8_t enc_payload;
    uint32_t Session_ID;
    uint8_t Ranging_Round_Usage;
    uint8_t Multi_Node_Mode;
    uint8_t Rframe_Config;
    uint8_t ToF_Report;
    uint8_t AoA_Azimuth_Report;
    uint8_t AoA_Elevation_Report;
    uint8_t AoA_FOM_Report;
    uint8_t nonDeferred_Mode;
    uint8_t STS_Config;
    uint8_t Round_Hopping;
    uint8_t Block_Striding;
    uint32_t Block_Duration_ms;
    uint32_t Round_Duration_RSTU;
    uint32_t Slot_Duration_RSTU;
    uint8_t Channel_Number;
    uint8_t Preamble_Code;
    uint8_t PRF_Mode;
    uint8_t SP0_PHY_Set;
    uint8_t SP1_PHY_Set;
    uint8_t SP3_PHY_Set;
    uint32_t MAX_RR_Retry;
    uint8_t Constraint_Length_Conv_Code_HPRF;
    uint32_t UWB_Init_Time_ms;
    uint16_t Block_Timing_Stability;
    uint8_t Key_Rotation;
    uint8_t Key_Rotation_Rate;
    uint8_t MAC_FCS_TYPE;
    uint8_t MAC_ADDRESS_MODE;
    uint8_t SRC_ADDR[2];
    uint8_t Number_of_Controlee;
    uint8_t DST_ADDR[2];
        
    uint8_t Vendor_ID[2];
    uint8_t Static_STS_IV[6];
};

typedef struct fira_device_configure_s fira_device_configure_t;

#endif // NEARBY_INTERACTION_QORVO_H
