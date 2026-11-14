#include "provisioning.h"

#include <errno.h>
#include <esp_attr.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>
#include <zephyr/sys/byteorder.h>

#include "ca_certificate.h"

LOG_MODULE_REGISTER(provisioning, LOG_LEVEL_INF);

#define SETTINGS_SUBTREE "prov"
#define SETTINGS_NAME "v1"
#define SETTINGS_KEY SETTINGS_SUBTREE "/" SETTINGS_NAME
#define FIELD_HEADER_BYTES 3
#define STORED_FIELDS_MAX_BYTES 3072
#define WINDOW_DURATION_MS (5 * 60 * MSEC_PER_SEC)
#define CERTIFICATE_HEADER "-----BEGIN CERTIFICATE-----"

struct provisioning_field {
  const char *name;
  uint16_t max_length;
  bool is_secret;
  bool (*is_valid)(const char *value, uint16_t length);
  char *value;
  bool is_stored;
};

static bool is_text(const char *value, uint16_t length) {
  return memchr(value, '\0', length) == NULL;
}

static bool is_digit(char character) {
  return character >= '0' && character <= '9';
}

static bool is_port(const char *value, uint16_t length) {
  if (length == 0 || length > 5) {
    return false;
  }
  for (uint16_t index = 0; index < length; index++) {
    if (!is_digit(value[index])) {
      return false;
    }
  }

  long port = strtol(value, NULL, 10);

  return port >= 1 && port <= 65535;
}

static bool is_coordinate(const char *value, uint16_t length) {
  for (uint16_t index = 0; index < length; index++) {
    bool is_leading_minus = index == 0 && value[index] == '-';

    if (!is_digit(value[index]) && value[index] != '.' && !is_leading_minus) {
      return false;
    }
  }
  return length > 0;
}

static bool is_certificate(const char *value, uint16_t length) {
  return is_text(value, length) && length > strlen(CERTIFICATE_HEADER) &&
         strncmp(value, CERTIFICATE_HEADER, strlen(CERTIFICATE_HEADER)) == 0;
}

static EXT_RAM_BSS_ATTR char wifi_ssid[32 + 1];
static EXT_RAM_BSS_ATTR char wifi_password[64 + 1];
static EXT_RAM_BSS_ATTR char device_id[36 + 1];
static EXT_RAM_BSS_ATTR char broker_host[128 + 1];
static EXT_RAM_BSS_ATTR char broker_port[5 + 1];
static EXT_RAM_BSS_ATTR char homestation_topic[64 + 1];
static EXT_RAM_BSS_ATTR char wifi_scan_topic[64 + 1];
static EXT_RAM_BSS_ATTR char broker_ca_certificate[2048 + 1];
static EXT_RAM_BSS_ATTR char home_latitude[16 + 1];
static EXT_RAM_BSS_ATTR char home_longitude[16 + 1];

#define FIELD(id, storage, validator, secret)                                  \
  [id] = {.name = #storage,                                                    \
          .max_length = sizeof(storage) - 1,                                   \
          .is_secret = secret,                                                 \
          .is_valid = validator,                                               \
          .value = storage}

static struct provisioning_field fields[PROVISIONING_FIELD_ID_END] = {
    FIELD(PROVISIONING_WIFI_SSID, wifi_ssid, is_text, false),
    FIELD(PROVISIONING_WIFI_PASSWORD, wifi_password, is_text, true),
    FIELD(PROVISIONING_DEVICE_ID, device_id, is_text, false),
    FIELD(PROVISIONING_BROKER_HOST, broker_host, is_text, false),
    FIELD(PROVISIONING_BROKER_PORT, broker_port, is_port, false),
    FIELD(PROVISIONING_HOMESTATION_TOPIC, homestation_topic, is_text, false),
    FIELD(PROVISIONING_WIFI_SCAN_TOPIC, wifi_scan_topic, is_text, false),
    FIELD(PROVISIONING_BROKER_CA_CERTIFICATE, broker_ca_certificate,
          is_certificate, true),
    FIELD(PROVISIONING_HOME_LATITUDE, home_latitude, is_coordinate, false),
    FIELD(PROVISIONING_HOME_LONGITUDE, home_longitude, is_coordinate, false),
};

static struct provisioning_field *find_field(uint8_t field_id) {
  if (field_id == 0 || field_id >= PROVISIONING_FIELD_ID_END) {
    return NULL;
  }
  return &fields[field_id];
}

static bool field_accepts(const struct provisioning_field *field,
                          const uint8_t *value, uint16_t length) {
  return length <= field->max_length &&
         (length == 0 || field->is_valid((const char *)value, length));
}

