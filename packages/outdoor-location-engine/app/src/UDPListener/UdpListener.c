#include "UdpListener.h"
#include "../Dispatcher/Dispatcher.h"
#include "../led/led.h"
#include <errno.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/socket.h>

LOG_MODULE_REGISTER(new_udp_listener,
                    CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

#define UDP_PORT 12345
#define MAX_DATA_SIZE 256

struct sockaddr_in my_addr;
int udp_socket;
char rx_data[MAX_DATA_SIZE];
ssize_t recv_len;

/* A ping runs the WiFi scan and a full MQTT-SN session on this thread. */
#define UDP_LISTENER_STACK_SIZE 4096
#define UDP_LISTENER_THREAD_PRIORITY 5

K_THREAD_STACK_DEFINE(udp_listener_stack_area, UDP_LISTENER_STACK_SIZE);
struct k_thread udp_listener_thread_data;
k_tid_t udp_thread_id;

#define RECEIVE_POLL_TIMEOUT_MS -1
#define UDP_RX_FLASH_MS 100

static uint32_t inbound_datagram_count;

int create_udp_listener() {
  LOG_INF("Creating UDP Data Receiver(IPv4)");

  // Create a UDP socket
  udp_socket = zsock_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (udp_socket < 0) {
    LOG_ERR("Error: Unable to create UDP socket");
    return -1;
  }

  // Bind the socket to the UDP_PORT
  memset(&my_addr, 0, sizeof(my_addr));
  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(UDP_PORT);
  if (zsock_bind(udp_socket, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0) {
    int err = -errno;

    LOG_ERR("Error: Unable to bind UDP port %d (%d)", UDP_PORT, err);
    zsock_close(udp_socket);
    return err;
  }

  LOG_INF("Listening on UDP port %d", UDP_PORT);
  return 0;
}

int destroy_udp_listener() { return zsock_close(udp_socket); }

void start_listening() {
  while (true) {
    struct zsock_pollfd poll_descriptor = {
        .fd = udp_socket,
        .events = ZSOCK_POLLIN,
    };

    int ready = zsock_poll(&poll_descriptor, 1, RECEIVE_POLL_TIMEOUT_MS);
    if (ready < 0) {
      LOG_ERR("Error: Poll on UDP socket failed (%d)", errno);
      k_sleep(K_SECONDS(1));
      continue;
    }

    if (ready == 0) {
      continue;
    }

    // Receive data on the UDP socket
    // Note that the recv call is blocking until data is received.
    recv_len = zsock_recv(udp_socket, rx_data, sizeof(rx_data) - 1, 0);
    if (recv_len < 0) {
      LOG_ERR("Error: Unable to receive data (%d)", errno);

      if (errno == EAGAIN || errno == EINTR) {
        continue;
      }

      LOG_WRN("Recreating UDP listener socket");
      destroy_udp_listener();
      if (create_udp_listener() != 0) {
        k_sleep(K_SECONDS(5));
      }
      continue;
    }

    rx_data[recv_len] = '\0';
    inbound_datagram_count++;

    LOG_INF("Command %s, datagram %u", rx_data, inbound_datagram_count);

    /* Visible proof a datagram arrived, before the command runs. */
    green__led_on();
    k_sleep(K_MSEC(UDP_RX_FLASH_MS));
    green__led_off();

    (void)dispatchCommand(rx_data, recv_len);
  }
}

void start_udp_listener_thread() {
  udp_thread_id = k_thread_create(
      &udp_listener_thread_data, udp_listener_stack_area,
      K_THREAD_STACK_SIZEOF(udp_listener_stack_area), start_listening, NULL,
      NULL, NULL, UDP_LISTENER_THREAD_PRIORITY, 0, K_NO_WAIT);
  k_thread_name_set(udp_thread_id, "udp_listener");
}

void stop_udp_listener_thread() { k_thread_abort(udp_thread_id); }
