#include "diagnostics.h"

#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
#include "../led/diagnostics.h"
#include "../wifi/diagnostics.h"

LOG_MODULE_REGISTER(diagnostics, LOG_LEVEL_INF);

static const struct {
  const char *name;
  int (*run)(void);
} checks[] = {
    {"led", status_led_diagnostics},
    {"geolocation", wifi_geolocation_diagnostics},
};

static const char *const only[] = {
    "led",
    "geolocation",
};

static bool selected(const char *name) {
  if (ARRAY_SIZE(only) == 0) {
    return true;
  }

  for (size_t i = 0; i < ARRAY_SIZE(only); i++) {
    if (strcmp(only[i], name) == 0) {
      return true;
    }
  }

  return false;
}

enum result {
  RESULT_PASS,
  RESULT_FAIL,
  RESULT_SKIPPED,
};

static struct {
  const char *name;
  enum result result;
  int err;
} results[ARRAY_SIZE(checks)];

static size_t result_count;

static void record(const char *name, enum result result, int err) {
  if (result_count >= ARRAY_SIZE(results)) {
    return;
  }

  results[result_count].name = name;
  results[result_count].result = result;
  results[result_count].err = err;
  result_count++;
}

static void print_summary(void) {
  size_t failed = 0;
  size_t ran = 0;

  LOG_INF("==== summary ====");

  for (size_t i = 0; i < result_count; i++) {
    switch (results[i].result) {
    case RESULT_PASS:
      LOG_INF("  PASS    %s", results[i].name);
      ran++;
      break;
    case RESULT_FAIL:
      LOG_ERR("  FAIL    %s (%d)", results[i].name, results[i].err);
      failed++;
      ran++;
      break;
    case RESULT_SKIPPED:
      LOG_WRN("  SKIPPED %s", results[i].name);
      break;
    }
  }

  if (failed == 0) {
    LOG_INF("  all %u checks passed", (unsigned)ran);
  } else {
    LOG_ERR("  %u of %u checks failed", (unsigned)failed, (unsigned)ran);
  }
}

void diagnostics_run(void) {
  result_count = 0;

  LOG_INF("==== diagnostics ====");

  for (size_t i = 0; i < ARRAY_SIZE(checks); i++) {
    const char *name = checks[i].name;

    if (!selected(name)) {
      record(name, RESULT_SKIPPED, 0);
      continue;
    }

    int err = checks[i].run();

    record(name, err == 0 ? RESULT_PASS : RESULT_FAIL, err);
  }

  print_summary();

  LOG_INF("==== diagnostics done ====");
}
