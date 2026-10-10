#include "SmsListener.h"

#include <date_time.h>
#include <modem/sms.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/timeutil.h>

#include "../Dispatcher/Dispatcher.h"

LOG_MODULE_REGISTER(sms_listener,
                    CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

/* A ping runs the WiFi scan and a full MQTT-SN session on this thread. */
#define SMS_COMMAND_STACK_SIZE 4096
#define SMS_COMMAND_PRIORITY 5
#define SMS_COMMAND_MAX_LEN 32

struct sms_command {
  char text[SMS_COMMAND_MAX_LEN];
  size_t len;
};

K_MSGQ_DEFINE(sms_commands, sizeof(struct sms_command), 2, 4);

static int listener_handle = -1;
static uint32_t received_count;

/* SMSC stamps local time plus a zone in quarter hours. */
static int64_t service_centre_unix_ms(const struct sms_time *time) {
  struct tm tm = {
      .tm_year = 100 + time->year,
      .tm_mon = time->month - 1,
      .tm_mday = time->day,
      .tm_hour = time->hour,
      .tm_min = time->minute,
      .tm_sec = time->second,
  };

  return (timeutil_timegm64(&tm) - (int64_t)time->timezone * 15 * 60) *
         MSEC_PER_SEC;
}

/* Runs on the AT monitor work queue, so the command is handed to a thread. */
static void sms_callback(struct sms_data *const data, void *context) {
  ARG_UNUSED(context);

  if (data->type != SMS_TYPE_DELIVER) {
    return;
  }

  received_count++;

  int64_t now_ms;

  if (date_time_now(&now_ms) == 0) {
    int64_t latency_ms =
        now_ms - service_centre_unix_ms(&data->header.deliver.time);

    LOG_INF("SMS %u from %s: \"%s\", %lld ms after the SMSC took it",
            received_count, data->header.deliver.originating_address.address_str,
            (char *)data->payload, latency_ms);
  } else {
    LOG_INF("SMS %u from %s: \"%s\", latency unknown without network time",
            received_count, data->header.deliver.originating_address.address_str,
            (char *)data->payload);
  }

  struct sms_command command = {0};

  command.len = MIN((size_t)data->payload_len, SMS_COMMAND_MAX_LEN - 1);
  memcpy(command.text, data->payload, command.len);

  if (k_msgq_put(&sms_commands, &command, K_NO_WAIT) != 0) {
    LOG_WRN("SMS command dropped, previous one still running");
  }
}

static void run_commands(void *unused1, void *unused2, void *unused3) {
  ARG_UNUSED(unused1);
  ARG_UNUSED(unused2);
  ARG_UNUSED(unused3);

  struct sms_command command;

  while (true) {
    k_msgq_get(&sms_commands, &command, K_FOREVER);
    (void)dispatchCommand(command.text, command.len);
  }
}

K_THREAD_DEFINE(sms_command_thread, SMS_COMMAND_STACK_SIZE, run_commands, NULL,
                NULL, NULL, SMS_COMMAND_PRIORITY, 0, 0);

int sms_listener_start(void) {
  if (listener_handle >= 0) {
    return 0;
  }

  int handle = sms_register_listener(sms_callback, NULL);

  if (handle < 0) {
    LOG_ERR("Error: SMS subscribe failed (%d)", handle);
    return handle;
  }

  listener_handle = handle;
  LOG_INF("Listening for commands over SMS");
  return 0;
}
