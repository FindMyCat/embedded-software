#include "lora_link.h"

#include <errno.h>
#include <string.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/util.h>

#include "../Dispatcher/Dispatcher.h"
#include "../WiFi/wifi_scan.h"
#include "../shared.h"
#include "rak3172.h"

LOG_MODULE_REGISTER(lora_link,
                    CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

#define RECEIVE_STACK_SIZE 4096
#define RECEIVE_THREAD_PRIORITY 5

#define CADRD_PREFIX "AT+CADRD="
#define CADRD_PREFIX_LEN (sizeof(CADRD_PREFIX) - 1)

#define CADRD_LINE_BYTES 300

#define WAKE_PULSE_SETTLE_MS 20
/* Safety net for a missed edge; the module re-pulses every sniff interval. */
#define WAKE_POLL_MS 30000
#define WAKE_CONFIRM_MS 10

#define PING_FILTER_HEX "FC0101"
#define PRESENCE_FILTER_HEX "FC01"
/* Lone byte the station counts as the collar being in LoRa range. */
#define HEARTBEAT_HEX "FD"

/* Covers gathering a position and sending it, so a ping arriving mid-answer is
 * acked rather than answered twice. */
#define PING_DEBOUNCE_MS 5000

/* Sizes the module's receive window, and a bigger one costs current on every
 * CAD hit. No WiFi body: those travel collar to station. */
#define MAX_RECEIVED_FRAME_BYTES (9 + LORA_FRAME_MAX_DEVICE_ID + 12 + 8)

static const struct gpio_dt_spec lora_wake_line =
    GPIO_DT_SPEC_GET_OR(DT_ALIAS(lora_wake), gpios, {0});

static K_THREAD_STACK_DEFINE(receive_stack, RECEIVE_STACK_SIZE);
static struct k_thread receive_thread;
static struct gpio_callback wake_line_callback;
static K_SEM_DEFINE(wake_line_signal, 0, 1);

static char device_id[LORA_FRAME_MAX_DEVICE_ID + 1] = UNCONFIGURED_DEVICE_ID;
static uint32_t outgoing_counter;

static bool homestation_absent = true;
static bool sniffing;
static int64_t last_ping_answered_ms;

static atomic_t link_enabled = ATOMIC_INIT(1);

/* The module runs the burst schedule off its own timer; the nRF stays asleep. */
#define CAD_BURST_MS CONFIG_LORA_LINK_BURST_MS
#define CAD_BURST_PERIOD_MS (CONFIG_LORA_LINK_BURST_PERIOD_S * MSEC_PER_SEC)

static bool absence_line_asserted(void) {
  if (gpio_pin_get_dt(&lora_wake_line) <= 0) {
    return false;
  }

  k_sleep(K_MSEC(WAKE_CONFIRM_MS));

  return gpio_pin_get_dt(&lora_wake_line) > 0;
}

static void wake_line_isr(const struct device *port, struct gpio_callback *cb,
                          gpio_port_pins_t pins) {
  ARG_UNUSED(port);
  ARG_UNUSED(cb);
  ARG_UNUSED(pins);

  k_sem_give(&wake_line_signal);
}

static int start_cad_sniffing(void) {
  rak3172_low_power(false);

  int err = 0;

  if (rak3172_cad_rx_configure(CONFIG_LORA_LINK_CAD_SYMBOLS,
                               CONFIG_LORA_LINK_CAD_DET_PEAK,
                               CONFIG_LORA_LINK_CAD_DET_MIN,
                               CONFIG_LORA_LINK_CAD_INTERVAL_MS,
                               MAX_RECEIVED_FRAME_BYTES,
                               CONFIG_LORA_LINK_PEER_PREAMBLE) != 0 ||
      rak3172_cad_rx_set_pin(CONFIG_LORA_LINK_CAD_PIN) != 0 ||
      rak3172_cad_rx_set_filter(PING_FILTER_HEX) != 0 ||
      rak3172_cad_rx_set_presence(PRESENCE_FILTER_HEX) != 0 ||
      rak3172_cad_rx_set_absence(CONFIG_LORA_LINK_ABSENCE_S) != 0 ||
      rak3172_cad_rx_set_duty(CAD_BURST_MS, CAD_BURST_PERIOD_MS) != 0 ||
      rak3172_cad_rx_start() != 0) {
    err = -EIO;
  }

  /* Best effort: older module firmware lacks AT+HBEAT, and CAD works without
   * it. The module sends it on its own, so the nRF stays asleep. */
  if (err == 0) {
    (void)rak3172_set_heartbeat(CONFIG_LORA_LINK_HEARTBEAT_S, HEARTBEAT_HEX);
  }

  rak3172_low_power(true);

  if (err == 0) {
    LOG_INF("Sniffing every %d ms, waking on %s, present on %s",
            CONFIG_LORA_LINK_CAD_INTERVAL_MS, PING_FILTER_HEX,
            PRESENCE_FILTER_HEX);
  }

  return err;
}

/* The prefix is followed by RSSI:SNR:payload-hex. */
static int decode_cadrx_line(char *line, struct lora_frame *decoded) {
  uint8_t payload[LORA_FRAME_MAX_BYTES];
  char *after_rssi = strchr(line + CADRD_PREFIX_LEN, ':');

  if (!after_rssi) {
    return -EBADMSG;
  }
  *after_rssi++ = '\0';

  char *payload_hex = strchr(after_rssi, ':');

  if (!payload_hex) {
    return -EBADMSG;
  }
  *payload_hex++ = '\0';

  size_t payload_len =
      hex2bin(payload_hex, strlen(payload_hex), payload, sizeof(payload));

  if (payload_len == 0) {
    return -EBADMSG;
  }

  return lora_frame_decode(payload, payload_len, decoded);
}

static void act_on_frame(const struct lora_frame *frame) {
  if (strcmp(frame->device_id, device_id) != 0) {
    LOG_DBG("Frame addressed to %s ignored", frame->device_id);
    return;
  }

  if (frame->type != LORA_FRAME_TYPE_PING) {
    /* AT+CADFILT should have dropped this; reaching here means it never applied. */
    LOG_DBG("Frame type %d ignored", frame->type);
    return;
  }

  /* A lost ack lands several pings here; answering each starts a WiFi scan so we debounce it. */
  int64_t now = k_uptime_get();

  if (last_ping_answered_ms != 0 &&
      now - last_ping_answered_ms < PING_DEBOUNCE_MS) {
    LOG_INF("Ping counter %u already being answered, acking only",
            frame->counter);
    lora_link_send_ack(frame->counter);
    return;
  }

  last_ping_answered_ms = now;

  LOG_INF("Ping over LoRa, counter %u", frame->counter);

  /* Before the position: the station is half duplex and still pinging. */
  lora_link_send_ack(frame->counter);

  changeDispatcherState(DISPATCHER_STATE_RESPOND_TO_PING);
}

static void receive_loop(void *unused1, void *unused2, void *unused3) {
  ARG_UNUSED(unused1);
  ARG_UNUSED(unused2);
  ARG_UNUSED(unused3);

  char event_line[CADRD_LINE_BYTES];

  while (true) {
    bool woken = k_sem_take(&wake_line_signal, K_MSEC(WAKE_POLL_MS)) == 0;

    if (atomic_get(&link_enabled) == 0) {
      if (sniffing) {
        LOG_INF("LORA Link disabled, stopping CAD sniffing");

        rak3172_uart_claim();
        rak3172_cad_rx_stop();
        sniffing = false;
        rak3172_uart_release();
      }

      continue;
    }

    if (!sniffing) {
      LOG_INF("LORA Link enabled, starting CAD sniffing");

      rak3172_uart_claim();
      int err = start_cad_sniffing();
      sniffing = err == 0;
      rak3172_uart_release();

      if (err != 0) {
        LOG_ERR("Error: could not restart CAD sniffing (%d)", err);
        continue;
      }

      homestation_absent = true;
      continue;
    }

    /* Let the wake pulse finish, so the level below tells packet from absence.
     * A timed-out poll has no pulse to wait out. */
    if (woken) {
      k_sleep(K_MSEC(WAKE_PULSE_SETTLE_MS));
    }

    bool lora_module_homestation_absence_flag = absence_line_asserted();

    if (woken) {
      LOG_INF("LORA Link GPIO line %d, %s",
              lora_module_homestation_absence_flag,
              lora_module_homestation_absence_flag ? "homestation absent"
                                                   : "packet recieved");
    }

    bool changed = lora_module_homestation_absence_flag != homestation_absent;

    if (changed) {
      LOG_INF("Home station %s",
              lora_module_homestation_absence_flag ? "absent" : "back");
      homestation_absent = lora_module_homestation_absence_flag;
    }

    if (!woken || lora_module_homestation_absence_flag || changed) {
      continue;
    }

    rak3172_uart_claim();
    rak3172_send_at("AT+CADRD");
    int err =
        rak3172_wait_event(CADRD_PREFIX, event_line, sizeof(event_line), 3000);
    rak3172_uart_release();

    if (err != 0) {
      LOG_WRN("Woken by a pulse but AT+CADRD returned nothing");
      continue;
    }

    struct lora_frame frame;

    err = decode_cadrx_line(event_line, &frame);
    if (err != 0) {
      LOG_DBG("Undecodable frame (%d)", err);
      continue;
    }

    act_on_frame(&frame);
  }
}

/* Logs sniffs:detects:packets:filtered:presence, plus the module's wake reason
 * beside the level the nRF reads. Disagreement points at the line between. */
void lora_link_log_stats(void) {
  rak3172_uart_claim();
  rak3172_cad_rx_stats();
  int event = rak3172_cad_rx_event();
  rak3172_uart_release();

  /* event: 0 none, 1 packet, 2 absent, negative on no answer */
  LOG_INF("CAD event %d, wake line %d", event,
          gpio_pin_get_dt(&lora_wake_line));
}

void lora_link_set_device_id(const char *new_device_id) {
  if (!new_device_id || new_device_id[0] == '\0' ||
      strlen(new_device_id) > LORA_FRAME_MAX_DEVICE_ID) {
    LOG_ERR("Error: rejecting device id of %u bytes",
            new_device_id ? (unsigned)strlen(new_device_id) : 0U);
    return;
  }

  strcpy(device_id, new_device_id);
  LOG_INF("LoRa link identity now %s", device_id);
}

bool lora_link_homestation_in_range(void) {
  if (atomic_get(&link_enabled) == 0) {
    LOG_WRN("LORA Link is not enabled, cannot detect homestation presence");
    return false;
  }

  return !homestation_absent;
}

void lora_link_set_enabled(bool enabled) {
  atomic_val_t wanted = enabled ? 1 : 0;

  if (atomic_set(&link_enabled, wanted) == wanted) {
    return;
  }

  k_sem_give(&wake_line_signal);
}

/* CAD sniffing owns the radio: stop it to transmit, restart it after or the
 * collar goes deaf. Only restart if it was running; the link may be disabled. */
static int send_payload_around_cad(const uint8_t *payload, size_t len) {
  rak3172_uart_claim();

  /* Under AT+LPM the module spends the next command's first byte on waking, so
   * it arrives truncated. Low power is restored below. */
  rak3172_low_power(false);

  rak3172_cad_rx_stop();

  int err = rak3172_send_payload(payload, len);

  if (!sniffing) {
    rak3172_low_power(true);
  } else if (start_cad_sniffing() != 0) {
    LOG_ERR("Error: could not resume CAD sniffing after transmit");
    sniffing = false;
  }

  rak3172_uart_release();

  return err;
}

int lora_link_send_position(const struct lora_frame_position *position) {
  uint8_t frame[LORA_FRAME_MAX_BYTES];
  int frame_len = lora_frame_encode_position(frame, sizeof(frame), device_id,
                                             outgoing_counter, position);

  if (frame_len < 0) {
    LOG_ERR("Error: encoding position frame failed (%d)", frame_len);
    return frame_len;
  }

  int err = send_payload_around_cad(frame, (size_t)frame_len);

  if (err == 0) {
    outgoing_counter++;
  }

  return err;
}

/* Sent the moment a ping decodes, before anything slow: the station stops
 * pinging on it and listens. */
int lora_link_send_ack(uint32_t counter) {
  uint8_t frame[LORA_FRAME_MAX_BYTES];
  int frame_len =
      lora_frame_encode_ack(frame, sizeof(frame), device_id, counter);

  if (frame_len < 0) {
    LOG_ERR("Error: encoding ack frame failed (%d)", frame_len);
    return frame_len;
  }

  return send_payload_around_cad(frame, (size_t)frame_len);
}

int lora_link_send_wifi_scan(const struct wifi_scan_frame *scan) {
  if (!scan || scan->len == 0) {
    LOG_ERR("Error: refusing to send an empty WiFi scan");
    return -EINVAL;
  }

  return send_payload_around_cad(scan->bytes, scan->len);
}

int lora_link_start(void) {
  if (!gpio_is_ready_dt(&lora_wake_line)) {
    LOG_ERR("Error: RAK3172 packet-ready line not ready");
    return -ENODEV;
  
  }

  int err = gpio_pin_configure_dt(&lora_wake_line, GPIO_INPUT);

  if (err == 0) {
    err = gpio_pin_interrupt_configure_dt(&lora_wake_line, GPIO_INT_EDGE_BOTH);
  }

  if (err != 0) {
    LOG_ERR("Error: configuring packet-ready interrupt failed (%d)", err);
    return err;
  }

  gpio_init_callback(&wake_line_callback, wake_line_isr,
                     BIT(lora_wake_line.pin));
  gpio_add_callback(lora_wake_line.port, &wake_line_callback);

  if (atomic_get(&link_enabled) != 0) {
    rak3172_uart_claim();
    err = start_cad_sniffing();
    rak3172_uart_release();

    if (err != 0) {
      LOG_ERR("Error: could not start CAD sniffing");
      return err;
    }

    sniffing = true;
  } else {
    /* An nRF reset leaves the module running; AT+NWM=0 only restarts it on a
     * mode change, so a previous boot's sniffing would carry on. */
    LOG_INF("LoRa Link disabled, stopping any CAD left from a previous boot");
    rak3172_uart_claim();
    rak3172_cad_rx_stop();
    rak3172_uart_release();
  }

  k_thread_create(&receive_thread, receive_stack,
                  K_THREAD_STACK_SIZEOF(receive_stack), receive_loop, NULL,
                  NULL, NULL, RECEIVE_THREAD_PRIORITY, 0, K_NO_WAIT);
  k_thread_name_set(&receive_thread, "lora_link");

  LOG_INF("LoRa link listening as %s", device_id);
  return 0;
}
