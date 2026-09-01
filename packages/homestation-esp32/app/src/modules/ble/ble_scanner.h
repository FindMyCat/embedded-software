#ifndef BLE_SCANNER_H
#define BLE_SCANNER_H

#include <stdbool.h>
#include <stdint.h>

int ble_scanner_start(void);

/* True while the tag has advertised within CONFIG_BLE_SCANNER_ABSENCE_S. */
bool ble_scanner_tag_in_range(void);

/* Battery percentage from the tag's last advertisement, or UINT8_MAX if it has
 * not reported one. */
uint8_t ble_scanner_tag_battery_percent(void);

#endif /* BLE_SCANNER_H */