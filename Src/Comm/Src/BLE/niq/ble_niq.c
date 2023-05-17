/**
 * @file      ble_niq.c
 * 
 * @brief     functions for niq BLE 
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



#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "nordic_common.h"
#include "app_error.h"
#include "nrf.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_qnis.h"
#include "ble_anis.h"
#include "ble_conn_params.h"
#include "ble_conn_state.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_freertos.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "nrf_log.h"
#include "nrf_nvic.h"

#include "appConfig.h"

#include "fira_device_config.h"
#include "niq.h"

static void send_ble_data(uint8_t * buffer, uint16_t data_len);
static void send_accessory_config_data();

static ble_qnis_t * p_qnis;
static uint16_t * p_conn_handle;

void get_qnis_data(ble_qnis_t * pqnis) {
    p_qnis = pqnis;
}
void get_conn_handle(uint16_t * pconn_handle) {
    p_conn_handle = pconn_handle;
}

static void send_ack_uwb_started(void)
{
      // Send "UWB did start" BLE message to the app
      uint8_t buffer[1];
      ni_packet_t * packet = (ni_packet_t *)buffer;
      packet->message_id = (uint8_t)MessageId_accessoryUwbDidStart;
      send_ble_data(buffer, 1);
}

static void send_ack_uwb_stopped(void)
{
    uint8_t buffer[1];
    ni_packet_t * packet = (ni_packet_t *)buffer;
    packet->message_id = (uint8_t)MessageId_accessoryUwbDidStop;
    send_ble_data(buffer, 1);
}

static void send_ble_data(uint8_t * buffer, uint16_t data_len)
{
    uint32_t err_code;
    uint16_t length = data_len;

    do
    {
        err_code = ble_qnis_data_send(p_qnis, buffer, &length, *p_conn_handle);
        if ((err_code != NRF_ERROR_INVALID_STATE) &&
            (err_code != NRF_ERROR_RESOURCES) &&
            (err_code != NRF_ERROR_NOT_FOUND))
        {
            APP_ERROR_CHECK(err_code);
        }

    } while (err_code == NRF_ERROR_RESOURCES);
}

void send_ios_notification(uint8_t distance, char * txt_message)
{
    uint8_t buffer[NI_MAX_PACKET_SIZE];
    uint8_t payload_size;

    payload_size = strlen(txt_message) + 2; // txt_message len + 1 byte payload_size + 1 byte distance

    ni_packet_t * packet = (ni_packet_t *)buffer;
    packet->message_id = (uint8_t)UserId_iOSNotify;
    packet->payload[0] = payload_size;
    packet->payload[1] = distance;
    memcpy(&packet->payload[2], txt_message, strlen(txt_message));
    
    send_ble_data(buffer, payload_size + 1);
}

/**@brief Sending for handling new data from BLE */
static void send_accessory_config_data() {
    NRF_LOG_INFO("App requests accessory config data");

    niq_reinit();

    // The Accessory Configuration Data in intended to be constructed by the embedded application.

    // The message structure is the following:
    // ------------------------
    //     majorVersion         -- the major version from section 3.3 of the Nearby Interaction Accessory Protocol Specification, Developer Preview, Release 1.
    //     minorVersion         -- the minor version from section 3.3 of the Nearby Interaction Accessory Protocol Specification, Developer Preview, Release 1.
    //     preferredUpdateRate  -- a selection of one of the options from table 3-3 in the Nearby Interaction Accessory Protocol Specification, Developer Preview, Release 1.
    //     rfu[10]              -- reserved for future use.
    //     uwbConfigDataLength  -- the length of the UWB config data as provided by the UWB middleware.
    //     uwbConfigData        -- the UWB config data as provided by the UWB middleware, according to section 4.4.2 on the UWB Interoperability Specification, Developer Preview, Release 1.
    // ------------------------
    //
    // In order to populate `uwbConfigData` and `uwbConfigDataLength`, the embedded appliaction needs to query
    // the UWB middleware which is compliant with the UWB Interoperability Specification.
    //
    // Once the Accessory Configuration Data is constucted and populated, the embedded application needs to send it to the iOS app.

    uint8_t buffer[sizeof(struct AccessoryConfigurationData)+1]; // + 1 for the MessageId
    memset(buffer, 0, sizeof(struct AccessoryConfigurationData)+1);

    ni_packet_t * packet = (ni_packet_t *)buffer;
    packet->message_id = (uint8_t)MessageId_accessoryConfigurationData;

    // Embeded appliaction responsibility
    struct AccessoryConfigurationData *config = (struct AccessoryConfigurationData *)packet->payload;
    config->majorVersion = NI_ACCESSORY_PROTOCOL_SPEC_MAJOR_VERSION;
    config->minorVersion = NI_ACCESSORY_PROTOCOL_SPEC_MINOR_VERSION;
    config->preferredUpdateRate = PreferredUpdateRate_UserInteractive;
    
    // UWB middleware responsibility
    niq_populate_accessory_uwb_config_data(&config->uwbConfigData, &config->uwbConfigDataLength);

    set_accessory_uwb_config_data(packet->payload);

    send_ble_data(buffer, (uint16_t)(config->uwbConfigDataLength + ACCESSORY_CONFIGURATION_DATA_FIX_LEN + 1));
}


/**@brief Function for handling new data from BLE */
void handle_new_data(const uint8_t * data, int data_len)
{
    if (data_len < 1) {
        NRF_LOG_INFO("data_len must be at least 1. Return.");
        return;
    }

    ni_packet_t * packet = (ni_packet_t *)data;

    switch (packet->message_id) {
        case MessageId_init: {
            // Reply AccessoryConfigurationData to the app
            send_accessory_config_data();
            break;
        }
        case MessageId_configure_and_start: {
            // Configure accessory UWB and start
            int ret;
            ret = niq_configure_and_start_uwb(packet->payload, data_len-1, (void*)&fira_config);

            switch(ret)
            {
            case -E_NIQ_INPVAL:
                 NRF_LOG_ERROR("Data len wrong");
                 break;
            case -E_NIQ_VERSIONNOTSUPPORTED:
                 NRF_LOG_ERROR("Protocol version not supported");
                 break;
            default:
                send_ack_uwb_started();
                break;
            }

            break;
        }
        case MessageId_stop: {
            // Stop accessory UWB and reset state
            niq_stop_uwb();

            send_ack_uwb_stopped();
            break;
        }
        case UserId_getDeviceStruct: {
            // Request device info
            NRF_LOG_INFO("Device Info requested.");
            break;
        }
        case UserId_setDeviceStruct: {
            // Set device info
            NRF_LOG_INFO("Device Info changed.");
            break;
        }
        default:
            // unsupported message ID
            NRF_LOG_INFO("Unsupported message ID.");
            break;
    }
}



void ble_evt_disconnected_handler(){
    niq_stop_uwb();
}



