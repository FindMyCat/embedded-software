#include <errno.h>
#include <string.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/pm/device.h>
#include <zephyr/pm/device_runtime.h>
#include <zephyr/sys/ring_buffer.h>

#include "rak3172.h"

LOG_MODULE_REGISTER(rak3172,
                    CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

static const struct device *uart_dev = DEVICE_DT_GET(DT_ALIAS(lora_uart));

/* RAK3172 reset (active low, idle high). Optional: a {0} spec if not wired. */
static const struct gpio_dt_spec lora_reset =
    GPIO_DT_SPEC_GET_OR(DT_ALIAS(lora_reset), gpios, {0});

#define CADEVT_PREFIX "AT+CADEVT="
#define CADEVT_PREFIX_LEN (sizeof(CADEVT_PREFIX) - 1)

#define RX_RING_BUF_SIZE 256
RING_BUF_DECLARE(rx_ring_buf, RX_RING_BUF_SIZE);
static K_SEM_DEFINE(rx_sem, 0, RX_RING_BUF_SIZE);

static K_MUTEX_DEFINE(rak3172_lock);

static void uart_irq_cb(const struct device *dev, void *user_data) {
  ARG_UNUSED(user_data);

  if (!uart_irq_update(dev)) {
    return;
  }

  while (uart_irq_rx_ready(dev)) {
    uint8_t c;

    if (uart_fifo_read(dev, &c, 1) == 1) {
      ring_buf_put(&rx_ring_buf, &c, 1);
      k_sem_give(&rx_sem);
    }
  }
}

static int rx_get_byte(uint8_t *c, int64_t remaining_ms) {
  if (k_sem_take(&rx_sem, K_MSEC(remaining_ms)) != 0) {
    return -EAGAIN;
  }
  ring_buf_get(&rx_ring_buf, c, 1);
  return 0;
}

/* A read that timed out leaves its late reply behind; the next command would
 * then read that stale line as its own. */
static void rx_flush(void) {
  unsigned int key = irq_lock();

  ring_buf_reset(&rx_ring_buf);
  k_sem_reset(&rx_sem);
  irq_unlock(key);
}

void rak3172_send_at(const char *cmd) {
  /* Under AT+LPM the module spends the first byte on waking its UART, so spend
   * a newline on that rather than the command. Its OK is flushed below. */
  uart_poll_out(uart_dev, '\n');
  k_sleep(K_MSEC(10));

  rx_flush();
  LOG_INF(">> %s", cmd);
  for (const char *s = cmd; *s; s++) {
    uart_poll_out(uart_dev, *s);
  }
  uart_poll_out(uart_dev, '\r');
  uart_poll_out(uart_dev, '\n');
}

int rak3172_read_response(int timeout_ms) {
  int64_t deadline = k_uptime_get() + timeout_ms;
  char buf[128];
  int i = 0;

  LOG_INF("RAK3172 response:");

  while (true) {
    int64_t remaining = deadline - k_uptime_get();

    if (remaining <= 0) {
      break;
    }

    uint8_t c;

    if (rx_get_byte(&c, remaining) != 0) {
      break;
    }

    if (c == '\n') {
      buf[i] = '\0';
      LOG_INF("        << %s", buf);
      /* Banners and notices are skipped; conclude only on OK or ERROR. */
      if (strcmp(buf, "OK") == 0) {
        return 0;
      }
      if (strstr(buf, "ERROR") != NULL) {
        return -EIO;
      }
      /* RUI3 errors are AT_ prefixed; successes echo the command as AT+.
       * Without this any error lacking "ERROR" waits out the timeout. */
      if (strncmp(buf, "AT_", 3) == 0) {
        LOG_WRN("RAK3172 rejected command: %s", buf);
        return -EIO;
      }
      i = 0;
      continue;
    }

    if (c != '\r' && i < (int)sizeof(buf) - 1) {
      buf[i++] = c;
    }
  }

  if (i > 0) {
    buf[i] = '\0';
    LOG_INF("  << %s", buf);
  }
  return -ETIMEDOUT;
}

static int rak3172_configure_p2p(void) {
  LOG_INF("Configuring LORA P2P parameters");
  /* US915, SF7, BW 125kHz, CR 4/5, preamble 8, 5 dBm */
  rak3172_send_at("AT+P2P=915000000:7:125:0:8:14");
  return rak3172_read_response(3000);
}

void rak3172_hw_reset(void) {
  if (!gpio_is_ready_dt(&lora_reset)) {
    LOG_WRN("RAK3172 reset line not wired; cannot hardware-reset");
    return;
  }
  LOG_WRN("RAK3172 hardware reset");
  gpio_pin_set_dt(&lora_reset, 1); /* assert (active low) */
  k_sleep(K_MSEC(10));
  gpio_pin_set_dt(&lora_reset, 0); /* deassert */
  k_sleep(K_MSEC(300));            /* module boot time */
}

int rak3172_init(void) {
  if (!device_is_ready(uart_dev)) {
    LOG_ERR("Error: RAK3172 UART device not ready");
    return -ENODEV;
  }

  uart_irq_callback_set(uart_dev, uart_irq_cb);

  int err = pm_device_runtime_enable(uart_dev);

  if (err != 0 && err != -EALREADY) {
    LOG_ERR("Error: UART runtime PM enable failed (%d)", err);
  }

  if (gpio_is_ready_dt(&lora_reset)) {
    /* idle high = reset deasserted (active low) */
    gpio_pin_configure_dt(&lora_reset, GPIO_OUTPUT_INACTIVE);
  }

  rak3172_uart_claim();

  LOG_INF("RAK3172 UART ready");

  /* AT+P2P's OK is the liveness signal; no ACK means hardware-reset and retry. */
  for (int attempt = 0; attempt < 3; attempt++) {
    LOG_INF("Enabling P2P mode (RAK3172 will restart automatically.)");
    rak3172_send_at("AT+NWM=0");
    rak3172_read_response(8000);

    if (rak3172_configure_p2p() != 0) {
      LOG_WRN("RAK3172 not responding (attempt %d/3); hardware reset",
              attempt + 1);
      rak3172_hw_reset();
      continue;
    }
    k_sleep(K_MSEC(500));

    rak3172_cad_rx_set_pin(CONFIG_LORA_LINK_CAD_PIN);

    rak3172_low_power(true);

    LOG_INF("RAK3172 ready");
    rak3172_uart_release();
    return 0;
  }

  LOG_ERR("Error: RAK3172 unresponsive after hardware resets (tried 3 times)");
  rak3172_uart_release();
  return -EIO;
}

int rak3172_send_payload(const uint8_t *data, size_t len) {
  char cmd[272] = "AT+PSEND=";
  char *hex = cmd + strlen("AT+PSEND=");

  if (len > 127) {
    LOG_ERR("Error: payload too long (%u bytes, max 127)", (unsigned)len);
    return -1;
  }

  for (size_t i = 0; i < len; i++) {
    snprintf(&hex[i * 2], 3, "%02X", data[i]);
  }

  rak3172_send_at(cmd);

  /* NOTE (sahas): may be deleted once stabilized. "OK" means accepted;
   * +EVT:TXP2P DONE means the air transmission finished. Under LPM the module
   * sometimes reboots after TX, from RF coupling into the RX/TX wires; this
   * detects that and reconfigures P2P. */
  bool rebooted = false;
  bool tx_done = false;
  int64_t deadline = k_uptime_get() + 10000;
  char buf[128];
  int i = 0;

  while (true) {
    int64_t remaining = deadline - k_uptime_get();
    if (remaining <= 0) {
      break;
    }

    uint8_t c;
    if (rx_get_byte(&c, remaining) != 0) {
      break;
    }

    if (c == '\n') {
      buf[i] = '\0';
      if (i > 0) {
        LOG_INF("  << %s", buf);
      }
      if (strncmp(buf, "+EVT:TXP2P DONE", 15) == 0) {
        LOG_INF("TX done");
        tx_done = true;
        break;
      }
      /* Rejected, so no transmission is coming; waiting out the timeout would
       * trigger the hardware reset below for a retryable error. */
      if (strncmp(buf, "AT_", 3) == 0) {
        LOG_WRN("RAK3172 rejected the transmit: %s", buf);
        return -EIO;
      }
      if (strncmp(buf, "RAKwireless", 11) == 0) {
        rebooted = true;
      }
      if (rebooted && strncmp(buf, "Current Work Mode", 17) == 0) {
        break;
      }
      i = 0;
      continue;
    }

    if (c != '\r' && i < (int)sizeof(buf) - 1) {
      buf[i++] = c;
    }
  }

  if (rebooted) {
    LOG_WRN("RAK3172 rebooted after TX, reconfiguring P2P");
    rak3172_configure_p2p();
  } else if (!tx_done) {
    LOG_WRN("RAK3172 no TX confirmation; resetting and reinitialising");
    rak3172_hw_reset();
    rak3172_init();
    return -EIO;
  }

  return 0;
}

int rak3172_send_message(const char *msg) {
  return rak3172_send_payload((const uint8_t *)msg, strlen(msg));
}

int rak3172_sleep(void) {
  LOG_INF("Putting rak3172 to sleep");
  rak3172_send_at("AT+SLEEP");
  return rak3172_read_response(4000);
}

int rak3172_low_power(bool enable) {
  rak3172_send_at(enable ? "AT+LPM=1" : "AT+LPM=0");

  int err = rak3172_read_response(1000);

  if (err != 0 || !enable) {
    return err;
  }

  rak3172_send_at("AT+LPMLVL=2");
  return rak3172_read_response(1000);
}

static int claim_depth;

void rak3172_uart_claim(void) {
  k_mutex_lock(&rak3172_lock, K_FOREVER);

  if (claim_depth++ > 0) {
    return;
  }

  int err = pm_device_runtime_get(uart_dev);

  if (err) {
    LOG_ERR("Error: UART resume failed (%d)", err);
  }

  uart_irq_rx_enable(uart_dev);
}

void rak3172_uart_release(void) {
  if (--claim_depth == 0) {
    uart_irq_rx_disable(uart_dev);

    int err = pm_device_runtime_put(uart_dev);

    if (err) {
      LOG_ERR("Error: UART suspend failed (%d)", err);
    }
  }

  k_mutex_unlock(&rak3172_lock);
}

void rak3172_print_version_info(void) {
  rak3172_send_at("AT+VER=?");
  rak3172_read_response(3000);
}

int rak3172_wait_event(const char *prefix, char *buf, size_t buf_len,
                       int timeout_ms) {
  int64_t deadline = k_uptime_get() + timeout_ms;
  size_t prefix_len = strlen(prefix);
  int i = 0;

  while (true) {
    int64_t remaining = deadline - k_uptime_get();

    if (remaining <= 0) {
      return -ETIMEDOUT;
    }

    uint8_t c;

    if (rx_get_byte(&c, remaining) != 0) {
      return -ETIMEDOUT;
    }

    if (c == '\n') {
      buf[i] = '\0';
      if (i > 0) {
        LOG_INF("  << %s", buf);
        if (strncmp(buf, prefix, prefix_len) == 0) {
          return 0;
        }
      }
      i = 0;
      continue;
    }

    if (c != '\r' && i < (int)buf_len - 1) {
      buf[i++] = c;
    }
  }
}

/* peer_preamble is the home station's, (not this collar's LORA send Preamble): the module sizes its receive
 * window from it, and ours closes the window mid-preamble. */
int rak3172_cad_rx_configure(uint8_t symbols, uint8_t det_peak, uint8_t det_min,
                             uint32_t interval_ms, uint8_t max_payload,
                             uint16_t peer_preamble) {
  char cmd[56];

  snprintf(cmd, sizeof(cmd), "AT+CADP=%u:%u:%u:%u:%u:%u", symbols, det_peak,
           det_min, interval_ms, max_payload, peer_preamble);
  rak3172_send_at(cmd);

  int err = rak3172_read_response(3000);

  if (err != 0) {
    LOG_ERR("Error: AT+CADP rejected (%d); is the CAD firmware flashed?", err);
  }

  return err;
}

int rak3172_cad_rx_start(void) {
  rak3172_send_at("AT+CADRX=1");

  int err = rak3172_read_response(3000);

  if (err != 0) {
    LOG_ERR("Error: AT+CADRX=1 failed (%d)", err);
  }

  return err;
}

int rak3172_cad_rx_stop(void) {
  rak3172_send_at("AT+CADRX=0");
  return rak3172_read_response(3000);
}

int rak3172_cad_rx_read(void) {
  rak3172_send_at("AT+CADRD");
  return rak3172_read_response(3000);
}

int rak3172_cad_rx_set_pin(uint32_t pin) {
  char cmd[32];

  snprintf(cmd, sizeof(cmd), "AT+CADPIN=%u", pin);
  rak3172_send_at(cmd);

  int err = rak3172_read_response(3000);

  if (err != 0) {
    LOG_ERR("Error: AT+CADPIN rejected (%d)", err);
  }

  return err;
}

int rak3172_cad_rx_set_filter(const char *hex) {
  char cmd[32];

  snprintf(cmd, sizeof(cmd), "AT+CADFILT=%s", hex);
  rak3172_send_at(cmd);

  int err = rak3172_read_response(3000);

  if (err != 0) {
    LOG_ERR("Error: AT+CADFILT rejected (%d)", err);
  }

  return err;
}

int rak3172_cad_rx_set_presence(const char *hex) {
  char cmd[32];

  snprintf(cmd, sizeof(cmd), "AT+CADPRES=%s", hex);
  rak3172_send_at(cmd);

  int err = rak3172_read_response(3000);

  if (err != 0) {
    LOG_ERR("Error: AT+CADPRES rejected (%d)", err);
  }

  return err;
}

int rak3172_cad_rx_set_absence(uint32_t seconds) {
  char cmd[32];

  snprintf(cmd, sizeof(cmd), "AT+CADABS=%u", seconds);
  rak3172_send_at(cmd);

  int err = rak3172_read_response(3000);

  if (err != 0) {
    LOG_ERR("Error: AT+CADABS rejected (%d)", err);
  }

  return err;
}

/* The module owns the schedule; only the wake line brings the nRF up.
 * burst_ms of 0 sniffs continuously. */
int rak3172_set_heartbeat(uint32_t period_s, const char *hex) {
  char cmd[64];

  if (period_s == 0) {
    snprintf(cmd, sizeof(cmd), "AT+HBEAT=0");
  } else {
    snprintf(cmd, sizeof(cmd), "AT+HBEAT=%u:%s", period_s, hex);
  }
  rak3172_send_at(cmd);

  int err = rak3172_read_response(3000);

  if (err != 0) {
    LOG_WRN("AT+HBEAT rejected (%d); is the heartbeat firmware flashed?", err);
  }

  return err;
}

int rak3172_cad_rx_set_duty(uint32_t burst_ms, uint32_t period_ms) {
  char cmd[40];

  snprintf(cmd, sizeof(cmd), "AT+CADDUTY=%u:%u", burst_ms, period_ms);
  rak3172_send_at(cmd);

  int err = rak3172_read_response(3000);

  if (err != 0) {
    LOG_ERR("Error: AT+CADDUTY rejected (%d)", err);
  }

  return err;
}

int rak3172_cad_rx_event(void) {
  char line[64];

  rak3172_send_at("AT+CADEVT=?");

  if (rak3172_wait_event(CADEVT_PREFIX, line, sizeof(line), 3000) != 0) {
    return -EIO;
  }

  const char *value = line + CADEVT_PREFIX_LEN;

  if (strcmp(value, "ABSENT") == 0) {
    return RAK3172_CAD_EVENT_ABSENT;
  }
  if (strcmp(value, "PKT") == 0) {
    return RAK3172_CAD_EVENT_PACKET;
  }

  return RAK3172_CAD_EVENT_NONE;
}

int rak3172_cad_rx_stats(void) {
  rak3172_send_at("AT+CADSTAT=?");
  return rak3172_read_response(3000);
}
