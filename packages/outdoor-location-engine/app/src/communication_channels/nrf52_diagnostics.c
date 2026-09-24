#include "nrf52_diagnostics.h"

#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "nrf52_link.h"

LOG_MODULE_REGISTER(nrf52_diag, CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

/* We raise the request line ourselves; a live 52 answers within ms. */
#define WINDOW_WAIT_MS  1000
#define WINDOW_POLL_MS  50

int nrf52_diagnostics(void)
{
    struct fmc_config config = {0};
    int err = -EIO;

    LOG_INF("nRF52 link diagnostics:");

    bool raised = nrf52_session_begin();
    int waited = 0;

    for (; waited < WINDOW_WAIT_MS; waited += WINDOW_POLL_MS) {
        err = nrf52_probe();
        if (err == 0) {
            break;
        }
        k_sleep(K_MSEC(WINDOW_POLL_MS));
    }

    if (err == 0) {
        err = nrf52_read_config(&config);
    }

    nrf52_session_end(raised);

    if (err == -ENODATA) {
        LOG_INF("  PASS nRF52 answers at 0x%02x after %d ms", NRF52_I2C_ADDR, waited);
        LOG_WRN("  no FindMyCat config stored yet, use the mobile app to set this.");
        return 0;
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