#include "nrf52_diagnostics.h"

#include <string.h>
#include <zephyr/logging/log.h>

#include "nrf52_link.h"

LOG_MODULE_REGISTER(nrf52_diag, CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

int nrf52_diagnostics(void)
{
    struct fmc_config config = {0};

    LOG_INF("nRF52 link diagnostics:");

    int err = nrf52_read_config(&config);

    if (err) {
        LOG_ERR("  FAIL config read from 0x%02x (%d): check the nRF52 is "
                "powered, running, and that SDA/SCL are wired",
                NRF52_I2C_ADDR, err);
        return err;
    }

    LOG_INF("  PASS nRF52 answers at 0x%02x", NRF52_I2C_ADDR);

    if (config.device_id[0] == '\0' || config.host[0] == '\0') {
        LOG_WRN("  no FindMyCat config stored yet, use the mobile app to set this.");
        return 0;
    }

    LOG_INF("  device %s, host %s:%u", config.device_id, config.host,
            config.port);

    return 0;
}