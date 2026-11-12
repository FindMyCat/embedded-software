#ifndef PROVISIONING_H
#define PROVISIONING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <zephyr/bluetooth/uuid.h>

#define PROVISIONING_UUID_VALUE(suffix)                                        \
	BT_UUID_128_ENCODE(0xF1D10000 | (suffix), 0x6E8B, 0x4C2A, 0x9F3E, 0x2B7A5C0D4E11)
#define PROVISIONING_SERVICE_UUID_VALUE PROVISIONING_UUID_VALUE(0x0001)

/* Ids go over the air, so they are append only. */
enum provisioning_field_id {
	PROVISIONING_WIFI_SSID = 1,
	PROVISIONING_WIFI_PASSWORD = 2,
	PROVISIONING_DEVICE_ID = 3,
	PROVISIONING_BROKER_HOST = 4,
	PROVISIONING_BROKER_PORT = 5,
	PROVISIONING_HOMESTATION_TOPIC = 6,
	PROVISIONING_WIFI_SCAN_TOPIC = 7,
	PROVISIONING_BROKER_CA_CERTIFICATE = 8,
	PROVISIONING_HOME_LATITUDE = 9,
	PROVISIONING_HOME_LONGITUDE = 10,
	PROVISIONING_FIELD_ID_END,
};

int provisioning_init(void);

/* Falls back to the Kconfig value for a field never provisioned. Never NULL. */
const char *provisioning_get(enum provisioning_field_id field_id);

bool provisioning_is_complete(void);

/* Holding the board button opens the window. */
void provisioning_button_start(void);

void provisioning_window_open(void);
bool provisioning_window_is_open(void);

/* Fields as [id][length, 2 bytes LE][value], leaving out the secret ones.
 * Returns bytes written, or -ENOMEM. */
int provisioning_encode_readable_fields(uint8_t *destination, size_t capacity);

/* Saves every field in the update over the stored ones, all or nothing.
 * Takes effect after a reboot. */
int provisioning_save_update(const uint8_t *update, size_t update_length,
			     char *failure_reason, size_t failure_reason_capacity);

#endif /* PROVISIONING_H */