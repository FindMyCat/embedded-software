#ifndef PRESENCE_H
#define PRESENCE_H

#include <stdbool.h>

/* Which link reaches the home station, best first. LTE means neither BLE nor
 * LoRa hears it. */
enum presence_link {
  PRESENCE_LINK_BLE,
  PRESENCE_LINK_LORA,
  PRESENCE_LINK_LTE,
};

void presence_set_ble(bool in_range);
void presence_set_lora(bool in_range);

enum presence_link presence_link_get(void);
const char *presence_link_name(enum presence_link link);

#endif /* PRESENCE_H */
