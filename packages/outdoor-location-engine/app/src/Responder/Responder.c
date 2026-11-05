#include "Responder.h"
#include <errno.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>

#include "../CellScan/cell_scan.h"
#include "../DeviceStatus/device_status_reporting.h"
#include "../Dispatcher/Dispatcher.h"
#include "../FuelGauge/fuel_gauge.h"
#include "../communication_channels/transport.h"
#include "../LocationEngine/LocationEngine.h"
#include "../Presence/presence.h"
#include "../WiFi/wifi_scan.h"

LOG_MODULE_REGISTER(responder,
                    CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

#define PING_ACCESS_POINTS 6

/* needs at least two access points to place a WiFi scan. */
#define MINIMUM_ACCESS_POINTS_FOR_WIFI_LOCATION 2

#define LOST_MODE_THREAD_STACK_SIZE_BYTES 4096
#define LOST_MODE_THREAD_PRIORITY 7

// To ensure radios aren't double called
static K_MUTEX_DEFINE(location_radios_mutex);

static K_THREAD_STACK_DEFINE(lost_mode_thread_stack,
                             LOST_MODE_THREAD_STACK_SIZE_BYTES);
static struct k_work_q lost_mode_work_queue;
static struct k_work_delayable lost_mode_location_attempt_work;
static atomic_t is_lost_mode_on;

/* Reported once per new reason */
static char last_reported_lost_mode_failure[32];

/* Status goes out over MQTT-SN, so only report while LTE is the link; on
 * LoRa the home station sees the failure itself. */
static void report_ping_failure(const char *command, const char *reason) {
  if (presence_link_get() != PRESENCE_LINK_LTE) {
    return;
  }

  device_status_report_command(command, "fail", reason);
}

/* Returns how many access points were packed, or a negative error. */
static int scan_wifi_access_points(struct wifi_scan_frame *scan) {
  wifi_scan_init();

  /* Blocking; returns once the scan has finished. */
  int err = wifi_scan_start();

  if (err == 0) {
    err = wifi_scan_pack_strongest_access_points(scan, PING_ACCESS_POINTS,
                                                 fuel_gauge_battery_percent());
  }

  /* Cuts the nRF7002's supply, so it runs whether or not the scan worked. */
  wifi_scan_stop();

  return err == 0 ? scan->bytes[1] : err;
}

void turn_on_active_mode(void) {
  stop_lost_mode();
  location_gnss_periodic_get(
      CONFIG_ACTIVE_MODE_LOCATION_REFRESH_PERIOD_SECONDS);
}

static const char *locate_by_wifi_then_gnss_then_cell(bool use_low_accuracy_gnss) {
  struct wifi_scan_frame wifi_scan;
  int access_point_count = scan_wifi_access_points(&wifi_scan);

  if (access_point_count >= MINIMUM_ACCESS_POINTS_FOR_WIFI_LOCATION &&
      transport_publish_wifi_scan(&wifi_scan) == 0) {
    return NULL;
  }

  if (use_low_accuracy_gnss) {
    location_gnss_low_accuracy_get();
  } else {
    location_gnss_normal_accuracy_get();
  }

  const struct location_result *gnss_result = location_last_result();

  if (gnss_result->has_fix) {
    return NULL;
  }

  /* Cell scans resolve on the server over LTE only. */
  struct cell_scan_frame cell_scan;

  if (presence_link_get() == PRESENCE_LINK_LTE &&
      cell_scan_measure_nearby_cells(&cell_scan, fuel_gauge_battery_percent()) == 0 &&
      transport_publish_cell_scan(&cell_scan) == 0) {
    return NULL;
  }

  return gnss_result->reason;
}

void respond_to_ping(void) {
  k_mutex_lock(&location_radios_mutex, K_FOREVER);
  const char *failure_reason = locate_by_wifi_then_gnss_then_cell(false);
  k_mutex_unlock(&location_radios_mutex);

  if (failure_reason != NULL) {
    report_ping_failure("ping", failure_reason);
  }

  returnToBackgroundMode();
}

void respond_to_wifi_ping(void) {
  struct wifi_scan_frame scan;

  k_mutex_lock(&location_radios_mutex, K_FOREVER);
  int access_point_count = scan_wifi_access_points(&scan);
  k_mutex_unlock(&location_radios_mutex);

  if (access_point_count < 0) {
    LOG_ERR("Error: no WiFi scan to answer the ping with (%d)",
            access_point_count);
    report_ping_failure("ping-wifi", "wifi scan failed");
  } else if (transport_publish_wifi_scan(&scan) != 0) {
    LOG_ERR("Error: sending the WiFi scan failed");
    report_ping_failure("ping-wifi", "scan not sent");
  }

  returnToBackgroundMode();
}

/* A fix publishes itself from the location event handler. */
void respond_to_gps_ping(void) {
  k_mutex_lock(&location_radios_mutex, K_FOREVER);
  location_gnss_normal_accuracy_get();
  k_mutex_unlock(&location_radios_mutex);

  const struct location_result *result = location_last_result();

  if (!result->has_fix) {
    report_ping_failure("ping-gps", result->reason);
  }

  returnToBackgroundMode();
}

void respond_to_cell_ping(void) {
  struct cell_scan_frame scan;

  k_mutex_lock(&location_radios_mutex, K_FOREVER);
  int err = cell_scan_measure_nearby_cells(&scan, fuel_gauge_battery_percent());
  k_mutex_unlock(&location_radios_mutex);

  if (err == -ENODATA) {
    report_ping_failure("ping-cell", "no cells found");
  } else if (err) {
    report_ping_failure("ping-cell", "cell scan failed");
  } else if (transport_publish_cell_scan(&scan) != 0) {
    LOG_ERR("Error: sending the cell scan failed");
    report_ping_failure("ping-cell", "scan not sent");
  }

  returnToBackgroundMode();
}

/**
 * @brief Answer a spoof ping with a made up position, skipping GNSS.
 */
void respond_to_spoof_ping(void) {
  location_publish_spoofed(TRANSPORT_PATH_AUTO);
  returnToBackgroundMode();
}

static void report_lost_mode_outcome(const char *failure_reason) {
  if (failure_reason == NULL) {
    last_reported_lost_mode_failure[0] = '\0';
    return;
  }

  if (strcmp(failure_reason, last_reported_lost_mode_failure) == 0) {
    return;
  }

  LOG_WRN("Lost mode found no location: %s", failure_reason);
  report_ping_failure("lost", failure_reason);
  strncpy(last_reported_lost_mode_failure, failure_reason,
          sizeof(last_reported_lost_mode_failure) - 1);
}

static void run_lost_mode_location_attempt(struct k_work *work) {
  k_mutex_lock(&location_radios_mutex, K_FOREVER);
  const char *failure_reason = locate_by_wifi_then_gnss_then_cell(true);
  k_mutex_unlock(&location_radios_mutex);

  /* Stopped mid-attempt: the failure is our own cancel, not worth reporting. */
  if (!atomic_get(&is_lost_mode_on)) {
    return;
  }

  report_lost_mode_outcome(failure_reason);

  /* Counted from the end of this attempt, so a long GNSS timeout spaces the
   * next one out rather than stacking attempts. */
  k_work_reschedule_for_queue(
      &lost_mode_work_queue, &lost_mode_location_attempt_work,
      K_SECONDS(CONFIG_LOST_MODE_LOCATION_REFRESH_PERIOD_SECONDS));
}

static void start_lost_mode_work_queue_once(void) {
  static bool is_started;

  if (is_started) {
    return;
  }

  k_work_queue_init(&lost_mode_work_queue);
  k_work_queue_start(&lost_mode_work_queue, lost_mode_thread_stack,
                     K_THREAD_STACK_SIZEOF(lost_mode_thread_stack),
                     LOST_MODE_THREAD_PRIORITY, NULL);
  k_thread_name_set(&lost_mode_work_queue.thread, "lost_mode");
  k_work_init_delayable(&lost_mode_location_attempt_work,
                        run_lost_mode_location_attempt);
  is_started = true;
}

void turn_on_lost_mode(void) {
  if (atomic_set(&is_lost_mode_on, 1) == 1) {
    return;
  }

  /* Active mode's periodic request would hold GNSS. */
  location_cancel_and_wake_waiter();

  start_lost_mode_work_queue_once();
  last_reported_lost_mode_failure[0] = '\0';
  k_work_reschedule_for_queue(&lost_mode_work_queue,
                              &lost_mode_location_attempt_work, K_NO_WAIT);
}

/* An attempt already running finishes, but schedules no next one. */
void stop_lost_mode(void) {
  if (atomic_clear(&is_lost_mode_on) == 1) {
    k_work_cancel_delayable(&lost_mode_location_attempt_work);
  }
}

void stop_responding() {
  LOG_INF("Responder stopping. Performing cleanup.");
  stop_lost_mode();
  /* No mqttsn_disconnect() here: every publish closes its own session under
   * the session lock, and an unlocked one tears down another thread's. */
  location_cancel_and_wake_waiter();
}