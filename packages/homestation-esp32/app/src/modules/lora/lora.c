#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>

#include "message_channel.h"
#include "lora.h"
#if defined(CONFIG_BLE_SCANNER)
#include "ble_scanner.h"
#endif
#include "lora_frame.h"
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

/*
 * Body of a +EVT:RXP2P line, formatted by RUI3 as RSSI:SNR:payload-hex.
 * Kept free of anything slow so the caller can re-arm the receiver promptly.
 *
 * @return payload length in bytes, or a negative errno.
 */
static int unpack_rxp2p(char *body, uint8_t *payload, size_t payload_len) {
  char *snr = strchr(body, ':');

  if (!snr) {
    LOG_WRN("Malformed RXP2P event: %s", body);
    return -EBADMSG;
  }
  *snr++ = '\0';

  char *hex = strchr(snr, ':');

  if (!hex) {
    LOG_WRN("Malformed RXP2P event: %s", body);
    return -EBADMSG;
  }
  *hex++ = '\0';

  size_t len = hex2bin(hex, strlen(hex), payload, payload_len);

  if (len == 0) {
    LOG_WRN("Undecodable RXP2P payload: %s", hex);
    return -EBADMSG;
  }

  LOG_INF("LoRa RX: %u bytes, RSSI %s dBm, SNR %s", (unsigned)len, body, snr);

  return (int)len;
}

#if defined(CONFIG_WIFI_GEOLOCATION)
/* Blocks for as long as the request takes, so the receiver must already be
 * re-armed before this is called. */
static void resolve_position(const struct wifi_scan_report *report) {
  struct wifi_location location;

  if (!wifi_station_is_ready()) {
    LOG_WRN("WiFi station not ready, skipping geolocation lookup");
    return;
  }

  if (wifi_geolocation_resolve(report, &location) == 0) {
    LOG_INF("Position: %.6f, %.6f (accuracy %.0f m)", location.latitude,
            location.longitude, location.accuracy_m);
  }
}
#endif

#if defined(CONFIG_LORA_PING)
static uint32_t ping_counter;

static void send_ping(void) {
  uint8_t frame[LORA_FRAME_MAX_BYTES];
  int len = lora_frame_encode_ping(frame, sizeof(frame),
                                   CONFIG_LORA_PING_DEVICE_ID, ping_counter);

  if (len < 0) {
    LOG_ERR("Error: encoding ping failed (%d)", len);
    return;
  }

  LOG_INF("Pinging %s, counter %u", CONFIG_LORA_PING_DEVICE_ID, ping_counter);

  if (rak3172_send_payload(frame, (size_t)len) == 0) {
    ping_counter++;
  }

  /* Transmitting drops the module out of receive; the collar needs a GNSS fix
   * before it answers, so get back in before that lands. */
  rak3172_start_rx();
}

#if defined(CONFIG_LORA_BEACON)
static uint32_t beacon_counter;

/* Keeps the collar's absence timer clear. Its module resets that on the CAD
 * detection alone, so this never has to be decoded to do its job. */
static void send_beacon(void) {
  uint8_t frame[LORA_FRAME_MAX_BYTES];
  int len = lora_frame_encode_beacon(frame, sizeof(frame),
                                     CONFIG_LORA_PING_DEVICE_ID,
                                     beacon_counter);

  if (len < 0) {
    LOG_ERR("Error: encoding beacon failed (%d)", len);
    return;
  }

  LOG_INF("Beacon %u", beacon_counter);

  if (rak3172_send_payload(frame, (size_t)len) == 0) {
    beacon_counter++;
  }

  rak3172_start_rx();
}
#endif

/* A %d.%06d pair loses the sign between -1 and 0 degrees, where the whole part
 * is 0 and carries no minus of its own. */
static void format_microdegrees(char *out, size_t out_len,
                                int32_t microdegrees) {
  int32_t whole = microdegrees / 1000000;
  int32_t fraction = microdegrees % 1000000;

  snprintf(out, out_len, "%s%d.%06d",
           (microdegrees < 0 && whole == 0) ? "-" : "", whole,
           fraction < 0 ? -fraction : fraction);
}

