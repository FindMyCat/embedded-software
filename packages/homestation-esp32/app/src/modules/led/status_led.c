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

enum presence {
  PRESENCE_AWAY,
  PRESENCE_LORA,
  PRESENCE_BLE,
};

/* BLE outranks LoRa: it means the collar is close. */
static enum presence collar_presence(void) {
#if defined(CONFIG_BLE_SCANNER)
  if (ble_scanner_tag_in_range()) {
    return PRESENCE_BLE;
  }
#endif
  return lora_collar_in_range() ? PRESENCE_LORA : PRESENCE_AWAY;
}

int status_led_show_rgb(uint8_t r, uint8_t g, uint8_t b) {
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

/* Green for BLE, purple for LoRa only, red when neither hears it. */
static void show(enum presence presence) {
  switch (presence) {
  case PRESENCE_BLE:
    status_led_show_rgb(0, BRIGHTNESS, 0);
    break;
  case PRESENCE_LORA:
    status_led_show_rgb(BRIGHTNESS, 0, BRIGHTNESS);
    break;
  default:
    status_led_show_rgb(BRIGHTNESS, 0, 0);
    break;
  }
}

static void poll(struct k_work *work) {
  static const char *const names[] = {
      [PRESENCE_AWAY] = "away",
      [PRESENCE_LORA] = "in LoRa range",
      [PRESENCE_BLE] = "nearby",
  };
  static int shown = -1;
  enum presence presence = collar_presence();

  if (shown != (int)presence) {
    LOG_INF("Collar %s", names[presence]);
    show(presence);
    shown = (int)presence;
  }

  k_work_reschedule(k_work_delayable_from_work(work), K_MSEC(POLL_MS));
}

static K_WORK_DELAYABLE_DEFINE(poll_work, poll);

void status_led_start(void) {
  if (!device_is_ready(strip)) {
    LOG_ERR("Error: LED strip device not ready");
    return;
  }

  k_work_reschedule(&poll_work, K_NO_WAIT);
}
