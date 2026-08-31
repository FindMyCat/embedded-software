#include "lora_link.h"

#include <errno.h>
#include <string.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include "../Dispatcher/Dispatcher.h"
#include "rak3172.h"

LOG_MODULE_REGISTER(lora_link,
                    CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

#define RECEIVE_STACK_SIZE 4096
#define RECEIVE_THREAD_PRIORITY 5

/* The module prints +EVT:CADRX: when the packet lands, which a sleeping host
 * misses, and answers AT+CADRD with the same body. Parse the answer. */
#define CADRD_PREFIX "AT+CADRD="
#define CADRD_PREFIX_LEN (sizeof(CADRD_PREFIX) - 1)

#define CADRD_LINE_BYTES 300

/* Longer than the module's CAD_RX_PULSE_MS, so the level read after a wake is
 * the steady state rather than the pulse itself. */
#define WAKE_PULSE_SETTLE_MS 20

/* Absence is a level, not an event, so it cannot be tracked from edges alone: a
 * single missed one would leave this side wrong for as long as the module's
 * mind did not change again. The interrupt stays as the low-latency path and
 * this is the backstop, costing one GPIO read and no UART traffic. */
#define WAKE_POLL_MS 2000

/* The module repeats its packet pulse every sniff, so two highs this far apart
 * cannot both fall inside one pulse. Absence holds the line and passes. */
#define WAKE_CONFIRM_MS 10

/* LORA_FRAME_MAGIC, LORA_FRAME_VERSION, LORA_FRAME_TYPE_PING: the first three
 * bytes of a ping. The module drops everything else without waking us, so the
 * home station's beacons cost nothing here beyond the detection that keeps the
 * absence timer clear. */
#define PING_FILTER_HEX "FC0101"

/* LORA_FRAME_MAGIC and LORA_FRAME_VERSION: any frame of ours, ping or beacon.
 * Presence has to key off this rather than off a bare CAD detection, which
 * cannot tell our home station from anyone else's LoRa traffic on the band. */
#define PRESENCE_FILTER_HEX "FC01"

static const struct gpio_dt_spec lora_packet_ready_pin =
    GPIO_DT_SPEC_GET_OR(DT_ALIAS(lora_wake), gpios, {0});

static K_THREAD_STACK_DEFINE(receive_stack, RECEIVE_STACK_SIZE);
static struct k_thread receive_thread;
static struct gpio_callback packet_ready_callback;
static K_SEM_DEFINE(packet_ready, 0, 1);

static K_MUTEX_DEFINE(rak3172_lock);

static char device_id[LORA_FRAME_MAX_DEVICE_ID + 1] = CONFIG_DEVICE_ID;
static uint32_t outgoing_counter;

/* Absent until the module says otherwise, so LTE carries the link until LoRa
 * has proven itself. */
static bool homestation_absent = true;

/* True only while the line is held, which is absence. A packet pulse cannot
 * span both samples. */
static bool wake_line_held(void) {
  if (gpio_pin_get_dt(&lora_packet_ready_pin) <= 0) {
    return false;
  }

  k_sleep(K_MSEC(WAKE_CONFIRM_MS));

  return gpio_pin_get_dt(&lora_packet_ready_pin) > 0;
}

static void packet_ready_isr(const struct device *port,
                             struct gpio_callback *cb, gpio_port_pins_t pins) {
  k_sem_give(&packet_ready);
}

/* Low power mode gates the UART: the module sleeps between commands and the
 * byte that wakes it is lost, so an AT command sent under LPM can arrive
 * mangled and be rejected as AT_COMMAND_NOT_FOUND. Configure awake, then hand
 * the module back to LPM for the sniff loop itself. */
static void set_low_power(bool enable) {
  rak3172_send_at(enable ? "AT+LPM=1" : "AT+LPM=0");
  rak3172_read_response(1000);

  if (enable) {
    rak3172_send_at("AT+LPMLVL=2");
    rak3172_read_response(1000);
  }
}

/* TODO: enable the absence alarm here (rak3172_cad_rx_set_absence) and let it
 * gate the LTE modem. While the home station is heard, LoRa carries pings and
 * positions on its own and LTE stays down; +EVT:CADABSENT is what brings the
 * modem up. The wake pin then means either PKT or ABSENT, so the receive path
 * has to ask AT+CADEVT which one it is. See lora_link_homestation_in_range. */
static int start_CAD_sniffing(void) {
  set_low_power(false);

  int err = 0;

  if (rak3172_cad_rx_configure(CONFIG_LORA_LINK_CAD_SYMBOLS,
                               CONFIG_LORA_LINK_CAD_DET_PEAK,
                               CONFIG_LORA_LINK_CAD_DET_MIN,
                               CONFIG_LORA_LINK_CAD_INTERVAL_MS) != 0 ||
      rak3172_cad_rx_set_pin(CONFIG_LORA_LINK_CAD_PIN) != 0 ||
      rak3172_cad_rx_set_filter(PING_FILTER_HEX) != 0 ||
      rak3172_cad_rx_set_presence(PRESENCE_FILTER_HEX) != 0 ||
      rak3172_cad_rx_set_absence(CONFIG_LORA_LINK_ABSENCE_S) != 0 ||
      rak3172_cad_rx_start() != 0) {
    err = -EIO;
  }

  set_low_power(true);

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
    /* Beacons should be dropped by the module's AT+CADFILT, so one reaching
     * this far means the filter never took. */
    LOG_DBG("Frame type %d ignored", frame->type);
    return;
  }

  LOG_INF("Ping over LoRa, counter %u", frame->counter);
  changeDispatcherState(DISPATCHER_STATE_RESPOND_TO_PING);
}

