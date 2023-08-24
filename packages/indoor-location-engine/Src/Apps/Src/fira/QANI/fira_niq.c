/**
 *  @file     fira_niq.c
 *
 *  @brief    Fira processes control
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
#include <inttypes.h>
#include <stdint.h>
#include "HAL_error.h"
#include "app.h"
#include "boards.h"
#include "common_fira.h"
#include "deca_error.h"
#include "dw3000_mcps_mcu.h"
#include "int_priority.h"
#include "reporter.h"
#include "task_signal.h"
#include "uwbmac.h"
#include "fira_device_config.h"
#include "util.h"
#include "fira_niq.h"
#include "HAL_uwb.h"
#include "EventManager.h"
#include "common_fira.h"

#define OUTPUT_PDOA_ENABLE (1)
#define STR_SIZE (256)

#define MIN_CM_DISTANCE_THRESHOLD   40
#define MAX_CM_DISTANCE_THRESHOLD  120

extern void pdoaupdate_lut(void);
extern void send_ios_notification(uint8_t distance, char * txt_message);

static void report_cb(const struct ranging_results *results, void *user_data);
static void construct_fira_param_from_config(fira_param_t *fira_param, fira_device_configure_t *config, int controller_type);
static error_e fira_niq_app_process_init(bool controller, fira_param_t *fira_param);
static void fira_niq_app_process_start(void);
static error_e fira_niq_app_process_terminate(void);
static void fira_niq_helper(void const *argument);
static void fira_niq_terminate(void);


static struct string_measurement output_result;
static struct uwbmac_context *uwbmac_ctx = NULL;
static struct fira_context fira_ctx;
static uint32_t session_id = 42;
static bool started = false;

/* Initialize fira_config with default config */
fira_device_configure_t fira_config = FIRA_DEFAULT_DEVICE_CONFIG;

static error_e fira_niq_app_process_init(bool controller, fira_param_t *fira_param) {

  session_id = fira_param->session_id;
  uint16_t string_len =
      STR_SIZE * (controller ? fira_param->controlees_params.n_controlees : 1);

  output_result.str = malloc(string_len);
  if (!(output_result.str))
  {
    char *err = "not enough memory";
    reporter_instance.print((char*)err, strlen(err));
    return _ERR_Cannot_Alloc_Memory;
  }
  output_result.len = string_len;

  set_local_pavrg_size();
  pdoaupdate_lut();

  fira_uwb_mcps_init(fira_param);

  int r = uwbmac_init(&uwbmac_ctx);
  assert(r == UWBMAC_SUCCESS);

  // unset promiscuous to accept only filtered frames.
  uwbmac_set_promiscuous_mode(uwbmac_ctx, true);
  // set local short address.
  uwbmac_set_short_addr(uwbmac_ctx, fira_param->session.short_addr);
  // register report cb
  r = fira_helper_open(&fira_ctx, uwbmac_ctx, &report_cb, "endless", 0,
                       &output_result);
  assert(r == UWBMAC_SUCCESS);
  // Set fira scheduler;
  r = fira_helper_set_scheduler(&fira_ctx);
  assert(r == UWBMAC_SUCCESS);
  // init session;
  r = fira_helper_init_session(&fira_ctx, session_id);
  assert(r == UWBMAC_SUCCESS);
  // Set session parameters;
  r = fira_set_session_parameters(&fira_ctx, session_id, &fira_param->session);
  assert(r == UWBMAC_SUCCESS);
  if (controller)
  {
    // Add controlee session parameters;
    r = fira_helper_add_controlees(&fira_ctx, session_id,
                                   &fira_param->controlees_params);
    assert(r == UWBMAC_SUCCESS);
  }
  return _NO_ERR;
}

static void fira_niq_app_process_start(void) {
  /* OK, let's start. */
  int r = uwbmac_start(uwbmac_ctx);
  assert(r == UWBMAC_SUCCESS);
  // Start session;
  r = fira_helper_start_session(&fira_ctx, session_id);
  assert(r == UWBMAC_SUCCESS);
  started = true;
}

