#include <errno.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "status_led.h"
#if defined(CONFIG_BLE_SCANNER)
#include "ble_scanner.h"
#endif
#include "lora.h"

LOG_MODULE_REGISTER(status_led, LOG_LEVEL_INF);

static const struct device *strip = DEVICE_DT_GET(DT_ALIAS(led_strip));

#define BRIGHTNESS 24

#define POLL_MS 1000

static bool collar_present(void)
{
#if defined(CONFIG_BLE_SCANNER)
	if (ble_scanner_tag_in_range()) {
		return true;
	}
#endif
	return lora_collar_in_range();
}

int status_led_show_rgb(uint8_t r, uint8_t g, uint8_t b)
{
	struct led_rgb pixel = {.r = r, .g = g, .b = b};

	if (!device_is_ready(strip)) {
		return -ENODEV;
	}

	int err = led_strip_update_rgb(strip, &pixel, 1);

	if (err) {
		LOG_ERR("Error: pixel update failed (%d)", err);
	}

	return err;
}

static void show(bool present)
{
	status_led_show_rgb(present ? 0 : BRIGHTNESS, present ? BRIGHTNESS : 0, 0);
}

static void poll(struct k_work *work)
{
	static int shown = -1;
	bool present = collar_present();

	if (shown != (int)present) {
		LOG_INF("Collar %s", present ? "nearby" : "away");
		show(present);
		shown = (int)present;
	}

	k_work_reschedule(k_work_delayable_from_work(work), K_MSEC(POLL_MS));
}

static K_WORK_DELAYABLE_DEFINE(poll_work, poll);

void status_led_start(void)
{
	if (!device_is_ready(strip)) {
		LOG_ERR("Error: LED strip device not ready");
		return;
	}

	k_work_reschedule(&poll_work, K_NO_WAIT);
}
