#include "Dispatcher.h"
#include "../Responder/Responder.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(dispatcher, CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);
static int currentDispatcherState = DISPATCHER_STATE_IDLE;

/**
 * @brief Callback function for when dispatcher state changes.
*/
void dispatcherCallback() {
    LOG_INF("dispatcher callback");
    if (currentDispatcherState == DISPATCHER_STATE_IDLE) {
        LOG_INF("dispatcherCallback: currentDispatcherState == DISPATCHER_STATE_IDLE");
        stop_responding();
    } else if (currentDispatcherState == DISPATCHER_STATE_LOST_MODE) {
        LOG_INF("dispatcherCallback: currentDispatcherState == DISPATCHER_STATE_LOST_MODE");
        turn_on_lost_mode();
    } else if (currentDispatcherState == DISPATCHER_STATE_ACTIVE_MODE) {
        LOG_INF("dispatcherCallback: currentDispatcherState == DISPATCHER_STATE_ACTIVE_MODE");
        turn_on_active_mode();
    } else if (currentDispatcherState == DISPATCHER_STATE_RESPOND_TO_PING) {
        LOG_INF("dispatcherCallback: currentDispatcherState == DISPATCHER_STATE_RESPOND_TO_PING");
        respond_to_ping();
    } else if (currentDispatcherState == DISPATCHER_STATE_RESPOND_TO_SPOOF_PING) {
        LOG_INF("dispatcherCallback: currentDispatcherState == DISPATCHER_STATE_RESPOND_TO_SPOOF_PING");
        respond_to_spoof_ping();
    }
}

/**
 * @brief Function to change dispatcher state.
*/
void changeDispatcherState(int newState) {
    currentDispatcherState = newState;
    dispatcherCallback();
}

/**
 * @brief Function to get dispatcher state.
*/
int getDispatcherState() {
    return currentDispatcherState;
}
