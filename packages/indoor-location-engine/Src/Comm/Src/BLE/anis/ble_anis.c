/**
 * Copyright (c) 2012 - 2019, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
//#include "sdk_common.h"
#include "ble.h"
#include "ble_anis.h"
#include "ble_srv_common.h"

#include "niq.h"

#define NRF_LOG_MODULE_NAME ble_anis
#if BLE_ANIS_CONFIG_LOG_ENABLED
#define NRF_LOG_LEVEL       BLE_ANIS_CONFIG_LOG_LEVEL
#define NRF_LOG_INFO_COLOR  BLE_ANIS_CONFIG_INFO_COLOR
#define NRF_LOG_DEBUG_COLOR BLE_ANIS_CONFIG_DEBUG_COLOR
#else // BLE_ANIS_CONFIG_LOG_ENABLED
#define NRF_LOG_LEVEL       0
#endif // BLE_ANIS_CONFIG_LOG_ENABLED
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#define BLE_UUID_ANIS_SERVICE             0x3E40     /**< The UUID of the Apple Nearby Interaction Service. */
#define BLE_UUID_ANIS_ACD_CHARACTERISTIC  0xD9D5     /**< The UUID of the ACD Characteristic. */

#define ANIS_BASE_UUID      {{0xBA, 0x2D, 0x9C, 0x68, 0x73, 0x30, 0x33, 0x86, 0xB2, 0x4B, 0x17, 0x08, 0x00, 0x00, 0xFE, 0x48}} /**< Used vendor specific UUID. */
#define ACD_CHAR_BASE_UUID  {{0x28, 0x33, 0xF5, 0x75, 0x73, 0x80, 0x4E, 0x9A, 0x21, 0x47, 0xEF, 0xD8, 0x00, 0x00, 0xE8, 0x95}} /**< Used vendor specific UUID. */


static uint8_t accessory_config_data_buffer[sizeof(struct AccessoryConfigurationData)];
static uint8_t accessory_config_data_length;

/**@brief Function for handling the @ref BLE_GAP_EVT_CONNECTED event from the SoftDevice.
 *
 * @param[in] p_anis     Apple Nearby Interaction Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_connect(ble_anis_t * p_anis, ble_evt_t const * p_ble_evt)
{
    ret_code_t                 err_code;
    ble_anis_evt_t              evt;
    ble_gatts_value_t          gatts_val;
    uint8_t                    cccd_value[2];
    ble_anis_client_context_t * p_client = NULL;

    err_code = blcm_link_ctx_get(p_anis->p_link_ctx_storage,
                                 p_ble_evt->evt.gap_evt.conn_handle,
                                 (void *) &p_client);
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_ERROR("Link context for 0x%02X connection handle could not be fetched.",
                      p_ble_evt->evt.gap_evt.conn_handle);
    }

    /* Check the hosts CCCD value to inform of readiness to send data using the RX characteristic */
    memset(&gatts_val, 0, sizeof(ble_gatts_value_t));
    gatts_val.p_value = cccd_value;
    gatts_val.len     = sizeof(cccd_value);
    gatts_val.offset  = 0;

    err_code = sd_ble_gatts_value_get(p_ble_evt->evt.gap_evt.conn_handle,
                                      p_anis->tx_handles.cccd_handle,
                                      &gatts_val);

    if ((err_code == NRF_SUCCESS)     &&
        (p_anis->data_handler != NULL) &&
        ble_srv_is_notification_enabled(gatts_val.p_value))
    {
        if (p_client != NULL)
        {
            p_client->is_notification_enabled = true;
        }

        memset(&evt, 0, sizeof(ble_anis_evt_t));
        evt.type        = BLE_ANIS_EVT_COMM_STARTED;
        evt.p_anis      = p_anis;
        evt.conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
        evt.p_link_ctx  = p_client;

        p_anis->data_handler(&evt);
    }
}


