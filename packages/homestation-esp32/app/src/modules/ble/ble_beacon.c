#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/hci_vs.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>

#include "ble_beacon.h"

LOG_MODULE_REGISTER(ble_beacon, CONFIG_BLE_BEACON_LOG_LEVEL);

#define ADV_TX_POWER_DBM 9

/* 20 ms in 0.625 ms units. */
#define ADV_INTERVAL_20_MS 32

/* Connectable only because BLE 4.2 floors non-connectable adverts at 100 ms,
 * longer than the collar's 62.5 ms scan window. 20 ms puts about three
 * adverts in every window. */
static const struct bt_le_adv_param adv_param =
	BT_LE_ADV_PARAM_INIT(BT_LE_ADV_OPT_CONN, ADV_INTERVAL_20_MS, ADV_INTERVAL_20_MS, NULL);

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

static int start_advertising(void)
{
	int err = bt_le_adv_start(&adv_param, advertisement,
				  ARRAY_SIZE(advertisement), NULL, 0);

	if (err) {
		LOG_ERR("Error: bt_le_adv_start failed (%d)", err);
	}

	return err;
}

/* A connection stops advertising and the collar would lose the station, so
 * drop every one; nothing here serves a GATT database anyway. */
static void connected(struct bt_conn *conn, uint8_t err)
{
	if (!err) {
		(void)bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
	}
}

/* The connection object is free again, so advertising can restart. */
static void recycled(void)
{
	(void)start_advertising();
}

BT_CONN_CB_DEFINE(beacon_conn_callbacks) = {
	.connected = connected,
	.recycled = recycled,
};

int ble_beacon_start(void)
{
	set_adv_tx_power(ADV_TX_POWER_DBM);

	int err = start_advertising();

	if (err) {
		return err;
	}

	LOG_INF("Advertising as \"%s\"", CONFIG_BT_DEVICE_NAME);

	return 0;
}