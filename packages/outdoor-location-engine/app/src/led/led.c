#include "led.h"

#include <errno.h>
#include <zephyr/device.h>
#include <zephyr/drivers/mfd/npm13xx.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>

/* nPM1300 LEDDRV registers. Written directly */
#define NPM_LED_BASE 0x0AU
#define NPM_LED_OFFSET_MODE 0x00U
#define NPM_LED_OFFSET_SET 0x03U
#define NPM_LED_OFFSET_CLR 0x04U
#define NPM_LED_MODE_HOST 2U
#define NPM_LED_NODE DT_NODELABEL(npm1300_leds)
#define RED_DT_MODE DT_ENUM_IDX(NPM_LED_NODE, nordic_led2_mode)

static const struct device *const pmic = DEVICE_DT_GET(DT_PARENT(NPM_LED_NODE));

static atomic_t blinking_channels;
static bool blink_lit;

static void led_blink_work_handler(struct k_work *work);
static K_WORK_DELAYABLE_DEFINE(blink_work, led_blink_work_handler);

static void light_end_work_handler(struct k_work *work);
static K_WORK_DELAYABLE_DEFINE(light_end_work, light_end_work_handler);

static int pmic_led_write(uint8_t offset, uint8_t val) {
  if (!device_is_ready(pmic)) {
    return -ENODEV;
  }

  return mfd_npm13xx_reg_write(pmic, NPM_LED_BASE, offset, val);
}

static void led_claim(uint32_t ch, bool claim) {
  if (ch != LED_RED) {
    return;
  }

  (void)pmic_led_write(NPM_LED_OFFSET_MODE + ch,
                       claim ? NPM_LED_MODE_HOST : RED_DT_MODE);
}

static int drive_led(uint32_t ch, bool on) {
  return pmic_led_write((on ? NPM_LED_OFFSET_SET : NPM_LED_OFFSET_CLR) + ch * 2U,
                        1U);
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
      (void)drive_led(ch, blink_lit);
    }
  }

  k_work_reschedule(&blink_work, K_MSEC(BLINK_PERIOD_MS));
}

static void led_steady(uint32_t ch, bool on) {
  atomic_and(&blinking_channels, ~BIT(ch));

  if (on) {
    led_claim(ch, true);
  }

  (void)drive_led(ch, on);

  if (!on) {
    led_claim(ch, false);
  }
}

static void led_start_blink(uint32_t ch) {
  led_claim(ch, true);

  if (atomic_or(&blinking_channels, BIT(ch)) == 0) {
    blink_lit = false;
    k_work_reschedule(&blink_work, K_NO_WAIT);
  } else {
    (void)drive_led(ch, blink_lit);
  }
}

static void light_end_work_handler(struct k_work *work) {
  ARG_UNUSED(work);

  for (uint32_t ch = 0; ch < LED_COUNT; ch++) {
    led_steady(ch, false);
  }
}

void boot_led_blink(void) {
  /* Not red: the PMIC owns it for charging. */
  for (int i = 0; i < 2; i++) {
    if (drive_led(LED_BLUE, true) != 0) {
      return;
    }
    k_sleep(K_MSEC(BOOT_BLINK_MS));
    (void)drive_led(LED_BLUE, false);
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

void light__led_blink(uint8_t mask) {
  if (mask == 0) {
    mask = BIT_MASK(LED_COUNT);
  }

  /* A new color replaces the old one rather than mixing with it. */
  for (uint32_t ch = 0; ch < LED_COUNT; ch++) {
    if (mask & BIT(ch)) {
      led_start_blink(ch);
    } else {
      led_steady(ch, false);
    }
  }

  k_work_reschedule(&light_end_work, K_MSEC(LIGHT_BLINK_MS));
}