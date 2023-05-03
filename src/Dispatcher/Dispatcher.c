#include "Dispatcher.h"

static int currentDispatcherState = DISPATCHER_STATE_IDLE;

/**
 * @brief Callback function for when dispatcher state changes.
*/
void dispatcherCallback() {
    LOG_INF("dispatcher callback");
    if (currentDispatcherState == DISPATCHER_STATE_IDLE) {
        LOG_INF("dispatcherCallback: currentDispatcherState == DISPATCHER_STATE_IDLE");
    }
    LOG_INF("dispatcherCallback: currentDispatcherState == DISPATCHER_STATE_RESPONDING");
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
