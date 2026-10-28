#include "Dispatcher.h"
#include "../Responder/Responder.h"
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(dispatcher,
                    CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);
static int currentDispatcherState = DISPATCHER_STATE_IDLE;

/**
 * @brief Callback function for when dispatcher state changes.
 */
void dispatcherCallback() {
  if (currentDispatcherState == DISPATCHER_STATE_IDLE) {
    LOG_INF("State IDLE");
    stop_responding();
  } else if (currentDispatcherState == DISPATCHER_STATE_LOST_MODE) {
    LOG_INF("State LOST_MODE");
    turn_on_lost_mode();
  } else if (currentDispatcherState == DISPATCHER_STATE_ACTIVE_MODE) {
    LOG_INF("State ACTIVE_MODE");
    turn_on_active_mode();
  } else if (currentDispatcherState == DISPATCHER_STATE_RESPOND_TO_PING) {
    LOG_INF("State RESPOND_TO_PING");
    respond_to_ping();
  } else if (currentDispatcherState == DISPATCHER_STATE_RESPOND_TO_GPS_PING) {
    LOG_INF("State RESPOND_TO_GPS_PING");
    respond_to_gps_ping();
  } else if (currentDispatcherState == DISPATCHER_STATE_RESPOND_TO_SPOOF_PING) {
    LOG_INF("State RESPOND_TO_SPOOF_PING");
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
int getDispatcherState() { return currentDispatcherState; }

/* Exact match: comparing only `len` bytes let "ping" also match "ping-gps". */
static bool is_command(const char *command, size_t len, const char *name) {
  return len == strlen(name) && memcmp(command, name, len) == 0;
}

// Commands are kept short so each one costs as little data as possible.
// The dispatcher logs the state it moves to, so no need to log it here.
int dispatchCommand(const char *command, size_t len) {
  if (is_command(command, len, "spoof-ping")) {
    changeDispatcherState(DISPATCHER_STATE_RESPOND_TO_SPOOF_PING);
  } else if (is_command(command, len, "ping") || is_command(command, len, "ping-wifi")) {
    changeDispatcherState(DISPATCHER_STATE_RESPOND_TO_PING);
  } else if (is_command(command, len, "ping-gps")) {
    changeDispatcherState(DISPATCHER_STATE_RESPOND_TO_GPS_PING);
  } else if (is_command(command, len, "lost")) {
    changeDispatcherState(DISPATCHER_STATE_LOST_MODE);
  } else if (is_command(command, len, "active")) {
    changeDispatcherState(DISPATCHER_STATE_ACTIVE_MODE);
  } else if (is_command(command, len, "idle")) {
    changeDispatcherState(DISPATCHER_STATE_IDLE);
  } else {
    LOG_ERR("Error: Unknown command: %s", command);
    return -EINVAL;
  }

  return 0;
}
