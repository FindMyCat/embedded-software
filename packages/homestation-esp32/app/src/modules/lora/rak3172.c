#include <string.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/ring_buffer.h>

#include "rak3172.h"

LOG_MODULE_REGISTER(rak3172, LOG_LEVEL_DBG);

static const struct device *uart_dev = DEVICE_DT_GET(DT_ALIAS(rak3172_uart));

#define RX_RING_BUF_SIZE 512
RING_BUF_DECLARE(rx_ring_buf, RX_RING_BUF_SIZE);
static K_SEM_DEFINE(rx_sem, 0, RX_RING_BUF_SIZE);

static uint16_t configured_preamble;

static void uart_irq_cb(const struct device *dev, void *user_data) {
  ARG_UNUSED(user_data);

  if (!uart_irq_update(dev)) {
    return;
  }

  while (uart_irq_rx_ready(dev)) {
    uint8_t c;

    if (uart_fifo_read(dev, &c, 1) == 1) {
      if (ring_buf_put(&rx_ring_buf, &c, 1) == 1) {
        k_sem_give(&rx_sem);
      }
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

/* Drop whatever is still buffered from an earlier command. A read that timed
 * out, or an event that arrived on its own, otherwise gets read as the next
 * command's reply and misattributes every response after it. */
static void rx_flush(void) {
  unsigned int key = irq_lock();

  ring_buf_reset(&rx_ring_buf);
  k_sem_reset(&rx_sem);
  irq_unlock(key);
}

void rak3172_send_at(const char *cmd) {
  /* The module spends the first byte it receives on waking its UART, so spend
   * a newline on that rather than the command, which would otherwise arrive
   * truncated and be rejected as AT_COMMAND_NOT_FOUND. */
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

void rak3172_read_response(int timeout_ms) {
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
      LOG_INF("  << %s", buf);
      /* RUI3 answers OK, or an AT_-prefixed failure such as AT_PARAM_ERROR
       * or AT_COMMAND_NOT_FOUND. Either way the command is finished. */
      if (strcmp(buf, "OK") == 0 || strncmp(buf, "AT_", 3) == 0 ||
          strstr(buf, "ERROR") != NULL) {
        return;
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
}

static void rak3172_configure_p2p(void) {
  char cmd[48];

  LOG_INF("Configuring P2P parameters");
  /* US915, SF7, BW 125kHz, CR 4/5, preamble 8 */
  snprintf(cmd, sizeof(cmd), "AT+P2P=915000000:7:125:0:8:%d",
           CONFIG_LORA_TX_POWER_DBM);
  rak3172_send_at(cmd);
  rak3172_read_response(3000);
}

static int uart_init(void) {
  if (!device_is_ready(uart_dev)) {
    LOG_ERR("Error: RAK3172 UART device not ready");
    return -ENODEV;
  }

  uart_irq_callback_set(uart_dev, uart_irq_cb);
  uart_irq_rx_enable(uart_dev);

  LOG_INF("RAK3172 UART ready");
  return 0;
}

int rak3172_init(void) {
  int err = uart_init();

  if (err) {
    return err;
  }

  LOG_INF("Enabling P2P mode");
  rak3172_send_at("AT+NWM=0");
  rak3172_read_response(8000);

  rak3172_configure_p2p();
  k_sleep(K_MSEC(500));

  // LOG_INF("Enabling low power mode");
  rak3172_send_at("AT+LPM=0");
  rak3172_read_response(1000);

  LOG_INF("RAK3172 ready");
  return 0;
}

int rak3172_send_message(const char *msg) {
  return rak3172_send_payload((const uint8_t *)msg, strlen(msg));
}

int rak3172_send_payload(const uint8_t *payload, size_t len) {
  char cmd[272] = "AT+PSEND=";
  char *hex = cmd + strlen("AT+PSEND=");

  if (len == 0 || len > 127) {
    LOG_ERR("Error: payload of %u bytes, max 127", (unsigned)len);
    return -EINVAL;
  }

  for (size_t i = 0; i < len; i++) {
    snprintf(&hex[i * 2], 3, "%02X", payload[i]);
  }

  rak3172_send_at("AT+PRECV=0");
  rak3172_read_response(1000);

  /* A brownout reboot mid-transmit comes back on AT+P2P's preamble of 8, which
   * is far too short for the collar's duty-cycled receiver. Re-applied per
   * transmit rather than trusting it to persist; the module rejects AT+PPL
   * while P2P RX is on, so it has to follow AT+PRECV=0. */
  if (configured_preamble != 0) {
    rak3172_set_preamble(configured_preamble);
  }

  /* The radio needs a moment to settle before it will accept a transmit. */
  k_sleep(K_MSEC(50));

  rak3172_send_at(cmd);

  /* "OK" returns immediately (command accepted); +EVT:TXP2P DONE fires when the
   * air transmission finishes. The module was observed to reboot after TX due to
   * RF coupling into the RX/TX wires; detect that and reconfigure P2P. */
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
    if (configured_preamble != 0) {
      rak3172_set_preamble(configured_preamble);
    }
    return -EIO;
  }

  if (!tx_done) {
    LOG_WRN("RAK3172 no TX confirmation");
    return -EIO;
  }

  return 0;
}

void rak3172_suspend(void) { uart_irq_rx_disable(uart_dev); }

void rak3172_resume(void) { uart_irq_rx_enable(uart_dev); }

void rak3172_print_version_info(void) {
  rak3172_send_at("AT+VER=?");
  rak3172_read_response(3000);
}

void rak3172_set_preamble(uint16_t symbols) {
  char cmd[32];

  configured_preamble = symbols;
  snprintf(cmd, sizeof(cmd), "AT+PPL=%u", symbols);
  rak3172_send_at(cmd);
  rak3172_read_response(3000);
}

void rak3172_start_rx(void) {
  LOG_INF("Entering continuous RX mode");

  /* RUI3 leaves receive mode once a packet arrives, and ignores or rejects
   * the first command issued after +EVT:RXP2P. Stopping first absorbs that,
   * so a short timeout is enough: the reply is often never sent. */
  rak3172_send_at("AT+PRECV=0");
  rak3172_read_response(200);
  k_sleep(K_MSEC(50));

  rak3172_send_at("AT+PRECV=65535");
  rak3172_read_response(3000);
}

/* Callers must pass the same buffer every time: a line that is still
 * incomplete when the timeout expires stays in it, and the next call resumes
 * where this one stopped. Resetting per call would drop the head of any line
 * that happened to start near the end of a timeout window. */
int rak3172_read_event(char *buf, size_t buf_len, int timeout_ms) {
  static int i;
  int64_t deadline = k_uptime_get() + timeout_ms;

  while (true) {
    int64_t remaining = deadline - k_uptime_get();

    if (remaining <= 0) {
      return -EAGAIN;
    }

    uint8_t c;

    if (rx_get_byte(&c, remaining) != 0) {
      return -EAGAIN;
    }

    if (c == '\n') {
      buf[i] = '\0';
      if (i > 0) {
        i = 0;
        return 0;
      }
      continue;
    }

    if (c != '\r' && i < (int)buf_len - 1) {
      buf[i++] = c;
    }
  }
}
