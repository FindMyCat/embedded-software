#include "diagnostics.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "../LocationEngine/LocationEngine.h"

LOG_MODULE_REGISTER(transport_diag, CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

#define SETTLE_S 3

static const char *path_name(enum transport_path path)
{
    switch (path) {
    case TRANSPORT_PATH_LORA:
        return "LoRa";
    case TRANSPORT_PATH_CELLULAR:
        return "MQTT-SN over LTE-M";
    default:
        return "auto";
    }
}

int transport_diagnostics(enum transport_path path)
{
    LOG_INF("Transport, %s:", path_name(path));

    k_sleep(K_SECONDS(SETTLE_S));

    int err = location_publish_spoofed(path);

    if (err != 0) {
        LOG_ERR("  FAIL test position (%d)", err);
        return err;
    }

    LOG_INF("  PASS test position published");

    return 0;
}