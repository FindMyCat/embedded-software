#include "LocationEngine.h"
#include <zephyr/kernel.h>
#include <modem/location.h>
#include <zephyr/logging/log.h>

#include "../mqtt/mqttsn.h"

K_SEM_DEFINE(location_event, 0, 1);

LOG_MODULE_REGISTER(gnss, 4);

void location_event_handler(const struct location_event_data *event_data)
{
	switch (event_data->id) {
	case LOCATION_EVT_LOCATION:
		LOG_INF("Got location:\n");
		printk("  method: %s\n", location_method_str(event_data->method));
		printk("  latitude: %.06f\n", event_data->location.latitude);
		printk("  longitude: %.06f\n", event_data->location.longitude);
		printk("  accuracy: %.01f m\n", event_data->location.accuracy);
		char location_str[100]; // create a character array to store the formatted string
		snprintk(location_str, sizeof(location_str), "%.06f, %.06f", event_data->location.latitude, event_data->location.longitude);
		printk("%s\n", location_str); // print the formatted string
		
		// Todo: Any better way to architect this? Separate the location engine from the MQTT service?
		mqttsn_check_input();
   		mqttsn_publish(location_str);
    	mqttsn_check_input();
		if (event_data->location.datetime.valid) {
			printk("  date: %04d-%02d-%02d\n",
				event_data->location.datetime.year,
				event_data->location.datetime.month,
				event_data->location.datetime.day);
			printk("  time: %02d:%02d:%02d.%03d UTC\n",
				event_data->location.datetime.hour,
				event_data->location.datetime.minute,
				event_data->location.datetime.second,
				event_data->location.datetime.ms);
		}
		printk("  Google maps URL: https://maps.google.com/?q=%.06f,%.06f\n\n",
			event_data->location.latitude, event_data->location.longitude);
		break;

	case LOCATION_EVT_TIMEOUT:
		LOG_ERR("Getting location timed out\n\n");
		break;

	case LOCATION_EVT_ERROR:
		LOG_ERR("Getting location failed\n\n");
		break;

	case LOCATION_EVT_GNSS_ASSISTANCE_REQUEST:
		LOG_INF("Getting location assistance requested (A-GPS). Not doing anything.\n\n");
		break;

	case LOCATION_EVT_GNSS_PREDICTION_REQUEST:
		LOG_INF("Getting location assistance requested (P-GPS). Not doing anything.\n\n");
		break;

	default:
		LOG_INF("Getting location: Unknown event\n\n");
		break;
	}

	k_sem_give(&location_event);
}

void location_event_wait(void)
{
	k_sem_take(&location_event, K_FOREVER);
}

/**
 * @brief Retrieve location so that fallback is applied.
 *
 * @details This is achieved by setting GNSS as first priority method and giving it too short
 * timeout. Then a fallback to next method, which is cellular in this example, occurs.
 */
void location_with_fallback_get(void)
{
	int err;
	struct location_config config;
	enum location_method methods[] = {LOCATION_METHOD_GNSS, LOCATION_METHOD_CELLULAR};

	location_config_defaults_set(&config, ARRAY_SIZE(methods), methods);
	/* GNSS timeout is set to 1 second to force a failure. */
	config.methods[0].gnss.timeout = 1 * MSEC_PER_SEC;
	/* Default cellular configuration may be overridden here. */
	config.methods[1].cellular.timeout = 40 * MSEC_PER_SEC;

	LOG_INF("Requesting location with short GNSS timeout to trigger fallback to cellular...\n");

	err = location_request(&config);
	if (err) {
		LOG_ERR("Requesting location failed, error: %d\n", err);
		return;
	}

	location_event_wait();
}

/**
 * @brief Retrieve location with default configuration.
 *
 * @details This is achieved by not passing configuration at all to location_request().
 */
void location_default_get(void)
{
	int err;

	LOG_INF("Requesting location with the default configuration...\n");

	err = location_request(NULL);
	if (err) {
		LOG_ERR("Requesting location failed, error: %d\n", err);
		return;
	}

	location_event_wait();
}

/**
 * @brief Retrieve location with GNSS low accuracy.
 */
void location_gnss_low_accuracy_get(void)
{
	int err;
	struct location_config config;
	enum location_method methods[] = {LOCATION_METHOD_GNSS};

	location_config_defaults_set(&config, ARRAY_SIZE(methods), methods);
	config.methods[0].gnss.accuracy = LOCATION_ACCURACY_LOW;

	LOG_INF("Requesting low accuracy GNSS location...\n");

	err = location_request(&config);
	if (err) {
		LOG_ERR("Requesting location failed, error: %d\n", err);
		return;
	}

	location_event_wait();
}

/**
 * @brief Retrieve location with GNSS high accuracy.
 */
void location_gnss_high_accuracy_get(void)
{
	int err;
	struct location_config config;
	enum location_method methods[] = {LOCATION_METHOD_GNSS};

	location_config_defaults_set(&config, ARRAY_SIZE(methods), methods);
	config.methods[0].gnss.accuracy = LOCATION_ACCURACY_HIGH;

	LOG_INF("Requesting high accuracy GNSS location...\n");

	err = location_request(&config);
	if (err) {
		LOG_ERR("Requesting location failed, error: %d\n", err);
		return;
	}

	location_event_wait();
}

/**
 * @brief Retrieve location periodically with GNSS as first priority and cellular as second.
 */
void location_gnss_periodic_get(int period)
{
	int err;
	struct location_config config;
	enum location_method methods[] = {LOCATION_METHOD_GNSS, LOCATION_METHOD_CELLULAR};

	location_config_defaults_set(&config, ARRAY_SIZE(methods), methods);
	config.interval = period;

	printk("Requesting %d s periodic GNSS location with cellular fallback...\n", period);

	err = location_request(&config);
	if (err) {
		LOG_ERR("Requesting location failed, error: %d\n", err);
		return;
	}
}
