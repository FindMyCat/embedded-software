#include <errno.h>
#include <stdlib.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/socket_ncs.h>
#include <zephyr/logging/log.h>
#include "../Responder/Responder.h"
#include "../Dispatcher/Dispatcher.h"
#include "../utils/network.h"
#include "NewUdpListener.h"

LOG_MODULE_REGISTER(new_udp_listener, CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

#define UDP_PORT 12345
#define MAX_DATA_SIZE 256

struct sockaddr_in my_addr;
int udp_socket;
char rx_data[MAX_DATA_SIZE];
ssize_t recv_len;

#define UDP_LISTENER_STACK_SIZE 2048
#define UDP_LISTENER_THREAD_PRIORITY 5

K_THREAD_STACK_DEFINE(udp_listener_stack_area, UDP_LISTENER_STACK_SIZE);
struct k_thread udp_listener_thread_data;
k_tid_t udp_thread_id;

/* Block until a datagram or an error arrives. A timeout here would wake the
 * CPU on every expiry for nothing, which is visible in a current trace. */
#define RECEIVE_POLL_TIMEOUT_MS -1

static int64_t last_uplink_uptime_ms = -1;
static int64_t last_inbound_uptime_ms = -1;
static uint32_t uplink_datagram_count;
static uint32_t inbound_datagram_count;
static struct sockaddr_in touch_destination;
static bool touch_destination_resolved;

/**
 * @brief Seconds since the given uptime stamp, or -1 if it never happened.
 */
static int64_t seconds_since(int64_t stamp_ms) {
    if (stamp_ms < 0) {
        return -1;
    }

    return (k_uptime_get() - stamp_ms) / MSEC_PER_SEC;
}

int64_t udp_listener_seconds_since_uplink(void) {
    return seconds_since(last_uplink_uptime_ms);
}

int64_t udp_listener_seconds_since_inbound(void) {
    return seconds_since(last_inbound_uptime_ms);
}

#define AGE_TEXT_SIZE 16

/**
 * @brief Render an age from seconds_since() as "Ns", or "never" for -1.
 */
static const char *format_age(int64_t seconds, char *buffer, size_t size) {
    if (seconds < 0) {
        snprintk(buffer, size, "never");
    } else {
        snprintk(buffer, size, "%llds ago", seconds);
    }

    return buffer;
}

/**
 * @brief Ask the network to release the connection after the next send.
 *
 * Without this the modem stays in connected mode until the network's RRC
 * inactivity timer expires, drawing connected-mode current the whole time for
 * no traffic. eDRX only applies once released to idle.
 */
static void request_release_after_send(int socket) {
    int rai = RAI_LAST;

    if (zsock_setsockopt(socket, SOL_SOCKET, SO_RAI, &rai, sizeof(rai)) != 0) {
        LOG_WRN("RAI_LAST not accepted (%d), sending without it", errno);
    }
}

/**
 * @brief Log the reachability counters that the carrier idle-timer test reads.
 */
void udp_listener_log_status(void) {
    char uplink_age[AGE_TEXT_SIZE];
    char inbound_age[AGE_TEXT_SIZE];

    LOG_INF("UDP status: uplink %u (%s), inbound %u (%s)", uplink_datagram_count,
            format_age(udp_listener_seconds_since_uplink(), uplink_age,
                       sizeof(uplink_age)),
            inbound_datagram_count,
            format_age(udp_listener_seconds_since_inbound(), inbound_age,
                       sizeof(inbound_age)));
}

/**
 * @brief Resolve the cloud host once and cache it as the touch destination.
 */
static int resolve_touch_destination(void) {
    if (touch_destination_resolved) {
        return 0;
    }

    char *ip_str = getIpAddressFromHostname(CONFIG_FINDMYCAT_CLOUD_HOSTNAME);
    if (!ip_str) {
        LOG_ERR("Error: Unable to resolve %s for uplink touch",
                CONFIG_FINDMYCAT_CLOUD_HOSTNAME);
        return -1;
    }

    memset(&touch_destination, 0, sizeof(touch_destination));
    touch_destination.sin_family = AF_INET;
    touch_destination.sin_port = htons(CONFIG_MQTT_SN_GATEWAY_PORT);

    int err = zsock_inet_pton(AF_INET, ip_str, &touch_destination.sin_addr);
    free(ip_str);

    if (err != 1) {
        LOG_ERR("Error: Unable to parse touch destination address (%d)", err);
        return -1;
    }

    touch_destination_resolved = true;
    return 0;
}

/**
 * @brief Send one datagram so the carrier sees uplink traffic on this path.
 *
 * @param use_listener_socket Send from the socket bound to UDP_PORT. When
 *        false, sends from a throwaway socket on an ephemeral port instead,
 *        which separates a per-flow idle timer from a per-session one.
 */
int udp_listener_touch_uplink(bool use_listener_socket) {
    if (resolve_touch_destination() != 0) {
        return -1;
    }

    static const uint8_t touch_byte = 0;
    int socket_to_use = udp_socket;
    int throwaway_socket = -1;

    if (!use_listener_socket) {
        throwaway_socket = zsock_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (throwaway_socket < 0) {
            LOG_ERR("Error: Unable to create throwaway socket (%d)", errno);
            return -1;
        }
        socket_to_use = throwaway_socket;
    }

    request_release_after_send(socket_to_use);

    ssize_t sent = zsock_sendto(socket_to_use, &touch_byte, sizeof(touch_byte), 0,
                                (struct sockaddr *)&touch_destination,
                                sizeof(touch_destination));

    if (throwaway_socket >= 0) {
        zsock_close(throwaway_socket);
    }

    if (sent < 0) {
        LOG_ERR("Error: Uplink touch send failed (%d)", errno);
        return -1;
    }

    last_uplink_uptime_ms = k_uptime_get();
    uplink_datagram_count++;
    LOG_INF("Uplink touch sent from %s port, total %u",
            use_listener_socket ? "listener" : "ephemeral", uplink_datagram_count);

    return 0;
}

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
    zsock_bind(udp_socket, (struct sockaddr *)&my_addr, sizeof(my_addr));

    return 0;
}

