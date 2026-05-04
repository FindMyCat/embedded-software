#include "wifi_scan.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/net_event.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/wifi_mgmt.h>

LOG_MODULE_REGISTER(wifi_scan, LOG_LEVEL_DBG);

static K_SEM_DEFINE(scan_done, 0, 1);
static struct net_mgmt_event_callback wifi_cb;

static void wifi_event_handler(struct net_mgmt_event_callback *cb,
                               uint64_t event, struct net_if *iface) {
  if (event == NET_EVENT_WIFI_SCAN_RESULT) {
    const struct wifi_scan_result *r =
        (const struct wifi_scan_result *)cb->info;

    LOG_INF("  SSID: %-32s  BSSID: %02x:%02x:%02x:%02x:%02x:%02x  RSSI: %d dBm "
            " ch: %d",
            r->ssid_length ? (char *)r->ssid : "<hidden>", r->mac[0], r->mac[1],
            r->mac[2], r->mac[3], r->mac[4], r->mac[5], r->rssi, r->channel);
  }

  if (event == NET_EVENT_WIFI_SCAN_DONE) {
    LOG_INF("Scan done");
    k_sem_give(&scan_done);
  }
}

int wifi_scan_init(void) {
  net_mgmt_init_event_callback(&wifi_cb, wifi_event_handler,
                               NET_EVENT_WIFI_SCAN_RESULT |
                                   NET_EVENT_WIFI_SCAN_DONE);
  net_mgmt_add_event_callback(&wifi_cb);
  return 0;
}

int wifi_scan_start(void) {
  struct net_if *iface = net_if_get_first_wifi();
  struct wifi_scan_params params = {
      .scan_type = WIFI_SCAN_TYPE_ACTIVE,
      .dwell_time_active = 50,   /* ms per channel, active */
      .dwell_time_passive = 130, /* ms per channel, passive */
      .bands =
          (1 << WIFI_FREQ_BAND_2_4_GHZ), /* skip 5 GHz if you don't need it */
      .max_bss_cnt = 0, /* 0 = no cap; set e.g. 5 to stop early */
  };
  if (!iface) {
    LOG_ERR("Error: no WiFi interface found");
    return -ENODEV;
  }

  int err = net_if_up(iface);

  if (err && err != -EALREADY) {
    LOG_ERR("Error: net_if_up failed (%d)", err);
    return err;
  }

  LOG_INF("Starting WiFi scan...");

  err = net_mgmt(NET_REQUEST_WIFI_SCAN, iface, &params, sizeof(params));
  if (err) {
    LOG_ERR("Error: scan request failed (%d)", err);
    return err;
  }

  k_sem_take(&scan_done, K_FOREVER);
  return 0;
}
