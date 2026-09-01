#include <string.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/byteorder.h>

#include "ble_scanner.h"

LOG_MODULE_REGISTER(ble_scanner, CONFIG_BLE_SCANNER_LOG_LEVEL);

#define FMC_COMPANY_ID     0xFFFF
#define FMC_MAGIC          "FMC"
#define FMC_MAGIC_LEN      3
#define COMPANY_ID_BYTES   2
#define FMC_MIN_LEN        (COMPANY_ID_BYTES + FMC_MAGIC_LEN)
#define FMC_BATTERY_OFFSET FMC_MIN_LEN

#define ABSENCE_MS (CONFIG_BLE_SCANNER_ABSENCE_S * MSEC_PER_SEC)
#define REPORT_INTERVAL_S 10
#define REPORT_INTERVAL_MS (REPORT_INTERVAL_S * MSEC_PER_SEC)

static uint32_t last_report_ms;
static uint32_t seen_since_report;

/* Written from the controller's receive thread and read from anywhere, so both
 * are atomic. Uptime 0 means never seen: scanning only starts once the WiFi
 * bring-up is done, long after the first millisecond. */
static atomic_t last_seen_ms = ATOMIC_INIT(0);
static atomic_t battery_percent = ATOMIC_INIT(UINT8_MAX);

/* An advertising payload is 31 bytes, so a name cannot exceed that. */
#define NAME_MAX_BYTES 31

struct advertisement {
    bool is_tag;
    uint8_t battery_percent;
    char name[NAME_MAX_BYTES + 1];
};

/* True when this manufacturer data is the tag's, taking the battery out of it
 * when one is present. */
static bool parse_manufacturer_data(const uint8_t *data, uint8_t len, uint8_t *battery_percent)
{
    if (len < FMC_MIN_LEN) {
	return false;
    }

    if (sys_get_le16(data) != FMC_COMPANY_ID ||
	memcmp(data + COMPANY_ID_BYTES, FMC_MAGIC, FMC_MAGIC_LEN) != 0) {
	return false;
    }

    if (len > FMC_BATTERY_OFFSET && data[FMC_BATTERY_OFFSET] <= 100) {
	*battery_percent = data[FMC_BATTERY_OFFSET];
    }

    return true;
}

static bool parse_field(struct bt_data *data, void *user_data)
{
    struct advertisement *advertisement = user_data;

    switch (data->type) {
    case BT_DATA_NAME_COMPLETE:
    case BT_DATA_NAME_SHORTENED: {
	uint8_t len = MIN(data->data_len, NAME_MAX_BYTES);

	memcpy(advertisement->name, data->data, len);
	advertisement->name[len] = '\0';

	break;
    }
    case BT_DATA_MANUFACTURER_DATA:
	if (parse_manufacturer_data(data->data, data->data_len, &advertisement->battery_percent)) {
	    advertisement->is_tag = true;
	}
	break;
    }

    return true;
}

static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
			 struct net_buf_simple *ad)
{
    ARG_UNUSED(type);

    struct advertisement advertisement = {
	    .is_tag = false,
	    .battery_percent = UINT8_MAX,
    };
    /* bt_data_parse consumes what it walks, and the caller still owns the
     * buffer. The copy shares the data and keeps the original intact. */
    struct net_buf_simple fields = *ad;

    bt_data_parse(&fields, parse_field, &advertisement);

    if (!advertisement.is_tag) {
	return;
    }

    bool was_in_range = ble_scanner_tag_in_range();

    atomic_set(&last_seen_ms, (atomic_val_t)k_uptime_get_32());

    if (advertisement.battery_percent != UINT8_MAX) {
	atomic_set(&battery_percent, advertisement.battery_percent);
    }

    /* The tag advertises several times a second, so only the arrival is
     * worth a line. */
    if (!was_in_range) {
	char address[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(addr, address, sizeof(address));

	if (advertisement.battery_percent == UINT8_MAX) {
	    LOG_INF("Tag nearby: %s \"%s\", RSSI %d dBm", address, advertisement.name, rssi);
	} else {
	    LOG_INF("Tag nearby: %s \"%s\", RSSI %d dBm, battery %u%%", address, advertisement.name,
		    rssi, advertisement.battery_percent);
	}
	seen_since_report = 0;
	last_report_ms = k_uptime_get_32();

	return;
    }

    seen_since_report++;

    if (k_uptime_get_32() - last_report_ms >= REPORT_INTERVAL_MS) {
	LOG_INF("Tag still in range: RSSI %d dBm, %u advertisements in %d s", rssi,
		seen_since_report, REPORT_INTERVAL_S);

	seen_since_report = 0;
	last_report_ms = k_uptime_get_32();
    }
}

bool ble_scanner_tag_in_range(void)
{
    uint32_t last_seen = (uint32_t)atomic_get(&last_seen_ms);

    if (last_seen == 0) {
	return false;
    }

    return k_uptime_get_32() - last_seen < ABSENCE_MS;
}

uint8_t ble_scanner_tag_battery_percent(void)
{
    return (uint8_t)atomic_get(&battery_percent);
}

int ble_scanner_start(void)
{
    int err = bt_enable(NULL);

    if (err) {
	LOG_ERR("Error: bt_enable failed (%d)", err);
	return err;
    }

    static const struct bt_le_scan_param scan_param = {
	    .type = BT_LE_SCAN_TYPE_PASSIVE,
	    .options = BT_LE_SCAN_OPT_NONE,
	    .interval = BT_GAP_SCAN_FAST_WINDOW,
	    .window = BT_GAP_SCAN_FAST_WINDOW,
    };

    err = bt_le_scan_start(&scan_param, device_found);

    if (err) {
	LOG_ERR("Error: bt_le_scan_start failed (%d)", err);
	return err;
    }

    LOG_INF("Scanning for tags advertising %s in manufacturer data", FMC_MAGIC);

    return 0;
}
