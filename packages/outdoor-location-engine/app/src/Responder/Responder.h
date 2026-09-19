#ifndef RESPONDER_DOT_H       /* This is an "include guard" */
#define RESPONDER_DOT_H       /* prevents the file from being included twice. */
                              /* Including a header file twice causes all kinds */
                              /* of interesting problems.*/

#include <modem/lte_lc.h>

void publish_command_status_to_device_status_topic(const char *command,
                                                   const char *status,
                                                   const char *reason);

void publish_network_status_to_device_status_topic(
    const struct lte_lc_edrx_cfg *cfg);

void turn_on_active_mode();
void turn_on_lost_mode();
void respond_to_ping();
void respond_to_spoof_ping();
void stop_responding();


#endif
