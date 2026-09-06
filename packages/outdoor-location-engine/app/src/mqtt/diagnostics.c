#include "diagnostics.h"

#include <zephyr/logging/log.h>

#include "mqttsn.h"

LOG_MODULE_REGISTER(mqttsn_diag, CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

void mqttsn_diagnostics(void)
{
    LOG_INF("MQTT-SN:");

    int err = mqttsn_publish_test();

    if (err != 0) {
        LOG_ERR("  FAIL test publish (%d): check the gateway host and that "
                "%d/udp is reachable",
                err, CONFIG_MQTT_SN_GATEWAY_PORT);
        return;
    }

    LOG_INF("  PASS test position published to MQTT-SN broker");
}