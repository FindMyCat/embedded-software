#include "nrf91_power_mode_manager.h"
#include "FreeRTOS.h"
#include "app_ble.h"
#include "app_timer.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "task.h"

// Constants
#define COMMAND_NRF91_PIN NRF_GPIO_PIN_MAP(0, 16)
#define POWER_COMMAND_PSM 1
#define POWER_COMMAND_EDRX 0

APP_TIMER_DEF(timer);
int TWO_MINUTES = 120;

// Holder variables
static long home_station_last_seen_timestamp_s = 0;
static TaskHandle_t m_nrf91_power_manager_thread; /**< Definition of nrf91 Power
                                                     Manager thread. */
uint32_t system_uptime = 0;

/* Timer callback that increments the uptime variable */
void timer_handler(void *p_context) { system_uptime++; }

/* Function to command nrf91 to PSM mode */
void command_nrf91_to_psm() {
  NRF_LOG_INFO("Commanding nrf91 to PSM");
  nrf_gpio_pin_write(COMMAND_NRF91_PIN, POWER_COMMAND_PSM);
}

/* Function to command nrf91 to EDRX mode */
void command_nrf91_to_edrx() {
  NRF_LOG_INFO("Commanding nrf91 to EDRX");
  nrf_gpio_pin_write(COMMAND_NRF91_PIN, POWER_COMMAND_EDRX);
}

/* Function to initialize the GPIO pins used to command the nrf91 to PSM or
 * EDRX */
void init_gpio_command_pins() {
  NRF_LOG_INFO("Initializing nrf91 command pin(s)");
  NRF_LOG_INFO("COMMAND_NRF91_PIN = %u (P%u.%u)", COMMAND_NRF91_PIN,
               COMMAND_NRF91_PIN >> 5, COMMAND_NRF91_PIN & 0x1F);

  nrf_gpio_cfg_output(COMMAND_NRF91_PIN);

  // explicity command the nrf91 to PSM upon startup
  command_nrf91_to_psm();
}

/* Function to set the last time the home station was seen */
void set_home_station_last_seen_timestamp(long timestamp) {
  home_station_last_seen_timestamp_s = timestamp;
}

/* Function to get the last time the home station was seen */
long get_home_station_last_seen_timestamp() {
  return home_station_last_seen_timestamp_s;
}

void home_station_seen() { home_station_last_seen_timestamp_s = system_uptime; }

/* nrf91 power manager thread */
static void nrf91_power_manager_thread(void *arg) {
  UNUSED_PARAMETER(arg);

  app_timer_init();
  app_timer_create(&timer, APP_TIMER_MODE_REPEATED, timer_handler);
  app_timer_start(timer, APP_TIMER_TICKS(1000), NULL);

  while (1) {
    // sleep 1 second
    vTaskDelay(1000);

    // check if we have seen the home station recently
    // Scan duration is every 30 seconds,
    // so if we haven't seen the home station in 2 minutes,
    // then we should switch to EDRX mode
    if (ble_is_connected()) {
      command_nrf91_to_psm();
    } else if (system_uptime - home_station_last_seen_timestamp_s >
               TWO_MINUTES) {
      command_nrf91_to_edrx();
    } else {
      command_nrf91_to_psm();
    }
  }
}

void power_mode_manager_thread_init() {
  init_gpio_command_pins();

  if (pdPASS != xTaskCreate(nrf91_power_manager_thread, "nrf91 pwr mngr",
                            256, NULL, 1, &m_nrf91_power_manager_thread)) {
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
  }
}
