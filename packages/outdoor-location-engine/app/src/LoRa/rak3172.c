#include <string.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/pm/device.h>
#include <zephyr/pm/device_runtime.h>
#include <zephyr/sys/ring_buffer.h>

#include "rak3172.h"

LOG_MODULE_REGISTER(rak3172, LOG_LEVEL_DBG);

static const struct device *uart_dev = DEVICE_DT_GET(DT_NODELABEL(uart2));
static const struct device *gpio0_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));

#define UART2_TX_PIN 24

#define RX_RING_BUF_SIZE 256
RING_BUF_DECLARE(rx_ring_buf, RX_RING_BUF_SIZE);
static K_SEM_DEFINE(rx_sem, 0, RX_RING_BUF_SIZE);

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

void rak3172_send_at(const char *cmd) {
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
      if (strcmp(buf, "OK") == 0 || strstr(buf, "ERROR") != NULL) {
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
  LOG_INF("Configuring P2P parameters");
  /* US915, SF7, BW 125kHz, CR 4/5, preamble 8, 5 dBm */
  rak3172_send_at("AT+P2P=915000000:7:125:0:8:5");
  rak3172_read_response(3000);
}

int rak3172_init(void) {
  if (!device_is_ready(uart_dev)) {
    LOG_ERR("Error: RAK3172 UART device not ready");
    return -ENODEV;
  }

  // pm_device_runtime_enable(uart_dev);
  uart_irq_callback_set(uart_dev, uart_irq_cb);
  uart_irq_rx_enable(uart_dev);

  LOG_INF("RAK3172 UART ready");

  LOG_INF("Factory resetting RAK3172");
  rak3172_send_at("ATR");
  rak3172_read_response(5000);

  LOG_INF("Enabling P2P mode");
  rak3172_send_at("AT+NWM=0");
  rak3172_read_response(8000);

  rak3172_configure_p2p();
  k_sleep(K_MSEC(500));

  LOG_INF("Enabling low power mode");
  rak3172_send_at("AT+LPM=1");
  rak3172_read_response(1000);

  LOG_INF("RAK3172 ready");
  return 0;
}

int rak3172_send_message(const char *msg) {
  char cmd[272] = "AT+PSEND=";
  char *hex = cmd + strlen("AT+PSEND=");
  int len = strlen(msg);

  if (len > 127) {
    LOG_ERR("Error: message too long (%d bytes, max 127)", len);
    return -1;
  }

  for (int i = 0; i < len; i++) {
    snprintf(&hex[i * 2], 3, "%02X", (uint8_t)msg[i]);
  }

  rak3172_send_at("AT+PRECV=0");
  rak3172_read_response(1000);
  k_sleep(K_MSEC(50));

  rak3172_send_at(cmd);

  /* Hold TX line high during RF burst to prevent noise coupling into RAK RX */
  gpio_pin_configure(gpio0_dev, UART2_TX_PIN, GPIO_OUTPUT_HIGH);

  /* OK returns immediately (command accepted); +EVT:TXP2P DONE fires when
   * the air transmission finishes. If LPM is on, the module may reboot
   * after TX — detect and reconfigure P2P in that case. */
  bool rebooted = false;
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

  gpio_pin_configure(gpio0_dev, UART2_TX_PIN, GPIO_DISCONNECTED);

  if (rebooted) {
    LOG_WRN("RAK3172 rebooted after TX, reconfiguring P2P");
    rak3172_configure_p2p();
  }

  return 0;
}

void rak3172_suspend(void) {
  uart_irq_rx_disable(uart_dev);
  int err = pm_device_runtime_put(uart_dev);
  if (err) {
    LOG_ERR("Error: UART suspend failed (%d)", err);
  }
}

void rak3172_resume(void) {
  int err = pm_device_runtime_get(uart_dev);
  if (err) {
    LOG_ERR("Error: UART resume failed (%d)", err);
  }
  uart_irq_rx_enable(uart_dev);
}

void rak3172_print_version_info(void) {
  rak3172_send_at("AT+VER=?");
  rak3172_read_response(3000);
}
