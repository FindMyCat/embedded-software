// Libraries
#include <date_time.h>
#include <modem/location.h>
#include <modem/lte_lc.h>
#include <modem/nrf_modem_lib.h>
#include <nrf_modem_at.h>
#include <stdbool.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/led.h>

#include <zephyr/sys/atomic.h>

// Application specific imports
#include "Diagnostics/diagnostics.h"
#include "Dispatcher/Dispatcher.h"
#include "FuelGauge/fuel_gauge.h"
#include "LTE/lte.h"
#include "LoRa/lora_link.h"
#include "LoRa/rak3172.h"
#include "LocationEngine/LocationEngine.h"
#include "UDPListener/UDPListener.h"
#include "WiFi/wifi_location.h"
#include "WiFi/wifi_scan.h"
#include "communication_channels/nrf52_link.h"
#include "led/led.h"
#include "mqtt/mqttsn.h"

LOG_MODULE_REGISTER(main, CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

/* Indoor location engine sends 1 for PSM mode and 0 for eDRX mode */
#define POWER_MODE_COMMAND_PIN DT_ALIAS(gpiocus10)

static const struct gpio_dt_spec power_mode_command_recv_pin =
    GPIO_DT_SPEC_GET_OR(POWER_MODE_COMMAND_PIN, gpios, {0});

#define NRF52_CONFIG_READY_PIN DT_ALIAS(nrf52_config_ready)
static const struct gpio_dt_spec nrf52_config_ready_pin =
    GPIO_DT_SPEC_GET_OR(NRF52_CONFIG_READY_PIN, gpios, {0});

static struct fmc_config fmc_config_nrf52;
static struct k_work_delayable fmc_nrf52_config__work;
static struct gpio_callback fmc_config_nrf52_cb_data;

/**
 * @brief Initialize location services.
 */
static int initialize_location_services() {
  /* A-GPS/P-GPS needs to know the current time. */
  if (IS_ENABLED(CONFIG_DATE_TIME)) {
    LOG_INF("Waiting for current time");

    /* Wait for an event from the Date Time library. */
    lte_wait_for_time(K_MINUTES(10));

    if (!date_time_is_valid()) {
      LOG_INF("Failed to get current time. Continuing anyway.");
    }
  }

  int err = location_ensure_init();
  if (err) {
    LOG_INF("Initializing the Location library failed, error: %d", err);
    return -1;
  }

  return err;
}

#define POWER_MODE_COMMAND_DEBOUNCE_TIME 50
uint32_t last_powermode_command_received_timestamp;

static atomic_t homestation_in_ble_range = ATOMIC_INIT(0);

static void set_homestation_in_ble_range(bool in_range) {
  atomic_set(&homestation_in_ble_range, in_range ? 1 : 0);

#if defined(CONFIG_LORA_LINK)
  lora_link_set_enabled(!in_range);
#endif
}
/**
 * @brief Callback function for when the power mode command is received through
 * gpio interrupt.
 */
static void powermode_command_recieved__cb() {
  k_sleep(K_MSEC(100));
  int power_mode_command = gpio_pin_get_dt(&power_mode_command_recv_pin);
  uint32_t command_recv_time = k_uptime_get_32();

  if (k_uptime_get_32() - last_powermode_command_received_timestamp <
      POWER_MODE_COMMAND_DEBOUNCE_TIME) {
    return;
  }

  if (power_mode_command == 0) {
    LOG_INF("Power mode set to eDRX");
    lte_lc_psm_req(false);
    lte_lc_edrx_req(true);

  } else if (power_mode_command == 1) {
    LOG_INF("Power mode set to PSM");
    lte_lc_psm_req(true);
    lte_lc_edrx_req(false);
  }

  set_homestation_in_ble_range(power_mode_command == 1);

  last_powermode_command_received_timestamp = command_recv_time;
}

static struct gpio_callback pwr_mode_cb_data;

#define NRF52_CONFIG_MAX_RETRIES 5

static int config_retries;

static void nrf52_config_work_handler(struct k_work *work) {
  ARG_UNUSED(work);

  (void)fuel_gauge_sample_and_push();

  /* On a transient failure, retry while the data-ready line is still asserted:
   * the nRF52 holds it high until we ack, so a held-high line means "still
   * pending" and there is no fresh edge to re-trigger us. */
  if (nrf52_read_config(&fmc_config_nrf52) != 0) {
    if (gpio_pin_get_dt(&nrf52_config_ready_pin) > 0 &&
        ++config_retries < NRF52_CONFIG_MAX_RETRIES) {
      k_work_reschedule(&fmc_nrf52_config__work, K_MSEC(100));
    } else {
      LOG_WRN("nRF52 config unreadable after %d tries", config_retries);
    }
    return;
  }

  config_retries = 0;

  initialize_mqttsn_config(fmc_config_nrf52.device_id, fmc_config_nrf52.host);
#if defined(CONFIG_LORA_LINK)
  lora_link_set_device_id(fmc_config_nrf52.device_id);
#endif
}

/**
 * @brief ISR for the nRF52 config-ready line; defers the i2c read to a
 * workitem.
 */
static void nrf52_config_ready_isr(const struct device *port,
                                   struct gpio_callback *cb, uint32_t pins) {
  ARG_UNUSED(port);
  ARG_UNUSED(cb);
  ARG_UNUSED(pins);
  config_retries = 0;
  k_work_schedule(&fmc_nrf52_config__work, K_NO_WAIT);
}

/**
 * @brief Application entry point.
 */
int main(void) {

  LOG_INF("*******************************");
  LOG_INF("Welcome to FindMyCat v3.0 Preview");
  LOG_INF("*******************************");

  int err;
  int retry_count = 0;

  err = nrf_modem_lib_init();
  if (err) {
    LOG_ERR("Error: Failed to initialize nrf modem library, error: %d", err);
    return -1;
  }

  err = rak3172_init();
  if (err != 0) {
    LOG_ERR(
        "Error: RAK3172 LoRa initializing failed (%d), continuing without LoRa",
        err);
  }

  boot_led_blink();

#if defined(CONFIG_FINDMYCAT_DIAGNOSTICS) &&                                   \
    !defined(CONFIG_FINDMYCAT_LOW_POWER_HOLD)
  onboard_diagnostics_run();
#endif

retry:
  if (!device_is_ready(power_mode_command_recv_pin.port)) {
    LOG_ERR("Error: Device %s is not ready",
            power_mode_command_recv_pin.port->name);
    goto retry;
  }

  lte_lc_power_off();
  err = gpio_pin_configure_dt(&power_mode_command_recv_pin, GPIO_INPUT);
  if (err != 0) {
    LOG_ERR("Error %d: failed to configure %s pin %d", err,
            power_mode_command_recv_pin.port->name,
            power_mode_command_recv_pin.pin);
    goto retry;
  }

  err = gpio_pin_interrupt_configure_dt(&power_mode_command_recv_pin,
                                        GPIO_INT_EDGE_BOTH);
  if (err != 0) {
    LOG_ERR("Error %d: failed to configure interrupt on %s pin %d", err,
            power_mode_command_recv_pin.port->name,
            power_mode_command_recv_pin.pin);
    return -1;
  }

  gpio_init_callback(&pwr_mode_cb_data, powermode_command_recieved__cb,
                     BIT(power_mode_command_recv_pin.pin));
  gpio_add_callback(power_mode_command_recv_pin.port, &pwr_mode_cb_data);

  /* nRF52 bus-window line: 52 drives it high on its beacon and whenever the
   * iOS app pushes new config. High means its I2C slave is powered up. */
  if (device_is_ready(nrf52_config_ready_pin.port)) {
    k_work_init_delayable(&fmc_nrf52_config__work, nrf52_config_work_handler);
    gpio_pin_configure_dt(&nrf52_config_ready_pin, GPIO_INPUT | GPIO_PULL_DOWN);
    gpio_pin_interrupt_configure_dt(&nrf52_config_ready_pin,
                                    GPIO_INT_EDGE_TO_ACTIVE);
    gpio_init_callback(&fmc_config_nrf52_cb_data, nrf52_config_ready_isr,
                       BIT(nrf52_config_ready_pin.pin));
    gpio_add_callback(nrf52_config_ready_pin.port, &fmc_config_nrf52_cb_data);

    if (gpio_pin_get_dt(&nrf52_config_ready_pin) > 0) {
      k_work_schedule(&fmc_nrf52_config__work, K_NO_WAIT);
    }
  } else {
    LOG_ERR("Error: nRF52 config-ready pin not ready");
  }

// #if !defined(CONFIG_FINDMYCAT_LOW_POWER_HOLD)
//   err = fuel_gauge_start();
//   if (err != 0) {
//     LOG_ERR("Error: Fuel gauge start failed (%d)", err);
//   }
// #endif

  set_homestation_in_ble_range(gpio_pin_get_dt(&power_mode_command_recv_pin) ==
                               1);

#if defined(CONFIG_FINDMYCAT_LOW_POWER_HOLD)
  diagnostics_low_power_hold();
  return 0;
#endif


#if defined(CONFIG_LORA_LINK)
  err = lora_link_start();
  if (err != 0) {
    LOG_ERR("Error: LoRa link start failed (%d), pings arrive over LTE only",
            err);
  }
#endif

  // Connect to LTE network with retries if necessary.
  // while (retry_count < CONFIG_LTE_CONNECT_MAX_RETRIES) {
  //   err = lte_connect();
  //
  //   if (err) {
  //     LOG_INF("Failed to connect to LTE with error: %d", err);
  //     retry_count++;
  //     k_sleep(K_MSEC(CONFIG_LTE_CONNECT_RETRY_DELAY_S * 1000));
  //   } else {
  //     break;
  //   }
  // }
  //
  // if (retry_count == CONFIG_LTE_CONNECT_MAX_RETRIES) {
  //   LOG_ERR("Max retry limit reached for connecting to LTE, program exiting.");
  //   return -1;
  // }
  //
  // retry_count = 0;
  // // Initialize location services with retries if necessary.
  // while (retry_count < CONFIG_LOCATION_SERVICES_INIT_MAX_RETRIES) {
  //   err = initialize_location_services();
  //
  //   if (err) {
  //     LOG_INF("Failed to initialize location services with error: %d", err);
  //     retry_count++;
  //     k_sleep(K_MSEC(CONFIG_LOCATION_SERVICES_INIT_RETRY_DELAY_S * 1000));
  //   } else {
  //     break;
  //   }
  // }
  //
  // if (retry_count == CONFIG_LTE_CONNECT_MAX_RETRIES) {
  //   LOG_ERR("Max retry limit reached for initializing location services, "
  //           "program exiting.");
  //   return -1;
  // }
  //
  // retry_count = 0;
  // // Initialize UDP listener with retries (using the same retry count as
  // // location services)
  // while (retry_count < CONFIG_LOCATION_SERVICES_INIT_MAX_RETRIES) {
  //   err = create_udp_listener();
  //
  //   if (err) {
  //     LOG_INF("Failed to create UDP Listener with error: %d", err);
  //     destroy_udp_listener();
  //     retry_count++;
  //     k_sleep(K_MSEC(CONFIG_LOCATION_SERVICES_INIT_RETRY_DELAY_S * 1000));
  //   } else {
  //     LOG_INF("Created UDP Listener.");
  //     break;
  //   }
  // }
  //
  // // Start the UDP listener thread.
  // start_udp_listener_thread();
  // //
  int tick = 0;

  while (true) {
#if defined(CONFIG_LORA_LINK)
    bool home = atomic_get(&homestation_in_ble_range) != 0 ||
                lora_link_homestation_in_range();

    LOG_INF("Main thread is running. Home station %s",
            home ? "in range" : "out of range");

    if (++tick % 8 == 0) {
      // lora_link_log_stats();
    }
#else
    LOG_INF("Main thread is running.");
#endif
    /* A heartbeat, so it pays for a wake every time it prints. Presence changes
     * are logged from the wake line in lora_link.c as they happen; this only
     * has to say the thread is alive and what the current answer is. */
    k_sleep(K_MSEC(30000));
  }

  return 0;
}
