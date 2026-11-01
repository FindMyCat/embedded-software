#ifndef LED_H
#define LED_H

#include <stdint.h>

#define BOOT_BLINK_MS 300
/* nPM1300 LED driver channels as wired on the board. */
#define LED_BLUE 0
#define LED_GREEN 1
#define LED_RED 2
#define LED_COUNT 3

#define BLINK_PERIOD_MS 500
#define LIGHT_BLINK_MS 10000

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

/* App light command. Mask bits are LED_* channels; 0 means all (white).
 * Blinks for LIGHT_BLINK_MS, then off. */
void light__led_blink(uint8_t mask);

#endif
