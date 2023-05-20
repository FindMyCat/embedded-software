#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>

#include <errno.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/printk.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <bluetooth/scan.h>


#include "message_channel.h"

/* Register log module */
LOG_MODULE_REGISTER(ble_scanner, CONFIG_MQTT_SAMPLE_BLE_SCANNER_LOG_LEVEL);

int THIRTY_SECONDS = 30000;
/* Last seen timestamp for the tag relative to system uptime */
// TODO: do we need to use epoch time instead?
// https://github.com/ChitlangeSahas/HomeStation/issues/1
static int64_t tagLastSeenTimeStamp = 0;
static int64_t serverLastUpdatedTimeStamp = 0;

/* Method to send message to Zbus channel */
static void message_send(void)
{
	int not_used = -1;
	int err;

	err = zbus_chan_pub(&TRIGGER_CHAN, &not_used, K_SECONDS(1));
	if (err) {
		LOG_ERR("zbus_chan_pub, error: %d", err);
		SEND_FATAL_ERROR();
	}
}

/* Advertizing data */
static const struct bt_data ad[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1)
};

/* Scanning data */
static const struct bt_data sd[] = {
	BT_DATA_BYTES(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME)
};

/* Scan filter match callback */
static void scan_filter_match(struct bt_scan_device_info *device_info,
			      struct bt_scan_filter_match *filter_match,
			      bool connectable)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(device_info->recv_info->addr, addr, sizeof(addr));

	LOG_INF("Tracker nearby: address -> %s ", addr);

	tagLastSeenTimeStamp = k_uptime_get();
}

/* Scan connecting error callback */
static void scan_connecting_error(struct bt_scan_device_info *device_info)
{
	LOG_WRN("Connecting failed");
}

/* Scan connecting callback */
static void scan_connecting(struct bt_scan_device_info *device_info,
			    struct bt_conn *conn)
{
	// default_conn = bt_conn_ref(conn);
}

/* Scan parameters initialization macro */
BT_SCAN_CB_INIT(scan_cb, scan_filter_match, NULL,
		scan_connecting_error, scan_connecting);

#define DEVICE_NAME_FIRST "FindMyDumbCat (30274891)"

/* Scan initialization */
static int scan_init(void)
{
	int err;
	struct bt_scan_init_param scan_init = {
		.connect_if_match = 0,
	};

	bt_scan_init(&scan_init);
	bt_scan_cb_register(&scan_cb);

	err = bt_scan_filter_add(BT_SCAN_FILTER_TYPE_NAME, DEVICE_NAME_FIRST);
	if (err) {
		LOG_ERR("Scanning filters cannot be set (err %d)", err);
		return err;
	}
	
	err = bt_scan_filter_enable(BT_SCAN_NAME_FILTER, false);
	if (err) {
		LOG_ERR("Filters cannot be turned on (err %d)", err);
		return err;
	}


	LOG_INF("Scan module initialized");
	return err;
}

/* BLE scanning task (thread) */
static void ble_scanner_task() {
    int err;

	err = bt_enable(NULL);
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)", err);
		return;
	}
	LOG_INF("Bluetooth initialized");

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	int (*module_init[])(void) = {scan_init};
	for (size_t i = 0; i < ARRAY_SIZE(module_init); i++) {
		err = (*module_init[i])();
		if (err) {
			return;
		}
	}

	printk("Starting Bluetooth homestation\n");


	err = bt_scan_start(BT_SCAN_TYPE_SCAN_ACTIVE);
	if (err) {
		LOG_ERR("Scanning failed to start (err %d)", err);
		return;
	}

	LOG_INF("Scanning successfully started");


	struct bt_le_adv_param adv_param = *BT_LE_ADV_CONN;

	adv_param.interval_min = 0x20;

	err = bt_le_adv_start(&adv_param, ad, ARRAY_SIZE(ad),
			      sd, ARRAY_SIZE(sd));
	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}

	printk("Advertising started\n");

	while (true) {

		bool tagSeenRecently = k_uptime_get() - tagLastSeenTimeStamp < THIRTY_SECONDS;

		if (tagSeenRecently)
		{
			if(tagLastSeenTimeStamp - serverLastUpdatedTimeStamp > THIRTY_SECONDS) {
				LOG_INF("Server not updated in 30 seconds, sending message");
				message_send();
				serverLastUpdatedTimeStamp = tagLastSeenTimeStamp;
			}
			
		} else {
			LOG_WRN("Tag not seen in 30 seconds");
		}

	
		k_sleep(K_SECONDS(CONFIG_MQTT_SAMPLE_TRIGGER_TIMEOUT_SECONDS));
	}
}

K_THREAD_DEFINE(ble_scanner_task_id,
		CONFIG_MQTT_SAMPLE_BLE_SCANNER_THREAD_STACK_SIZE,
		ble_scanner_task, NULL, NULL, NULL, 3, 0, 0);
