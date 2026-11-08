#ifndef DEVICE_SETTINGS_H
#define DEVICE_SETTINGS_H

#include <stddef.h>
#include <stdint.h>

/* Mirror of the nRF91's DeviceSettings/device_settings.h, which owns the
 * settings: it range-checks them and writes the block into fmc_config. This
 * side only reads it. Keep the enum identical: the stored id is the position. */
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
  DEVICE_SETTING_BLE_SCAN_INTERVAL_MS,
  DEVICE_SETTING_BLE_SCAN_WINDOW_MS,
  DEVICE_SETTING_BLE_HOME_STATION_TIMEOUT_S,
  DEVICE_SETTING_BLE_ADVERT_INTERVAL_MS,
  /* Append only, in step with the nRF91. */
  DEVICE_SETTING_COUNT,
};

int32_t device_settings_get(enum device_setting setting);

/* Replaces every value with the block's, defaults for the rest. An empty or
 * unknown-version block leaves all defaults. */
void device_settings_load_from_config(const uint8_t *settings_block,
                                      size_t block_length);

#endif /* DEVICE_SETTINGS_H */
