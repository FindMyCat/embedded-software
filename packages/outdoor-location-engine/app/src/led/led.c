#include "led.h"

#include <zephyr/device.h>
#include <zephyr/drivers/led.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>


static atomic_t blinking_channels;
static bool blink_lit;

static void led_blink_work_handler(struct k_work *work);
static K_WORK_DELAYABLE_DEFINE(blink_work, led_blink_work_handler);

static void drive_led(uint32_t channel, bool on) {
  if (!device_is_ready(findmycat_RGB_led)) {
    return;
  }

  if (on) {
    led_on(findmycat_RGB_led, channel);
  } else {
    led_off(findmycat_RGB_led, channel);
  }
}

static void led_blink_work_handler(struct k_work *work) {
  ARG_UNUSED(work);

  atomic_val_t mask = atomic_get(&blinking_channels);

  if (mask == 0) {
    return;
  }

  blink_lit = !blink_lit;

  for (uint32_t ch = 0; ch < LED_COUNT; ch++) {
    if (mask & BIT(ch)) {
      drive_led(ch, blink_lit);
    }
  }

  k_work_reschedule(&blink_work, K_MSEC(BLINK_PERIOD_MS));
}

static void led_steady(uint32_t ch, bool on) {
  atomic_and(&blinking_channels, ~BIT(ch));
  drive_led(ch, on);
}

static void led_start_blink(uint32_t ch) {
  if (atomic_or(&blinking_channels, BIT(ch)) == 0) {
    blink_lit = false;
    k_work_reschedule(&blink_work, K_NO_WAIT);
  } else {
    drive_led(ch, blink_lit);
  }
}

void boot_led_blink(void) {
  if (!device_is_ready(findmycat_RGB_led)) {
    return;
  }

  /* Not red: the PMIC owns it for charging. */
  for (int i = 0; i < 2; i++) {
    if (led_on(findmycat_RGB_led, LED_BLUE) != 0) {
      return;
    }
    k_sleep(K_MSEC(BOOT_BLINK_MS));
    led_off(findmycat_RGB_led, LED_BLUE);
    k_sleep(K_MSEC(BOOT_BLINK_MS));
  }
}

void red__led_on(void) { led_steady(LED_RED, true); }
void green__led_on(void) { led_steady(LED_GREEN, true); }
void blue__led_on(void) { led_steady(LED_BLUE, true); }

void red__led_off(void) { led_steady(LED_RED, false); }
void green__led_off(void) { led_steady(LED_GREEN, false); }
void blue__led_off(void) { led_steady(LED_BLUE, false); }

void red__led_blink(void) { led_start_blink(LED_RED); }
void green__led_blink(void) { led_start_blink(LED_GREEN); }
void blue__led_blink(void) { led_start_blink(LED_BLUE); }
