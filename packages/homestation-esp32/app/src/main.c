#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "lora.h"
#include "wifi_station.h"
#if defined(CONFIG_BLE_SCANNER)
#include "ble_scanner.h"
#endif
#if defined(CONFIG_BLE_BEACON)
#include "ble_beacon.h"
#endif
#if defined(CONFIG_MQTT_PUBLISHER)
#include "mqtt_publisher.h"
#endif
#if defined(CONFIG_ESP32_SW_COEXIST_ENABLE)
#include <private/esp_coexist_internal.h>
#endif

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

int main(void)
{
	LOG_INF("FindMyCat HomeStation ESP32 starting");

	int err = wifi_station_connect();

	if (err) {
		LOG_ERR("Error: WiFi station connect failed (%d), geolocation "
			"lookups will be skipped", err);
	}

#if defined(CONFIG_ESP32_SW_COEXIST_ENABLE)
	// Prefering BT for COEX as the collar only advertises so often and packets are being missed due to wifi taking up more Radio time.
	coex_preference_set(COEX_PREFER_BT);
	LOG_INF("Radio arbitration set to prefer Bluetooth");
#endif

#if defined(CONFIG_BLE_SCANNER)
	ble_scanner_start();
#endif
	/* After the scanner, which is what calls bt_enable(). */
#if defined(CONFIG_BLE_BEACON)
	ble_beacon_start();
#endif

#if defined(CONFIG_MQTT_PUBLISHER)
	mqtt_publisher_start();
#endif

	lora_start();

	return 0;
}
