#include "diagnostics.h"

#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/net_event.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/wifi_mgmt.h>

#include "wifi_geolocation.h"
#include "wifi_scan_frame.h"
#include "wifi_station.h"

LOG_MODULE_REGISTER(wifi_diagnostics, CONFIG_WIFI_LOG_LEVEL);

#define SCAN_TIMEOUT_S 20

/* Google rejects a single access point. */
#define MINIMUM_ACCESS_POINTS 2

static K_SEM_DEFINE(scan_done, 0, 1);
static struct net_mgmt_event_callback scan_cb;
static struct wifi_scan_report report;

static void scan_event_handler(struct net_mgmt_event_callback *cb,
                               uint64_t event, struct net_if *iface) {
  ARG_UNUSED(iface);

  if (event == NET_EVENT_WIFI_SCAN_RESULT) {
    const struct wifi_scan_result *entry =
        (const struct wifi_scan_result *)cb->info;

    if (entry->mac_length != sizeof(report.access_points[0].bssid) ||
        report.count >= WIFI_SCAN_MAX_ACCESS_POINTS) {
      return;
    }

    struct wifi_access_point *ap = &report.access_points[report.count++];

    memcpy(ap->bssid, entry->mac, sizeof(ap->bssid));
    ap->rssi = entry->rssi;
  } else if (event == NET_EVENT_WIFI_SCAN_DONE) {
    k_sem_give(&scan_done);
  }
}

int wifi_geolocation_diagnostics(void) {
  struct net_if *iface = net_if_get_first_wifi();

  LOG_INF("Geolocation diagnostics:");

  if (!iface) {
    LOG_ERR("  FAIL no WiFi interface");
    return -ENODEV;
  }

  if (!wifi_station_is_ready()) {
    LOG_ERR("  FAIL station not joined");
    return -ENETDOWN;
  }

  report.count = 0;
  k_sem_reset(&scan_done);

  net_mgmt_init_event_callback(&scan_cb, scan_event_handler,
                               NET_EVENT_WIFI_SCAN_RESULT |
                                   NET_EVENT_WIFI_SCAN_DONE);
  net_mgmt_add_event_callback(&scan_cb);

  int err = net_mgmt(NET_REQUEST_WIFI_SCAN, iface, NULL, 0);

  if (err) {
    LOG_ERR("  FAIL scan request (%d)", err);
    net_mgmt_del_event_callback(&scan_cb);
    return err;
  }

  if (k_sem_take(&scan_done, K_SECONDS(SCAN_TIMEOUT_S)) != 0) {
    LOG_ERR("  FAIL no scan result within %d s", SCAN_TIMEOUT_S);
    net_mgmt_del_event_callback(&scan_cb);
    return -ETIMEDOUT;
  }

  net_mgmt_del_event_callback(&scan_cb);

  if (report.count < MINIMUM_ACCESS_POINTS) {
    LOG_ERR("  FAIL %u access points, need %d", (unsigned)report.count,
            MINIMUM_ACCESS_POINTS);
    return -ENODATA;
  }

  LOG_INF("  %u access points, strongest %d dBm", (unsigned)report.count,
          report.access_points[0].rssi);

  struct wifi_location location;

  err = wifi_geolocation_resolve(&report, &location);

  if (err) {
    LOG_ERR("  FAIL lookup (%d)", err);
    return err;
  }

  LOG_INF("  PASS %.6f, %.6f (accuracy %.0f m)", location.latitude,
          location.longitude, location.accuracy_m);

  return 0;
}