static const char *kconfig_value(enum provisioning_field_id field_id) {
  static char kconfig_broker_port[6];

  switch (field_id) {
  case PROVISIONING_WIFI_SSID:
    return CONFIG_WIFI_STATION_SSID;
  case PROVISIONING_WIFI_PASSWORD:
    return CONFIG_WIFI_STATION_PSK;
  case PROVISIONING_DEVICE_ID:
    return CONFIG_DEVICE_ID;
  case PROVISIONING_BROKER_HOST:
    return CONFIG_MQTT_PUBLISHER_BROKER_HOST;
  case PROVISIONING_BROKER_PORT:
    snprintf(kconfig_broker_port, sizeof(kconfig_broker_port), "%d",
             CONFIG_MQTT_PUBLISHER_BROKER_PORT);
    return kconfig_broker_port;
  case PROVISIONING_HOMESTATION_TOPIC:
    return CONFIG_MQTT_PUBLISHER_LOCATION_TOPIC;
  case PROVISIONING_WIFI_SCAN_TOPIC:
    return CONFIG_MQTT_PUBLISHER_WIFI_SCAN_TOPIC;
  case PROVISIONING_BROKER_CA_CERTIFICATE:
    return (const char *)ca_certificate;
  case PROVISIONING_HOME_LATITUDE:
    return CONFIG_MQTT_PUBLISHER_HOME_LATITUDE;
  case PROVISIONING_HOME_LONGITUDE:
    return CONFIG_MQTT_PUBLISHER_HOME_LONGITUDE;
  default:
    return "";
  }
}

const char *provisioning_get(enum provisioning_field_id field_id) {
  const struct provisioning_field *field = find_field(field_id);

  if (field == NULL) {
    return "";
  }
  return field->is_stored ? field->value : kconfig_value(field_id);
}

bool provisioning_is_complete(void) {
  static const enum provisioning_field_id required_fields[] = {
      PROVISIONING_WIFI_SSID,       PROVISIONING_DEVICE_ID,
      PROVISIONING_BROKER_HOST,     PROVISIONING_HOMESTATION_TOPIC,
      PROVISIONING_WIFI_SCAN_TOPIC, PROVISIONING_BROKER_CA_CERTIFICATE,
      PROVISIONING_HOME_LATITUDE,   PROVISIONING_HOME_LONGITUDE,
  };

  for (size_t index = 0; index < ARRAY_SIZE(required_fields); index++) {
    if (provisioning_get(required_fields[index])[0] == '\0') {
      return false;
    }
  }
  return true;
}

static atomic_t window_closes_at_ms;

void provisioning_window_open(void) {
  atomic_set(&window_closes_at_ms,
             (atomic_val_t)(k_uptime_get() + WINDOW_DURATION_MS));
  LOG_INF("Provisioning open for %d s", WINDOW_DURATION_MS / MSEC_PER_SEC);
}

bool provisioning_window_is_open(void) {
  return !provisioning_is_complete() ||
         k_uptime_get() < (int64_t)atomic_get(&window_closes_at_ms);
}

typedef int (*field_visitor)(uint8_t field_id, const uint8_t *value,
                             uint16_t length, void *context);

static int visit_encoded_fields(const uint8_t *encoded, size_t encoded_length,
                                field_visitor visit, void *context) {
  size_t offset = 0;

  while (offset < encoded_length) {
    if (encoded_length - offset < FIELD_HEADER_BYTES) {
      return -EBADMSG;
    }

    uint8_t field_id = encoded[offset];
    uint16_t value_length = sys_get_le16(&encoded[offset + 1]);

    offset += FIELD_HEADER_BYTES;
    if (value_length > encoded_length - offset) {
      return -EBADMSG;
    }

    int error = visit(field_id, &encoded[offset], value_length, context);

    if (error != 0) {
      return error;
    }
    offset += value_length;
  }
  return 0;
}

static int append_encoded_field(uint8_t *destination, size_t capacity,
                                size_t *offset, uint8_t field_id,
                                const void *value, uint16_t length) {
  if (capacity - *offset < FIELD_HEADER_BYTES + (size_t)length) {
    return -ENOMEM;
  }
  destination[*offset] = field_id;
  sys_put_le16(length, &destination[*offset + 1]);
  memcpy(&destination[*offset + FIELD_HEADER_BYTES], value, length);
  *offset += FIELD_HEADER_BYTES + length;
  return 0;
}

static int load_stored_field(uint8_t field_id, const uint8_t *value,
                             uint16_t length, void *context) {
  ARG_UNUSED(context);
  struct provisioning_field *field = find_field(field_id);

  if (field == NULL || !field_accepts(field, value, length)) {
    LOG_WRN("Stored field %u ignored", field_id);
    return 0;
  }
  memcpy(field->value, value, length);
  field->value[length] = '\0';
  field->is_stored = true;
  return 0;
}

static int load_stored_fields(const char *name, size_t length,
                              settings_read_cb read_stored,
                              void *read_context) {
  if (!settings_name_steq(name, SETTINGS_NAME, NULL)) {
    return -ENOENT;
  }
  if (length > STORED_FIELDS_MAX_BYTES) {
    return -EINVAL;
  }

  uint8_t *stored = k_malloc(length);

  if (stored == NULL) {
    return -ENOMEM;
  }

  int error =
      read_stored(read_context, stored, length) == (ssize_t)length
          ? visit_encoded_fields(stored, length, load_stored_field, NULL)
          : -EIO;

  k_free(stored);
  return error;
}

