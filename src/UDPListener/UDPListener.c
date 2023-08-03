#include <zephyr/net/socket.h>
#include "../Responder/Responder.h"
#include "../Dispatcher/Dispatcher.h"
#include <zephyr/logging/log.h>

#define UDP_PORT 12345
#define MAX_DATA_SIZE 256

LOG_MODULE_REGISTER(udp_listener, 4);


void udp_listener_init() {
	struct sockaddr_in my_addr;
    int udp_socket;
    char rx_data[MAX_DATA_SIZE];
    ssize_t recv_len;

    LOG_INF("UDP Data Receiver is listening for remote commands(IPv4)");

    // Create a UDP socket
    udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udp_socket < 0) {
        LOG_ERR("Unable to create UDP socket");
        return;
    }

    // Bind the socket to the UDP_PORT
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(UDP_PORT);
    bind(udp_socket, (struct sockaddr *)&my_addr, sizeof(my_addr));

    while (true) {
        // Receive data on the UDP socket
        recv_len = recv(udp_socket, rx_data, sizeof(rx_data), 0);
        if (recv_len < 0) {
            printk("Error: Unable to receive data\n");
            break;
        }

        // Print the received data
        printk("Received data: %.*s\n", (int)recv_len, rx_data);

		// Two commands are supported over SMS. 
		// We should keep these command strings small as possible for least data transfer.
		if (strncmp(rx_data, "activate", recv_len) == 0) {
			changeDispatcherState(DISPATCHER_STATE_RESPONDING);
			LOG_INF("Dispatcher state changed to DISPATCHER_STATE_RESPONDING\n");
		} else if (strncmp(rx_data, "deactivate", recv_len) == 0) {
			changeDispatcherState(DISPATCHER_STATE_IDLE);
			LOG_INF("Dispatcher state changed to DISPATCHER_STATE_IDLE\n");
		} else {
			LOG_ERR("Unknown command: %s\n", rx_data);
		}
    }

    // Close the socket
    // close(udp_socket);
}