/**@brief Function for handling the @ref BLE_GATTS_EVT_WRITE event from the SoftDevice.
 *
 * @param[in] p_anis     Apple Nearby Interaction Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_write(ble_anis_t * p_anis, ble_evt_t const * p_ble_evt)
{
    ret_code_t                    err_code;
    ble_anis_evt_t                 evt;
    ble_anis_client_context_t    * p_client;
    ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    err_code = blcm_link_ctx_get(p_anis->p_link_ctx_storage,
                                 p_ble_evt->evt.gatts_evt.conn_handle,
                                 (void *) &p_client);
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_ERROR("Link context for 0x%02X connection handle could not be fetched.",
                      p_ble_evt->evt.gatts_evt.conn_handle);
    }

    memset(&evt, 0, sizeof(ble_anis_evt_t));
    evt.p_anis       = p_anis;
    evt.conn_handle = p_ble_evt->evt.gatts_evt.conn_handle;
    evt.p_link_ctx  = p_client;

    if ((p_evt_write->handle == p_anis->tx_handles.cccd_handle) &&
        (p_evt_write->len == 2))
    {
        if (p_client != NULL)
        {
            if (ble_srv_is_notification_enabled(p_evt_write->data))
            {
                p_client->is_notification_enabled = true;
                evt.type                          = BLE_ANIS_EVT_COMM_STARTED;
            }
            else
            {
                p_client->is_notification_enabled = false;
                evt.type                          = BLE_ANIS_EVT_COMM_STOPPED;
            }

            if (p_anis->data_handler != NULL)
            {
                p_anis->data_handler(&evt);
            }

        }
    }
    else if ((p_evt_write->handle == p_anis->rx_handles.value_handle) &&
             (p_anis->data_handler != NULL))
    {
        evt.type                  = BLE_ANIS_EVT_RX_DATA;
        evt.params.rx_data.p_data = p_evt_write->data;
        evt.params.rx_data.length = p_evt_write->len;

        p_anis->data_handler(&evt);
    }
    else
    {
        // Do Nothing. This event is not relevant for this service.
    }
}


/**@brief Function for handling the @ref BLE_GATTS_EVT_HVN_TX_COMPLETE event from the SoftDevice.
 *
 * @param[in] p_anis     Apple Nearby Interaction Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_hvx_tx_complete(ble_anis_t * p_anis, ble_evt_t const * p_ble_evt)
{
    ret_code_t                 err_code;
    ble_anis_evt_t              evt;
    ble_anis_client_context_t * p_client;

    err_code = blcm_link_ctx_get(p_anis->p_link_ctx_storage,
                                 p_ble_evt->evt.gatts_evt.conn_handle,
                                 (void *) &p_client);
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_ERROR("Link context for 0x%02X connection handle could not be fetched.",
                      p_ble_evt->evt.gatts_evt.conn_handle);
        return;
    }

    if (p_client->is_notification_enabled)
    {
        memset(&evt, 0, sizeof(ble_anis_evt_t));
        evt.type        = BLE_ANIS_EVT_TX_RDY;
        evt.p_anis      = p_anis;
        evt.conn_handle = p_ble_evt->evt.gatts_evt.conn_handle;
        evt.p_link_ctx  = p_client;

        p_anis->data_handler(&evt);
    }
}


void ble_anis_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    if ((p_context == NULL) || (p_ble_evt == NULL))
    {
        return;
    }

    ble_anis_t * p_anis = (ble_anis_t *)p_context;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_anis, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_anis, p_ble_evt);
            break;

        case BLE_GATTS_EVT_HVN_TX_COMPLETE:
            on_hvx_tx_complete(p_anis, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}


uint32_t ble_anis_init(ble_anis_t * p_anis)
{
    ret_code_t            err_code;
    ble_uuid_t            ble_uuid;
    ble_uuid128_t         anis_base_uuid     = ANIS_BASE_UUID;
    ble_uuid128_t         acd_char_base_uuid = ACD_CHAR_BASE_UUID;
    ble_add_char_params_t add_char_params;

    uint8_t               acd_uuid_type;

    VERIFY_PARAM_NOT_NULL(p_anis);

    // Using niq lib functions to get the actual accessory config data size
    niq_populate_accessory_uwb_config_data(accessory_config_data_buffer, &accessory_config_data_length);
    accessory_config_data_length += ACCESSORY_CONFIGURATION_DATA_FIX_LEN;

    /**@snippet [Adding proprietary Service to the SoftDevice] */
    // Add a custom base UUID.
    err_code = sd_ble_uuid_vs_add(&anis_base_uuid, &p_anis->uuid_type);
    VERIFY_SUCCESS(err_code);

    ble_uuid.type = p_anis->uuid_type;
    ble_uuid.uuid = BLE_UUID_ANIS_SERVICE;

    // Add the service.
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_anis->service_handle);
    /**@snippet [Adding proprietary Service to the SoftDevice] */
    VERIFY_SUCCESS(err_code);

    // Add the ACD Characteristic.
    // ACD Characteristic has a custom UUID, very different from the ANIS Service.
    err_code = sd_ble_uuid_vs_add(&acd_char_base_uuid, &acd_uuid_type);
    VERIFY_SUCCESS(err_code);

    memset(&add_char_params, 0, sizeof(add_char_params));
    add_char_params.uuid              = BLE_UUID_ANIS_ACD_CHARACTERISTIC;
    add_char_params.uuid_type         = acd_uuid_type;
    add_char_params.max_len           = (uint16_t)(accessory_config_data_length);
    add_char_params.p_init_value      = accessory_config_data_buffer;
    add_char_params.is_value_user     = true;
    add_char_params.char_props.read   = 1;

    add_char_params.read_access       = SEC_JUST_WORKS;

    return characteristic_add(p_anis->service_handle, &add_char_params, &p_anis->tx_handles);
}

void set_accessory_uwb_config_data(uint8_t * curr_config)
{   
    if (accessory_config_data_length > sizeof(accessory_config_data_buffer))
    {
        NRF_LOG_ERROR("Buffer allocation too small for Accessory Config Data. \n\r%d bytes allocated, %d bytes needed.",
                       sizeof(accessory_config_data_buffer), accessory_config_data_length);
        return;
    }
    memcpy(accessory_config_data_buffer, curr_config, accessory_config_data_length);
}