#ifndef MQTT_SN_DOT_H       /* This is an "include guard" */
#define MQTT_SN_DOT_H       /* prevents the file from being included twice. */
                            /* Including a header file twice causes all kinds */
                            /* of interesting problems.*/

#include <stdbool.h>

/**
 * @brief Use the device id the iOS app pushed over BLE as the client id.
 *
 * Traccar matches positions on this string, so it has to be the collar's own
 * identity rather than the compile-time default.
 *
 * @return 0 if applied, -EINVAL if empty or unchanged.
 */
int mqttsn_set_client_id(const char *device_id);

int mqttsn_initialize();
int mqttsn_disconnect();
int mqttsn_check_input();
int mqttsn_publish(char *data_str);

/**
 * @brief Publish a fixed dummy position at 0,0.
 *
 * Exercises the gateway, rule engine and Traccar without waiting for a GNSS
 * fix. The caller must already be connected and must keep calling
 * mqttsn_check_input() so replies are processed.
 *
 * @return 0 on success, negative errno otherwise.
 */
int mqttsn_publish_test(void);

bool get_mqttsn_connection_status();
#endif
