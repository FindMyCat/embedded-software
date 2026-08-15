#ifndef WIFI_SCAN_FRAME_H
#define WIFI_SCAN_FRAME_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Mirrors the collar wire format in outdoor-location-engine WiFi/wifi_scan.h */
#define WIFI_SCAN_MESSAGE_TYPE 0x01
#define WIFI_SCAN_MAX_ACCESS_POINTS 16
#define WIFI_SCAN_BYTES_PER_ACCESS_POINT 7
#define WIFI_SCAN_HEADER_BYTES 6

struct wifi_access_point {
    uint8_t bssid[6];
    int8_t rssi;
};

struct wifi_scan_report {
    uint32_t collar_uptime_s;
    size_t count;
    struct wifi_access_point access_points[WIFI_SCAN_MAX_ACCESS_POINTS];
};

/* Cheap type-byte check, so callers can route a payload before decoding it. */
bool wifi_scan_frame_matches(const uint8_t *payload, size_t len);

/**
 * @brief Decode a packed access point frame received from the collar.
 *
 * @return 0 on success, or a negative errno if the frame is malformed.
 */
int wifi_scan_frame_decode(const uint8_t *payload, size_t len,
                           struct wifi_scan_report *out);

void wifi_scan_report_log(const struct wifi_scan_report *report);

#endif /* WIFI_SCAN_FRAME_H */
