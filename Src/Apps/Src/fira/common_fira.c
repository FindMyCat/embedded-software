/**
 *  @file     common_fira.c
 *
 *  @brief    Fira params control
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

#include <stdlib.h>
#include <inttypes.h>

#include "app.h"
#include "deca_dbg.h"
#include "default_config.h"
#include "util.h"
#include "task_signal.h"
#include "common_fira.h"
#include "apps_common.h"
#include "appConfig.h"
#include "driver_app_config.h"
#include "reporter.h"

#include "HAL_uwb.h"
#include "rf_tuning_config.h"
#include "dw3000_pdoa.h"

#ifdef FIRA_PARAM_CUSTOM
#include "fira_custom_params.h"
#else
#include "fira_default_params.h"
#endif

int sscanf(const char *__restrict, const char *__restrict, ...);

static int local_pavrg_size;

void show_fira_params()
{
#define REMAINING (2048 - strlen(str))
    /* Display the Fira session */
    char *str = malloc(2048);
    int hlen;

    if (!str)
    {
        char *err = "not enough memory";
        reporter_instance.print((char *)err, strlen(err));
        return;
    }
    fira_param_t *fira_param = get_fira_config();
    hlen = sprintf(str, "JS%04X",
                   0x5A5A); // reserve space for length of JS object
    snprintf(&str[strlen(str)], REMAINING, "{\"F PARAMS\":{\r\n");

    snprintf(&str[strlen(str)], REMAINING, "\"SLOT, rstu\":%" PRIu32 ",\r\n",
             fira_param->session.slot_duration_rstu);
    snprintf(&str[strlen(str)], REMAINING, "\"Ranging Period, ms\":%" PRIu32 ",\r\n",
             fira_param->session.block_duration_ms);
    snprintf(&str[strlen(str)], REMAINING,
             "\"Ranging round, slots\":%" PRIu32 ",\r\n",
             fira_param->session.round_duration_slots);
    snprintf(&str[strlen(str)], REMAINING,
             "\"Ranging round usage (Unicast,SS,DS)\":%u,\r\n",
             fira_param->session.ranging_round_usage);
    snprintf(&str[strlen(str)], REMAINING, "\"Session_ID\":%u,\r\n",
             (unsigned int)fira_param->session_id);
    snprintf(&str[strlen(str)], REMAINING, "\"RFRAME\":%u,\r\n",
             fira_param->session.rframe_config);
    if (fira_param->session.rframe_config == FIRA_RFRAME_CONFIG_SP1)
    {
        snprintf(&str[strlen(str)], REMAINING,
                 "\"Vendor OUI\" 0x%06" PRIX32 ",\r\n",
                 fira_param->session.data_vendor_oui);
    }
    snprintf(&str[strlen(str)], REMAINING, "\"SFD ID\":%u,\r\n",
             fira_param->session.sfd_id);
    snprintf(&str[strlen(str)], REMAINING, "\"Multi node mode\":%u,\r\n",
             fira_param->session.multi_node_mode);
    snprintf(&str[strlen(str)], REMAINING, "\"Round hopping\":%d,\r\n",
             fira_param->session.round_hopping);
    uint8_t *v = fira_param->session.vupper64;
    snprintf(&str[strlen(str)], REMAINING,
             "\"Vupper64\":\"%02x%02x%02x%02x%02x%02x%02x%02x\",\r\n",
             v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);

    snprintf(&str[strlen(str)], REMAINING, "\"Initiator Addr\":\"0x%04X\",\r\n",
             (fira_param->session.device_type == FIRA_DEVICE_TYPE_CONTROLLER) ? (fira_param->session.short_addr) : (fira_param->session.destination_short_address));


    for (int i = 0; i < fira_param->controlees_params.n_controlees; i++)
    {
        /* Remove the trailing , in the last param */

        snprintf(&str[strlen(str)], REMAINING,
                 "\"Responder[%d] Addr\":\"0x%04X\"", i,
                 fira_param->controlees_params.controlees[i].address);
        /* Check if it is the last item */
        if (i == fira_param->controlees_params.n_controlees - 1)
        {
            /* Don't append a , at the end */
            snprintf(&str[strlen(str)], REMAINING, "\r\n");
        }
        else
        {
            snprintf(&str[strlen(str)], REMAINING, ",\r\n");
        }
    }
    snprintf(
        &str[2], REMAINING, "%04X",
        strlen(str) - hlen); // add formatted 4X of length, this will erase first '{'
    str[hlen] = '{';         // restore the start bracket
    sprintf(&str[strlen(str)], "}}\r\n");

    reporter_instance.print((char *)str, strlen(str));
    free(str);
    /* Wait a little bit for printing. */
    osDelay(100);
