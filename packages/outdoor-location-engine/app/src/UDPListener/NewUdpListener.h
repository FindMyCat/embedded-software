#ifndef NEW_UDP_LISTENER_DOT_H    /* This is an "include guard" */
#define NEW_UDP_LISTENER_DOT_H    /* prevents the file from being included twice. */
                              /* Including a header file twice causes all kinds */
                              /* of interesting problems.*/

#include <stdbool.h>
#include <stdint.h>

void start_udp_listener_thread();
void stop_udp_listener_thread();

int create_udp_listener();
int destroy_udp_listener();

/**
 * @brief Send one datagram to the cloud host so the carrier sees uplink.
 *
 * @param use_listener_socket Send from the socket bound to the listener port.
 *        When false, uses a throwaway ephemeral port instead, which separates
 *        a per-flow carrier idle timer from a per-session one.
 *
 * @return 0 on success, -1 otherwise.
 */
int udp_listener_touch_uplink(bool use_listener_socket);

/**
 * @brief Seconds since the last uplink datagram, or -1 if none was sent.
 */
int64_t udp_listener_seconds_since_uplink(void);

/**
 * @brief Seconds since the last inbound datagram, or -1 if none arrived.
 */
int64_t udp_listener_seconds_since_inbound(void);

/**
 * @brief Log the uplink and inbound counters with their ages.
 */
void udp_listener_log_status(void);

#endif
