#ifndef MQTT_PUBLISHER_H
#define MQTT_PUBLISHER_H

#include <stddef.h>
#include <stdint.h>

void mqtt_publisher_start(void);

/**
 * Queue a position heard over LoRa for publishing on the location topic, in
 * the same format as the home position.
 *
 * @param battery Percent, UINT8_MAX when unknown.
 */
void mqtt_publisher_report_position(double latitude, double longitude,
				    double accuracy_m, uint8_t battery);

/**
 * Queue a WiFi scan frame heard over LoRa for publishing, as hex, on the WiFi
 * scan topic, where the server resolves it to a position.
 */
void mqtt_publisher_report_wifi_scan(const uint8_t *frame, size_t len);

#endif /* MQTT_PUBLISHER_H */