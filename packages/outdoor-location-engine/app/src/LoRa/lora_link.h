#ifndef LORA_LINK_H
#define LORA_LINK_H

#include <stdbool.h>

#include "../WiFi/wifi_scan.h"
#include "lora_frame.h"

int lora_link_start(void);

void lora_link_set_device_id(const char *device_id);

/* True while the module still hears the home station.*/ 
bool lora_link_homestation_in_range(void);

void lora_link_set_enabled(bool enabled);

/* Logs the module's sniffs:detects:packets counters. */
void lora_link_log_stats(void);

int lora_link_send_position(const struct lora_frame_position *position);

/* Echoes the ping's counter, so the station stops pinging and listens. */
int lora_link_send_ack(uint32_t counter);

/* Sends access points as seen; the station resolves them into a position. */
int lora_link_send_wifi_scan(const struct wifi_scan_frame *scan);

#endif /* LORA_LINK_H */
