#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf9160_power_mode_manager.h"
#include "FreeRTOS.h"
#include "task.h"

#define COMMAND_NRF9160_PIN     NRF_GPIO_PIN_MAP(0,4)
#define POWER_COMMAND_PSM       1
#define POWER_COMMAND_EDRX      0

static long home_station_last_seen_timestamp = 0;
static TaskHandle_t m_nrf9160_power_manager_thread; /**< Definition of nrf9160 Power Manager thread. */


void command_nrf9160_to_psm() {
    NRF_LOG_INFO("Commanding nrf9160 to PSM");
    nrf_gpio_pin_write(COMMAND_NRF9160_PIN, POWER_COMMAND_PSM);
}

void command_nrf9160_to_edrx() {
    NRF_LOG_INFO("Commanding nrf9160 to EDRX");
    nrf_gpio_pin_write(COMMAND_NRF9160_PIN, POWER_COMMAND_EDRX);
}

void init_gpio_command_pins() {
    NRF_LOG_INFO("Initializing GPIO pins");
    // Initialize GPIO pins
    nrf_gpio_cfg_output(COMMAND_NRF9160_PIN);	
}

void set_home_station_last_seen_timestamp(long timestamp) {
    home_station_last_seen_timestamp = timestamp;
}

long get_home_station_last_seen_timestamp() {
    return home_station_last_seen_timestamp;
}


static void nrf9160_power_manager_thread(void *arg) {
    UNUSED_PARAMETER(arg);
    bool on = true;
    while (1) {

        NRF_LOG_INFO("nrf9160_power_manager_thread: In loop");
        // sleep 1 second
        vTaskDelay(1000);
        // vTaskSuspend(NULL); // Suspend myself
        if (on)
        {
            command_nrf9160_to_edrx();
        } else {
            command_nrf9160_to_psm();
        }

        on = !on;
        
    }
}

void power_mode_manager_thread_init() {
    init_gpio_command_pins();

    if (pdPASS != xTaskCreate(
        nrf9160_power_manager_thread, 
        "nrf9160 pwr mngr", 
        256, 
        NULL, 
        1, &m_nrf9160_power_manager_thread)) {
        APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
    }
}