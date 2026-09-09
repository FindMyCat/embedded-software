#include "diagnostics.h"

#include <zephyr/logging/log.h>

#include "wifi_scan.h"

LOG_MODULE_REGISTER(wifi_diag, CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

#define ACCESS_POINTS 6

int wifi_diagnostics(void)
{
    struct wifi_access_point access_points[ACCESS_POINTS];

    LOG_INF("WiFi diagnostics:");

    int err = wifi_scan_init();

    if (err) {
        LOG_ERR("  FAIL scan init (%d)", err);
        return err;
    }

    err = wifi_scan_start();

    if (err) {
        LOG_ERR("  FAIL scan start (%d)", err);
        return err;
    }

    wifi_scan_stop();

    int found = wifi_scan_get_strongest_access_points(access_points, ACCESS_POINTS);

    if (found < 0) {
        LOG_ERR("  FAIL reading scan results (%d)", found);
        return found;
    }

    if (found == 0) {
        LOG_WRN("  no usable access points, so a WiFi position is not possible here");
        return 0;
    }

    LOG_INF("  PASS %d usable access points, strongest %d dBm", found,
            access_points[0].rssi);

    return 0;
}