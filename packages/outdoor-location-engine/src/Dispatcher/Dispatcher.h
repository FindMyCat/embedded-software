#ifndef DISPATCHER_DOT_H    /* This is an "include guard" */
#define DISPATCHER_DOT_H    /* prevents the file from being included twice. */
                            /* Including a header file twice causes all kinds */
                            /* of interesting problems.*/


enum dispatcherState {
    DISPATCHER_STATE_IDLE,
    DISPATCHER_STATE_RESPOND_TO_PING,
    DISPATCHER_STATE_ACTIVE_MODE,
    DISPATCHER_STATE_LOST_MODE,
};

void changeDispatcherState(int newState);
int getDispatcherState();

#endif
