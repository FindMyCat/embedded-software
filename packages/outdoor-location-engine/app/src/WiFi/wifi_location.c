#include "wifi_location.h"
#include <modem/location.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(wifi_location, LOG_LEVEL_DBG);

static K_SEM_DEFINE(wifi_location_event, 0, 1);

static void wifi_location_event_handler(const struct location_event_data *event_data)
{
	switch (event_data->id) {
	case LOCATION_EVT_LOCATION:
		LOG_INF("WiFi location fix:");
		LOG_INF("  latitude:  %.06f", event_data->location.latitude);
		LOG_INF("  longitude: %.06f", event_data->location.longitude);
		LOG_INF("  accuracy:  %.01f m", event_data->location.accuracy);
		break;
	case LOCATION_EVT_TIMEOUT:
		LOG_ERR("Error: WiFi location timed out");
		break;
	case LOCATION_EVT_ERROR:
		LOG_ERR("Error: WiFi location failed");
		break;
	default:
		break;
	}

	k_sem_give(&wifi_location_event);
}

int wifi_location_init(void)
{
	int err = location_init(wifi_location_event_handler);

	if (err) {
		LOG_ERR("Error: location_init failed (%d)", err);
	}

	return err;
}

int wifi_location_get(void)
{
	struct location_config config;
	enum location_method methods[] = {LOCATION_METHOD_WIFI};

	location_config_defaults_set(&config, ARRAY_SIZE(methods), methods);

	LOG_INF("Requesting WiFi location...");

	int err = location_request(&config);

	if (err) {
		LOG_ERR("Error: location_request failed (%d)", err);
		return err;
	}

	k_sem_take(&wifi_location_event, K_FOREVER);

	return 0;
}
