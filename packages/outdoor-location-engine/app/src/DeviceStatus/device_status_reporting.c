#include "device_status_reporting.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "../FuelGauge/fuel_gauge.h"
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

static void encode_command(const struct device_status *status, char *payload,
                           size_t size) {
  const struct location_result *result = location_last_result();
  float state_of_charge;

  if (fuel_gauge_get_state_of_charge(&state_of_charge) != 0) {
    state_of_charge = -1.f;
  }

  snprintk(
      payload, size,
      "{\"command\":\"%s\",\"status\":\"%s\",\"reason\":\"%s\",\"method\":\"%"
      "s\","
      "\"satellites\":%u,\"elapsed_s\":%u,\"battery\":%.01f,\"uptime_s\":%lld}",
      status->command.command, status->command.status, status->command.reason,
      location_method_str(result->method), result->num_satellites,
      result->elapsed_ms / 1000U, (double)state_of_charge,
      k_uptime_get() / MSEC_PER_SEC);
}

static void encode_network(const struct device_status *status, char *payload,
                           size_t size) {
  snprintk(payload, size,
           "{\"command\":\"NET\",\"status\":\"ok\",\"mode\":%d,\"edrx_s\":%.2f,"
           "\"wake_ms\":%lld}",
           status->network.edrx.mode, (double)status->network.edrx.edrx,
           status->network.wake_ms);
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
    }

    int err = mqttsn_publish_with_session(CONFIG_MQTT_SN_STATUS_TOPIC, payload);
    if (err) {
      LOG_ERR("Error: publishing device status failed (%d)", err);
    }
  }
}

K_THREAD_DEFINE(device_status_reporting, REPORTING_STACK_SIZE, report_loop,
                NULL, NULL, NULL, REPORTING_PRIORITY, 0, 0);
