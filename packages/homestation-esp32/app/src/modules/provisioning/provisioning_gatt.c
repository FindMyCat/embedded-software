#include <errno.h>
#include <esp_attr.h>
#include <string.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/reboot.h>

#include "provisioning.h"

LOG_MODULE_REGISTER(provisioning_gatt, LOG_LEVEL_INF);

#define SERVICE_UUID BT_UUID_DECLARE_128(PROVISIONING_SERVICE_UUID_VALUE)
#define UPDATE_UUID BT_UUID_DECLARE_128(PROVISIONING_UUID_VALUE(0x0002))
#define RESULT_UUID BT_UUID_DECLARE_128(PROVISIONING_UUID_VALUE(0x0003))
#define FIELDS_UUID BT_UUID_DECLARE_128(PROVISIONING_UUID_VALUE(0x0004))

enum update_command {
	UPDATE_COMMAND_BEGIN = 0x01,
	UPDATE_COMMAND_APPEND = 0x02,
	UPDATE_COMMAND_SAVE = 0x03,
};

enum update_result_code {
	UPDATE_RESULT_SAVED = 0x00,
	UPDATE_RESULT_FAILED = 0x01,
};

#define UPDATE_MAX_BYTES 3072
#define READABLE_FIELDS_MAX_BYTES 512
#define FAILURE_REASON_MAX_BYTES 40
#define REBOOT_DELAY_MS 1000

static uint8_t *pending_update;
static size_t pending_update_length;

static struct {
	uint8_t code;
	char reason[FAILURE_REASON_MAX_BYTES];
} __packed update_result;

static void discard_pending_update(void)
{
	k_free(pending_update);
	pending_update = NULL;
	pending_update_length = 0;
}

static void reboot_into_new_provisioning(struct k_work *work)
{
	ARG_UNUSED(work);
	LOG_INF("Rebooting into the new provisioning");
	sys_reboot(SYS_REBOOT_COLD);
}

static K_WORK_DELAYABLE_DEFINE(reboot_work, reboot_into_new_provisioning);

static void report_update_result(struct bt_conn *connection, uint8_t code,
				 const char *reason);

static void begin_update(struct bt_conn *connection)
{
	discard_pending_update();
	pending_update = k_malloc(UPDATE_MAX_BYTES);
	if (pending_update == NULL) {
		report_update_result(connection, UPDATE_RESULT_FAILED, "out of memory");
	}
}

static void append_to_update(struct bt_conn *connection, const uint8_t *data,
			     size_t length)
{
	if (pending_update == NULL) {
		report_update_result(connection, UPDATE_RESULT_FAILED, "no update begun");
		return;
	}
	if (length > UPDATE_MAX_BYTES - pending_update_length) {
		discard_pending_update();
		report_update_result(connection, UPDATE_RESULT_FAILED, "update too big");
		return;
	}
	memcpy(&pending_update[pending_update_length], data, length);
	pending_update_length += length;
}

static void save_update(struct bt_conn *connection)
{
	if (pending_update == NULL) {
		report_update_result(connection, UPDATE_RESULT_FAILED, "no update begun");
		return;
	}

	char failure_reason[FAILURE_REASON_MAX_BYTES];
	int error = provisioning_save_update(pending_update, pending_update_length,
					     failure_reason, sizeof(failure_reason));

	discard_pending_update();

	if (error != 0) {
		LOG_WRN("Provisioning update refused: %s", failure_reason);
		report_update_result(connection, UPDATE_RESULT_FAILED, failure_reason);
		return;
	}

	LOG_INF("Provisioning saved");
	report_update_result(connection, UPDATE_RESULT_SAVED, "");
	k_work_reschedule(&reboot_work, K_MSEC(REBOOT_DELAY_MS));
}

