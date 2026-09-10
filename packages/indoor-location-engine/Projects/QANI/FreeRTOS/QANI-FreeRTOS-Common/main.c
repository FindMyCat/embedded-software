/**
 * @file      main.c
 * 
 * @brief     Implementation of the Qorvo Apple Nearby Interaction example
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
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "EventManager.h"
#include "cmsis_os.h"
#include "defaultTask.h"
#include "fira_niq.h"
#include "niq.h"
#include "nrf_crypto_init.h"
#include "nrf_crypto_rng.h"
#include "nrf_log_ctrl.h"
#include "SEGGER_RTT.h"
#include "nrf_drv_clock.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "reporter.h"
#include "rtls_version.h"
#include "util.h"

#include "HAL_uwb.h"
#include "deca_dbg.h"
#include "niq.h"
#include "thisBoard.h"
#include "uwbmac.h"
#ifdef SOFTDEVICE_PRESENT
#include "nrf_sdh.h"
#endif


#include "nrf9160_power_mode_manager.h"
#include "buzzer.h"
#include "i2c_nrf9151.h"
#include "persistent_config.h"

#ifndef ACCESSORY_RANGING_ROLE
#define ACCESSORY_RANGING_ROLE (1) /**< Responder 0, Initiator 1 */
#endif

extern void ble_init(char *gap_name);
#if NRF_LOG_ENABLED
void init_logger_thread();
#endif // NRF_LOG_ENABLED

extern const char ApplicationName[]; /**< Name of Application release. */
extern const char OsName[];
extern const char BoardName[]; /**< Name of Target. Indicated in the advertising data. */

#define DEAD_BEEF 0xDEADBEEF /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#if NRF_LOG_ENABLED
static TaskHandle_t m_logger_thread; /**< Definition of Logger thread. */
#endif

/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in]   line_num   Line number of the failing ASSERT call.
 * @param[in]   file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t *p_file_name) {
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Function for initializing the clock.
 */
static void clock_init(void) {
    ret_code_t err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for application main entry.
 */

int main(void) {
    if ((NRF_UICR->APPROTECT & UICR_APPROTECT_PALL_Msk) != UICR_APPROTECT_PALL_HwDisabled) {
        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {}
        NRF_UICR->APPROTECT = 0xFFFFFF5AUL;
        __DSB();
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {}
        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {}
        NVIC_SystemReset();
    }
    NRF_APPROTECT->DISABLE = APPROTECT_DISABLE_DISABLE_SwDisable;

/* Floor-current measurement hold. Set to 1 to park every pin the board can leak
 * through and enter System OFF, so the PPK reads the static floor of the indoor
 * rails. Off, main continues into the normal application init below. Pins are
 * from the IndoorLocation schematic, UWB and BLE sheets. */
#define INDOOR_FLOOR_CURRENT_HOLD 0

#if INDOOR_FLOOR_CURRENT_HOLD
    nrf_delay_ms(5000); /* SWD reattach window before the chip stops answering */

    /* Hold DW3000 RSTn asserted. Released, the chip free-runs in IDLE_RC at
     * milliamps because nothing here runs uwb_init() to deep-sleep it. Driving
     * this pin low is the reset function; only driving it high is forbidden.
     * nRF52 System OFF retains output levels, so it stays held. */
    nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(0, 8));
    nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(0, 8));
    /* IRQ and MISO: buffer disconnected with a pull-down. nrf_gpio_cfg_input()
     * would leave the buffer connected, and PIN_CNF survives System OFF, so a
     * floating pin sits at mid-rail and draws shoot-through for good. With no
     * DWM3000 fitted there is nothing external holding either line. */
    nrf_gpio_cfg(NRF_GPIO_PIN_MAP(0, 17), NRF_GPIO_PIN_DIR_INPUT,
		 NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_PULLDOWN,
		 NRF_GPIO_PIN_S0S1, NRF_GPIO_PIN_NOSENSE);
    nrf_gpio_cfg(NRF_GPIO_PIN_MAP(0, 22), NRF_GPIO_PIN_DIR_INPUT,
		 NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_PULLDOWN,
		 NRF_GPIO_PIN_S0S1, NRF_GPIO_PIN_NOSENSE);

    /* WAKEUP and CS both wake the DW3000 out of sleep if they float, and
     * neither has an external pull. Drive them to the inactive level. */
    nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(0, 19));
    nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(0, 19));
    nrf_gpio_pin_set(NRF_GPIO_PIN_MAP(0, 20));
    nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(0, 20));

    nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(0, 21)); /* SPI MOSI */
    nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(0, 21));
    nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(0, 23)); /* SPI SCK */
    nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(0, 23));

    /* Q2 gate on the buzzer FET. A floating gate partially conducts off VSYS. */
    nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(0, 27));
    nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(0, 27));

    /* J10 lines into the nRF9151. Both are inputs over there, so drive them low
     * rather than leaving the far side floating. */
    nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(0, 5)); /* config ready */
    nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(0, 5));
    /* P0.16 is LED4 on the nRF52833 DK and the LEDs are active low, so driving
     * it costs milliamps there. Restore for the indoor board. */
    /* nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(0, 16)); */ /* NRF_91_CELL_MODE */
    /* nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(0, 16)); */

    /* I2C SDA P0.26 and SCL P0.04 stay Hi-Z: the outdoor board has 10k pull-ups
     * on both, so driving either low would cost 330uA per line. */

    NRF_POWER->SYSTEMOFF = 1;
    for (;;) {}