/*
 * @brief Terminate the niq_fira
 * */

static error_e fira_niq_app_process_terminate(void) {
  if (started)
  {
    // Stop session;
    int r = fira_helper_stop_session(&fira_ctx, session_id);
    assert(!r);
    // Stop.
    uwbmac_stop(uwbmac_ctx);
    // Uninit session;
    r = fira_helper_deinit_session(&fira_ctx, session_id);
    assert(!r);
    fira_helper_close(&fira_ctx);

    // unregister driver;
    fira_uwb_mcps_deinit();

    free(output_result.str);

    started = false;
  }
  return _NO_ERR;
}

__attribute__((unused)) 
static float convert_aoa_2pi_q16_to_deg(int16_t aoa_2pi_q16)
{
	return (360.0 * aoa_2pi_q16 / (1 << 16));
}

/*
 * @brief   Gets the ranging results and print it
 * */

static void report_cb(const struct ranging_results *results, void *user_data) {
  static bool notify = true;
  
  int len = 0;
  int distance_cm;
  struct string_measurement *str_result = (struct string_measurement *)user_data;
  struct ranging_measurements *rm;
  fira_param_t *fira_param = get_fira_config();

  if (results->stopped_reason != 0xFF)
    return;

  session_id = fira_param->session_id;

  len = sprintf(str_result->str, "{\"Block\":%" PRIu32 ", \"results\":[",
                results->block_index);

  for (int i = 0; i < results->n_measurements; i++) 
  {
    if (i > 0) 
    {
      len += snprintf(&str_result->str[len], str_result->len - len, ",");
    }
 
    rm = (struct ranging_measurements *)(&results->measurements[i]);

    len += snprintf(&str_result->str[len], str_result->len - len,
                    "{\"Addr\":\"0x%04x\",\"Status\":\"%s\"",
                    rm->short_addr,
                    (rm->status) ? ("Err") : ("Ok"));

    if (rm->status == 0)
    {
      distance_cm = (int)rm->distance_mm / 10;

      len +=
          snprintf(&str_result->str[len], str_result->len - len, ",\"D_cm\":%d",
                   (int)rm->distance_mm / 10);

      if(hal_uwb.is_aoa() == AOA_ENABLED)
      {
        len += snprintf(&str_result->str[len], str_result->len - len, 
                ",\"LPDoA_deg\":%0.2f,\"LAoA_deg\":%0.2f,\"LFoM\":%d,\"RAoA_deg\":%0.2f",
                    convert_aoa_2pi_q16_to_deg(rm->local_aoa_measurements[0].pdoa_2pi),
                    convert_aoa_2pi_q16_to_deg(rm->local_aoa_measurements[0].aoa_2pi),
                    rm->local_aoa_measurements[0].aoa_fom,
                    convert_aoa_2pi_q16_to_deg(rm->remote_aoa_azimuth_2pi)
            );
      }

      len += snprintf(&str_result->str[len], str_result->len - len,
                      ",\"CFO_100ppm\":%d", (int)fira_uwb_mcps_get_cfo_ppm());

      // Take action based on distance
      if (notify && (distance_cm < MIN_CM_DISTANCE_THRESHOLD))
      {
#if LEDS_NUMBER > 1
          // Visual indication on the device for "inside the bubble"
          bsp_board_led_on(BSP_BOARD_LED_2);  // Red LED On
          bsp_board_led_off(BSP_BOARD_LED_0); // Green LED Off
#endif

        // Send message to trigger notification on the iOS side
        send_ios_notification((uint8_t)distance_cm, "You are in the secure bubble.");
        notify = false;
      }
      else if (!notify && (distance_cm > MAX_CM_DISTANCE_THRESHOLD))
      {
#if LEDS_NUMBER > 1
          // Visual indication on the device for "outside the bubble"
          bsp_board_led_off(BSP_BOARD_LED_2); // Red LED Off
          bsp_board_led_on(BSP_BOARD_LED_0);  // Green LED On
#endif

        // Send message to trigger notification on the iOS side
        send_ios_notification((uint8_t)distance_cm, "You are out of the secure bubble.");
        notify = true;
      }

    }
    len += snprintf(&str_result->str[len], str_result->len - len, "}");
  }

  len += snprintf(&str_result->str[len], str_result->len - len, "]");

  /* Display RSSI, CFO and NLOS */
  if (fira_uwb_is_diag_enabled())
  {
    len = fira_uwb_add_diag(str_result->str, len, str_result->len);
  }

  len += snprintf(&str_result->str[len], str_result->len - len, "}\r\n");
  reporter_instance.print((char*)str_result->str, len);
}