static ssize_t write_update(struct bt_conn *connection, const struct bt_gatt_attr *attribute,
			    const void *buffer, uint16_t length, uint16_t offset,
			    uint8_t flags)
{
	ARG_UNUSED(attribute);
	ARG_UNUSED(flags);

	if (!provisioning_window_is_open()) {
		return BT_GATT_ERR(BT_ATT_ERR_WRITE_NOT_PERMITTED);
	}
	if (offset != 0 || length == 0) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	const uint8_t *data = buffer;

	switch (data[0]) {
	case UPDATE_COMMAND_BEGIN:
		begin_update(connection);
		break;
	case UPDATE_COMMAND_APPEND:
		append_to_update(connection, &data[1], length - 1);
		break;
	case UPDATE_COMMAND_SAVE:
		save_update(connection);
		break;
	default:
		return BT_GATT_ERR(BT_ATT_ERR_VALUE_NOT_ALLOWED);
	}
	return length;
}

static ssize_t read_update_result(struct bt_conn *connection,
				  const struct bt_gatt_attr *attribute, void *buffer,
				  uint16_t length, uint16_t offset)
{
	return bt_gatt_attr_read(connection, attribute, buffer, length, offset,
				 &update_result,
				 sizeof(update_result.code) + strlen(update_result.reason));
}

static ssize_t read_fields(struct bt_conn *connection, const struct bt_gatt_attr *attribute,
			   void *buffer, uint16_t length, uint16_t offset)
{
	static EXT_RAM_BSS_ATTR uint8_t readable_fields[READABLE_FIELDS_MAX_BYTES];
	int encoded_length =
		provisioning_encode_readable_fields(readable_fields, sizeof(readable_fields));

	if (encoded_length < 0) {
		return BT_GATT_ERR(BT_ATT_ERR_UNLIKELY);
	}
	return bt_gatt_attr_read(connection, attribute, buffer, length, offset,
				 readable_fields, encoded_length);
}

BT_GATT_SERVICE_DEFINE(provisioning_service,
	BT_GATT_PRIMARY_SERVICE(SERVICE_UUID),
	BT_GATT_CHARACTERISTIC(UPDATE_UUID, BT_GATT_CHRC_WRITE,
			       BT_GATT_PERM_WRITE_ENCRYPT, NULL, write_update, NULL),
	BT_GATT_CHARACTERISTIC(RESULT_UUID, BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_READ_ENCRYPT, read_update_result, NULL, NULL),
	BT_GATT_CCC(NULL, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE_ENCRYPT),
	BT_GATT_CHARACTERISTIC(FIELDS_UUID, BT_GATT_CHRC_READ,
			       BT_GATT_PERM_READ_ENCRYPT, read_fields, NULL, NULL),
);

static void report_update_result(struct bt_conn *connection, uint8_t code,
				 const char *reason)
{
	update_result.code = code;
	strncpy(update_result.reason, reason, sizeof(update_result.reason) - 1);
	update_result.reason[sizeof(update_result.reason) - 1] = '\0';

	(void)bt_gatt_notify(connection, &provisioning_service.attrs[4], &update_result,
			     sizeof(update_result.code) + strlen(update_result.reason));
}

static void require_secure_connections(struct bt_conn *connection, bt_security_t level,
				       enum bt_security_err error)
{
	struct bt_conn_info info;

	if (error != BT_SECURITY_ERR_SUCCESS || bt_conn_get_info(connection, &info) != 0) {
		return;
	}
	if (level >= BT_SECURITY_L2 && !(info.security.flags & BT_SECURITY_FLAG_SC)) {
		LOG_WRN("Legacy pairing refused");
		(void)bt_conn_disconnect(connection, BT_HCI_ERR_AUTH_FAIL);
	}
}

static void forget_update_on_disconnect(struct bt_conn *connection, uint8_t reason)
{
	ARG_UNUSED(connection);
	ARG_UNUSED(reason);
	discard_pending_update();
}

BT_CONN_CB_DEFINE(provisioning_connection_callbacks) = {
	.disconnected = forget_update_on_disconnect,
	.security_changed = require_secure_connections,
};