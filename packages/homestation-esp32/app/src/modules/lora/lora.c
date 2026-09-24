#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/util.h>
#include <zephyr/zbus/zbus.h>

#include "ble_scanner.h"
#include "lora.h"
#include "lora_frame.h"
#include "message_channel.h"
#include "rak3172.h"
#include "wifi_geolocation.h"
#include "wifi_scan_frame.h"
#include "wifi_station.h"

LOG_MODULE_REGISTER(lora, CONFIG_LORA_LOG_LEVEL);

ZBUS_SUBSCRIBER_DEFINE(lora, CONFIG_LORA_MESSAGE_QUEUE_SIZE);

#define EVT_RXP2P_PREFIX "+EVT:RXP2P:"

#define LORA_MAX_PAYLOAD_BYTES 127

#define ABSENCE_MS (CONFIG_LORA_ABSENCE_S * MSEC_PER_SEC)

/* Uptime of the last decoded frame from the collar, 0 before the first. */
static atomic_t last_heard_ms = ATOMIC_INIT(0);

static void mark_collar_heard(void) {
  atomic_set(&last_heard_ms, (atomic_val_t)k_uptime_get_32());
}

bool lora_collar_in_range(void) {
  uint32_t last_heard = (uint32_t)atomic_get(&last_heard_ms);

  if (last_heard == 0) {
    return false;
  }

  return k_uptime_get_32() - last_heard < ABSENCE_MS;
}

/* Body of a +EVT:RXP2P line: RSSI:SNR:payload-hex. Kept free of anything slow
 * so the caller can re-arm the receiver promptly. Returns bytes, or -errno. */
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

static uint32_t ping_counter;

/* TODO (sahas): one transmit is not enough. The collar duty-cycles its receiver
 * (AT+CADDUTY), so a single ping usually lands while it is asleep. Repeat until
 * the position comes back, or send inside a known sniff window. */
static void send_ping(void) {
  uint8_t frame[LORA_FRAME_MAX_BYTES];
  int len = lora_frame_encode_ping(frame, sizeof(frame), CONFIG_DEVICE_ID,
                                   ping_counter);

  if (len < 0) {
    LOG_ERR("Error: encoding ping failed (%d)", len);
    return;
  }

  LOG_INF("Pinging %s, counter %u", CONFIG_DEVICE_ID, ping_counter);

  if (rak3172_send_payload(frame, (size_t)len) == 0) {
    ping_counter++;
  }

  /* Transmitting drops the module out of receive; get back in before the
   * collar answers. */
  rak3172_start_rx();
}

static uint32_t beacon_counter;

static void send_beacon(void) {
  uint8_t frame[LORA_FRAME_MAX_BYTES];
  int len = lora_frame_encode_beacon(frame, sizeof(frame), CONFIG_DEVICE_ID,
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

static const struct gpio_dt_spec locate_button =
    GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw0), gpios, {0});
static struct gpio_callback locate_button_cb;
static atomic_t locate_requested;
static int64_t locate_until;

enum locate_state {
  LOCATE_IDLE,
  /* Repeating the ping until one lands in the collar's listening burst. */
  LOCATE_PINGING,
  /* The collar acked, so it is off gathering a position. Transmitting now
   * would deafen this side for the answer it is waiting for. */
  LOCATE_WAITING,
};

static enum locate_state locate_state;

static void locate_button_isr(const struct device *port,
                              struct gpio_callback *cb, gpio_port_pins_t pins) {
  ARG_UNUSED(port);
  ARG_UNUSED(cb);
  ARG_UNUSED(pins);

  atomic_set(&locate_requested, 1);
}

static void locate_button_init(void) {
  if (!gpio_is_ready_dt(&locate_button)) {
    LOG_WRN("No locate button wired; pings can only be triggered in code");
    return;
  }

  if (gpio_pin_configure_dt(&locate_button, GPIO_INPUT) != 0 ||
      gpio_pin_interrupt_configure_dt(&locate_button,
                                      GPIO_INT_EDGE_TO_ACTIVE) != 0) {
    LOG_ERR("Error: configuring the locate button failed");
    return;
  }

  gpio_init_callback(&locate_button_cb, locate_button_isr,
                     BIT(locate_button.pin));
  gpio_add_callback(locate_button.port, &locate_button_cb);

  LOG_INF("Locate button ready");
}

