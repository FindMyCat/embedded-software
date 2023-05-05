#ifndef MQTT_SN_DOT_H       /* This is an "include guard" */
#define MQTT_SN_DOT_H       /* prevents the file from being included twice. */
                            /* Including a header file twice causes all kinds */
                            /* of interesting problems.*/

int mqttsn_initialize();
int mqttsn_check_input();
int mqttsn_publish();

#endif
