#include "Responder.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>


#include "../mqtt/mqttsn.h"
#include "../LocationEngine/LocationEngine.h"

LOG_MODULE_REGISTER(responder, 4);

static struct k_timer periodic_timer;

void periodic_work_handler(struct k_timer *dummy)
{
    // do the periodic work here
    LOG_INF("Periodic work executed!\n");

    LOG_INF("Getting location from location engine");
    location_gnss_high_accuracy_get();
}

void respond(void)
{
    mqttsn_initialize();
    k_timer_init(&periodic_timer, periodic_work_handler, NULL);
    k_timer_start(&periodic_timer, K_SECONDS(2), K_SECONDS(40));
}

void stop_responding()
{
    LOG_INF("Responder stopping. Performing cleanup.");
    k_timer_stop(&periodic_timer);
    mqttsn_disconnect();
}