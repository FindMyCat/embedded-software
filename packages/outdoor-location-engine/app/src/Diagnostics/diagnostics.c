#include "diagnostics.h"

#include <zephyr/logging/log.h>

#include "../FuelGauge/diagnostics.h"
#include "../LTE/diagnostics.h"
#include "../mqtt/diagnostics.h"

LOG_MODULE_REGISTER(diagnostics, CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

void diagnostics_run(void)
{
    LOG_INF("==== diagnostics ====");

    fuel_gauge_diagnostics();

    /* MQTT-SN diagnostics depend on the modem passing tests */
    if (lte_diagnostics() == 0) {
        mqttsn_diagnostics();
    } else {
        LOG_WRN("MQTT-SN: skipped diagnostics, no network");
    }

    LOG_INF("==== diagnostics done ====");
}