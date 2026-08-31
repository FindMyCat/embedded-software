#ifndef LORA_LINK_H
#define LORA_LINK_H

#include <stdbool.h>

#include "lora_frame.h"

int lora_link_start(void);

/* Identity carried in outgoing frames. Defaults to CONFIG_DEVICE_ID; */
void lora_link_set_device_id(const char *device_id);

/* True while the module still hears the home station. Driven by the module's
 * own absence timer, not by counting decoded pings here. */
bool lora_link_homestation_in_range(void);

/* Logs the module's sniffs:detects:packets counters. */
void lora_link_log_stats(void);

int lora_link_send_position(const struct lora_frame_position *position);

#endif /* LORA_LINK_H */
