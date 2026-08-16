#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "wifi_station.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

int main(void)
{
	LOG_INF("FindMyCat HomeStation ESP32 starting");

	int err = wifi_station_connect();

	if (err) {
		LOG_ERR("Error: WiFi station connect failed (%d), geolocation "
			"lookups will be skipped", err);
	}

	return 0;
}
