#ifndef MQTT_SN_DOT_H       /* This is an "include guard" */
#define MQTT_SN_DOT_H       /* prevents the file from being included twice. */
                            /* Including a header file twice causes all kinds */
                            /* of interesting problems.*/

#include <stdbool.h>

/**
 * @brief Use the device id and hostname the iOS/Android app pushed over BLE.
 *
 * @return 0 if applied, -EINVAL if empty or unchanged.
 */
int initialize_mqttsn_config(const char *device_id, const char *hostname);

int mqttsn_initialize();
int mqttsn_disconnect();
int mqttsn_check_input();
int mqttsn_publish(char *data_str);

/**
 * @brief Publish a payload to the given topic.
 *
 * @return 0 if queued, -ENOTCONN if no session is live, negative errno otherwise.
 */
int mqttsn_publish_topic(const char *topic, const char *data_str);

/**
 * @brief Publish a payload to the given topic, opening a session if needed.
 *
 * @return 0 if the message was queued, negative errno otherwise.
 */
int mqttsn_publish_with_session(const char *topic, const char *data_str);

/**
 * @brief Publish a fixed dummy position.
 *
 * @return 0 on success, negative errno otherwise.
 */
int mqttsn_publish_test(void);

bool get_mqttsn_connection_status();
#endif
