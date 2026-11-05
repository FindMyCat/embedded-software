#include "supl_assistance.h"

#include <errno.h>
#include <stdio.h>
#include <modem/lte_lc.h>
#include <supl_os_client.h>
#include <supl_session.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/socket.h>

LOG_MODULE_REGISTER(supl_assistance,
                    CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

/* The SUPL library expects its read callback to give up after one second. */
#define SUPL_SOCKET_READ_TIMEOUT_SECONDS 1

static int supl_server_socket = -1;

static ssize_t read_from_supl_server(void *buffer, size_t buffer_length,
                                     void *user_data) {
  ARG_UNUSED(user_data);

  ssize_t received_bytes =
      zsock_recv(supl_server_socket, buffer, buffer_length, 0);

  if (received_bytes < 0 && errno == EAGAIN) {
    return 0; /* The library treats 0 as a timeout and asks again. */
  }
  if (received_bytes == 0) {
    return -1; /* The server closed the connection. */
  }
  return received_bytes;
}

static ssize_t write_to_supl_server(const void *buffer, size_t buffer_length,
                                    void *user_data) {
  ARG_UNUSED(user_data);
  return zsock_send(supl_server_socket, buffer, buffer_length, 0);
}

static int inject_assistance_into_modem(void *assistance_data,
                                        size_t assistance_length_bytes,
                                        uint16_t assistance_type,
                                        void *user_data) {
  ARG_UNUSED(user_data);

  int err = nrf_modem_gnss_agnss_write(assistance_data, assistance_length_bytes,
                                       assistance_type);

  if (err) {
    LOG_ERR("Error: writing assistance type %u into the modem failed (%d)",
            assistance_type, err);
    return -1;
  }

  LOG_INF("Injected assistance type %u (%u bytes)", assistance_type,
          (unsigned)assistance_length_bytes);
  return 0;
}

static int log_supl_library_message(int level, const char *format, ...) {
  ARG_UNUSED(level);
  char message[256];
  va_list args;

  va_start(args, format);
  int length = vsnprintk(message, sizeof(message), format, args);
  va_end(args);

  LOG_DBG("%s", message);
  return length;
}

static int connect_to_supl_server(void) {
  char port[6];
  struct zsock_addrinfo *addresses;
  struct zsock_addrinfo hints = {
      .ai_flags = AI_NUMERICSERV,
      .ai_family = AF_UNSPEC,
      .ai_socktype = SOCK_STREAM,
  };

  snprintf(port, sizeof(port), "%d", CONFIG_FINDMYCAT_SUPL_SERVER_PORT);

  int err = zsock_getaddrinfo(CONFIG_FINDMYCAT_SUPL_SERVER_HOSTNAME, port,
                              &hints, &addresses);
  if (err) {
    LOG_ERR("Error: resolving %s failed (%d)",
            CONFIG_FINDMYCAT_SUPL_SERVER_HOSTNAME, err);
    return -EHOSTUNREACH;
  }

  err = -ECONNREFUSED;
  for (struct zsock_addrinfo *address = addresses; address != NULL;
       address = address->ai_next) {
    supl_server_socket =
        zsock_socket(address->ai_family, SOCK_STREAM, IPPROTO_TCP);
    if (supl_server_socket < 0) {
      continue;
    }

    struct timeval read_timeout = {
        .tv_sec = SUPL_SOCKET_READ_TIMEOUT_SECONDS,
    };
    zsock_setsockopt(supl_server_socket, SOL_SOCKET, SO_RCVTIMEO,
                     &read_timeout, sizeof(read_timeout));

    if (zsock_connect(supl_server_socket, address->ai_addr,
                      address->ai_addrlen) == 0) {
      err = 0;
      break;
    }

    zsock_close(supl_server_socket);
    supl_server_socket = -1;
  }

  zsock_freeaddrinfo(addresses);

  if (err) {
    LOG_ERR("Error: could not connect to SUPL server %s:%d",
            CONFIG_FINDMYCAT_SUPL_SERVER_HOSTNAME,
            CONFIG_FINDMYCAT_SUPL_SERVER_PORT);
  }
  return err;
}

static int initialize_supl_library_once(void) {
  static bool is_initialized;
  static struct supl_api supl_callbacks = {
      .read = read_from_supl_server,
      .write = write_to_supl_server,
      .handler = inject_assistance_into_modem,
      .logger = log_supl_library_message,
      .counter_ms = k_uptime_get,
  };

  if (is_initialized) {
    return 0;
  }

  if (supl_init(&supl_callbacks) != 0) {
    LOG_ERR("Error: SUPL library initialization failed");
    return -EIO;
  }

  is_initialized = true;
  return 0;
}

static bool is_lte_registered(void) {
  enum lte_lc_nw_reg_status registration = LTE_LC_NW_REG_NOT_REGISTERED;

  lte_lc_nw_reg_status_get(&registration);
  return registration == LTE_LC_NW_REG_REGISTERED_HOME ||
         registration == LTE_LC_NW_REG_REGISTERED_ROAMING;
}

int supl_assistance_fetch_and_inject(
    const struct nrf_modem_gnss_agnss_data_frame *requested_data) {
  /* Off LTE (BLE or LoRa link) there is no route to the server; GNSS then
   * starts without assistance, as before. */
  if (!is_lte_registered()) {
    LOG_INF("Skipping SUPL assistance, LTE is not registered");
    return -ENOTCONN;
  }

  int err = initialize_supl_library_once();
  if (err) {
    return err;
  }

  err = connect_to_supl_server();
  if (err) {
    return err;
  }

  int64_t session_started_ms = k_uptime_get();

  err = supl_session(requested_data);
  zsock_close(supl_server_socket);
  supl_server_socket = -1;

  LOG_INF("SUPL session %s after %lld ms", err ? "failed" : "done",
          k_uptime_get() - session_started_ms);
  return err;
}