#ifndef DISPATCHER_DOT_H
#define DISPATCHER_DOT_H

#include <stddef.h>

enum dispatcherState {
  DISPATCHER_STATE_IDLE,
  DISPATCHER_STATE_RESPOND_TO_PING,
  DISPATCHER_STATE_ACTIVE_MODE,
  DISPATCHER_STATE_LOST_MODE,
  DISPATCHER_STATE_RESPOND_TO_SPOOF_PING,
  DISPATCHER_STATE_RESPOND_TO_GPS_PING,
};

void changeDispatcherState(int newState);
int getDispatcherState();

/* Runs a text command from UDP or SMS on the calling thread. -EINVAL if
 * unknown. */
int dispatchCommand(const char *command, size_t len);

#endif
