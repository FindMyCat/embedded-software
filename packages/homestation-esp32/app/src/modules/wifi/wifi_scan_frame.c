#include "wifi_scan_frame.h"

#include <errno.h>
#include <string.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>

LOG_MODULE_REGISTER(wifi_scan_frame, CONFIG_WIFI_LOG_LEVEL);

bool wifi_scan_frame_matches(const uint8_t *payload, size_t len) {
  return payload && len > 0 && payload[0] == WIFI_SCAN_MESSAGE_TYPE;
}

int wifi_scan_frame_decode(const uint8_t *payload, size_t len,
                           struct wifi_scan_report *out) {
  if (!payload || !out) {
    return -EINVAL;
  }

  if (len < WIFI_SCAN_HEADER_BYTES) {
    LOG_WRN("Frame truncated (%u bytes)", (unsigned)len);
    return -EBADMSG;
  }

  if (payload[0] != WIFI_SCAN_MESSAGE_TYPE) {
    return -EINVAL;
  }

  uint8_t count = payload[1];
  size_t body = WIFI_SCAN_HEADER_BYTES + count * WIFI_SCAN_BYTES_PER_ACCESS_POINT;

  /* The battery byte trails the access points; older collars omit it. */
  if (count > WIFI_SCAN_MAX_ACCESS_POINTS ||
      (len != body && len != body + WIFI_SCAN_TRAILER_BYTES)) {
    LOG_WRN("Frame length mismatch (count %u, %u bytes)", count, (unsigned)len);
    return -EBADMSG;
  }

  out->collar_uptime_s = sys_get_le32(&payload[2]);
  out->count = count;
  out->battery_percent = len > body ? payload[body] : UINT8_MAX;

  for (uint8_t i = 0; i < count; i++) {
    const uint8_t *entry =
        &payload[WIFI_SCAN_HEADER_BYTES + i * WIFI_SCAN_BYTES_PER_ACCESS_POINT];

    memcpy(out->access_points[i].bssid, entry, 6);
    out->access_points[i].rssi = (int8_t)entry[6];
  }

  return 0;
}

void wifi_scan_report_log(const struct wifi_scan_report *report) {
  if (!report) {
    return;
  }

  LOG_INF("WiFi scan: %u access points, collar uptime %u s, battery %u%%",
          (unsigned)report->count, report->collar_uptime_s,
          report->battery_percent);

  for (size_t i = 0; i < report->count; i++) {
    const struct wifi_access_point *access_point = &report->access_points[i];

    LOG_INF("  %02x:%02x:%02x:%02x:%02x:%02x  %d dBm", access_point->bssid[0],
            access_point->bssid[1], access_point->bssid[2],
            access_point->bssid[3], access_point->bssid[4],
            access_point->bssid[5], access_point->rssi);
  }
}
