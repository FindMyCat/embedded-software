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

/* Per channel, of 255. Two channels on (purple) across 16 ring pixels draws
 * roughly 200-250 mA at this level, well within USB. */
#define BRIGHTNESS 20

#if defined(CONFIG_STATUS_LED_CANDLE)
#define STEADY_STEP_MS 60
#else
#define STEADY_STEP_MS 30
/* 3 s a breath. */
#define BREATHE_STEPS 100
#endif
#define PINGING_STEP_MS 300
#define DANCE_STEP_MS 25
/* Three pulses, each fading up and back down over PULSE_STEPS. */
#define PULSE_STEPS 16
#define DANCE_STEPS (3 * PULSE_STEPS)

static atomic_t dance_steps;

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

/* External ring is the status LED, every pixel the same colour. */
#define RING_LENGTH DT_PROP(DT_ALIAS(led_ring), chain_length)
static const struct device *ring = DEVICE_DT_GET(DT_ALIAS(led_ring));

int status_led_show_rgb(uint8_t r, uint8_t g, uint8_t b) {
  static struct led_rgb pixels[RING_LENGTH];

  if (!device_is_ready(ring)) {
    return -ENODEV;
  }

  for (size_t i = 0; i < RING_LENGTH; i++) {
    pixels[i] = (struct led_rgb){.r = r, .g = g, .b = b};
  }

  int err = led_strip_update_rgb(ring, pixels, RING_LENGTH);

  if (err) {
    LOG_ERR("Error: ring update failed (%d)", err);
  }

  return err;
}

/* Green for BLE, purple for LoRa only, red when neither hears it. */
static struct led_rgb presence_colour(enum presence presence) {
  switch (presence) {
  case PRESENCE_BLE:
    return (struct led_rgb){.g = BRIGHTNESS};
  case PRESENCE_LORA:
    return (struct led_rgb){.r = BRIGHTNESS, .b = BRIGHTNESS};
  default:
    return (struct led_rgb){.r = BRIGHTNESS};
  }
}

static void show_scaled(struct led_rgb colour, uint32_t level) {
  status_led_show_rgb(colour.r * level / 255, colour.g * level / 255,
                      colour.b * level / 255);
}

/* Fade up from dark and back over steps, as a level of 255. Squared so the
 * fade looks even to the eye. */
static uint32_t fade_level(uint32_t phase, uint32_t steps) {
  uint32_t half = steps / 2;
  uint32_t ramp = phase < half ? phase : steps - phase;

  return 255 * ramp * ramp / (half * half);
}

#if defined(CONFIG_STATUS_LED_CANDLE)
/* Candle flicker: smoothed random level, upper half of range, rare dips. */
static void show_steady(struct led_rgb colour) {
  static uint32_t seed = 0x2545F491;
  static uint32_t level = 255;

  seed ^= seed << 13;
  seed ^= seed >> 17;
  seed ^= seed << 5;

  uint32_t target = ((seed >> 8) & 0xF) == 0 ? 64 : 128 + (seed & 0x7F);

  level = (level * 3 + target) / 4;

  show_scaled(colour, level);
}
#else
/* Slow breathing: one fade in and out every BREATHE_STEPS. */
static void show_steady(struct led_rgb colour) {
  static uint32_t phase;

  show_scaled(colour, fade_level(phase, BREATHE_STEPS));
  phase = (phase + 1) % BREATHE_STEPS;
}
#endif

static void poll(struct k_work *work) {
  static const char *const names[] = {
      [PRESENCE_AWAY] = "away",
      [PRESENCE_LORA] = "in LoRa range",
      [PRESENCE_BLE] = "nearby",
  };
  static int shown = -1;

  /* Heartbeat dance: cyan fading in and out, then back to whatever is due. */
  int step = (int)atomic_dec(&dance_steps);

  if (step > 0) {
    show_scaled((struct led_rgb){.g = BRIGHTNESS, .b = BRIGHTNESS},
                fade_level((DANCE_STEPS - step) % PULSE_STEPS, PULSE_STEPS));
    shown = -1;

    k_work_reschedule(k_work_delayable_from_work(work),
                      K_MSEC(DANCE_STEP_MS));
    return;
  }
  atomic_set(&dance_steps, 0);

  /* Cycling red, green, blue until the collar acks. The ack counts as hearing
   * it over LoRa, so success lands on purple below. */
  if (lora_locate_pinging()) {
    static const struct led_rgb cycle[] = {
        {.r = BRIGHTNESS}, {.g = BRIGHTNESS}, {.b = BRIGHTNESS}};
    static size_t step;

    status_led_show_rgb(cycle[step].r, cycle[step].g, cycle[step].b);
    step = (step + 1) % ARRAY_SIZE(cycle);
    shown = -1;

    k_work_reschedule(k_work_delayable_from_work(work),
                      K_MSEC(PINGING_STEP_MS));
    return;
  }

  /* Blue: the ping got through and the collar is gathering its position.
   * Distinct from the purple of merely hearing a heartbeat. */
  struct led_rgb colour = {.b = BRIGHTNESS};

  if (lora_locate_acked()) {
    shown = -1;
  } else {
    enum presence presence = collar_presence();

    if (shown != (int)presence) {
      LOG_INF("Collar %s", names[presence]);
      shown = (int)presence;
    }
    colour = presence_colour(presence);
  }

  show_steady(colour);

  k_work_reschedule(k_work_delayable_from_work(work), K_MSEC(STEADY_STEP_MS));
}

static K_WORK_DELAYABLE_DEFINE(poll_work, poll);

void status_led_heartbeat(void) {
  atomic_set(&dance_steps, DANCE_STEPS);
  k_work_reschedule(&poll_work, K_NO_WAIT);
}

void status_led_start(void) {
  if (!device_is_ready(ring)) {
    LOG_ERR("Error: LED ring device not ready");
    return;
  }
  LOG_INF("LED ring ready, %d pixels", RING_LENGTH);

  k_work_reschedule(&poll_work, K_NO_WAIT);
}
