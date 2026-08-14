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

// Application specific imports
#include "comm/nrf52_link.h"
#include "Dispatcher/Dispatcher.h"
#include "FuelGauge/fuel_gauge.h"
#include "LoRa/rak3172.h"
#include "WiFi/wifi_location.h"
#include "WiFi/wifi_scan.h"
#include "LocationEngine/LocationEngine.h"
#include "Responder/Responder.h"
#include "UDPListener/NewUDPListener.h"

LOG_MODULE_REGISTER(main, 4);

/* Indoor location engine sends 1 for PSM mode and 0 for eDRX mode */
#define POWER_MODE_COMMAND_PIN DT_ALIAS(gpiocus10)

static const struct gpio_dt_spec power_mode_command_recv_pin =
    GPIO_DT_SPEC_GET_OR(POWER_MODE_COMMAND_PIN, gpios, {0});

/* RAK3172 raises this line for a buffered packet or when the home station has
 * gone silent; AT+CADEVT says which. */
static const struct gpio_dt_spec lora_wake_pin =
    GPIO_DT_SPEC_GET_OR(DT_ALIAS(lora_wake), gpios, {0});

/* nRF52 raises this line when the iOS app has pushed new config over BLE */
#define NRF52_CONFIG_READY_PIN DT_ALIAS(nrf52_config_ready)
static const struct gpio_dt_spec nrf52_config_ready_pin =
    GPIO_DT_SPEC_GET_OR(NRF52_CONFIG_READY_PIN, gpios, {0});

static struct fmc_config nrf52_config;
static struct k_work_delayable nrf52_config_work;
static struct gpio_callback nrf52_config_cb_data;

static K_SEM_DEFINE(lte_connected, 0, 1);

static K_SEM_DEFINE(time_update_finished, 0, 1);

static void date_time_evt_handler(const struct date_time_evt *evt) {
  k_sem_give(&time_update_finished);
}

static void lte_event_handler(const struct lte_lc_evt *const evt) {
  LOG_INF("************************************* LTE event: %d", evt->type);
  switch (evt->type) {
  case LTE_LC_EVT_NW_REG_STATUS:
    if ((evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_HOME) ||
        (evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_ROAMING)) {
      LOG_INF("Connected to LTE");
      k_sem_give(&lte_connected);
    }
    break;
  default:
    break;
  }
}

/**
 * @brief Initialize LTE and wait for connection.
 */
static int initializeLte() {
  int err;

  if (IS_ENABLED(CONFIG_DATE_TIME)) {
    /* Registering early for date_time event handler to avoid missing
     * the first event after LTE is connected.
     */
    date_time_register_handler(date_time_evt_handler);
  }

  LOG_INF("Connecting to LTE...");

  lte_lc_register_handler(lte_event_handler);

  /* Enable PSM. */
  /* Turn on LTE power saving features. Also,
   * request power saving features before network registration. Some
   * networks rejects timer updates after the device has registered to the
   * LTE network.
   */
  lte_lc_psm_req(false);
  /** enhanced Discontinuous Reception */

  LOG_INF("Requesting EDRX mode");
  // Todo: Turn on EDRX mode after development complete.
  err = lte_lc_edrx_req(true);

  if (err) {
    LOG_INF("lte_lc_edrx_req, error: %d\n", err);
  }

  err = lte_lc_connect();

  k_sem_take(&lte_connected, K_FOREVER);

  return err;
}

/**
 * @brief Initialize location services.
 *
 * DOES NOT call method to request location, only initializes the library.
 */
static int initializeLocation() {
  /* A-GPS/P-GPS needs to know the current time. */
  if (IS_ENABLED(CONFIG_DATE_TIME)) {
    LOG_INF("Waiting for current time");

    /* Wait for an event from the Date Time library. */
    k_sem_take(&time_update_finished, K_MINUTES(10));

    if (!date_time_is_valid()) {
      LOG_INF("Failed to get current time. Continuing anyway.");
    }
  }

  int err = location_init(location_event_handler);
  if (err) {
    LOG_INF("Initializing the Location library failed, error: %d", err);
    return -1;
  }

  return err;
}

#define BUTTON_DEBOUNCE_TIME 50
uint32_t last_button_press_time;
/**
 * @brief Callback function for when the power mode command is received through
 * gpio interrupt.
 */
static void power_mode_command_recieved() {
  k_sleep(K_MSEC(100));
  int power_mode_command = gpio_pin_get_dt(&power_mode_command_recv_pin);
  uint32_t newPressTime = k_uptime_get_32();

  if (k_uptime_get_32() - last_button_press_time < BUTTON_DEBOUNCE_TIME) {
    return;
  }

  if (power_mode_command == 0) {
    LOG_INF("Power mode set to eDRX");
    // lte_lc_psm_req(false);
    lte_lc_edrx_req(true);

  } else if (power_mode_command == 1) {
    LOG_INF("Power mode set to PSM");
    lte_lc_psm_req(true);
    lte_lc_edrx_req(false);
  }

  last_button_press_time = newPressTime;
}

