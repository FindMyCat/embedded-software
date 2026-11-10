#include "Dispatcher.h"
#include "../DeviceSettings/device_settings.h"
#include "../DeviceStatus/device_status_reporting.h"
#include "../FOTA/fota.h"
#include "../LTE/lte.h"
#include "../LoRa/lora_link.h"
#include "../Presence/presence.h"
#include "../Responder/Responder.h"
#include "../SelfRecovery/self_recovery.h"
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(dispatcher,
                    CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);
static int currentDispatcherState = DISPATCHER_STATE_IDLE;
static int backgroundModeState = DISPATCHER_STATE_IDLE;

static bool isBackgroundMode(int state) {
  return state == DISPATCHER_STATE_IDLE || state == DISPATCHER_STATE_LOST_MODE ||
         state == DISPATCHER_STATE_ACTIVE_MODE;
}

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
  } else if (currentDispatcherState == DISPATCHER_STATE_RESPOND_TO_CELL_PING) {
    LOG_INF("State RESPOND_TO_CELL_PING");
    respond_to_cell_ping();
  } else if (currentDispatcherState == DISPATCHER_STATE_RESPOND_TO_WIFI_PING) {
    LOG_INF("State RESPOND_TO_WIFI_PING");
    respond_to_wifi_ping();
  }
}

/**
 * @brief Function to change dispatcher state.
 */
void changeDispatcherState(int newState) {
  currentDispatcherState = newState;
  if (isBackgroundMode(newState)) {
    backgroundModeState = newState;
  }
  dispatcherCallback();
}

void returnToBackgroundMode(void) {
  currentDispatcherState = backgroundModeState;
  LOG_INF("Back to background state %d", backgroundModeState);
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
/* "update-settings key=value,key=value" */
#define UPDATE_SETTINGS_PREFIX "update-settings "

static bool is_update_settings_command(const char *command, size_t len) {
  size_t prefix_length = strlen(UPDATE_SETTINGS_PREFIX);

  return len > prefix_length &&
         memcmp(command, UPDATE_SETTINGS_PREFIX, prefix_length) == 0;
}

static int run_update_settings_command(const char *command, size_t len) {
  size_t prefix_length = strlen(UPDATE_SETTINGS_PREFIX);
  int32_t old_values[DEVICE_SETTING_COUNT];

  for (int setting = 0; setting < DEVICE_SETTING_COUNT; setting++) {
    old_values[setting] = device_settings_get(setting);
  }

  int err = device_settings_update_from_pairs(command + prefix_length,
                                              len - prefix_length);

  if (err != 0) {
    return err;
  }

#define CHANGED(setting) (device_settings_get(setting) != old_values[setting])

  /* Only these change something live; the rest are read where they're used.
   * Only on a change: each costs a network update or a CAD restart. BLE
   * settings are applied by the nRF52 when it stores the block. */
  if (CHANGED(DEVICE_SETTING_EDRX_CYCLE_CODE) ||
      CHANGED(DEVICE_SETTING_EDRX_PAGING_WINDOW_CODE)) {
    lte_request_edrx_from_settings();
  }

  if (CHANGED(DEVICE_SETTING_LORA_ENABLED)) {
    presence_reevaluate();
  }

#if defined(CONFIG_LORA_LINK)
  if (CHANGED(DEVICE_SETTING_LORA_ABSENCE_TIMEOUT_S) ||
      CHANGED(DEVICE_SETTING_LORA_BURST_MS) ||
      CHANGED(DEVICE_SETTING_LORA_BURST_PERIOD_S) ||
      CHANGED(DEVICE_SETTING_LORA_HEARTBEAT_S)) {
    lora_link_restart_sniffing();
  }
#endif

#undef CHANGED

  return 0;
}

/* Long enough for the status thread to publish the ack first. */
#define REBOOT_DELAY K_SECONDS(5)

static void reboot_work_handler(struct k_work *work) {
  ARG_UNUSED(work);
  /* Cold, so the peripherals reset too, a stuck I2C controller included. */
  self_recovery_reboot("command");
}

static K_WORK_DELAYABLE_DEFINE(reboot_work, reboot_work_handler);

int dispatchCommand(const char *command, size_t len) {
  if (is_update_settings_command(command, len)) {
    return run_update_settings_command(command, len);
  } else if (is_command(command, len, "spoof-ping")) {
    changeDispatcherState(DISPATCHER_STATE_RESPOND_TO_SPOOF_PING);
  } else if (is_command(command, len, "ping")) {
    changeDispatcherState(DISPATCHER_STATE_RESPOND_TO_PING);
  } else if (is_command(command, len, "ping-wifi")) {
    changeDispatcherState(DISPATCHER_STATE_RESPOND_TO_WIFI_PING);
  } else if (is_command(command, len, "ping-gps")) {
    changeDispatcherState(DISPATCHER_STATE_RESPOND_TO_GPS_PING);
  } else if (is_command(command, len, "ping-cell")) {
    changeDispatcherState(DISPATCHER_STATE_RESPOND_TO_CELL_PING);
  } else if (is_command(command, len, "lost")) {
    changeDispatcherState(DISPATCHER_STATE_LOST_MODE);
  } else if (is_command(command, len, "active")) {
    changeDispatcherState(DISPATCHER_STATE_ACTIVE_MODE);
  } else if (is_command(command, len, "update")) {
    /* Downloads in the background and reboots when done. */
    fota_start_update();
  } else if (is_command(command, len, "idle")) {
    changeDispatcherState(DISPATCHER_STATE_IDLE);
  } else if (is_command(command, len, "reboot")) {
    LOG_INF("Reboot requested");
    device_status_report_command("reboot", "ok", "rebooting");
    k_work_schedule(&reboot_work, REBOOT_DELAY);
  } else {
    LOG_ERR("Error: Unknown command: %s", command);
    return -EINVAL;
  }

  return 0;
}
