#include "nrf52_diagnostics.h"

#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "nrf52_link.h"

LOG_MODULE_REGISTER(nrf52_diag, CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

/* The 52 opens a bus window every 30s; outside one it NACKs. Span a whole
 * period so a healthy link cannot be reported as dead wiring. */
#define WINDOW_WAIT_MS  32000
#define WINDOW_POLL_MS  250

int nrf52_diagnostics(void)
{
    struct fmc_config config = {0};
    int err = -EIO;

    LOG_INF("nRF52 link diagnostics:");

    int waited = 0;

    for (; waited < WINDOW_WAIT_MS; waited += WINDOW_POLL_MS) {
        err = nrf52_probe();
        if (err == 0) {
            break;
        }
        /* One line only; the rest of this loop is silent by design. */
        if (waited == 0) {
            LOG_INF("  waiting up to %d ms for the nRF52 to open a bus window",
                    WINDOW_WAIT_MS);
        }
        k_sleep(K_MSEC(WINDOW_POLL_MS));
    }

    if (err == 0) {
        err = nrf52_read_config(&config);
    }

    if (err) {
        LOG_ERR("  FAIL config read from 0x%02x (%d): no bus window in %d ms, "
                "check the nRF52 is powered, running, and that SDA/SCL are wired",
                NRF52_I2C_ADDR, err, WINDOW_WAIT_MS);
        return err;
    }

    LOG_INF("  PASS nRF52 answers at 0x%02x after %d ms", NRF52_I2C_ADDR, waited);

    if (config.device_id[0] == '\0' || config.host[0] == '\0') {
        LOG_WRN("  no FindMyCat config stored yet, use the mobile app to set this.");
        return 0;
    }

    LOG_INF("  device %s, host %s:%u", config.device_id, config.host,
            config.port);

    return 0;
}