int destroy_udp_listener() {
    return zsock_close(udp_socket);
}

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
            /* A dead socket must not kill the thread: the listener has to
             * survive the PDN drops that happen away from the home station. */
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
        last_inbound_uptime_ms = k_uptime_get();
        inbound_datagram_count++;

        char uplink_age[AGE_TEXT_SIZE];

        LOG_INF("Command %s, datagram %u, last uplink %u (%s)", rx_data,
                inbound_datagram_count, uplink_datagram_count,
                format_age(udp_listener_seconds_since_uplink(), uplink_age,
                           sizeof(uplink_age)));

		// Commands are kept short so each one costs as little data as possible.
		// The dispatcher logs the state it moves to, so no need to log it here.
        if (strncmp(rx_data, "spoof-ping", recv_len) == 0) {
            changeDispatcherState(DISPATCHER_STATE_RESPOND_TO_SPOOF_PING);
        } else if (strncmp(rx_data, "ping", recv_len) == 0) {
            changeDispatcherState(DISPATCHER_STATE_RESPOND_TO_PING);
        } else if(strncmp(rx_data, "lost", recv_len) == 0) {
            changeDispatcherState(DISPATCHER_STATE_LOST_MODE);
        } else if (strncmp(rx_data, "active", recv_len) == 0) {
            changeDispatcherState(DISPATCHER_STATE_ACTIVE_MODE);
		} else if (strncmp(rx_data, "idle", recv_len) == 0) {
            changeDispatcherState(DISPATCHER_STATE_IDLE);
		} else {
			LOG_ERR("Error: Unknown command: %s", rx_data);
		}

    }
}

void start_udp_listener_thread() {
    udp_thread_id = k_thread_create(&udp_listener_thread_data, udp_listener_stack_area,
                                 K_THREAD_STACK_SIZEOF(udp_listener_stack_area),
                                 start_listening,
                                 NULL, NULL, NULL,
                                 UDP_LISTENER_THREAD_PRIORITY, 0, K_NO_WAIT);
}

void stop_udp_listener_thread() {
    k_thread_abort(udp_thread_id);
}
