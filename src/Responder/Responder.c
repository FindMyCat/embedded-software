#include "Responder.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "../mqtt/mqttsn.h"

void periodic_work_handler(struct k_timer *dummy)
{
    // do the periodic work here
    printk("Periodic work executed!\n");
    mqttsn_check_input();
    mqttsn_publish();
}

void respond(void)
{
    mqttsn_initialize();
    static struct k_timer periodic_timer;
    k_timer_init(&periodic_timer, periodic_work_handler, NULL);
    k_timer_start(&periodic_timer, K_SECONDS(5), K_SECONDS(5));
    // do other initialization here
    while (1) {
        k_sleep(K_FOREVER);
    }
}