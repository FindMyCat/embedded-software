#ifndef STATUS_LED_H
#define STATUS_LED_H

#include <stdint.h>

void status_led_start(void);

int status_led_show_rgb(uint8_t r, uint8_t g, uint8_t b);

/* Brief flash for a collar heartbeat, then back to the usual colour. */
void status_led_heartbeat(void);

#endif /* STATUS_LED_H */
