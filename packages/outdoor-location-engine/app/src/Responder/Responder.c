#include "Responder.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "../Dispatcher/Dispatcher.h"
#include "../FuelGauge/fuel_gauge.h"
#include "../LoRa/lora_link.h"
#include "../LocationEngine/LocationEngine.h"
#include "../WiFi/wifi_scan.h"
#include "../mqtt/mqttsn.h"

LOG_MODULE_REGISTER(responder,
                    CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

#define STATUS_PAYLOAD_SIZE 192
#define NET_PAYLOAD_SIZE 96

#define PING_ACCESS_POINTS 6

// TODO: (sahas) this should have a new file/folder
void publish_command_status_to_device_status_topic(const char *command,
                                                   const char *status,
                                                   const char *reason) {
  const struct location_result *result = location_last_result();
  float state_of_charge;
  char payload[STATUS_PAYLOAD_SIZE];

  if (fuel_gauge_get_state_of_charge(&state_of_charge) != 0) {
    state_of_charge = -1.f;
  }

  snprintk(
      payload, sizeof(payload),
      "{\"command\":\"%s\",\"status\":\"%s\",\"reason\":\"%s\",\"method\":\"%"
      "s\","
      "\"satellites\":%u,\"elapsed_s\":%u,\"battery\":%.01f,\"uptime_s\":%lld}",
      command, status, reason, location_method_str(result->method),
      result->num_satellites, result->elapsed_ms / 1000U,
      (double)state_of_charge, k_uptime_get() / MSEC_PER_SEC);

  int err = mqttsn_publish_with_session(CONFIG_MQTT_SN_STATUS_TOPIC, payload);
  if (err) {
    LOG_ERR("Error: publishing device status failed (%d)", err);
  }
}

// TODO: (sahas) this should have a new file/folder
void publish_network_status_to_device_status_topic(
    const struct lte_lc_edrx_cfg *cfg) {
  char payload[NET_PAYLOAD_SIZE];

  snprintk(
      payload, sizeof(payload),
      "{\"command\":\"NET\",\"status\":\"ok\",\"mode\":%d,\"edrx_s\":%.2f}",
      cfg->mode, (double)cfg->edrx);

  int err = mqttsn_publish_with_session(CONFIG_MQTT_SN_STATUS_TOPIC, payload);
  if (err) {
    LOG_ERR("Error: publishing network config failed (%d)", err);
  }
}

void turn_on_active_mode(void) {
  location_gnss_periodic_get(
      CONFIG_ACTIVE_MODE_LOCATION_REFRESH_PERIOD_SECONDS);
}

void respond_to_ping(void) {
  struct wifi_scan_frame scan;

  wifi_scan_init();

  /* Blocking; returns once the scan has finished. */
  int err = wifi_scan_start();

  if (err == 0) {
    err = wifi_scan_pack_strongest_access_points(&scan, PING_ACCESS_POINTS);
  }

  /* Cuts the nRF7002's supply, so it runs whether or not the scan worked. */
  wifi_scan_stop();

  if (err != 0) {
    LOG_ERR("Error: no WiFi scan to answer the ping with (%d)", err);
    changeDispatcherState(DISPATCHER_STATE_IDLE);
    return;
  }

  if (lora_link_send_wifi_scan(&scan) != 0) {
    LOG_ERR("Error: sending the WiFi scan over LoRa failed");
  }

  changeDispatcherState(DISPATCHER_STATE_IDLE);
}

/**
 * @brief Answer a spoof ping with a made up position, skipping GNSS.
 */
void respond_to_spoof_ping(void) {
  location_publish_spoofed(TRANSPORT_PATH_AUTO);
  changeDispatcherState(DISPATCHER_STATE_IDLE);
}

void turn_on_lost_mode() {
  location_gnss_periodic_get(CONFIG_LOST_MODE_LOCATION_REFRESH_PERIOD_SECONDS);
}

void stop_responding() {
  LOG_INF("Responder stopping. Performing cleanup.");
  location_request_cancel();
  mqttsn_disconnect();
}
