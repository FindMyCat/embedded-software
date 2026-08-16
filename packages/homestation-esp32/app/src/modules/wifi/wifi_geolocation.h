#ifndef WIFI_GEOLOCATION_H
#define WIFI_GEOLOCATION_H

#include "wifi_scan_frame.h"

struct wifi_location {
    double latitude;
    double longitude;
    double accuracy_m;
};

/**
 * @brief Resolve an access point list to a position via Google Geolocation.
 *
 * Blocking. Needs a joined WiFi station and at least two access points; the
 * API rejects fewer.
 *
 * @return 0 on success, or a negative errno.
 */
int wifi_geolocation_resolve(const struct wifi_scan_report *report,
                             struct wifi_location *out);

#endif /* WIFI_GEOLOCATION_H */
