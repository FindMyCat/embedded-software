#include "Dispatcher.h"

static int currentDispatcherState = DISPATCHER_STATE_IDLE;

void changeDispatcherState(int newState) {
    currentDispatcherState = newState;
}

int getDispatcherState() {
    return currentDispatcherState;
}