static struct gpio_callback pwr_mode_cb_data;

/**
 * @brief Read the config the nRF52 published. Runs on the system workqueue.
 */
static void nrf52_config_work_handler(struct k_work *work) {
  ARG_UNUSED(work);
  /* On a transient failure, retry while the data-ready line is still asserted:
   * the nRF52 holds it high until we ack, so a held-high line means "still
   * pending" and there is no fresh edge to re-trigger us. */
  if (nrf52_read_config(&nrf52_config) != 0 &&
      gpio_pin_get_dt(&nrf52_config_ready_pin) > 0) {
    k_work_reschedule(&nrf52_config_work, K_MSEC(100));
  }
}

/**
 * @brief ISR for the nRF52 config-ready line; defers the i2c read to a workitem.
 */
static void nrf52_config_ready_isr(const struct device *port,
                                   struct gpio_callback *cb, uint32_t pins) {
  ARG_UNUSED(port);
  ARG_UNUSED(cb);
  ARG_UNUSED(pins);
  k_work_schedule(&nrf52_config_work, K_NO_WAIT);
}

/**
 * @brief Application entry point.
 */
int main(void) {

  LOG_INF("*******************************");
  LOG_INF("Welcome to FindMyCat v2.1");
  LOG_INF("*******************************");

retry:
  if (!device_is_ready(power_mode_command_recv_pin.port)) {
    LOG_ERR("Error: Device %s is not ready\n",
            power_mode_command_recv_pin.port->name);
    goto retry;
  }

  int err;
  int retry_count = 0;

  err = nrf_modem_lib_init();
  if (err) {
    LOG_ERR("Error: Failed to initialize modem library, error: %d", err);
    return -1;
  }
  lte_lc_power_off();
  err = gpio_pin_configure_dt(&power_mode_command_recv_pin,
                              GPIO_INPUT | GPIO_PULL_DOWN);
  if (err != 0) {
    LOG_ERR("Error %d: failed to configure %s pin %d\n", err,
            power_mode_command_recv_pin.port->name,
            power_mode_command_recv_pin.pin);
    goto retry;
  }

  err = gpio_pin_interrupt_configure_dt(&power_mode_command_recv_pin,
                                        GPIO_INT_EDGE_BOTH);
  if (err != 0) {
    LOG_ERR("Error %d: failed to configure interrupt on %s pin %d\n", err,
            power_mode_command_recv_pin.port->name,
            power_mode_command_recv_pin.pin);
    return -1;
  }

  gpio_init_callback(&pwr_mode_cb_data, power_mode_command_recieved,
                     BIT(power_mode_command_recv_pin.pin));
  gpio_add_callback(power_mode_command_recv_pin.port, &pwr_mode_cb_data);

  /* nRF52 config-ready line: 52 raises it when the iOS app pushes new config */
  if (device_is_ready(nrf52_config_ready_pin.port)) {
    k_work_init_delayable(&nrf52_config_work, nrf52_config_work_handler);
    gpio_pin_configure_dt(&nrf52_config_ready_pin, GPIO_INPUT);
    gpio_pin_interrupt_configure_dt(&nrf52_config_ready_pin,
                                    GPIO_INT_EDGE_TO_ACTIVE);
    gpio_init_callback(&nrf52_config_cb_data, nrf52_config_ready_isr,
                       BIT(nrf52_config_ready_pin.pin));
    gpio_add_callback(nrf52_config_ready_pin.port, &nrf52_config_cb_data);
    /* config may already be pending from before we armed the interrupt */
    if (gpio_pin_get_dt(&nrf52_config_ready_pin) > 0) {
      k_work_schedule(&nrf52_config_work, K_NO_WAIT);
    }
  } else {
    LOG_ERR("Error: nRF52 config-ready pin not ready");
  }

  /* nPM1300 fuel gauge hello world */
  err = fuel_gauge_start();
  if (err != 0) {
    LOG_ERR("Error: Fuel gauge start failed (%d)", err);
  }

  /* I2C link bench test: stream dummy battery frames to the nRF52 at 0x42
   * (no nPM1300 needed). Remove once the link is verified. */
  // while (true) {
  //   fuel_gauge_send_test_frame();
  //   k_sleep(K_SECONDS(2));
  // }

  /* RAK3172 hello world — send AT and print raw response */

  err = rak3172_init();
  if (err != 0) {
    LOG_ERR("Error: RAK3172 init failed (%d), continuing without LoRa", err);
  }

  // rak3172_print_version_info();

  // while (true) {
  //   rak3172_resume();
  //   rak3172_send_message("hi");
  //   rak3172_suspend();
  //   k_sleep(K_SECONDS(5));
  // }

#if defined(CONFIG_LORA_CAD_RX_TEST)
  LOG_INF("CAD receive bench test");

#if !defined(CONFIG_LORA_CAD_RX_TEST_LOW_POWER)
  /* Low power mode gates the UART; keep the module awake so AT stays reliable
   * while the sniff loop is being brought up. */
  rak3172_send_at("AT+LPM=0");
  rak3172_read_response(1000);
#endif

  if (gpio_is_ready_dt(&lora_wake_pin)) {
    gpio_pin_configure_dt(&lora_wake_pin, GPIO_INPUT);
  } else {
    LOG_WRN("RAK3172 wake line not wired; watching UART events only");
  }

  if (rak3172_cad_rx_configure(CONFIG_LORA_CAD_RX_TEST_SYMBOLS,
                               CONFIG_LORA_CAD_RX_TEST_DET_PEAK,
                               CONFIG_LORA_CAD_RX_TEST_DET_MIN,
                               CONFIG_LORA_CAD_RX_TEST_INTERVAL_MS) != 0 ||
      rak3172_cad_rx_set_pin(CONFIG_LORA_CAD_RX_TEST_WAKE_PIN) != 0 ||
      rak3172_cad_rx_set_absence(CONFIG_LORA_CAD_RX_TEST_ABSENCE_S) != 0 ||
      rak3172_cad_rx_start() != 0) {
    LOG_ERR("Error: CAD receive test setup failed");
  }

  char cad_event[160];
  int64_t next_stats = k_uptime_get();
  int wake_level = -1;

  while (true) {
    if (rak3172_wait_event("+EVT:CAD", cad_event, sizeof(cad_event), 1000) ==
        0) {
      LOG_INF("CAD event: %s", cad_event);
      if (strncmp(cad_event, "+EVT:CADRX:", 11) == 0) {
        rak3172_cad_rx_read();
      }
    }

    if (gpio_is_ready_dt(&lora_wake_pin)) {
      int level = gpio_pin_get_dt(&lora_wake_pin);

      if (level >= 0 && level != wake_level) {
        wake_level = level;
        LOG_INF("RAK3172 wake line %s", level ? "asserted" : "clear");
        if (level) {
          rak3172_cad_rx_event();
        }
      }
    }

#if !defined(CONFIG_LORA_CAD_RX_TEST_LOW_POWER)
    if (k_uptime_get() >= next_stats) {
      rak3172_cad_rx_stats();
      next_stats = k_uptime_get() + 10000;
    }
#endif
  }
#endif

  /* WiFi scan hello world */
  wifi_scan_init();
  while (true) {
    wifi_scan_start();
    wifi_scan_stop();
    k_sleep(K_SECONDS(15));
  }

  /* WiFi location hello world */
  // err = wifi_location_init();
  // if (err) {
  //   LOG_ERR("Error: WiFi location init failed (%d)", err);
  // } else {
  //   wifi_location_get();
  // }

  // Connect to LTE network with retries if necessary.
  while (retry_count < CONFIG_LTE_CONNECT_MAX_RETRIES) {
    err = initializeLte();

    if (err) {
      LOG_INF("Failed to connect to LTE with error: %d\n", err);
      retry_count++;
      k_sleep(K_MSEC(CONFIG_LTE_CONNECT_RETRY_DELAY_S * 1000));
    } else {
      break;
    }
  }

  if (retry_count == CONFIG_LTE_CONNECT_MAX_RETRIES) {
    LOG_ERR("Max retry limit reached for connecting to LTE, program exiting.");
    return -1;
  }

  retry_count = 0;
  // Initialize location services with retries if necessary.
  while (retry_count < CONFIG_LOCATION_SERVICES_INIT_MAX_RETRIES) {
    err = initializeLocation();

    if (err) {
      LOG_INF("Failed to initialize location services with error: %d\n", err);
      retry_count++;
      k_sleep(K_MSEC(CONFIG_LOCATION_SERVICES_INIT_RETRY_DELAY_S * 1000));
    } else {
      break;
    }
  }

  if (retry_count == CONFIG_LTE_CONNECT_MAX_RETRIES) {
    LOG_ERR("Max retry limit reached for initializing location services, "
            "program exiting.");
    return -1;
  }

  retry_count = 0;
  // Initialize UDP listener with retries (using the same retry count as
  // location services)
  while (retry_count < CONFIG_LOCATION_SERVICES_INIT_MAX_RETRIES) {
    err = create_udp_listener();

    if (err) {
      LOG_INF("Failed to create UDP Listener with error: %d\n", err);
      destroy_udp_listener();
      retry_count++;
      k_sleep(K_MSEC(CONFIG_LOCATION_SERVICES_INIT_RETRY_DELAY_S * 1000));
    } else {
      LOG_INF("Created UDP Listener.");
      break;
    }
  }

  // Start the UDP listener thread.
  start_udp_listener_thread();
//
  while (true) {
    LOG_INF("Main thread is running.");
    k_sleep(K_MSEC(4000));
  }

  return 0;
}
