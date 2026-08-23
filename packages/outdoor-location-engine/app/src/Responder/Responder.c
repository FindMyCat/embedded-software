#include "Responder.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>


#include "../mqtt/mqttsn.h"
#include "../LocationEngine/LocationEngine.h"
#include "../Dispatcher/Dispatcher.h"

LOG_MODULE_REGISTER(responder, CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

void turn_on_active_mode(void)
{
    location_gnss_periodic_get(CONFIG_ACTIVE_MODE_LOCATION_REFRESH_PERIOD_SECONDS);
}

void respond_to_ping(void) {
    /* Blocks until the request ends, so a slow fix is waited out instead of
     * being cancelled by a fixed timer. The publish already happened in the
     * location event handler by the time this returns, and location_publish()
     * pumps mqtt_sn_input() itself while it connects and flushes. */
    location_gnss_high_accuracy_get();
    changeDispatcherState(DISPATCHER_STATE_IDLE);
}

/**
 * @brief Answer a spoof ping with a made up position, skipping GNSS.
 *
 * The real ping path waits on a high accuracy fix. This one publishes
 * immediately, so the cloud pipeline can be exercised without a sky view or
 * the wait.
 */
void respond_to_spoof_ping(void) {
    location_publish_spoofed();
    changeDispatcherState(DISPATCHER_STATE_IDLE);
}

void turn_on_lost_mode() {
    location_gnss_periodic_get(CONFIG_LOST_MODE_LOCATION_REFRESH_PERIOD_SECONDS);
}

void stop_responding()
{
    LOG_INF("Responder stopping. Performing cleanup.");
    mqttsn_disconnect();
    location_request_cancel();
}
