#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(network_utils, 4);

char* getIpAddressFromHostname(char* hostname) {
    struct addrinfo *res, *ai;
    struct addrinfo hints = {
        .ai_family = AF_INET, // For IPv4
        .ai_socktype = SOCK_STREAM
    };
    int ret;
    
    ret = getaddrinfo(hostname, NULL, &hints, &res);
    if (ret) {
        LOG_INF("Unable to resolve hostname, error: %d\n", ret);
        return NULL;
    }

    char *ip_str = NULL;
    for (ai = res; ai != NULL; ai = ai->ai_next) {
        ip_str = malloc(INET_ADDRSTRLEN);
        if(!ip_str) {
            LOG_ERR("Memory allocation failed!\n");
            break;
        }
        
        inet_ntop(ai->ai_family, &((struct sockaddr_in *)ai->ai_addr)->sin_addr, ip_str, INET_ADDRSTRLEN);
        LOG_INF("IP Address for hostname %s: %s\n", hostname, ip_str);
        break;
    }
    freeaddrinfo(res);
    
    return ip_str;
}