#endif /* INDOOR_FLOOR_CURRENT_HOLD */

    // Initialize modules.
    clock_init();
    reporter_instance.init();

    hal_uwb.mcu_sleep_config();

#if NRF_LOG_ENABLED
    init_logger_thread();
#endif

    buzzer_init();
    buzzer_play(BUZZER_TUNE_BOOT);

    i2c_nrf9151_init();
    persistent_config_init(); /* register FDS handler before ble_init runs fds_init */

    // Accessory Nearby Interaction Initialization
    niq_init(ResumeUwbTasks, StopUwbTask, (const void *)nrf_crypto_init,
             (const void *)nrf_crypto_uninit,
             (const void *)nrf_crypto_rng_vector_generate);

    // Accessory instructed to act as a Responder or Initiator
    niq_set_ranging_role(ACCESSORY_RANGING_ROLE);

    // Create Default task: it responsible for WDT and starting of "applications"
    AppConfigInit(); /**< load the RAM Configuration parameters from NVM block */

    // Start BLE 
    char advertising_name[4];

    snprintf(advertising_name, sizeof(advertising_name), "%s", (char*)BoardName);
    ble_init(advertising_name);

    EventManagerInit();
    BoardInit();
    if (uwb_init() != DWT_SUCCESS) {
        diag_printf("UWB probe failed, continuing without ranging\r\n");
    }
    DefaultTaskInit();
   
    power_mode_manager_thread_init();
    // Driver version is available after probing of the DW chip
    const char ver[]    = FULL_VERSION;
    const char *drv_ver = dwt_version_string();
    const char *mac_ver = uwbmac_get_version();

    char str[256];
    int sz;

    sz  = sprintf(str, "Application: %s\r\n", ApplicationName);
    sz += sprintf(&str[sz], "BOARD: %s\r\n", BoardName);
    sz += sprintf(&str[sz], "OS: %s\r\n", OsName);
    sz += sprintf(&str[sz], "Version: %s\r\n", ver);
    sz += sprintf(&str[sz], "%s\r\n", drv_ver);
    sz += sprintf(&str[sz], "MAC: %s\r\n", mac_ver);
    sz += sprintf(&str[sz], "ACCESSORY_RANGING_ROLE: %s\r\n", (ACCESSORY_RANGING_ROLE) ? "Initiator" : "Responder");
    reporter_instance.print(str, sz);

    // Start FreeRTOS scheduler.
    osKernelStart();

    for (;;) {
        APP_ERROR_HANDLER(NRF_ERROR_FORBIDDEN);
    }
}


#if NRF_LOG_ENABLED
/**@brief Thread for handling the logger.
 *
 * @details This thread is responsible for processing log entries if logs are
 * deferred. Thread flushes all log entries and suspends. It is resumed by idle
 * task hook.
 *
 * @param[in]   arg   Pointer used for passing some arbitrary information
 * (context) from the osThreadCreate() call to the thread.
 */
static void logger_thread(void *arg) {
    UNUSED_PARAMETER(arg);

    while (1) {
        NRF_LOG_FLUSH();

        vTaskSuspend(NULL); // Suspend myself
    }
}
void init_logger_thread() {
    // Logging task is only required if NRF_LOG module is in use
    if (pdPASS != xTaskCreate(logger_thread, "logger", 256, NULL, 1, &m_logger_thread)) {
        APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
    }
}

void vApplicationIdleHook(void) {
#ifdef SOFTDEVICE_PRESENT
            if (nrf_sdh_is_enabled())
            {
                uint32_t err_code = sd_app_evt_wait();
                APP_ERROR_CHECK(err_code);
            }
            else
#endif
            {
               __WFI();
            }

    vTaskResume(m_logger_thread);
}

#endif // NRF_LOG_ENABLED