//-----------------------------------------------------------------------------

/* @fn      fira_niq_helper
 * @brief   this is a service function which starts the NIQ fira
 *          top-level  application.
 * */
static void fira_niq_helper(void const *argument) {
  error_e tmp;
  fira_param_t *fira_param = get_fira_config();
  bool controller = fira_config.role & FIRA_ROLE_MASK;

  enter_critical_section(); /**< When the app will setup RTOS tasks, then if
                           task has a higher priority,
                           the kernel will start it immediately, thus we need to
                           stop the scheduler.*/

  hal_uwb.mcu_resume_clk();

  if (controller)
  {
      construct_fira_param_from_config(fira_param, (void*)&fira_config, FIRA_DEVICE_ROLE_INITIATOR);
  }
  else
  {
      construct_fira_param_from_config(fira_param, (void*)&fira_config, FIRA_DEVICE_ROLE_RESPONDER);

  }
  tmp = fira_niq_app_process_init(controller, fira_param);

  if (tmp != _NO_ERR)
  {
    error_handler(1, tmp);
  }

  fira_niq_app_process_start(); /**< IRQ is enabled and it may generate IRQ
                               immediately after this point */

  leave_critical_section(); /**< all RTOS tasks can be scheduled */
}

/* @brief
 *      Kill all task and timers related to niq FiRa
 * */
static void fira_niq_terminate(void) 
{
  fira_niq_app_process_terminate();

  uwbmac_exit(uwbmac_ctx);
  
  hal_uwb.sleep_enter();
  hal_uwb.mcu_suspend_clk();
}

/*
 * @brief Fill the fira_param with config from fira_device configure
 */
