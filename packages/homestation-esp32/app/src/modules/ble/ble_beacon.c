#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/hci_vs.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>

#include "ble_beacon.h"

LOG_MODULE_REGISTER(ble_beacon, CONFIG_BLE_BEACON_LOG_LEVEL);

#define ADV_TX_POWER_DBM 9

/* Fixed 100 ms, the floor for non-connectable adverts on a BLE 4.2
 * controller: roughly two chances per collar scan window instead of the
 * default 100 to 150 ms. */
static const struct bt_le_adv_param adv_param =
	BT_LE_ADV_PARAM_INIT(0, BT_GAP_ADV_FAST_INT_MIN_2, BT_GAP_ADV_FAST_INT_MIN_2, NULL);

static const struct bt_data advertisement[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME,
		sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

BUILD_ASSERT(sizeof(CONFIG_BT_DEVICE_NAME) - 1 <= 29,
	     "bluetooth name leaves no room for its own header in a 31 byte payload");

/* Zephyr's vendor command; the ESP32 HCI driver maps it to esp_ble_tx_power_set. */
static void set_adv_tx_power(int8_t dbm)
{
	struct bt_hci_cp_vs_write_tx_power_level *cp;
	struct bt_hci_rp_vs_write_tx_power_level *rp;
	struct net_buf *buf, *rsp = NULL;

	buf = bt_hci_cmd_alloc(K_FOREVER);
	if (!buf) {
		LOG_ERR("Error: no HCI buffer for TX power");
		return;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	cp->handle = sys_cpu_to_le16(0);
	cp->handle_type = BT_HCI_VS_LL_HANDLE_TYPE_ADV;
	cp->tx_power_level = dbm;

	int err = bt_hci_cmd_send_sync(BT_HCI_OP_VS_WRITE_TX_POWER_LEVEL, buf, &rsp);

	if (err) {
		LOG_ERR("Error: setting advert TX power failed (%d)", err);
		return;
	}

	rp = (void *)rsp->data;
	LOG_INF("Advert TX power %d dBm", rp->selected_tx_power);
	net_buf_unref(rsp);
}

int ble_beacon_start(void)
{
	set_adv_tx_power(ADV_TX_POWER_DBM);

	/* Non-connectable: the collar only needs to hear that the station is
	 * there, and nothing here serves a GATT database. */
	int err = bt_le_adv_start(&adv_param, advertisement,
				  ARRAY_SIZE(advertisement), NULL, 0);

	if (err) {
		LOG_ERR("Error: bt_le_adv_start failed (%d)", err);
		return err;
	}

	LOG_INF("Advertising as \"%s\"", CONFIG_BT_DEVICE_NAME);

	return 0;
}