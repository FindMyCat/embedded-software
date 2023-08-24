#include "Responder.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>


#include "../mqtt/mqttsn.h"
#include "../LocationEngine/LocationEngine.h"

LOG_MODULE_REGISTER(responder, 4);

static struct k_timer periodic_timer;

void periodic_work_handler(struct k_timer *dummy)
{
    LOG_INF("Checking mqtt-sn input");
    mqttsn_check_input();
}

void respond(void)
{
    k_timer_init(&periodic_timer, periodic_work_handler, NULL);
    k_timer_start(&periodic_timer, K_SECONDS(2), K_SECONDS(CONFIG_MQTT_SN_REFRESH_PERIOD_SECONDS));
    location_gnss_periodic_get(CONFIG_LOCATION_REFRESH_PERIOD_SECONDS);
}

void stop_responding()
{
    LOG_INF("Responder stopping. Performing cleanup.");
    k_timer_stop(&periodic_timer);
    mqttsn_disconnect();
    location_request_cancel();
}