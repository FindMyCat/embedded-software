#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>

#include "message_channel.h"
#include "rak3172.h"
#include "wifi_scan_frame.h"
#if defined(CONFIG_WIFI_GEOLOCATION)
#include "wifi_geolocation.h"
#include "wifi_station.h"
#endif

LOG_MODULE_REGISTER(lora, CONFIG_LORA_LOG_LEVEL);

ZBUS_SUBSCRIBER_DEFINE(lora, CONFIG_LORA_MESSAGE_QUEUE_SIZE);

#define EVT_RXP2P_PREFIX "+EVT:RXP2P:"

#define LORA_MAX_PAYLOAD_BYTES 127

/* Body of a +EVT:RXP2P line, formatted by RUI3 as RSSI:SNR:payload-hex */
static void handle_rxp2p(char *body) {
  uint8_t payload[LORA_MAX_PAYLOAD_BYTES];
  struct wifi_scan_report report;
  char *snr = strchr(body, ':');

  if (!snr) {
    LOG_WRN("Malformed RXP2P event: %s", body);
    return;
  }
  *snr++ = '\0';

  char *hex = strchr(snr, ':');

  if (!hex) {
    LOG_WRN("Malformed RXP2P event: %s", body);
    return;
  }
  *hex++ = '\0';

  size_t len = hex2bin(hex, strlen(hex), payload, sizeof(payload));

  if (len == 0) {
    LOG_WRN("Undecodable RXP2P payload: %s", hex);
    return;
  }

  LOG_INF("LoRa RX: %u bytes, RSSI %s dBm, SNR %s", (unsigned)len, body, snr);

  if (!wifi_scan_frame_matches(payload, len)) {
    LOG_HEXDUMP_INF(payload, len, "Unrecognised payload");
    return;
  }

  if (wifi_scan_frame_decode(payload, len, &report) != 0) {
    return;
  }

  wifi_scan_report_log(&report);

#if defined(CONFIG_WIFI_GEOLOCATION)
  struct wifi_location location;

  if (!wifi_station_is_ready()) {
    LOG_WRN("WiFi station not ready, skipping geolocation lookup");
    return;
  }

  if (wifi_geolocation_resolve(&report, &location) == 0) {
    LOG_INF("Position: %.6f, %.6f (accuracy %.0f m)", location.latitude,
            location.longitude, location.accuracy_m);
  }
#endif
}

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

  char event[320];

  while (true) {
    if (rak3172_read_event(event, sizeof(event), 100) != 0) {
      continue;
    }

    if (strncmp(event, EVT_RXP2P_PREFIX, strlen(EVT_RXP2P_PREFIX)) == 0) {
      handle_rxp2p(event + strlen(EVT_RXP2P_PREFIX));
      rak3172_start_rx();
    } else {
      LOG_INF("  << %s", event);
    }
  }
}

K_THREAD_DEFINE(lora_task_id, CONFIG_LORA_THREAD_STACK_SIZE, lora_task, NULL,
                NULL, NULL, 3, 0, 0);
