#include <string.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "rak3172.h"

LOG_MODULE_REGISTER(rak3172, LOG_LEVEL_DBG);

static const struct device *uart_dev = DEVICE_DT_GET(DT_NODELABEL(uart2));

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

  while (k_uptime_get() < deadline) {
    uint8_t c;

    if (uart_poll_in(uart_dev, &c) < 0) {
      k_yield();
      continue;
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

/* Drains UART for timeout_ms and returns true if a module reboot was detected. */
static bool drain_check_reboot(int timeout_ms) {
  int64_t deadline = k_uptime_get() + timeout_ms;
  char buf[128];
  int i = 0;
  bool rebooted = false;

  while (k_uptime_get() < deadline) {
    uint8_t c;

    if (uart_poll_in(uart_dev, &c) < 0) {
      k_yield();
      continue;
    }

    if (c == '\n') {
      buf[i] = '\0';
      if (strncmp(buf, "RAKwireless", 11) == 0) {
        LOG_WRN("RAK3172 rebooted after TX — re-configuring P2P");
        rebooted = true;
      }
      i = 0;
      continue;
    }

    if (c != '\r' && i < (int)sizeof(buf) - 1) {
      buf[i++] = c;
    }
  }

  return rebooted;
}

int rak3172_init(void) {
  if (!device_is_ready(uart_dev)) {
    LOG_ERR("Error: RAK3172 UART device not ready");
    return -ENODEV;
  }
  LOG_INF("RAK3172 UART ready");

  LOG_INF("Enabling P2P mode");
  rak3172_send_at("AT+NWM=0");
  rak3172_read_response(8000);

  rak3172_configure_p2p();
  k_sleep(K_MSEC(500));
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
  rak3172_read_response(5000);

  if (drain_check_reboot(500)) {
    rak3172_read_response(8000);
    rak3172_configure_p2p();
  }

  return 0;
}

void rak3172_print_version_info(void) {
  rak3172_send_at("AT+VER=?");
  rak3172_read_response(3000);
}
