#include "LocationEngine.h"
#include <zephyr/kernel.h>
#include <modem/location.h>
#include <zephyr/logging/log.h>
#include <zephyr/random/random.h>

#include "../mqtt/mqttsn.h"

K_SEM_DEFINE(location_event, 0, 1);

LOG_MODULE_REGISTER(location_engine, CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

static bool location_engine_has_fix = false;

#define MQTT_SN_CONNECT_TIMEOUT_MS 10000
#define MQTT_SN_CONNECT_POLL_MS 100
#define MQTT_SN_FLUSH_MS 2000

/**
 * @brief Connect to the gateway if needed, then publish one position.
 */
void location_publish(double latitude, double longitude, int satellites, double accuracy)
{
	int err = 0;

	char location_str[100]; // create a character array to store the formatted string
	snprintk(location_str, sizeof(location_str), "{\"msg\": \"%.06f,%.06f,%d,%.02f,%d\"}",
		latitude,
		longitude,
		satellites,
		accuracy,
		25); // Todo: replace with actual battery reading

	int connecting_retries = 0;
try_connecting_mqttsn:
		mqttsn_check_input();
		if(get_mqttsn_connection_status() == false) {
			LOG_INF("MQTT-SN not connected. Initialing connection.");
			err = mqttsn_initialize();
			connecting_retries += 1;
			/* CONNACK only lands if input keeps being pumped, so poll for it
			 * rather than sleeping blind: a publish sent before the client is
			 * connected is dropped on the floor. */
			for (int waited_ms = 0;
			     waited_ms < MQTT_SN_CONNECT_TIMEOUT_MS && !get_mqttsn_connection_status();
			     waited_ms += MQTT_SN_CONNECT_POLL_MS) {
				mqttsn_check_input();
				k_sleep(K_MSEC(MQTT_SN_CONNECT_POLL_MS));
			}
		}
		if(err || get_mqttsn_connection_status() == false) {
			mqttsn_disconnect();
			if (connecting_retries <= 3)
			{
				LOG_ERR("MQTT-SN initialization failed, Retrying. Error: %d", err);
				goto try_connecting_mqttsn;
			} else {
				LOG_ERR("MQTT-SN initialization failed Max Retries exceeded. Error: %d", err);
			}
		}
		else {
			mqttsn_check_input();
			mqttsn_publish(location_str);
			/* A publish to a topic name is held until the gateway answers the
			 * REGISTER with a REGACK, so keep pumping input afterwards.
			 * Returning straight away lets the caller disconnect
			 * mid-registration and the position never leaves the device. */
			for (int waited_ms = 0; waited_ms < MQTT_SN_FLUSH_MS;
			     waited_ms += MQTT_SN_CONNECT_POLL_MS) {
				mqttsn_check_input();
				k_sleep(K_MSEC(MQTT_SN_CONNECT_POLL_MS));
			}
		}
}

/**
 * @brief Jitter in degrees, up to plus or minus the configured radius.
 */
static double random_offset_degrees(void)
{
	uint32_t span = (uint32_t)CONFIG_SPOOF_PING_JITTER_MICRODEGREES * 2 + 1;
	int32_t offset = (int32_t)(sys_rand32_get() % span) -
			 CONFIG_SPOOF_PING_JITTER_MICRODEGREES;

	return (double)offset / 1000000.0;
}

/**
 * @brief Publish a made up position near the configured base point.
 *
 * Exercises the gateway, rule engine and Traccar without waiting for a GNSS
 * fix, so a spoof ping answers in seconds rather than minutes. Reports zero
 * satellites, which no real fix does, so a spoofed point stays identifiable
 * once it reaches the cloud.
 */
void location_publish_spoofed(void)
{
	double latitude = (double)CONFIG_SPOOF_PING_BASE_LATITUDE_MICRODEGREES / 1000000.0 +
			  random_offset_degrees();
	double longitude = (double)CONFIG_SPOOF_PING_BASE_LONGITUDE_MICRODEGREES / 1000000.0 +
			   random_offset_degrees();

	LOG_INF("Publishing spoofed position %.06f,%.06f", latitude, longitude);
	location_publish(latitude, longitude, 0, 21.0);
}

void location_event_handler(const struct location_event_data *event_data)
{
	switch (event_data->id) {
	case LOCATION_EVT_LOCATION:
		location_engine_has_fix = true;

		LOG_INF("Got location by %s: %.06f,%.06f accuracy %.01f m, %d satellites",
			location_method_str(event_data->method),
			event_data->location.latitude,
			event_data->location.longitude,
			event_data->location.accuracy,
			event_data->location.details.gnss.satellites_tracked);

		location_publish(event_data->location.latitude,
			event_data->location.longitude,
			event_data->location.details.gnss.satellites_tracked,
			event_data->location.accuracy);

		if (event_data->location.datetime.valid) {
			LOG_INF("Fix time %04d-%02d-%02d %02d:%02d:%02d.%03d UTC",
				event_data->location.datetime.year,
				event_data->location.datetime.month,
				event_data->location.datetime.day,
				event_data->location.datetime.hour,
				event_data->location.datetime.minute,
				event_data->location.datetime.second,
				event_data->location.datetime.ms);
		}
		break;

	case LOCATION_EVT_TIMEOUT:
		LOG_ERR("Getting location timed out");
		break;

	case LOCATION_EVT_ERROR:
		LOG_ERR("Getting location failed");
		break;

	case LOCATION_EVT_GNSS_ASSISTANCE_REQUEST:
		LOG_INF("Getting location assistance requested (A-GPS). Not doing anything.");
		break;

	case LOCATION_EVT_GNSS_PREDICTION_REQUEST:
		LOG_INF("Getting location assistance requested (P-GPS). Not doing anything.");
		break;

	default:
		LOG_INF("Getting location: Unknown event");
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

	LOG_INF("Requesting location with short GNSS timeout to trigger fallback to cellular...");

	err = location_request(&config);
	if (err) {
		LOG_ERR("Requesting location failed, error: %d", err);
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

	LOG_INF("Requesting location with the default configuration...");

	err = location_request(NULL);
	if (err) {
		LOG_ERR("Requesting location failed, error: %d", err);
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

	LOG_INF("Requesting low accuracy GNSS location...");

	err = location_request(&config);
	if (err) {
		LOG_ERR("Requesting location failed, error: %d", err);
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

	LOG_INF("Requesting high accuracy GNSS location...");

	err = location_request(&config);
	if (err) {
		LOG_ERR("Requesting location failed, error: %d", err);
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

	LOG_INF("Requesting %d s periodic GNSS location with cellular fallback...", period);

	err = location_request(&config);
	if (err) {
		LOG_ERR("Requesting location failed, error: %d", err);
		return;
	}
}
