#include "wifi_scan.h"
#include <string.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/net_event.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/sys/byteorder.h>

LOG_MODULE_REGISTER(wifi_scan, LOG_LEVEL_DBG);

static K_SEM_DEFINE(scan_done, 0, 1);
static struct net_mgmt_event_callback wifi_cb;

static struct wifi_access_point seen[WIFI_SCAN_MAX_ACCESS_POINTS];
static size_t seen_count;

/* Randomised BSSIDs are phone hotspots that move, and _nomap networks have
 * opted out of geolocation databases. Both hurt more than they help. */
static bool is_access_point_useful(const struct wifi_scan_result *result) {
  if (result->mac_length != 6 || (result->mac[0] & 0x02)) {
    return false;
  }

  return !(result->ssid_length >= 6 &&
           strncmp((const char *)&result->ssid[result->ssid_length - 6],
                   "_nomap", 6) == 0);
}

static void record_access_point(const struct wifi_scan_result *result) {
  for (size_t i = 0; i < seen_count; i++) {
    if (memcmp(seen[i].bssid, result->mac, 6) == 0) {
      seen[i].rssi = MAX(seen[i].rssi, (int8_t)result->rssi);
      return;
    }
  }

  size_t slot = seen_count;

  if (seen_count == ARRAY_SIZE(seen)) {
    /* Table is full, so evict the weakest entry rather than dropping a
     * newcomer that may well be stronger than something already held. */
    slot = 0;

    for (size_t i = 1; i < seen_count; i++) {
      if (seen[i].rssi < seen[slot].rssi) {
        slot = i;
      }
    }

    if (result->rssi <= seen[slot].rssi) {
      return;
    }
  } else {
    seen_count++;
  }

  memcpy(seen[slot].bssid, result->mac, 6);
  seen[slot].rssi = (int8_t)result->rssi;
}

int wifi_scan_get_strongest_access_points(struct wifi_access_point *out,
                                          size_t max_access_points) {
  struct wifi_access_point ranked[WIFI_SCAN_MAX_ACCESS_POINTS];

  if (!out || max_access_points == 0) {
    return -EINVAL;
  }

  memcpy(ranked, seen, seen_count * sizeof(ranked[0]));

  for (size_t i = 1; i < seen_count; i++) {
    struct wifi_access_point key = ranked[i];
    size_t j = i;

    while (j > 0 && ranked[j - 1].rssi < key.rssi) {
      ranked[j] = ranked[j - 1];
      j--;
    }
    ranked[j] = key;
  }

  size_t count = MIN(max_access_points, seen_count);

  memcpy(out, ranked, count * sizeof(out[0]));
  return (int)count;
}

int wifi_scan_pack_strongest_access_points(struct wifi_scan_frame *out,
                                           size_t max_access_points) {
  struct wifi_access_point top[WIFI_SCAN_MAX_ACCESS_POINTS];

  if (!out || max_access_points == 0 ||
      max_access_points > WIFI_SCAN_MAX_ACCESS_POINTS) {
    return -EINVAL;
  }

  int count = wifi_scan_get_strongest_access_points(top, max_access_points);
  if (count < 0) {
    return count;
  }

  out->bytes[0] = WIFI_SCAN_MESSAGE_TYPE;
  out->bytes[1] = (uint8_t)count;
  sys_put_le32((uint32_t)(k_uptime_get() / 1000), &out->bytes[2]);

  for (int i = 0; i < count; i++) {
    uint8_t *entry = &out->bytes[WIFI_SCAN_HEADER_BYTES +
                                 i * WIFI_SCAN_BYTES_PER_ACCESS_POINT];

    memcpy(entry, top[i].bssid, 6);
    entry[6] = (uint8_t)top[i].rssi;
  }

  out->len = WIFI_SCAN_PACKED_LENGTH((size_t)count);
  return 0;
}

static void wifi_event_handler(struct net_mgmt_event_callback *cb,
                               uint64_t event, struct net_if *iface) {
  if (event == NET_EVENT_WIFI_SCAN_RESULT) {
    const struct wifi_scan_result *r =
        (const struct wifi_scan_result *)cb->info;

    LOG_INF("  SSID: %-32s  BSSID: %02x:%02x:%02x:%02x:%02x:%02x  RSSI: %d dBm "
            " ch: %d",
            r->ssid_length ? (char *)r->ssid : "<hidden>", r->mac[0], r->mac[1],
            r->mac[2], r->mac[3], r->mac[4], r->mac[5], r->rssi, r->channel);

    if (is_access_point_useful(r)) {
      record_access_point(r);
    }
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

  seen_count = 0;

  err = net_mgmt(NET_REQUEST_WIFI_SCAN, iface, &params, sizeof(params));
  if (err) {
    LOG_ERR("Error: scan request failed (%d)", err);
    return err;
  }

  k_sem_take(&scan_done, K_FOREVER);
  return 0;
}

int wifi_scan_stop(void) {
  struct net_if *iface = net_if_get_first_wifi();
  if (!iface) {
    LOG_ERR("Error: no WiFi interface found");
    return -ENODEV;
  }

  int err = net_if_down(iface);
  if (err && err != -EALREADY) {
    LOG_ERR("Error: net_if_down failed (%d)", err);
    return err;
  }

  const struct device *gpio0 = DEVICE_DT_GET(DT_NODELABEL(gpio0));
  if (device_is_ready(gpio0)) {
    gpio_pin_configure(gpio0, 27, GPIO_OUTPUT_LOW);
    gpio_pin_configure(gpio0, 28, GPIO_OUTPUT_LOW);
  }

  LOG_INF("WiFi interface down, nRF7002 powered off");
  return 0;
}
