#include "Responder.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "../Dispatcher/Dispatcher.h"
#include "../FuelGauge/fuel_gauge.h"
#include "../communication_channels/transport.h"
#include "../LocationEngine/LocationEngine.h"
#include "../WiFi/wifi_scan.h"

LOG_MODULE_REGISTER(responder,
                    CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

#define PING_ACCESS_POINTS 6

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
    err = wifi_scan_pack_strongest_access_points(&scan, PING_ACCESS_POINTS,
                                                 fuel_gauge_battery_percent());
  }

  /* Cuts the nRF7002's supply, so it runs whether or not the scan worked. */
  wifi_scan_stop();

  if (err != 0) {
    LOG_ERR("Error: no WiFi scan to answer the ping with (%d)", err);
    changeDispatcherState(DISPATCHER_STATE_IDLE);
    return;
  }

  if (transport_publish_wifi_scan(&scan) != 0) {
    LOG_ERR("Error: sending the WiFi scan failed");
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
  /* No mqttsn_disconnect() here: every publish closes its own session under
   * the session lock, and an unlocked one tears down another thread's. */
  location_request_cancel();
}
