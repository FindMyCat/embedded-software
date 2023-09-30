#include <zephyr/net/socket.h>
#include <zephyr/logging/log.h>
#include "../Responder/Responder.h"
#include "../Dispatcher/Dispatcher.h"

LOG_MODULE_REGISTER(new_udp_listener, 4);

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


int create_udp_listener() {
    LOG_INF("Creating UDP Data Receiver(IPv4)");

    // Create a UDP socket
    udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udp_socket < 0) {
        LOG_ERR("Unable to create UDP socket");
        return -1;
    }

    // Bind the socket to the UDP_PORT
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(UDP_PORT);
    bind(udp_socket, (struct sockaddr *)&my_addr, sizeof(my_addr));

    return 0;
}

int destroy_udp_listener() {
    return close(udp_socket);
}

void start_listening() {
    while (true) {
        LOG_INF("UDP Listener thread is running.");
        // Receive data on the UDP socket
        // Note that the recv call is blocking until data is received.
        recv_len = recv(udp_socket, rx_data, sizeof(rx_data), 0);
        if (recv_len < 0) {
            printk("Error: Unable to receive data\n");
            break;
        }

        // Print the received data
        printk("Received data: %.*s\n", (int)recv_len, rx_data);

		// Two commands are supported over UDP. 
		// We should keep these command strings small as possible for least data transfer.
        if (strncmp(rx_data, "ping", recv_len) == 0) {
            LOG_INF("Dispatcher state changed to DISPATCHER_STATE_PING_MODE\n");
            changeDispatcherState(DISPATCHER_STATE_RESPOND_TO_PING);
        } else if(strncmp(rx_data, "lost", recv_len) == 0) {
            LOG_INF("Dispatcher state changed to DISPATCHER_STATE_LOST_MODE\n");
            changeDispatcherState(DISPATCHER_STATE_LOST_MODE);
        } else if (strncmp(rx_data, "active", recv_len) == 0) {
			LOG_INF("Dispatcher state changed to DISPATCHER_STATE_ACTIVE_MODE\n");
            changeDispatcherState(DISPATCHER_STATE_ACTIVE_MODE);
		} else if (strncmp(rx_data, "idle", recv_len) == 0) {
			LOG_INF("Dispatcher state changed to DISPATCHER_STATE_IDLE\n");
            changeDispatcherState(DISPATCHER_STATE_IDLE);
		} else {
			LOG_ERR("Unknown command: %s\n", rx_data);
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