/* A position frame is the collar's answer to a ping. */
static void handle_position(const struct lora_frame *frame) {
  char latitude[16];
  char longitude[16];

  format_microdegrees(latitude, sizeof(latitude),
                      frame->position.latitude_microdegrees);
  format_microdegrees(longitude, sizeof(longitude),
                      frame->position.longitude_microdegrees);

  LOG_INF("Position from %s, counter %u: %s, %s", frame->device_id,
          frame->counter, latitude, longitude);

  if (frame->position.accuracy_decimetres != UINT16_MAX) {
    LOG_INF("  accuracy %u.%u m, %u satellites",
            frame->position.accuracy_decimetres / 10,
            frame->position.accuracy_decimetres % 10,
            frame->position.satellites);
  }

  if (frame->position.battery_percent != UINT8_MAX) {
    LOG_INF("  battery %u%%", frame->position.battery_percent);
  }
}
#endif /* CONFIG_LORA_PING */

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

/* Routes one received payload. The frame magic and the WiFi scan type byte are
 * distinct, so the first match wins without ambiguity. */
static void handle_payload(const uint8_t *payload, size_t len) {
#if defined(CONFIG_LORA_PING)
  struct lora_frame frame;

  if (lora_frame_decode(payload, len, &frame) == 0) {
    if (frame.type == LORA_FRAME_TYPE_POSITION) {
      handle_position(&frame);
    } else {
      LOG_DBG("Frame type %d ignored", frame.type);
    }
    return;
  }
#endif

  if (!wifi_scan_frame_matches(payload, len)) {
    LOG_HEXDUMP_INF(payload, len, "Unrecognised payload");
    return;
  }

  struct wifi_scan_report report;

  if (wifi_scan_frame_decode(payload, len, &report) != 0) {
    return;
  }

  wifi_scan_report_log(&report);

#if defined(CONFIG_WIFI_GEOLOCATION)
  resolve_position(&report);
#endif
}

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

#if defined(CONFIG_LORA_PING)
  /* The collar's receiver is duty cycled, so the preamble has to outlast one
   * of its sniff periods for anything to be heard at all. */
  rak3172_set_preamble(CONFIG_LORA_PING_PREAMBLE);
#endif

#if defined(CONFIG_LORA_BEACON)
  int64_t next_beacon = k_uptime_get();
#endif

  char event[320];
  bool radio_on = false;

  while (true) {
#if defined(CONFIG_BLE_SCANNER)
    if (ble_scanner_tag_in_range()) {
      if (radio_on) {
        LOG_INF("Tag in BLE range, pausing LoRa; back after %d s without an "
                "advertisement",
                CONFIG_BLE_SCANNER_ABSENCE_S);
        rak3172_stop_rx();
        radio_on = false;
      }

      k_sleep(K_MSEC(500));
      continue;
    }
#endif

    if (!radio_on) {
#if defined(CONFIG_BLE_SCANNER)
      LOG_INF("No tag advertisement for %d s, starting LoRa receive",
              CONFIG_BLE_SCANNER_ABSENCE_S);
#else
      LOG_INF("Starting LoRa receive");
#endif
      rak3172_start_rx();
      radio_on = true;
#if defined(CONFIG_LORA_BEACON)
      /* Beacon immediately: the collar has been hearing nothing and its
       * absence timer is already running. */
      next_beacon = k_uptime_get();
#endif
    }

#if defined(CONFIG_LORA_BEACON)
    if (k_uptime_get() >= next_beacon) {
      send_beacon();
      next_beacon =
          k_uptime_get() + CONFIG_LORA_BEACON_INTERVAL_S * MSEC_PER_SEC;
    }
#endif

    if (rak3172_read_event(event, sizeof(event), 100) != 0) {
      continue;
    }

    if (strncmp(event, EVT_RXP2P_PREFIX, strlen(EVT_RXP2P_PREFIX)) == 0) {
      uint8_t payload[LORA_MAX_PAYLOAD_BYTES];
      int len = unpack_rxp2p(event + strlen(EVT_RXP2P_PREFIX), payload,
                             sizeof(payload));

      /* Re-arm before anything slow: the module left receive mode when the
       * packet arrived, and stays deaf until this runs. */
      rak3172_start_rx();

      if (len > 0) {
        handle_payload(payload, (size_t)len);
      }
    } else {
      LOG_INF("  << %s", event);
    }
  }
}

K_THREAD_DEFINE(lora_task_id, CONFIG_LORA_THREAD_STACK_SIZE, lora_task, NULL,
                NULL, NULL, 3, 0, K_TICKS_FOREVER);

void lora_start(void) { k_thread_start(lora_task_id); }
