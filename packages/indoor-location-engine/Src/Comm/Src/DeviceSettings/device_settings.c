#include "device_settings.h"

#include <stdbool.h>

#include "nrf_log.h"

/* Block layout is the nRF91's (see its device_settings.c): format version,
 * then entries of 1-byte id (setting + 1) and an unsigned LEB128 value, until
 * an id of 0. */
#define SETTINGS_BLOCK_FORMAT_VERSION 1
/* Enough for any uint32. */
#define VARINT_MAX_BYTES 5

/* Only the settings this chip acts on; the rest are the nRF91's and stay 0.
 * The nRF91 reports these same numbers until one is set. */
#define DEFAULT_VALUES                                                         \
  {                                                                            \
    [DEVICE_SETTING_BLE_SCAN_INTERVAL_MS] = 30000,                             \
    [DEVICE_SETTING_BLE_SCAN_WINDOW_MS] = 62,                                  \
    [DEVICE_SETTING_BLE_HOME_STATION_TIMEOUT_S] = 120,                         \
    [DEVICE_SETTING_BLE_ADVERT_INTERVAL_MS] = 4000,                            \
  }

static const int32_t default_values[DEVICE_SETTING_COUNT] = DEFAULT_VALUES;

/* Starts at the defaults: with nothing saved, no block is ever loaded. Read
 * from tasks and written from the timer task; aligned 32-bit stores are single
 * writes on Cortex-M. */
static volatile int32_t current_values[DEVICE_SETTING_COUNT] = DEFAULT_VALUES;

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

int32_t device_settings_get(enum device_setting setting) {
  return current_values[setting];
}

void device_settings_load_from_config(const uint8_t *settings_block,
                                      size_t block_length) {
  for (int setting = 0; setting < DEVICE_SETTING_COUNT; setting++) {
    current_values[setting] = default_values[setting];
  }

  if (block_length < 1 ||
      settings_block[0] != SETTINGS_BLOCK_FORMAT_VERSION) {
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
      NRF_LOG_WARNING("Stored settings truncated at setting %d", setting);
      return;
    }

    offset += 1 + value_length;

    /* Newer than this firmware: the nRF91 may know settings we don't. */
    if (setting >= DEVICE_SETTING_COUNT) {
      continue;
    }

    current_values[setting] = (int32_t)value;
  }
}
