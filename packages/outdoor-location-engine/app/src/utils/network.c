#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(network_utils, 4);

char* getIpAddressFromHostname(char* hostname) {
    struct zsock_addrinfo *res, *ai;
    struct zsock_addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM
    };
    int ret;

    ret = zsock_getaddrinfo(hostname, NULL, &hints, &res);
    if (ret) {
        LOG_INF("Unable to resolve hostname, error: %d\n", ret);
        return NULL;
    }

    char *ip_str = NULL;
    for (ai = res; ai != NULL; ai = ai->ai_next) {
        ip_str = malloc(INET_ADDRSTRLEN);
        if(!ip_str) {
            LOG_ERR("Error: Memory allocation failed");
            break;
        }

        zsock_inet_ntop(ai->ai_family, &((struct sockaddr_in *)ai->ai_addr)->sin_addr, ip_str, INET_ADDRSTRLEN);
        LOG_INF("IP Address for hostname %s: %s\n", hostname, ip_str);
        break;
    }
    zsock_freeaddrinfo(res);

    return ip_str;
}