#undef REMAINING
}

void scan_fira_params(const char *text, bool controller)
{
    char cmd[20];
    char vupper64[FIRA_VUPPER64_SIZE * 3];
    int resp_addr[8] = {0};
    int bprf_set, slot_rstu, block_ms, round_slots, session_id;
    int multi_mode, round_hop, init_addr, rr_usage;
    int max = sizeof(resp_addr) / sizeof(resp_addr[0]);

    /* Get parameters from global configuration. */
    fira_param_t *fira_param = get_fira_config();
    dwt_config_t *dwt_config = get_dwt_config();

    int n = sscanf(
        text,
        "%20s %i %i %i %i %i %i %24s %i %i %i %i %i %i %i %i %i %i %i",
        cmd, &bprf_set, &slot_rstu, &block_ms, &round_slots, &rr_usage, &session_id,
        vupper64, &multi_mode, &round_hop, &init_addr, &resp_addr[0], &resp_addr[1],
        &resp_addr[2], &resp_addr[3], &resp_addr[4], &resp_addr[5], &resp_addr[6],
        &resp_addr[7]);

    /* Make sure to update number of argument before responder address */
    max = max > (n - 11) ? (n - 11) : max;

    fira_param->session.initiation_time_ms = 1000;
    fira_param->session.report_tof = 1;

    /* Set AoA parameters: set only if UWB chip and Antenna supports AoA */
    if (hal_uwb.is_aoa() == AOA_ENABLED)
    {
        fira_param->session.meas_seq.n_steps = 1;
        fira_param->session.meas_seq.steps[0].type = FIRA_MEASUREMENT_TYPE_AOA_AZIMUTH;
        fira_param->session.meas_seq.steps[0].n_measurements = 1;
        fira_param->session.meas_seq.steps[0].rx_ant_set_nonranging = 0xff;
        fira_param->session.meas_seq.steps[0].rx_ant_sets_ranging[0] = 0xff;
        fira_param->session.meas_seq.steps[0].rx_ant_sets_ranging[1] = 0xff;
        fira_param->session.meas_seq.steps[0].tx_ant_set_nonranging = 0xff;
        fira_param->session.meas_seq.steps[0].tx_ant_set_ranging = 0xff;
        fira_param->session.report_aoa_azimuth = 1;
        fira_param->session.report_aoa_fom = 1;
        fira_param->session.aoa_result_req = 1;
    }
    else
    {
        fira_param->session.meas_seq.n_steps = 1;
        fira_param->session.meas_seq.steps[0].type = FIRA_MEASUREMENT_TYPE_RANGE;
        fira_param->session.meas_seq.steps[0].n_measurements = 1;
        fira_param->session.meas_seq.steps[0].rx_ant_set_nonranging = 0xff;
        fira_param->session.meas_seq.steps[0].rx_ant_sets_ranging[0] = 0xff;
        fira_param->session.meas_seq.steps[0].rx_ant_sets_ranging[1] = 0xff;
        fira_param->session.meas_seq.steps[0].tx_ant_set_nonranging = 0xff;
        fira_param->session.meas_seq.steps[0].tx_ant_set_ranging = 0xff;
        fira_param->session.report_aoa_azimuth = 0;
        fira_param->session.report_aoa_fom = 0;
        fira_param->session.aoa_result_req = 1;
    }

    /* Request measurement report phase */
    fira_param->session.result_report_phase = true;

    /* set preamble 64 */
    fira_param->session.preamble_duration = 1; // 64

    /* set sts parameters in the session*/
    fira_param->session.number_of_sts_segments = FIRA_STS_SEGMENTS_1;
    fira_param->session.sts_length = FIRA_STS_LENGTH_64;


    /* BPRF set 1 & 2 are not allowed in Fira. */
    if (n > 1 && bprf_set > 2)
    {
        /* BPRF set 3 & 5 SP1, 4 & 6 SP3. */
        if ((bprf_set % 2) == 0)
        {
            fira_param->session.rframe_config = FIRA_RFRAME_CONFIG_SP3;
        }
        else
        {
            fira_param->session.rframe_config = FIRA_RFRAME_CONFIG_SP1;
            fira_param->session.data_vendor_oui = 0x0c0b0a;
        }
        /* BPRF set 3 & 4 SFD2, 5 & 6 SFD0. */
        fira_param->session.sfd_id = (bprf_set > 4) ? FIRA_SFD_ID_0 : FIRA_SFD_ID_2;
    }
    else
    {
        fira_param->session.rframe_config = FIRA_DEFAULT_RFRAME_CONFIG;
        fira_param->session.sfd_id = FIRA_DEFAULT_SFD_ID;
    }
    fira_param->session.slot_duration_rstu = (n > 2) ? (slot_rstu) : (FIRA_DEFAULT_SLOT_DURATION_RSTU);
    fira_param->session.block_duration_ms = (n > 3) ? (block_ms) : (FIRA_DEFAULT_BLOCK_DURATION_MS);
    fira_param->session.round_duration_slots = (n > 4) ? (round_slots) : (FIRA_DEFAULT_ROUND_DURATION_SLOTS);
    fira_param->session.ranging_round_usage = (n > 5) ? (rr_usage) : FIRA_DEFAULT_RANGING_ROUND_USAGE;
    fira_param->session_id = (n > 6) ? (session_id) : (FIRA_DEFAULT_SESSION_ID);
    fira_param->session.multi_node_mode = (n > 8) ? ((multi_mode == 1) ? FIRA_MULTI_NODE_MODE_ONE_TO_MANY : FIRA_MULTI_NODE_MODE_UNICAST) : (FIRA_DEFAULT_MULTI_NODE_MODE);
    fira_param->session.round_hopping = (n > 9) ? (round_hop == 1) : (FIRA_DEFAULT_ROUND_HOPPING);
    fira_param->controlees_params.n_controlees = 0;
    for (int i = 0; i < max; i++)
    {
        fira_param->controlees_params
            .controlees[fira_param->controlees_params.n_controlees++]
            .address = resp_addr[i];
    }
    if (controller)
    {
        // fira_param->session.destination_short_address = 55;
        fira_param->session.short_addr = (n > 10) ? (init_addr) : FIRA_DEFAULT_CONTROLLER_SHORT_ADDR;
        fira_param->session.device_type = FIRA_DEVICE_TYPE_CONTROLLER;
        fira_param->session.device_role = FIRA_DEVICE_ROLE_INITIATOR;
    }
    else
    {
        fira_param->session.destination_short_address = (n > 10) ? (init_addr) : (FIRA_DEFAULT_CONTROLLER_SHORT_ADDR);
        fira_param->session.short_addr = (n > 11) ? resp_addr[0] : FIRA_DEFAULT_CONTROLLEE_SHORT_ADDR;
        fira_param->session.device_type = FIRA_DEVICE_TYPE_CONTROLEE;
        fira_param->session.device_role = FIRA_DEVICE_ROLE_RESPONDER;
    }
    /* Reusing parameters, should always be at the end of params parsing. */
    uint8_t *v = fira_param->session.vupper64;
    if (n > 7)
    {
        int param[8];
        int x = sscanf(vupper64,
                       "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",
                       &param[0], &param[1], &param[2], &param[3],
                       &param[4], &param[5], &param[6], &param[7]);
        if (x != FIRA_VUPPER64_SIZE)
        {
            for (int i = 0; i < FIRA_VUPPER64_SIZE; i++)
                v[i] = i + 1;
        }
        else
        {
            for (int i = 0; i < FIRA_VUPPER64_SIZE; i++)
                v[i] = param[i];
        }
    }
    else
    {
        for (int i = 0; i < FIRA_VUPPER64_SIZE; i++)
            v[i] = i + 1;
    }

    if (fira_param->controlees_params.n_controlees == 0)
    {
        fira_param->controlees_params.controlees[0].address = FIRA_DEFAULT_CONTROLLEE_SHORT_ADDR;
        fira_param->controlees_params.n_controlees = 1;
    }


    fira_param->session.channel_number = dwt_config->chan;
    fira_param->session.preamble_code_index = dwt_config->txCode;
}


