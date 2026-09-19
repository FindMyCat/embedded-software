#ifndef LED_H
#define LED_H

#include <zephyr/drivers/led.h>

#define BOOT_BLINK_MS 300
#define LED_RED 0
#define LED_GREEN 1
#define LED_BLUE 2
#define LED_COUNT 3

#define BLINK_PERIOD_MS 500

static const struct device *const findmycat_RGB_led =
    DEVICE_DT_GET(DT_NODELABEL(npm1300_leds));

void boot_led_blink(void);

void red__led_on(void);
void green__led_on(void);
void blue__led_on(void);

void red__led_off(void);
void green__led_off(void);
void blue__led_off(void);

/* Non-blocking, toggles until the matching _off. */
void red__led_blink(void);
void green__led_blink(void);
void blue__led_blink(void);

#endif
