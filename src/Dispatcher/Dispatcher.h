#ifndef DISPATCHER_DOT_H    /* This is an "include guard" */
#define DISPATCHER_DOT_H    /* prevents the file from being included twice. */
                            /* Including a header file twice causes all kinds */
                            /* of interesting problems.*/


enum dispatcherState {
    DISPATCHER_STATE_IDLE,
    DISPATCHER_STATE_RESPONDING,
};

void changeDispatcherState(int newState);
int getDispatcherState();

#endif
