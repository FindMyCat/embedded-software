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
#include <zephyr/sys/atomic.h>

// Application specific imports
#include "comm/nrf52_link.h"
#include "Diagnostics/diagnostics.h"
#include "Dispatcher/Dispatcher.h"
#include "FuelGauge/fuel_gauge.h"
#include "LoRa/lora_link.h"
#include "LoRa/rak3172.h"
#include "LTE/lte.h"
#include "WiFi/wifi_location.h"
#include "WiFi/wifi_scan.h"
#include "LocationEngine/LocationEngine.h"
#include "mqtt/mqttsn.h"
#include "UDPListener/NewUDPListener.h"

LOG_MODULE_REGISTER(main, CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

/* Indoor location engine sends 1 for PSM mode and 0 for eDRX mode */
#define POWER_MODE_COMMAND_PIN DT_ALIAS(gpiocus10)

static const struct gpio_dt_spec power_mode_command_recv_pin =
	GPIO_DT_SPEC_GET_OR(POWER_MODE_COMMAND_PIN, gpios, {0});

static const struct gpio_dt_spec lora_wake_pin =
	GPIO_DT_SPEC_GET_OR(DT_ALIAS(lora_wake), gpios, {0});

#define NRF52_CONFIG_READY_PIN DT_ALIAS(nrf52_config_ready)
static const struct gpio_dt_spec nrf52_config_ready_pin =
	GPIO_DT_SPEC_GET_OR(NRF52_CONFIG_READY_PIN, gpios, {0});

static struct fmc_config fmc_config_nrf52;
static struct k_work_delayable fmc_nrf52_config__work;
static struct gpio_callback fmc_config_nrf52_cb_data;

/**
 * @brief Initialize location services.
 *
 * DOES NOT call method to request location, only initializes the library.
 */
static int initialize_location_services()
{
    /* A-GPS/P-GPS needs to know the current time. */
    if (IS_ENABLED(CONFIG_DATE_TIME)) {
	LOG_INF("Waiting for current time");

	/* Wait for an event from the Date Time library. */
	lte_wait_for_time(K_MINUTES(10));

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

#define POWER_MODE_COMMAND_DEBOUNCE_TIME 50
uint32_t last_powermode_command_received_timestamp;

static atomic_t homestation_in_ble_range = ATOMIC_INIT(0);

static void set_homestation_in_ble_range(bool in_range)
{
    atomic_set(&homestation_in_ble_range, in_range ? 1 : 0);

#if defined(CONFIG_LORA_LINK)
    lora_link_set_enabled(!in_range);
#endif
}
/**
 * @brief Callback function for when the power mode command is received through
 * gpio interrupt.
 */
static void powermode_command_recieved__cb()
{
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

/**
 * @brief Read the config the nRF52 published. Runs on the system workqueue.
 */
static void nrf52_config_work_handler(struct k_work *work)
{
    ARG_UNUSED(work);
    /* On a transient failure, retry while the data-ready line is still asserted:
     * the nRF52 holds it high until we ack, so a held-high line means "still
     * pending" and there is no fresh edge to re-trigger us. */
    if (nrf52_read_config(&fmc_config_nrf52) != 0) {
	if (gpio_pin_get_dt(&nrf52_config_ready_pin) > 0) {
	    k_work_reschedule(&fmc_nrf52_config__work, K_MSEC(100));
	}
	return;
    }

    initialize_mqttsn_config(fmc_config_nrf52.device_id, fmc_config_nrf52.host);
#if defined(CONFIG_LORA_LINK)
    lora_link_set_device_id(fmc_config_nrf52.device_id);
#endif
}

/**
 * @brief ISR for the nRF52 config-ready line; defers the i2c read to a workitem.
 */
static void nrf52_config_ready_isr(const struct device *port, struct gpio_callback *cb, uint32_t pins)
{
    ARG_UNUSED(port);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);
    k_work_schedule(&fmc_nrf52_config__work, K_NO_WAIT);
}

/**
 * @brief Application entry point.
 */
int main(void)
{

    LOG_INF("*******************************");
    LOG_INF("Welcome to FindMyCat v3.0 Preview");
    LOG_INF("*******************************");

    int err;
    int retry_count = 0;

    /* Ahead of the diagnostics below, which ask the modem about the SIM. */
    err = nrf_modem_lib_init();
    if (err) {
	LOG_ERR("Error: Failed to initialize modem library, error: %d", err);
	return -1;
    }

#if defined(CONFIG_FINDMYCAT_DIAGNOSTICS)
    diagnostics_run();
#endif

retry:
    if (!device_is_ready(power_mode_command_recv_pin.port)) {
	LOG_ERR("Error: Device %s is not ready", power_mode_command_recv_pin.port->name);
	goto retry;
    }

    lte_lc_power_off();
    err = gpio_pin_configure_dt(&power_mode_command_recv_pin, GPIO_INPUT | GPIO_PULL_DOWN);
    if (err != 0) {
	LOG_ERR("Error %d: failed to configure %s pin %d", err,
		power_mode_command_recv_pin.port->name, power_mode_command_recv_pin.pin);
	goto retry;
    }

    err = gpio_pin_interrupt_configure_dt(&power_mode_command_recv_pin, GPIO_INT_EDGE_BOTH);
    if (err != 0) {
	LOG_ERR("Error %d: failed to configure interrupt on %s pin %d", err,
		power_mode_command_recv_pin.port->name, power_mode_command_recv_pin.pin);
	return -1;
    }

    gpio_init_callback(&pwr_mode_cb_data, powermode_command_recieved__cb,
		       BIT(power_mode_command_recv_pin.pin));
    gpio_add_callback(power_mode_command_recv_pin.port, &pwr_mode_cb_data);

    /* nRF52 config-ready line: 52 sets to HIGH it when the iOS app pushes new config */
    if (device_is_ready(nrf52_config_ready_pin.port)) {
	k_work_init_delayable(&fmc_nrf52_config__work, nrf52_config_work_handler);
	gpio_pin_configure_dt(&nrf52_config_ready_pin, GPIO_INPUT);
	gpio_pin_interrupt_configure_dt(&nrf52_config_ready_pin, GPIO_INT_EDGE_TO_ACTIVE);
	gpio_init_callback(&fmc_config_nrf52_cb_data, nrf52_config_ready_isr,
			   BIT(nrf52_config_ready_pin.pin));
	gpio_add_callback(nrf52_config_ready_pin.port, &fmc_config_nrf52_cb_data);
	/* config may already be pending from before we armed the interrupt */
	if (gpio_pin_get_dt(&nrf52_config_ready_pin) > 0) {
	    k_work_schedule(&fmc_nrf52_config__work, K_NO_WAIT);
	}
    } else {
	LOG_ERR("Error: nRF52 config-ready pin not ready");
    }

    /* nPM1300 fuel gauge hello world */
    err = fuel_gauge_start();
    if (err != 0) {
	LOG_ERR("Error: Fuel gauge start failed (%d)", err);
    }

#if defined(CONFIG_LORA_RAK3172_INIT)
    err = rak3172_init();
    if (err != 0) {
	LOG_ERR("Error: RAK3172 init failed (%d), continuing without LoRa", err);
    }
#endif

    set_homestation_in_ble_range(gpio_pin_get_dt(&power_mode_command_recv_pin) == 1);

#if defined(CONFIG_LORA_LINK)
    err = lora_link_start();
    if (err != 0) {
	LOG_ERR("Error: LoRa link start failed (%d), pings arrive over LTE only", err);
    }
#endif

#if defined(CONFIG_LORA_CAD_RX_TEST)
    LOG_INF("CAD receive bench test");

#if defined(CONFIG_LORA_CAD_RX_TEST_RESET_FIRST)
    rak3172_reset_test();
    rak3172_init();
#endif

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

    if (rak3172_cad_rx_configure(CONFIG_LORA_CAD_RX_TEST_SYMBOLS, CONFIG_LORA_CAD_RX_TEST_DET_PEAK,
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
	if (rak3172_wait_event("+EVT:CAD", cad_event, sizeof(cad_event), 1000) == 0) {
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

#if defined(CONFIG_WIFI_SCAN_LORA_TEST)
    /* WiFi scan hello world */
#define WIFI_SCAN_LORA_ACCESS_POINTS CONFIG_WIFI_SCAN_LORA_TEST_ACCESS_POINTS

    wifi_scan_init();
    while (true) {
	struct wifi_scan_frame frame;

	wifi_scan_start();
	wifi_scan_stop();

	err = wifi_scan_pack_strongest_access_points(&frame, WIFI_SCAN_LORA_ACCESS_POINTS);
	if (err) {
	    LOG_ERR("Error: packing scan results failed (%d)", err);
	} else if (frame.bytes[1] == 0) {
	    LOG_WRN("No usable access points to send");
	} else {
	    LOG_INF("Sending %u access points over LoRa (%u bytes)", frame.bytes[1],
		    (unsigned)frame.len);

	    rak3172_resume();
	    err = rak3172_send_payload(frame.bytes, frame.len);
	    if (err) {
		LOG_ERR("Error: LoRa send failed (%d)", err);
	    }
	    rak3172_suspend();
	}

	k_sleep(K_SECONDS(CONFIG_WIFI_SCAN_LORA_TEST_INTERVAL_S));
    }
#endif

    // Connect to LTE network with retries if necessary.
    while (retry_count < CONFIG_LTE_CONNECT_MAX_RETRIES) {
	err = lte_connect();

	if (err) {
	    LOG_INF("Failed to connect to LTE with error: %d", err);
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
	err = initialize_location_services();

	if (err) {
	    LOG_INF("Failed to initialize location services with error: %d", err);
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
	    LOG_INF("Failed to create UDP Listener with error: %d", err);
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
    int tick = 0;

    while (true) {
#if defined(CONFIG_LORA_LINK)
	bool home = atomic_get(&homestation_in_ble_range) != 0 ||
		    lora_link_homestation_in_range();

	LOG_INF("Main thread is running. Home station %s",
		home ? "in range" : "out of range");

	/* Every 8th tick, so the counters land about once per beacon. */
	if (++tick % 8 == 0) {
	    lora_link_log_stats();
	}
#else
	LOG_INF("Main thread is running.");
#endif
	k_sleep(K_MSEC(4000));
    }

    return 0;
}
