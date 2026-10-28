#include "LocationEngine.h"
#include <modem/location.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/random/random.h>

#include "../communication_channels/transport.h"

K_SEM_DEFINE(location_event, 0, 1);

LOG_MODULE_REGISTER(location_engine,
                    CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

static bool location_engine_has_fix = false;

static struct location_result last_result = {
    .reason = "none",
};

/**
 * @brief Publish a location using the transport layer.
 */
int location_publish(double latitude, double longitude, int satellites,
                     double accuracy) {
  const struct position_report report = {
      .latitude = latitude,
      .longitude = longitude,
      .accuracy_metres = accuracy,
      .satellites = satellites,
  };

  int err = transport_publish_position(&report);

  if (err) {
    LOG_ERR("Error: publishing the position failed (%d)", err);
  }

  return err;
}

static double random_offset_degrees(void) {
  uint32_t span = (uint32_t)CONFIG_SPOOF_PING_JITTER_MICRODEGREES * 2 + 1;
  int32_t offset = (int32_t)(sys_rand32_get() % span) -
                   CONFIG_SPOOF_PING_JITTER_MICRODEGREES;

  return (double)offset / 1000000.0;
}

/**
 * @brief Publish a made up position near the configured base point.
 */
int location_publish_spoofed(enum transport_path path) {
  double latitude =
      (double)CONFIG_SPOOF_PING_BASE_LATITUDE_MICRODEGREES / 1000000.0 +
      random_offset_degrees();
  double longitude =
      (double)CONFIG_SPOOF_PING_BASE_LONGITUDE_MICRODEGREES / 1000000.0 +
      random_offset_degrees();

  LOG_INF("Publishing spoofed position %.06f,%.06f over %s", latitude,
          longitude,
          path == TRANSPORT_PATH_LORA       ? "LoRa"
          : path == TRANSPORT_PATH_CELLULAR ? "MQTT-SN"
                                            : "whichever link is available");

  const struct position_report report = {
      .latitude = latitude,
      .longitude = longitude,
      .accuracy_metres = 21.0,
      .satellites = 0,
  };

  int err = transport_publish_position_via(&report, path);

  if (err) {
    LOG_ERR("Error: publishing the spoofed position failed (%d)", err);
  }

  return err;
}

static void record_result_for_status(const char *reason,
                                     enum location_method method,
                                     uint8_t satellites, uint32_t elapsed_ms) {
  last_result.reason = reason;
  last_result.method = method;
  last_result.num_satellites = satellites;
  last_result.elapsed_ms = elapsed_ms;
  last_result.has_fix = (strcmp(reason, "fix") == 0);
}

int location_ensure_init(void) {
  static bool initialized;

  if (initialized) {
    return 0;
  }

  int err = location_init(location_event_handler);

  if (err == 0) {
    initialized = true;
  }

  return err;
}

const struct location_result *location_last_result(void) {
  return &last_result;
}

void location_event_handler(const struct location_event_data *event_data) {
  switch (event_data->id) {
  case LOCATION_EVT_LOCATION:
    location_engine_has_fix = true;
    record_result_for_status(
        "fix", event_data->method,
        event_data->location.details.gnss.satellites_tracked,
        event_data->location.details.elapsed_time_method);

    LOG_INF("Got location by %s: %.06f,%.06f accuracy %.01f m, %d satellites",
            location_method_str(event_data->method),
            event_data->location.latitude, event_data->location.longitude,
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
    record_result_for_status("timeout", event_data->method,
                             event_data->error.details.gnss.satellites_tracked,
                             event_data->error.details.elapsed_time_method);
    LOG_ERR("Getting location timed out after %u ms, %u satellites tracked",
            last_result.elapsed_ms, last_result.num_satellites);
    break;

  case LOCATION_EVT_ERROR:
    record_result_for_status("error", event_data->method,
                             event_data->error.details.gnss.satellites_tracked,
                             event_data->error.details.elapsed_time_method);
    LOG_ERR("Getting location failed");
    break;

  case LOCATION_EVT_GNSS_ASSISTANCE_REQUEST:
    LOG_INF(
        "Getting location assistance requested (A-GPS). Not doing anything.");
    break;

  case LOCATION_EVT_GNSS_PREDICTION_REQUEST:
    LOG_INF(
        "Getting location assistance requested (P-GPS). Not doing anything.");
    break;

  case LOCATION_EVT_STARTED:
    LOG_INF("Getting location by %s started",
            location_method_str(event_data->method));
    return;

  case LOCATION_EVT_FALLBACK:
    LOG_INF("Getting location fell back to %s",
            location_method_str(event_data->method));
    return;

  default:
    record_result_for_status("unknown", event_data->method, 0, 0);
    LOG_INF("Getting location: Unknown event");
    break;
  }

  k_sem_give(&location_event);
}

void location_event_wait(void) { k_sem_take(&location_event, K_FOREVER); }

/**
 * @brief Retrieve location so that fallback is applied.
 *
 * @details This is achieved by setting GNSS as first priority method and giving
 * it too short timeout. Then a fallback to next method, which is cellular in
 * this example, occurs.
 */
void location_with_fallback_get(void) {
  int err;
  struct location_config config;
  enum location_method methods[] = {LOCATION_METHOD_GNSS,
                                    LOCATION_METHOD_CELLULAR};

  location_config_defaults_set(&config, ARRAY_SIZE(methods), methods);
  /* GNSS timeout is set to 1 second to force a failure. */
  config.methods[0].gnss.timeout = 1 * MSEC_PER_SEC;
  /* Default cellular configuration may be overridden here. */
  config.methods[1].cellular.timeout = 40 * MSEC_PER_SEC;

  LOG_INF("Requesting location with short GNSS timeout to trigger fallback to "
          "cellular...");

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
 * @details This is achieved by not passing configuration at all to
 * location_request().
 */
void location_default_get(void) {
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
 * @brief Retrieve location with GNSS normal accuracy.
 */
void location_gnss_normal_accuracy_get(void) {
  int err;
  struct location_config config;
  enum location_method methods[] = {LOCATION_METHOD_GNSS};

  location_config_defaults_set(&config, ARRAY_SIZE(methods), methods);
  config.methods[0].gnss.accuracy = LOCATION_ACCURACY_NORMAL;

  LOG_INF("Requesting normal accuracy GNSS location...");

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
void location_gnss_high_accuracy_get(void) {
  int err;
  struct location_config config;
  enum location_method methods[] = {LOCATION_METHOD_GNSS};

  location_config_defaults_set(&config, ARRAY_SIZE(methods), methods);

  // // TODO: (sahas) high accuracy may not get a fix all the time, this may
  // need to be configurable build time or even run time. Device accepts config
  // over ble already, perhaps set it there?
  config.methods[0].gnss.accuracy = LOCATION_ACCURACY_HIGH;

  LOG_INF("Requesting high accuracy GNSS location...");

  err = location_request(&config);
  if (err) {
    record_result_for_status("rejected", LOCATION_METHOD_GNSS, 0, 0);
    LOG_ERR("Requesting location failed, error: %d", err);
    return;
  }

  location_event_wait();
}

/**
 * @brief Retrieve location periodically with GNSS as first priority and
 * cellular as second.
 */
void location_gnss_periodic_get(int period) {
  int err;
  struct location_config config;
  enum location_method methods[] = {LOCATION_METHOD_GNSS,
                                    LOCATION_METHOD_CELLULAR};

  location_config_defaults_set(&config, ARRAY_SIZE(methods), methods);
  config.interval = period;

  LOG_INF("Requesting %d s periodic GNSS location with cellular fallback...",
          period);

  err = location_request(&config);
  if (err) {
    LOG_ERR("Requesting location failed, error: %d", err);
    return;
  }
}
