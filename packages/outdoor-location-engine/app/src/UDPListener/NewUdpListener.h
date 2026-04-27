#ifndef NEW_UDP_LISTENER_DOT_H    /* This is an "include guard" */
#define NEW_UDP_LISTENER_DOT_H    /* prevents the file from being included twice. */
                              /* Including a header file twice causes all kinds */
                              /* of interesting problems.*/

void start_udp_listener_thread();
void stop_udp_listener_thread();

int create_udp_listener();
int destroy_udp_listener();

#endif
