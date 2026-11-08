#ifndef DEVICE_SETTINGS_H
#define DEVICE_SETTINGS_H

#include <stddef.h>
#include <stdint.h>

/* Remotely tunable values. The nRF52's fmc_config is the source of truth, so
 * they survive reboots and updates; Kconfig gives each one's default. Users
 * read them when needed; this module has no side effects. */
enum device_setting {
  DEVICE_SETTING_LOST_MODE_REPORT_INTERVAL_S,
  DEVICE_SETTING_ACTIVE_MODE_REPORT_INTERVAL_S,
  DEVICE_SETTING_LORA_ABSENCE_TIMEOUT_S,
  DEVICE_SETTING_EDRX_CYCLE_CODE,
  DEVICE_SETTING_EDRX_PAGING_WINDOW_CODE,
  DEVICE_SETTING_WIFI_MINIMUM_ACCESS_POINTS,
  DEVICE_SETTING_GNSS_TIMEOUT_S,
  DEVICE_SETTING_GNSS_ACCURACY,
  DEVICE_SETTING_GNSS_BEFORE_WIFI,
  DEVICE_SETTING_LORA_ENABLED,
  DEVICE_SETTING_LORA_BURST_MS,
  DEVICE_SETTING_LORA_BURST_PERIOD_S,
  DEVICE_SETTING_LORA_HEARTBEAT_S,
  /* Applied by the nRF52, which decodes the block itself; ids must match its
   * ble_settings.h. */
  DEVICE_SETTING_BLE_SCAN_INTERVAL_MS,
  DEVICE_SETTING_BLE_SCAN_WINDOW_MS,
  DEVICE_SETTING_BLE_HOME_STATION_TIMEOUT_S,
  DEVICE_SETTING_BLE_ADVERT_INTERVAL_MS,
  /* Append only: the stored id is the position here. */
  DEVICE_SETTING_COUNT,
};

/* Call before anything reads a setting. */
void device_settings_init_defaults(void);

int32_t device_settings_get(enum device_setting setting);

/* Takes the stored values from fmc_config.settings. An empty block keeps the
 * defaults. */
void device_settings_load_from_config(const uint8_t *settings_block,
                                      size_t block_length);

/* Takes "key=value,key=value", range-checks every pair, then stores all of
 * them on the nRF52 or none. Reports the outcome on the status topic. */
int device_settings_update_from_pairs(const char *key_value_pairs,
                                      size_t length);

/* Publishes every current value on the status topic, so the app shows what
 * the collar holds rather than what was sent. */
void device_settings_report_current_values(void);

#endif /* DEVICE_SETTINGS_H */
