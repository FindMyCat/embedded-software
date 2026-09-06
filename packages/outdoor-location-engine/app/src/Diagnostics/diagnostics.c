#include "diagnostics.h"

#include <zephyr/logging/log.h>

#include "../FuelGauge/diagnostics.h"

LOG_MODULE_REGISTER(diagnostics, CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

void diagnostics_run(void)
{
    LOG_INF("==== diagnostics ====");

    fuel_gauge_diagnostics();

    LOG_INF("==== diagnostics done ====");
}