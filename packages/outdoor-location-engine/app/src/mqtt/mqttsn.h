#ifndef MQTT_SN_DOT_H       /* This is an "include guard" */
#define MQTT_SN_DOT_H       /* prevents the file from being included twice. */
                            /* Including a header file twice causes all kinds */
                            /* of interesting problems.*/

#include <stdbool.h>

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