static void construct_fira_param_from_config(fira_param_t *fira_param, fira_device_configure_t *config, int controller_type)
{

    /* Session Config */
    fira_param->short_addr = AR2U16(config->SRC_ADDR);
    fira_param->session_id = config->Session_ID;
    fira_param->session.initiation_time_ms = config->UWB_Init_Time_ms;
    fira_config.role = controller_type;

    if (fira_config.role == FIRA_DEVICE_TYPE_CONTROLLER)
    {
        fira_param->session.device_type = FIRA_DEVICE_TYPE_CONTROLLER;
        fira_param->session.device_role = FIRA_DEVICE_ROLE_INITIATOR;
    } 
    else
    {
        fira_param->session.device_type = FIRA_DEVICE_TYPE_CONTROLEE;
        fira_param->session.device_role = FIRA_DEVICE_ROLE_RESPONDER;
    }
    
    /* Only supporting Deferred DS-TWR */
    fira_param->session.ranging_round_usage = config->Ranging_Round_Usage;
    fira_param->session.rframe_config = config->Rframe_Config;
    fira_param->session.sfd_id = (config->SP0_PHY_Set == BPRF_SET_2)?(FIRA_SFD_ID_2):(FIRA_SFD_ID_0);
    fira_param->session.slot_duration_rstu = config->Slot_Duration_RSTU;
    fira_param->session.block_duration_ms = config->Block_Duration_ms;
    fira_param->session.round_duration_slots = 1 + config->Round_Duration_RSTU / config->Slot_Duration_RSTU; /* +1 slot to satisfy corner case when the # of RR is exact the same as the # of slots in TWR. This is fine because hopping is disabled in the NI protocol.*/
    fira_param->session.multi_node_mode = config->Multi_Node_Mode;

    /*FIX The devryption issue*/
    fira_param->session.preamble_duration = FIRA_PREAMBULE_DURATION_64;
    /* Parameters added for the new version of library*/
    fira_param->session.result_report_phase = true;

    fira_param->session.round_hopping = config->Round_Hopping;
    fira_param->session.report_tof = config->ToF_Report;

    /* Without this addresses the tx does not work*/
    fira_param->session.short_addr = AR2U16(config->SRC_ADDR);

    fira_param->session.destination_short_address = AR2U16(config->DST_ADDR);
    fira_param->controlees_params.n_controlees = config->Number_of_Controlee;
    fira_param->controlees_params.controlees[0].address = fira_param->session.destination_short_address;

    fira_param->session.vupper64[7] = config->Vendor_ID[0];
    fira_param->session.vupper64[6] = config->Vendor_ID[1];
    fira_param->session.vupper64[5] = config->Static_STS_IV[0];
    fira_param->session.vupper64[4] = config->Static_STS_IV[1];
    fira_param->session.vupper64[3] = config->Static_STS_IV[2];
    fira_param->session.vupper64[2] = config->Static_STS_IV[3];
    fira_param->session.vupper64[1] = config->Static_STS_IV[4];
    fira_param->session.vupper64[0] = config->Static_STS_IV[5];

    /* Get parameters from global configuration. */
    fira_param->session.channel_number = config->Channel_Number;
    fira_param->session.preamble_code_index = config->Preamble_Code;

	/* Set AoA parameters: set only if UWB chip and Antenna supports AoA */
    if(hal_uwb.is_aoa() == AOA_ENABLED)
    {
        fira_param->session.meas_seq.n_steps = 1;
        fira_param->session.meas_seq.steps[0].type = FIRA_MEASUREMENT_TYPE_AOA_AZIMUTH;
        fira_param->session.meas_seq.steps[0].n_measurements = 1;
        fira_param->session.meas_seq.steps[0].rx_ant_set_nonranging = 0xff;
        fira_param->session.meas_seq.steps[0].rx_ant_sets_ranging[0] = 0xff;
        fira_param->session.meas_seq.steps[0].rx_ant_sets_ranging[1] = 0xff;
        fira_param->session.meas_seq.steps[0].tx_ant_set_nonranging = 0xff;
        fira_param->session.meas_seq.steps[0].tx_ant_set_ranging = 0xff;
        fira_param->session.report_aoa_azimuth = 0; //NI-1.0.0 protocol does not support RAoA report
        fira_param->session.report_aoa_fom = 0;     //NI-1.0.0 protocol does not support RAoA report
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

    /* set sts parameters in the session*/
    fira_param->session.number_of_sts_segments = FIRA_STS_SEGMENTS_1;
    fira_param->session.sts_length = FIRA_STS_LENGTH_64;
}


const app_definition_t helpers_fira_niq[] __attribute__((section(".known_apps"))) = 
{
    {"NIQ", mAPP, fira_niq_helper,  fira_niq_terminate}
};
/*
 * @brief callback used for niq lib to signal session start
 */
void ResumeUwbTasks(void)
{
    const app_definition_t *app_ptr = &helpers_fira_niq[0];
    EventManagerRegisterApp(&app_ptr);
}

/*
 * @brief callback used for niq lib to signal session stop
*/
void StopUwbTask(void)
{
    const app_definition_t *app_ptr = &idle_app[0];
    EventManagerRegisterApp(&app_ptr);
}