static void locate_tick(void) {
  if (atomic_cas(&locate_requested, 1, 0)) {
    locate_until = k_uptime_get() + CONFIG_LORA_LOCATE_WINDOW_S * MSEC_PER_SEC;
    locate_state = LOCATE_PINGING;
    LOG_INF("Locate requested, pinging for up to %d s",
            CONFIG_LORA_LOCATE_WINDOW_S);
    return;
  }

  if (locate_state != LOCATE_IDLE && k_uptime_get() >= locate_until) {
    LOG_WRN("Locate window closed with no position");
    locate_state = LOCATE_IDLE;
  }
}

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

/* Routes one received payload. The frame magic and the WiFi scan type byte are
 * distinct, so the first match wins without ambiguity. */
static void handle_payload(const uint8_t *payload, size_t len) {
  struct lora_frame frame;

  if (len == 1 && payload[0] == LORA_HEARTBEAT_BYTE) {
    LOG_INF("Collar heartbeat");
    mark_collar_heard();
    return;
  }

  if (lora_frame_decode(payload, len, &frame) == 0) {
    mark_collar_heard();

    switch (frame.type) {
    case LORA_FRAME_TYPE_ACK:
      /* Stop transmitting: this side cannot hear the position while it talks.
       */
      if (locate_state == LOCATE_PINGING) {
        LOG_INF("Collar acked ping %u, listening for the position",
                frame.counter);
        locate_state = LOCATE_WAITING;
      }
      break;
    case LORA_FRAME_TYPE_POSITION:
      if (locate_state != LOCATE_IDLE) {
        LOG_INF("Position received, locate window closed");
        locate_state = LOCATE_IDLE;
      }
      handle_position(&frame);
      break;
    default:
      LOG_DBG("Frame type %d ignored", frame.type);
      break;
    }
    return;
  }

  if (!wifi_scan_frame_matches(payload, len)) {
    LOG_HEXDUMP_INF(payload, len, "Unrecognised payload");
    return;
  }

  struct wifi_scan_report report;

  if (wifi_scan_frame_decode(payload, len, &report) != 0) {
    return;
  }

  mark_collar_heard();
  wifi_scan_report_log(&report);

  resolve_position(&report);
}

static void lora_task(void) {
  int err = rak3172_init();

  if (err) {
    LOG_ERR("Error: rak3172_init failed (%d)", err);
    SEND_FATAL_ERROR();
    return;
  }

  rak3172_set_preamble(CONFIG_LORA_PING_PREAMBLE);

  locate_button_init();

  int64_t next_beacon = k_uptime_get();

  char event[320];
  bool radio_on = false;

  while (true) {
    locate_tick();

    bool pause_for_ble = ble_scanner_tag_in_range();

    /* A locate request outranks the pause: BLE range is not a position. */
    pause_for_ble = pause_for_ble && locate_state == LOCATE_IDLE;

    if (pause_for_ble) {
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

    if (!radio_on) {
      LOG_INF("No tag advertisement for %d s, starting LoRa receive",
              CONFIG_BLE_SCANNER_ABSENCE_S);
      radio_on = true;
      next_beacon = k_uptime_get();
    }

    if (k_uptime_get() >= next_beacon) {
      switch (locate_state) {
      case LOCATE_PINGING:
        send_ping();
        break;
      case LOCATE_WAITING:
        /* Deliberately silent. */
        break;
      case LOCATE_IDLE:
        send_beacon();
        break;
      }

      next_beacon += CONFIG_LORA_BEACON_INTERVAL_S * MSEC_PER_SEC;

      int64_t now = k_uptime_get();

      if (next_beacon < now) {
        next_beacon = now;
      }
    }

    if (rak3172_read_event(event, sizeof(event), 100) != 0) {
      continue;
    }

    if (strncmp(event, EVT_RXP2P_PREFIX, strlen(EVT_RXP2P_PREFIX)) == 0) {
      uint8_t payload[LORA_MAX_PAYLOAD_BYTES];
      int len = unpack_rxp2p(event + strlen(EVT_RXP2P_PREFIX), payload,
                             sizeof(payload));

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