static void receive_loop(void *unused1, void *unused2, void *unused3) {
  ARG_UNUSED(unused1);
  ARG_UNUSED(unused2);
  ARG_UNUSED(unused3);

  char event_line[CADRD_LINE_BYTES];

  while (true) {
    bool woken = k_sem_take(&packet_ready, K_MSEC(WAKE_POLL_MS)) == 0;

    /* One line, two meanings, told apart by shape. A buffered packet is a short
     * pulse and is low again by now; absence is held until the module hears
     * something. Sampling the level costs nothing, where asking AT+CADEVT would
     * cost a UART round trip on every wake. */
    k_sleep(K_MSEC(WAKE_PULSE_SETTLE_MS));

    bool absent = wake_line_held();

    if (woken) {
      LOG_INF("Wake line %d, %s", absent, absent ? "absent" : "packet");
    }

    bool changed = absent != homestation_absent;

    if (changed) {
      LOG_INF("Home station %s", absent ? "absent" : "back");
      homestation_absent = absent;
    }

    /* A packet pulses; the level says nothing about one waiting, so only an edge
     * announces one. Absence owns the line while it is held, and the edge that
     * releases it is the module clearing absence rather than a packet. */
    if (!woken || absent || changed) {
      continue;
    }

    k_mutex_lock(&rak3172_lock, K_FOREVER);
    rak3172_send_at("AT+CADRD");
    int err =
        rak3172_wait_event(CADRD_PREFIX, event_line, sizeof(event_line), 3000);
    k_mutex_unlock(&rak3172_lock);

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

/* Logs sniffs:detects:packets:filtered:presence.
 *
 * With only beacons on the air, packets stays at zero by design: PING_FILTER_HEX
 * drops them into filtered first. So read the last two. filtered tracking
 * detects means the beacons are being decoded and the link is healthy;
 * detects climbing while filtered stays flat means the preamble is heard but
 * the packet never completes. presence is the count that actually holds the
 * absence timer off. */
void lora_link_log_stats(void) {
  k_mutex_lock(&rak3172_lock, K_FOREVER);
  rak3172_cad_rx_stats();
  k_mutex_unlock(&rak3172_lock);
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

/* The module owns this: it clears absence on any CAD detection, decoded or not,
 * and raises the wake line when CONFIG_LORA_LINK_ABSENCE_S passes without one.
 * Counting decoded pings here instead would call the home station gone every
 * time one was missed. */
bool lora_link_homestation_in_range(void) { return !homestation_absent; }

int lora_link_send_position(const struct lora_frame_position *position) {
  uint8_t frame[LORA_FRAME_MAX_BYTES];
  int frame_len = lora_frame_encode_position(
      frame, sizeof(frame), device_id, outgoing_counter, position);

  if (frame_len < 0) {
    LOG_ERR("Error: encoding position frame failed (%d)", frame_len);
    return frame_len;
  }

  k_mutex_lock(&rak3172_lock, K_FOREVER);

  /* CAD (Channel Activity Detection) Sniffing owns the radio; it has to stop before the module will transmit. */
  rak3172_cad_rx_stop();
  int err = rak3172_send_payload(frame, (size_t)frame_len);

  if (start_CAD_sniffing() != 0) {
    LOG_ERR("Error: could not resume CAD sniffing after transmit");
  }

  k_mutex_unlock(&rak3172_lock);

  if (err == 0) {
    outgoing_counter++;
  }

  return err;
}

int lora_link_start(void) {
  if (!gpio_is_ready_dt(&lora_packet_ready_pin)) {
    LOG_ERR("Error: RAK3172 packet-ready line not ready");
    return -ENODEV;
  }

  int err = gpio_pin_configure_dt(&lora_packet_ready_pin, GPIO_INPUT);

  if (err == 0) {
    /* Both edges: the rising one is a packet or the start of absence, and the
     * falling one is the module clearing absence after hearing the home
     * station again. Beacons are filtered out before they can pulse, so that
     * falling edge is the only news that the home station came back. */
    err = gpio_pin_interrupt_configure_dt(&lora_packet_ready_pin,
                                          GPIO_INT_EDGE_BOTH);
  }

  if (err != 0) {
    LOG_ERR("Error: configuring packet-ready interrupt failed (%d)", err);
    return err;
  }

  gpio_init_callback(&packet_ready_callback, packet_ready_isr,
                     BIT(lora_packet_ready_pin.pin));
  gpio_add_callback(lora_packet_ready_pin.port, &packet_ready_callback);

  k_mutex_lock(&rak3172_lock, K_FOREVER);
  err = start_CAD_sniffing();
  k_mutex_unlock(&rak3172_lock);

  if (err != 0) {
    LOG_ERR("Error: could not start sniffing; is the CAD firmware flashed?");
    return err;
  }

  k_thread_create(&receive_thread, receive_stack,
                  K_THREAD_STACK_SIZEOF(receive_stack), receive_loop, NULL,
                  NULL, NULL, RECEIVE_THREAD_PRIORITY, 0, K_NO_WAIT);
  k_thread_name_set(&receive_thread, "lora_link");

  LOG_INF("LoRa link listening as %s", device_id);
  return 0;
}
