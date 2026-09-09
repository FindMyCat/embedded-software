#include "diagnostics.h"

#include <string.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include "../FuelGauge/diagnostics.h"
#include "../LTE/diagnostics.h"
#include "../LocationEngine/diagnostics.h"
#include "../WiFi/diagnostics.h"
#include "../comm/diagnostics.h"
#include "../comm/nrf52_diagnostics.h"

LOG_MODULE_REGISTER(diagnostics, CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

static int check_cellular(void)
{
    return transport_diagnostics(TRANSPORT_PATH_CELLULAR);
}

static int check_lora(void)
{
    return transport_diagnostics(TRANSPORT_PATH_LORA);
}

static const struct {
    const char *name;
    int (*run)(void);
} checks[] = {
	{"power", fuel_gauge_diagnostics}, {"nrf52-link", nrf52_diagnostics},
	{"modem", lte_diagnostics},        {"transport/cellular", check_cellular},
	{"transport/lora", check_lora},    {"wifi", wifi_diagnostics},
	{"gnss", location_diagnostics},
};

static const char *const only[] = {
	"transport/lora", 
	//"power", "nrf52-link", "modem", "transport/cellular", "wifi", "gnss",
};

static bool selected(const char *name)
{
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

static void record(const char *name, enum result result, int err)
{
    if (result_count >= ARRAY_SIZE(results)) {
	return;
    }

    results[result_count].name = name;
    results[result_count].result = result;
    results[result_count].err = err;
    result_count++;
}

static void print_summary(void)
{
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

void diagnostics_run(void)
{
    /* Cellular needs the modem attached, so a failure there would report a
     * cause that is already above it. Only meaningful when the modem check
     * actually ran. */
    bool modem_failed = false;

    result_count = 0;

    LOG_INF("==== diagnostics ====");

    for (size_t i = 0; i < ARRAY_SIZE(checks); i++) {
	const char *name = checks[i].name;

	if (!selected(name)) {
	    record(name, RESULT_SKIPPED, 0);
	    continue;
	}

	if (modem_failed && strcmp(name, "transport/cellular") == 0) {
	    LOG_WRN("%s: skipped, no network", name);
	    record(name, RESULT_SKIPPED, 0);
	    continue;
	}

	int err = checks[i].run();

	if (err != 0 && strcmp(name, "modem") == 0) {
	    modem_failed = true;
	}

	record(name, err == 0 ? RESULT_PASS : RESULT_FAIL, err);
    }

    print_summary();

    LOG_INF("==== diagnostics done ====");
}

