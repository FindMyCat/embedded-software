#include "diagnostics.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include "status_led.h"

LOG_MODULE_REGISTER(led_diagnostics, LOG_LEVEL_INF);

#define STEP_MS 400

/* Bright enough to tell the channels apart by eye without being painful. */
#define LEVEL 32

static const struct {
  const char *name;
  uint8_t r;
  uint8_t g;
  uint8_t b;
} steps[] = {
    {"red", LEVEL, 0, 0},
    {"green", 0, LEVEL, 0},
    {"blue", 0, 0, LEVEL},
    {"off", 0, 0, 0},
};

int status_led_diagnostics(void) {
  LOG_INF("Status LED diagnostics:");

  for (size_t i = 0; i < ARRAY_SIZE(steps); i++) {
    int err = status_led_show_rgb(steps[i].r, steps[i].g, steps[i].b);

    if (err) {
      LOG_ERR("  FAIL %s (%d)", steps[i].name, err);
      return err;
    }

    LOG_INF("  %s", steps[i].name);
    k_sleep(K_MSEC(STEP_MS));
  }

  LOG_INF("  PASS cycled %u steps", (unsigned)ARRAY_SIZE(steps));

  return 0;
}
