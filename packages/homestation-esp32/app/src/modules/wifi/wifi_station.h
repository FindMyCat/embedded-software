#ifndef WIFI_STATION_H
#define WIFI_STATION_H

#include <stdbool.h>

/**
 * @brief Join the configured access point and wait for an IPv4 address.
 *
 * @return 0 once the interface is up and addressed, or a negative errno.
 */
int wifi_station_connect(void);

bool wifi_station_is_ready(void);

#endif /* WIFI_STATION_H */
