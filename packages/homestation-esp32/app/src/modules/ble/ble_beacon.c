#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/logging/log.h>

#include "ble_beacon.h"

LOG_MODULE_REGISTER(ble_beacon, CONFIG_BLE_BEACON_LOG_LEVEL);

static const struct bt_data advertisement[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME,
		sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

BUILD_ASSERT(sizeof(CONFIG_BT_DEVICE_NAME) - 1 <= 29,
	     "bluetooth name leaves no room for its own header in a 31 byte payload");

int ble_beacon_start(void)
{
	/* Non-connectable: the collar only needs to hear that the station is
	 * there, and nothing here serves a GATT database. */
	int err = bt_le_adv_start(BT_LE_ADV_NCONN, advertisement,
				  ARRAY_SIZE(advertisement), NULL, 0);

	if (err) {
		LOG_ERR("Error: bt_le_adv_start failed (%d)", err);
		return err;
	}

	LOG_INF("Advertising as \"%s\"", CONFIG_BT_DEVICE_NAME);

	return 0;
}