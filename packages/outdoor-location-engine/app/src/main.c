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

#include <zephyr/dfu/mcuboot.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/hwinfo.h>
#include <zephyr/drivers/led.h>

#include <zephyr/sys/atomic.h>

// Application specific imports
#include "Diagnostics/diagnostics.h"
#include "Dispatcher/Dispatcher.h"
#include "FOTA/fota.h"
#include "FuelGauge/fuel_gauge.h"
#include "LTE/lte.h"
#include "LoRa/lora_link.h"
#include "LoRa/rak3172.h"
#include "LocationEngine/LocationEngine.h"
#include "Presence/presence.h"
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

static struct fmc_config fmc_config_nrf52;
static struct k_work_delayable fmc_nrf52_config__work;

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

#define POWER_MODE_COMMAND_DEBOUNCE_MS 100

static struct k_work_delayable power_mode_work;

/* Low (eDRX) means the home station is out of BLE range, so CAD runs. */
static void power_mode_work_handler(struct k_work *work) {
  ARG_UNUSED(work);

  int power_mode_command = gpio_pin_get_dt(&power_mode_command_recv_pin);

  /* Forced LTE stays in eDRX: PSM would leave it unreachable for pings. */
  if (IS_ENABLED(CONFIG_FINDMYCAT_FORCE_LTE_LINK)) {
    presence_set_ble(power_mode_command == 1);
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

  presence_set_ble(power_mode_command == 1);
}

/* ISR: each edge pushes the read out, so it lands on the settled level. */
static void powermode_command_recieved__cb(const struct device *port,
                                           struct gpio_callback *cb,
                                           uint32_t pins) {
  ARG_UNUSED(port);
  ARG_UNUSED(cb);
  ARG_UNUSED(pins);
  k_work_reschedule(&power_mode_work, K_MSEC(POWER_MODE_COMMAND_DEBOUNCE_MS));
}

static struct gpio_callback pwr_mode_cb_data;

#define NRF52_CONFIG_MAX_RETRIES 5

static int config_retries;

static void nrf52_config_work_handler(struct k_work *work) {
  ARG_UNUSED(work);

  (void)fuel_gauge_push();

  /* On a transient failure, retry while the data-ready line is still asserted:
   * the nRF52 holds it high until we ack, so a held-high line means "still
   * pending" and there is no fresh edge to re-trigger us. */
  int ret = nrf52_read_config(&fmc_config_nrf52);
  if (ret == -ENODATA) {
    return;
  }
  if (ret != 0) {
    if (nrf52_link_requested() &&
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
 * @brief The 52 raised the request line; defers the i2c work to a workitem.
 */
static void nrf52_request_isr(void) {
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

  k_work_init_delayable(&fmc_nrf52_config__work, nrf52_config_work_handler);
  err = nrf52_link_init(nrf52_request_isr);
  if (err != 0) {
    LOG_ERR("Error: nRF52 link init failed (%d)", err);
  }

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

  /* Credentials can only be written with the modem offline, as it is here. */
  if (fota_provision_ca() != 0) {
    LOG_ERR("Error: update certificates not provisioned, updates will fail");
  }

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

  k_work_init_delayable(&power_mode_work, power_mode_work_handler);
  gpio_init_callback(&pwr_mode_cb_data, powermode_command_recieved__cb,
                     BIT(power_mode_command_recv_pin.pin));
  gpio_add_callback(power_mode_command_recv_pin.port, &pwr_mode_cb_data);

  err = fuel_gauge_start();
  if (err != 0) {
    LOG_ERR("Error: Fuel gauge start failed (%d)", err);
  }

  /* Init got this far, so keep this image. An update that crashes before
   * here is never confirmed, and MCUboot rolls back to the old one on reset. */
  if (!boot_is_img_confirmed()) {
    err = boot_write_img_confirmed();
    LOG_INF("Firmware image confirmed (%d)", err);
  }

#if defined(CONFIG_FINDMYCAT_LOW_POWER_HOLD)
  diagnostics_low_power_hold();
  return 0;
#endif

  presence_set_ble(gpio_pin_get_dt(&power_mode_command_recv_pin) == 1);


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
    enum presence_link link = presence_link_get();

    LOG_INF("Main thread is running. Link = %s%s", presence_link_name(link),
            link == PRESENCE_LINK_LTE ? ". Home station absent" : "");

#if defined(CONFIG_LORA_LINK)
    if (++tick % 8 == 0) {
      // lora_link_log_stats();
    }
#endif
    /* A heartbeat, so it pays for a wake every time it prints. Presence changes
     * are logged from the wake line in lora_link.c as they happen; this only
     * has to say the thread is alive and what the current answer is. */
    k_sleep(K_MSEC(30000));
  }

  return 0;
}