SETTINGS_STATIC_HANDLER_DEFINE(provisioning, SETTINGS_SUBTREE, NULL,
                               load_stored_fields, NULL, NULL);

int provisioning_init(void) {
  int error = settings_subsys_init();

  if (error == 0) {
    error = settings_load_subtree(SETTINGS_SUBTREE);
  }
  if (error != 0) {
    LOG_ERR("Error: loading provisioning failed (%d)", error);
  }

  LOG_INF("Provisioning %s",
          provisioning_is_complete() ? "complete" : "incomplete");
  return error;
}

int provisioning_encode_readable_fields(uint8_t *destination, size_t capacity) {
  size_t offset = 0;

  for (uint8_t field_id = 1; field_id < PROVISIONING_FIELD_ID_END; field_id++) {
    if (fields[field_id].is_secret) {
      continue;
    }

    const char *value = provisioning_get(field_id);

    if (append_encoded_field(destination, capacity, &offset, field_id, value,
                             strlen(value)) != 0) {
      return -ENOMEM;
    }
  }
  return (int)offset;
}

struct update_validation {
  uint32_t updated_field_ids;
  char *failure_reason;
  size_t failure_reason_capacity;
};

static int validate_update_field(uint8_t field_id, const uint8_t *value,
                                 uint16_t length, void *context) {
  struct update_validation *validation = context;
  const struct provisioning_field *field = find_field(field_id);

  if (field == NULL) {
    snprintf(validation->failure_reason, validation->failure_reason_capacity,
             "unknown field %u", field_id);
    return -EINVAL;
  }
  if (validation->updated_field_ids & BIT(field_id)) {
    snprintf(validation->failure_reason, validation->failure_reason_capacity,
             "%s sent twice", field->name);
    return -EINVAL;
  }
  if (!field_accepts(field, value, length)) {
    snprintf(validation->failure_reason, validation->failure_reason_capacity,
             "bad %s", field->name);
    return -EINVAL;
  }
  validation->updated_field_ids |= BIT(field_id);
  return 0;
}

struct update_field_lookup {
  uint8_t field_id;
  const uint8_t *value;
  uint16_t length;
};

static int match_update_field(uint8_t field_id, const uint8_t *value,
                              uint16_t length, void *context) {
  struct update_field_lookup *lookup = context;

  if (field_id == lookup->field_id) {
    lookup->value = value;
    lookup->length = length;
  }
  return 0;
}

static int merge_update_with_stored_fields(const uint8_t *update,
                                           size_t update_length,
                                           uint32_t updated_field_ids,
                                           uint8_t *merged,
                                           size_t *merged_length) {
  for (uint8_t field_id = 1; field_id < PROVISIONING_FIELD_ID_END; field_id++) {
    int error = 0;

    if (updated_field_ids & BIT(field_id)) {
      struct update_field_lookup lookup = {.field_id = field_id};

      (void)visit_encoded_fields(update, update_length, match_update_field,
                                 &lookup);
      error =
          append_encoded_field(merged, STORED_FIELDS_MAX_BYTES, merged_length,
                               field_id, lookup.value, lookup.length);
    } else if (fields[field_id].is_stored) {
      error = append_encoded_field(
          merged, STORED_FIELDS_MAX_BYTES, merged_length, field_id,
          fields[field_id].value, strlen(fields[field_id].value));
    }

    if (error != 0) {
      return error;
    }
  }
  return 0;
}

int provisioning_save_update(const uint8_t *update, size_t update_length,
                             char *failure_reason,
                             size_t failure_reason_capacity) {
  struct update_validation validation = {
      .failure_reason = failure_reason,
      .failure_reason_capacity = failure_reason_capacity,
  };

  failure_reason[0] = '\0';

  int error = visit_encoded_fields(update, update_length, validate_update_field,
                                   &validation);

  if (error == -EBADMSG) {
    snprintf(failure_reason, failure_reason_capacity, "malformed update");
  }
  if (error != 0) {
    return -EINVAL;
  }
  if (validation.updated_field_ids == 0) {
    snprintf(failure_reason, failure_reason_capacity, "empty update");
    return -EINVAL;
  }

  uint8_t *merged = k_malloc(STORED_FIELDS_MAX_BYTES);

  if (merged == NULL) {
    snprintf(failure_reason, failure_reason_capacity, "out of memory");
    return -ENOMEM;
  }

  size_t merged_length = 0;

  error = merge_update_with_stored_fields(update, update_length,
                                          validation.updated_field_ids, merged,
                                          &merged_length);
  if (error != 0) {
    snprintf(failure_reason, failure_reason_capacity, "too big");
  } else {
    error = settings_save_one(SETTINGS_KEY, merged, merged_length);
    if (error != 0) {
      snprintf(failure_reason, failure_reason_capacity, "not saved (%d)",
               error);
    }
  }

  k_free(merged);
  return error;
}
