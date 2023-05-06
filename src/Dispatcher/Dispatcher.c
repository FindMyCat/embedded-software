#include "Dispatcher.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(dispatcher, 4);
static int currentDispatcherState = DISPATCHER_STATE_IDLE;

/**
 * @brief Callback function for when dispatcher state changes.
*/
void dispatcherCallback() {
    LOG_INF("dispatcher callback");
    if (currentDispatcherState == DISPATCHER_STATE_IDLE) {
        LOG_INF("dispatcherCallback: currentDispatcherState == DISPATCHER_STATE_IDLE");
        stop_responding();
    } else if (currentDispatcherState == DISPATCHER_STATE_RESPONDING){
        LOG_INF("dispatcherCallback: currentDispatcherState == DISPATCHER_STATE_RESPONDING");
        respond();
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
