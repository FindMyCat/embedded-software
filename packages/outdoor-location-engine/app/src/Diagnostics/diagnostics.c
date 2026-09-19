#include "diagnostics.h"

#include <modem/lte_lc.h>
#include <modem/nrf_modem_lib.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/led.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include "../LoRa/lora_link.h"
#include "../LoRa/rak3172.h"
#include "../WiFi/wifi_scan.h"

#include "../FuelGauge/diagnostics.h"
#include "../LTE/diagnostics.h"
#include "../LocationEngine/diagnostics.h"
#include "../WiFi/diagnostics.h"
#include "../comm/diagnostics.h"
#include "../comm/nrf52_diagnostics.h"

LOG_MODULE_REGISTER(diagnostics,
                    CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

enum diagnostics_result {
  RESULT_PASS,
  RESULT_FAIL,
  RESULT_SKIPPED,
};

static int check_cellular(void) {
  return transport_diagnostics(TRANSPORT_PATH_CELLULAR);
}

static int check_lora(void) {
  return transport_diagnostics(TRANSPORT_PATH_LORA);
}

const struct {
  const char *name;
  int (*run)(void);
} checks_to_run[] = {
    {"power", fuel_gauge_diagnostics}, {"nrf52-link", nrf52_diagnostics},
    {"modem", lte_diagnostics},        {"transport/cellular", check_cellular},
    {"transport/lora", check_lora},    {"wifi-power", wifi_scan_power_on},
    {"wifi", wifi_diagnostics},        {"gnss", location_diagnostics},
};

// when populated, only runs the following, else runs all @checks_to_run
const char *const run_only[] = {
    "power",
    "nrf52-link",
    "wifi-power",
    "wifi",
};

bool should_run_diagnostic_for(const char *name) {
  if (ARRAY_SIZE(run_only) == 0) {
    return true;
  }

  for (size_t i = 0; i < ARRAY_SIZE(run_only); i++) {
    if (strcmp(run_only[i], name) == 0) {
      return true;
    }
  }

  return false;
}

struct {
  const char *name;
  enum diagnostics_result result;
  int err;
} results[ARRAY_SIZE(checks_to_run)];

static size_t result_count;

void record_diagnostics_result(const char *name, enum diagnostics_result result,
                               int err) {
  if (result_count >= ARRAY_SIZE(results)) {
    return;
  }

  results[result_count].name = name;
  results[result_count].result = result;
  results[result_count].err = err;
  result_count++;
}

void print_diagnostics_summary(void) {
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

void onboard_diagnostics_run(void) {
  bool modem_failed = false;

  result_count = 0;

  LOG_INF("==== self diagnostics ====");

  for (size_t i = 0; i < ARRAY_SIZE(checks_to_run); i++) {
    const char *name = checks_to_run[i].name;

    if (!should_run_diagnostic_for(name)) {
      record_diagnostics_result(name, RESULT_SKIPPED, 0);
      continue;
    }

    if (modem_failed && strcmp(name, "transport/cellular") == 0) {
      LOG_WRN("%s: skipped, no network", name);
      record_diagnostics_result(name, RESULT_SKIPPED, 0);
      continue;
    }

    int err = checks_to_run[i].run();

    if (err != 0 && strcmp(name, "modem") == 0) {
      modem_failed = true;
    }

    record_diagnostics_result(name, err == 0 ? RESULT_PASS : RESULT_FAIL, err);
  }

  print_diagnostics_summary();

  LOG_INF("==== diagnostics done ====");
}

void diagnostics_low_power_hold(void) {

  // turn on the blue LED when low power hold has activated.
  blue__led_blink();
  blue__led_off();

  LOG_WRN("==== low power hold ====");

#if defined(CONFIG_LORA_LINK)
  lora_link_set_enabled(false);
  k_sleep(K_SECONDS(3));
#endif

  rak3172_uart_claim();
  rak3172_low_power(true);
  rak3172_sleep();
  rak3172_uart_release();

  wifi_scan_init();

  lte_lc_power_off();
  nrf_modem_lib_shutdown();

  LOG_WRN("==== radios down, holding ====");
}