uwbmac_error fira_set_session_parameters(struct fira_context *fira_context, uint32_t session_id, struct session_parameters *session)
{
    struct session_parameters_builder param_builder;

    session_parameters_builder_init(fira_context, &param_builder, session_id);

    // UWB setup
    session_parameters_builder_set_channel_number(&param_builder, session->channel_number);
    session_parameters_builder_set_preamble_code_index(&param_builder, session->preamble_code_index);
    session_parameters_builder_set_preamble_duration(&param_builder, session->preamble_duration);
    session_parameters_builder_set_sfd_id(&param_builder, session->sfd_id);
    session_parameters_builder_set_psdu_data_rate(&param_builder, session->psdu_data_rate);
    // session_parameters_builder_set_phr_data_rate(&param_builder, session->phr_data_rate);
    // session_parameters_builder_set_prf_mode(&param_builder, session->prf_mode);
    // session_parameters_builder_set_tx_adaptative_payload_power(&param_builder, session->tx_adaptative_payload_power);

    // role
    session_parameters_builder_set_device_type(&param_builder, session->device_type);
    session_parameters_builder_set_device_role(&param_builder, session->device_role);

    // addresses
    session_parameters_builder_set_short_addr(&param_builder, session->short_addr);
    session_parameters_builder_set_destination_short_address(&param_builder, session->destination_short_address);
    session_parameters_builder_set_data_vendor_oui(&param_builder, session->data_vendor_oui);
    // session_parameters_builder_set_mac_fcs_type(&param_builder, session->mac_fcs_type);

    // frame config
    session_parameters_builder_set_rframe_config(&param_builder, session->rframe_config);
    session_parameters_builder_set_initiation_time_ms(&param_builder, session->initiation_time_ms);
    session_parameters_builder_set_slot_duration_rstu(&param_builder, session->slot_duration_rstu);
    session_parameters_builder_set_block_duration_ms(&param_builder, session->block_duration_ms);
    session_parameters_builder_set_round_duration_slots(&param_builder, session->round_duration_slots);
    session_parameters_builder_set_ranging_round_usage(&param_builder, session->ranging_round_usage);
    session_parameters_builder_set_round_hopping(&param_builder, session->round_hopping);
    session_parameters_builder_set_multi_node_mode(&param_builder, session->multi_node_mode);
    // session_parameters_builder_set_block_stride_length(&param_builder, session->block_stride_length);
    // session_parameters_builder_set_max_rr_retry(&param_builder, session->max_rr_retry);

    // security & STS
    session_parameters_builder_set_sts_length(&param_builder, session->sts_length);
    session_parameters_builder_set_number_of_sts_segments(&param_builder, session->number_of_sts_segments);
    session_parameters_builder_set_sts_config(&param_builder, session->sts_config);
    session_parameters_builder_set_vupper64(&param_builder, session->vupper64);
    // session_parameters_builder_set_key_rotation(&param_builder, session->key_rotation);
    // session_parameters_builder_set_key_rotation_rate(&param_builder, session->key_rotation_rate);
    // session_parameters_builder_set_session_key(&param_builder, const void *session_key,	uint8_t size);

    // reporting ranging
    session_parameters_builder_set_report_tof(&param_builder, session->report_tof);
    session_parameters_builder_set_result_report_phase(&param_builder, session->result_report_phase);
    session_parameters_builder_set_max_number_of_measurements(&param_builder, session->max_number_of_measurements);
    session_parameters_builder_set_meas_seq(&param_builder, &session->meas_seq);
    // session_parameters_builder_set_range_data_ntf_config(&param_builder, session->range_data_ntf_config);
    // session_parameters_builder_set_range_data_ntf_proximity_near_mm(&param_builder, session->range_data_ntf_proximity_near_mm);
    // session_parameters_builder_set_range_data_ntf_proximity_far_mm(&param_builder, session->range_data_ntf_proximity_far_mm);

    // reporting AOA
    session_parameters_builder_set_aoa_result_req(&param_builder, session->aoa_result_req);
    session_parameters_builder_set_report_aoa_azimuth(&param_builder, session->report_aoa_azimuth);
    session_parameters_builder_set_report_aoa_elevation(&param_builder, session->report_aoa_elevation);
    session_parameters_builder_set_report_aoa_fom(&param_builder, session->report_aoa_fom);

    // reporting RSSI
    // session_parameters_builder_set_report_rssi(&param_builder, session->report_rssi);

    // misc
    // session_parameters_builder_set_cap_size_max(&param_builder, session->cap_size_max);
    // session_parameters_builder_set_cap_size_min(&param_builder, session->cap_size_min);
    // session_parameters_builder_set_priority(&param_builder, session->priority);
    // session_parameters_builder_set_embedded_mode(&param_builder, session->embedded_mode);
    // session_parameters_builder_set_enable_diagnostics(&param_builder, session->enable_diagnostics);
    // session_parameters_builder_set_diags_frame_reports_fields(&param_builder, session->diags_frame_reports_fields);

    // finally, set the parameters
    int r = fira_helper_set_partial_session_parameters(fira_context, &param_builder);
    return r;
}

//-----------------------------------------------------------------------------
void set_local_pavrg_size(void)
{
    rf_tuning_t *rf_tuning = get_rf_tuning_config();
    local_pavrg_size = rf_tuning->paverage;
}

uint8_t get_local_pavrg_size(void)
{
    return (local_pavrg_size);
}

int get_rx_ctx_size(void)
{
    /* always allocating the minimum required size */
    return sizeof(struct avrg_s) + (sizeof(float) * get_local_pavrg_size());
}
//-----------------------------------------------------------------------------