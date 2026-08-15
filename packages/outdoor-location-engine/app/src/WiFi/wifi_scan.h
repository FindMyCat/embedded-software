#ifndef WIFI_SCAN_H
#define WIFI_SCAN_H

#include <stddef.h>
#include <stdint.h>

#define WIFI_SCAN_MAX_ACCESS_POINTS 16
#define WIFI_SCAN_MESSAGE_TYPE 0x01

/* BSSID (6 bytes) plus RSSI (1 byte). */
#define WIFI_SCAN_BYTES_PER_ACCESS_POINT 7

/* Message type, access point count, and a 32-bit uptime stamp. */
#define WIFI_SCAN_HEADER_BYTES 6

#define WIFI_SCAN_PACKED_LENGTH(access_point_count)                            \
    (WIFI_SCAN_HEADER_BYTES +                                                  \
     (access_point_count) * WIFI_SCAN_BYTES_PER_ACCESS_POINT)

/* Geolocation lookups key on BSSID, so the SSID is not worth its bytes. */
struct wifi_access_point {
    uint8_t bssid[6];
    int8_t rssi;
};

int wifi_scan_init(void);
int wifi_scan_start(void);
int wifi_scan_stop(void);

/**
 * @brief Copy the strongest access points from the last scan, RSSI descending.
 *
 * Randomised BSSIDs and _nomap networks are excluded.
 *
 * @return Number of access points written, or a negative errno.
 */
struct wifi_scan_frame {
    uint8_t bytes[WIFI_SCAN_PACKED_LENGTH(WIFI_SCAN_MAX_ACCESS_POINTS)];
    size_t len;
};

int wifi_scan_get_strongest_access_points(struct wifi_access_point *out,
                                          size_t max_access_points);

/**
 * @brief Pack the strongest access points into a binary frame, little endian.
 *
 * u8 message type, u8 count, u32 uptime seconds, then per access point a
 * 6 byte BSSID and an int8 RSSI. Six access points fit a LoRa SF12 frame.
 *
 * @return 0 on success, or a negative errno.
 */
int wifi_scan_pack_strongest_access_points(struct wifi_scan_frame *out,
                                           size_t max_access_points);

#endif
