#ifndef MQTT_PUBLISHER_H
#define MQTT_PUBLISHER_H

#include <stdint.h>

void mqtt_publisher_start(void);

/**
 * Queue a position heard over LoRa for publishing on the location topic, in
 * the same format as the home position. Safe from any thread; held until the
 * broker is connected, keeping the newest if several pile up.
 *
 * @param battery Percent, UINT8_MAX when unknown.
 */
void mqtt_publisher_report_position(double latitude, double longitude,
				    double accuracy_m, uint8_t battery);

#endif /* MQTT_PUBLISHER_H */