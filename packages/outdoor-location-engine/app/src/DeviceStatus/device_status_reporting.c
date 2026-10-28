#include "device_status_reporting.h"

#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "../LocationEngine/LocationEngine.h"
#include "../mqtt/mqttsn.h"

LOG_MODULE_REGISTER(device_status,
                    CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

#define PAYLOAD_SIZE 192
#define QUEUED_REPORTS 4

/* Publishing waits on the network, so it runs on its own thread: on the system
 * workqueue it blocked the MQTT-SN client's own work item, and the message
 * never left before the session closed. */
#define REPORTING_STACK_SIZE 2048
#define REPORTING_PRIORITY 7

K_MSGQ_DEFINE(device_status_queue, sizeof(struct device_status), QUEUED_REPORTS,
              4);

void device_status_report(const struct device_status *status) {
  if (k_msgq_put(&device_status_queue, status, K_NO_WAIT) != 0) {
    LOG_WRN("Device status queue full, dropping a report");
  }
}

void device_status_report_command(const char *command, const char *status,
                                  const char *reason) {
  struct device_status report = {.kind = DEVICE_STATUS_COMMAND};

  strncpy(report.command.command, command, sizeof(report.command.command) - 1);
  strncpy(report.command.status, status, sizeof(report.command.status) - 1);
  strncpy(report.command.reason, reason, sizeof(report.command.reason) - 1);
  device_status_report(&report);
}

void device_status_report_firmware(const char *version) {
  struct device_status report = {.kind = DEVICE_STATUS_FIRMWARE};

  strncpy(report.firmware, version, sizeof(report.firmware) - 1);
  device_status_report(&report);
}

static void encode_command(const struct device_status *status, char *payload,
                           size_t size) {
  const struct location_result *result = location_last_result();

  snprintk(
      payload, size,
      "{\"command\":\"%s\",\"status\":\"%s\",\"reason\":\"%s\",\"method\":\"%"
      "s\","
      "\"satellites\":%u}",
      status->command.command, status->command.status, status->command.reason,
      location_method_str(result->method), result->num_satellites);
}

static void encode_network(const struct device_status *status, char *payload,
                           size_t size) {
  snprintk(payload, size,
           "{\"command\":\"NET\",\"status\":\"ok\",\"mode\":%d,\"edrx_s\":%.2f}",
           status->network.mode, (double)status->network.edrx);
}

static void encode_firmware(const struct device_status *status, char *payload,
                            size_t size) {
  snprintk(payload, size, "{\"kind\":\"firmware\",\"version\":\"%s\"}",
           status->firmware);
}

static void report_loop(void *unused1, void *unused2, void *unused3) {
  ARG_UNUSED(unused1);
  ARG_UNUSED(unused2);
  ARG_UNUSED(unused3);

  struct device_status status;
  char payload[PAYLOAD_SIZE];

  while (true) {
    k_msgq_get(&device_status_queue, &status, K_FOREVER);

    switch (status.kind) {
    case DEVICE_STATUS_COMMAND:
      encode_command(&status, payload, sizeof(payload));
      break;
    case DEVICE_STATUS_NETWORK:
      encode_network(&status, payload, sizeof(payload));
      break;
    case DEVICE_STATUS_FIRMWARE:
      encode_firmware(&status, payload, sizeof(payload));
      break;
    }

    int err = mqttsn_publish_with_session(CONFIG_MQTT_SN_STATUS_TOPIC, payload);
    if (err) {
      LOG_ERR("Error: publishing device status failed (%d)", err);
    }
  }
}

K_THREAD_DEFINE(device_status_reporting, REPORTING_STACK_SIZE, report_loop,
                NULL, NULL, NULL, REPORTING_PRIORITY, 0, 0);
