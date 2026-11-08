#include "device_settings.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <modem/location.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/byteorder.h>

#include "../DeviceStatus/device_status_reporting.h"
#include "../communication_channels/nrf52_link.h"

LOG_MODULE_REGISTER(device_settings,
                    CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

/* Block: format version, then entries of 1-byte id (setting + 1) and an
 * unsigned LEB128 value (7 bits per byte, low first, top bit set on all but
 * the last), until an id of 0. Any other version loads as no settings. Only
 * explicitly set values are stored, so the rest keep following their
 * defaults. */
#define SETTINGS_BLOCK_FORMAT_VERSION 1
/* Enough for any uint32. */
#define VARINT_MAX_BYTES 5
/* Every setting at its widest is ~400 bytes; the UDP listener takes 255. */
#define KEY_VALUE_PAIRS_MAX_BYTES 240

struct setting_definition {
  const char *key;
  int32_t min_value;
  int32_t max_value;
};

static const struct setting_definition definitions[DEVICE_SETTING_COUNT] = {
    [DEVICE_SETTING_LOST_MODE_REPORT_INTERVAL_S] = {"lost_interval_s", 10,
                                                    86400},
    [DEVICE_SETTING_ACTIVE_MODE_REPORT_INTERVAL_S] = {"active_interval_s", 60,
                                                      86400},
    [DEVICE_SETTING_LORA_ABSENCE_TIMEOUT_S] = {"lora_absence_timeout_s", 10,
                                               600},
    /* 4-bit codes from 3GPP TS 24.008, as in CONFIG_LTE_EDRX_REQ_VALUE_LTE_M. */
    [DEVICE_SETTING_EDRX_CYCLE_CODE] = {"edrx_cycle_code", 0, 15},
    [DEVICE_SETTING_EDRX_PAGING_WINDOW_CODE] = {"edrx_paging_window_code", 0,
                                                15},
    /* Fewer access points than this falls through to GNSS. */
    [DEVICE_SETTING_WIFI_MINIMUM_ACCESS_POINTS] = {"wifi_minimum_access_points",
                                                   1, 6},
    [DEVICE_SETTING_GNSS_TIMEOUT_S] = {"gnss_timeout_s", 30, 300},
    /* Pings and lost mode. 0 low, 1 normal, 2 high, as enum location_accuracy. */
    [DEVICE_SETTING_GNSS_ACCURACY] = {"gnss_accuracy", 0, 2},
    /* 1 tries GNSS first, WiFi only if it fails. */
    [DEVICE_SETTING_GNSS_BEFORE_WIFI] = {"gnss_before_wifi", 0, 1},
    /* 0 skips LoRa: out of BLE range goes straight to LTE. */
    [DEVICE_SETTING_LORA_ENABLED] = {"lora_enabled", 0, 1},
    /* 0 sniffs continuously. */
    [DEVICE_SETTING_LORA_BURST_MS] = {"lora_burst_ms", 0, 10000},
    [DEVICE_SETTING_LORA_BURST_PERIOD_S] = {"lora_burst_period_s", 1, 600},
    /* 0 sends none. */
    [DEVICE_SETTING_LORA_HEARTBEAT_S] = {"lora_heartbeat_s", 0, 3600},
    /* SoftDevice limits: 2.5 ms to 40.96 s. */
    [DEVICE_SETTING_BLE_SCAN_INTERVAL_MS] = {"ble_scan_interval_ms", 3, 40960},
    [DEVICE_SETTING_BLE_SCAN_WINDOW_MS] = {"ble_scan_window_ms", 3, 40960},
    [DEVICE_SETTING_BLE_HOME_STATION_TIMEOUT_S] = {
        "ble_home_station_timeout_s", 10, 3600},
    /* 20 ms to 10.24 s, the legacy advertising range. */
    [DEVICE_SETTING_BLE_ADVERT_INTERVAL_MS] = {"ble_advert_interval_ms", 20,
                                               10240},
};

static atomic_t current_values[DEVICE_SETTING_COUNT];
static atomic_t explicitly_set_mask;

/* Current values with the parsed pairs laid over them. */
struct setting_changes {
  int32_t values[DEVICE_SETTING_COUNT];
  atomic_val_t explicitly_set_mask;
  int pair_count;
  char rejected_pair[32];
};

static int32_t default_value(enum device_setting setting) {
  switch (setting) {
  case DEVICE_SETTING_LOST_MODE_REPORT_INTERVAL_S:
    return CONFIG_LOST_MODE_LOCATION_REFRESH_PERIOD_SECONDS;
  case DEVICE_SETTING_ACTIVE_MODE_REPORT_INTERVAL_S:
    return CONFIG_ACTIVE_MODE_LOCATION_REFRESH_PERIOD_SECONDS;
  case DEVICE_SETTING_LORA_ABSENCE_TIMEOUT_S:
#if defined(CONFIG_LORA_LINK)
    return CONFIG_LORA_LINK_ABSENCE_S;
#else
    return 0;
#endif
  case DEVICE_SETTING_EDRX_CYCLE_CODE:
    return strtol(CONFIG_LTE_EDRX_REQ_VALUE_LTE_M, NULL, 2);
  case DEVICE_SETTING_EDRX_PAGING_WINDOW_CODE:
    return strtol(CONFIG_LTE_PTW_VALUE_LTE_M, NULL, 2);
  case DEVICE_SETTING_WIFI_MINIMUM_ACCESS_POINTS:
    return 2;
  case DEVICE_SETTING_GNSS_TIMEOUT_S:
    return CONFIG_LOCATION_REQUEST_DEFAULT_GNSS_TIMEOUT / MSEC_PER_SEC;
  case DEVICE_SETTING_GNSS_ACCURACY:
    return LOCATION_ACCURACY_LOW;
  case DEVICE_SETTING_GNSS_BEFORE_WIFI:
    return 0;
#if defined(CONFIG_LORA_LINK)
  case DEVICE_SETTING_LORA_ENABLED:
    return 1;
  case DEVICE_SETTING_LORA_BURST_MS:
    return CONFIG_LORA_LINK_BURST_MS;
  case DEVICE_SETTING_LORA_BURST_PERIOD_S:
    return CONFIG_LORA_LINK_BURST_PERIOD_S;
  case DEVICE_SETTING_LORA_HEARTBEAT_S:
    return CONFIG_LORA_LINK_HEARTBEAT_S;
#endif
  /* Mirror the nRF52's built-in values, which apply until one is set. */
  case DEVICE_SETTING_BLE_SCAN_INTERVAL_MS:
    return 30000;
  case DEVICE_SETTING_BLE_SCAN_WINDOW_MS:
    return 62;
  case DEVICE_SETTING_BLE_HOME_STATION_TIMEOUT_S:
    return 120;
  case DEVICE_SETTING_BLE_ADVERT_INTERVAL_MS:
    return 4000;
  default:
    return 0;
  }
}

static bool is_in_range(int setting, int32_t value) {
  return value >= definitions[setting].min_value &&
         value <= definitions[setting].max_value;
}

static int find_setting_by_key(const char *key) {
  for (int setting = 0; setting < DEVICE_SETTING_COUNT; setting++) {
    if (strcmp(key, definitions[setting].key) == 0) {
      return setting;
    }
  }

  return -1;
}

void device_settings_init_defaults(void) {
  for (int setting = 0; setting < DEVICE_SETTING_COUNT; setting++) {
    atomic_set(&current_values[setting], default_value(setting));
  }
}

int32_t device_settings_get(enum device_setting setting) {
  return (int32_t)atomic_get(&current_values[setting]);
}

/* Returns bytes read, or 0 if the value runs past the end. */
static size_t read_varint(const uint8_t *bytes, size_t available,
                          uint32_t *value) {
  *value = 0;

  for (size_t index = 0; index < available && index < VARINT_MAX_BYTES;
       index++) {
    *value |= (uint32_t)(bytes[index] & 0x7F) << (7 * index);

    bool is_last_byte = (bytes[index] & 0x80) == 0;

    if (is_last_byte) {
      return index + 1;
    }
  }

  return 0;
}

/* Returns bytes written; out needs VARINT_MAX_BYTES. */
static size_t write_varint(uint32_t value, uint8_t *out) {
  size_t length = 0;

  do {
    uint8_t low_bits = value & 0x7F;

    value >>= 7;
    bool is_last_byte = value == 0;

    out[length++] = is_last_byte ? low_bits : (low_bits | 0x80);
  } while (value != 0);

  return length;
}

void device_settings_load_from_config(const uint8_t *settings_block,
                                      size_t block_length) {
  if (block_length < 1 ||
      settings_block[0] != SETTINGS_BLOCK_FORMAT_VERSION) {
    LOG_INF("No stored settings, using defaults");
    return;
  }

  size_t offset = 1;

  while (offset < block_length && settings_block[offset] != 0) {
    int setting = settings_block[offset] - 1;
    uint32_t value;
    size_t value_length = read_varint(&settings_block[offset + 1],
                                      block_length - offset - 1, &value);

    /* Nothing after a cut-off value can be trusted to line up. */
    if (value_length == 0) {
      LOG_WRN("Stored settings truncated at setting %d", setting);
      return;
    }

    offset += 1 + value_length;

    /* Skipped, not trusted: ranges may have tightened since it was stored. */
    if (setting >= DEVICE_SETTING_COUNT || !is_in_range(setting, value)) {
      LOG_WRN("Stored setting %d=%d ignored", setting, value);
      continue;
    }

    atomic_set(&current_values[setting], value);
    atomic_or(&explicitly_set_mask, BIT(setting));
    LOG_INF("Setting %s=%d", definitions[setting].key, value);
  }
}

/* Fills changes from "key=value,key=value"; on a bad pair names it and fails,
 * so a typo changes nothing. */
static int parse_key_value_pairs(const char *key_value_pairs, size_t length,
                                 struct setting_changes *changes) {
  char pairs[KEY_VALUE_PAIRS_MAX_BYTES];

  for (int setting = 0; setting < DEVICE_SETTING_COUNT; setting++) {
    changes->values[setting] = device_settings_get(setting);
  }
  changes->explicitly_set_mask = atomic_get(&explicitly_set_mask);
  changes->pair_count = 0;

  if (length >= sizeof(pairs)) {
    strcpy(changes->rejected_pair, "too long");
    return -EINVAL;
  }

  memcpy(pairs, key_value_pairs, length);
  pairs[length] = '\0';

  char *remaining;

  for (char *pair = strtok_r(pairs, ",", &remaining); pair != NULL;
       pair = strtok_r(NULL, ",", &remaining)) {
    strncpy(changes->rejected_pair, pair, sizeof(changes->rejected_pair) - 1);
    changes->rejected_pair[sizeof(changes->rejected_pair) - 1] = '\0';

    char *equals_sign = strchr(pair, '=');

    if (equals_sign == NULL) {
      return -EINVAL;
    }

    *equals_sign = '\0';

    int setting = find_setting_by_key(pair);
    char *value_end;
    long value = strtol(equals_sign + 1, &value_end, 10);

    if (setting < 0 || value_end == equals_sign + 1 || *value_end != '\0' ||
        !is_in_range(setting, value)) {
      return -EINVAL;
    }

    changes->values[setting] = (int32_t)value;
    changes->explicitly_set_mask |= BIT(setting);
    changes->pair_count++;
  }

  if (changes->pair_count == 0) {
    strcpy(changes->rejected_pair, "no pairs");
    return -EINVAL;
  }

  return 0;
}

/* Mirror of device_settings_load_from_config. -ENOSPC if the set values don't
 * fit with the closing 0 id. */
static int encode_settings_block(const struct setting_changes *changes,
                                 uint8_t settings_block[FMC_SETTINGS_BYTES]) {
  memset(settings_block, 0, FMC_SETTINGS_BYTES);
  settings_block[0] = SETTINGS_BLOCK_FORMAT_VERSION;

  size_t offset = 1;

  for (int setting = 0; setting < DEVICE_SETTING_COUNT; setting++) {
    if (!(changes->explicitly_set_mask & BIT(setting))) {
      continue;
    }

    uint8_t value_bytes[VARINT_MAX_BYTES];
    /* Ranges are non-negative, so the cast keeps the value. */
    size_t value_length =
        write_varint((uint32_t)changes->values[setting], value_bytes);

    if (offset + 1 + value_length >= FMC_SETTINGS_BYTES) {
      return -ENOSPC;
    }

    settings_block[offset] = (uint8_t)(setting + 1);
    memcpy(&settings_block[offset + 1], value_bytes, value_length);
    offset += 1 + value_length;
  }

  return 0;
}

static void commit_changes(const struct setting_changes *changes) {
  for (int setting = 0; setting < DEVICE_SETTING_COUNT; setting++) {
    atomic_set(&current_values[setting], changes->values[setting]);
  }
  atomic_set(&explicitly_set_mask, changes->explicitly_set_mask);
}

/* Reported under the command's name, so the app matches it to the request. */
#define UPDATE_SETTINGS_COMMAND "update-settings"

static int report_failure(const char *reason) {
  LOG_WRN("Settings update failed: %s", reason);
  device_status_report_command(UPDATE_SETTINGS_COMMAND, "fail", reason);
  return -EINVAL;
}

static int report_success(int pair_count) {
  char reason[32];

  snprintf(reason, sizeof(reason), "%d saved", pair_count);
  LOG_INF("Settings updated: %s", reason);
  device_status_report_command(UPDATE_SETTINGS_COMMAND, "ok", reason);
  return 0;
}

int device_settings_update_from_pairs(const char *key_value_pairs,
                                      size_t length) {
  struct setting_changes changes;

  if (parse_key_value_pairs(key_value_pairs, length, &changes) != 0) {
    return report_failure(changes.rejected_pair);
  }

  uint8_t settings_block[FMC_SETTINGS_BYTES];

  if (encode_settings_block(&changes, settings_block) != 0) {
    return report_failure("block full");
  }

  /* The nRF52 holds the truth: nothing changes unless it took the block. */
  if (nrf52_write_settings(settings_block) != 0) {
    return report_failure("not saved");
  }

  commit_changes(&changes);
  device_settings_report_current_values();
  return report_success(changes.pair_count);
}

void device_settings_report_current_values(void) {
  struct device_status report = {.kind = DEVICE_STATUS_SETTINGS};

  for (int setting = 0; setting < DEVICE_SETTING_COUNT; setting++) {
    report.settings[setting] = device_settings_get(setting);
  }

  device_status_report(&report);
}
