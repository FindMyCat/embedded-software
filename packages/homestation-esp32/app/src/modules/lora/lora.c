#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>

#include "message_channel.h"
#include "rak3172.h"

LOG_MODULE_REGISTER(lora, CONFIG_LORA_LOG_LEVEL);

ZBUS_SUBSCRIBER_DEFINE(lora, CONFIG_LORA_MESSAGE_QUEUE_SIZE);

#define EVT_RXP2P_PREFIX "+EVT:RXP2P:"

#if defined(CONFIG_LORA_CAD_TEST)
static void lora_cad_test_beacon(void) {
  uint32_t seq = 0;
  uint32_t sent = 0;

  rak3172_set_preamble(CONFIG_LORA_CAD_TEST_PREAMBLE);

  LOG_INF("CAD test beacon: preamble %d symbols, period %d ms",
          CONFIG_LORA_CAD_TEST_PREAMBLE, CONFIG_LORA_CAD_TEST_PERIOD_MS);

  while (true) {
    char msg[16];

    snprintf(msg, sizeof(msg), "CAD%05u", seq++);
    LOG_INF("Beacon %s", msg);

    if (rak3172_send_message(msg) == 0) {
      sent++;
    }
    LOG_INF("Beacons confirmed %u/%u", sent, seq);

    k_sleep(K_MSEC(CONFIG_LORA_CAD_TEST_PERIOD_MS));
  }
}
#endif

static void lora_task(void) {
  int err = rak3172_init();

  if (err) {
    LOG_ERR("Error: rak3172_init failed (%d)", err);
    SEND_FATAL_ERROR();
    return;
  }

#if defined(CONFIG_LORA_CAD_TEST)
  lora_cad_test_beacon();
  return;
#endif

  rak3172_start_rx();

  char event[128];

  while (true) {
    if (rak3172_read_event(event, sizeof(event), 100) != 0) {
      continue;
    }

    if (strncmp(event, EVT_RXP2P_PREFIX, strlen(EVT_RXP2P_PREFIX)) == 0) {
      LOG_INF("LoRa RX: %s", event + strlen(EVT_RXP2P_PREFIX));
      rak3172_start_rx();
    }
  }
}

K_THREAD_DEFINE(lora_task_id, CONFIG_LORA_THREAD_STACK_SIZE, lora_task, NULL,
                NULL, NULL, 3, 0, 0);
