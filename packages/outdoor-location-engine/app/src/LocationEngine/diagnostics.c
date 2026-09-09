#include "diagnostics.h"

#include <errno.h>
#include <zephyr/logging/log.h>

#include "LocationEngine.h"

LOG_MODULE_REGISTER(location_diag, CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

int location_diagnostics(void)
{
    LOG_INF("GNSS diagnostics:");

    int err = location_ensure_init();

    if (err) {
        LOG_ERR("  FAIL could not initialise the location library (%d)", err);
        return err;
    }

    location_gnss_high_accuracy_get();

    const struct location_result *result = location_last_result();

    if (!result->has_fix) {
        LOG_WRN("  no fix: %s after %u ms, %u satellites tracked", result->reason,
                result->elapsed_ms, result->satellites);
        LOG_WRN("  expected indoors; needs a clear view of the sky");
        return -ETIMEDOUT;
    }

    LOG_INF("  PASS fix in %u ms, %u satellites", result->elapsed_ms,
            result->satellites);

    return 0;
}