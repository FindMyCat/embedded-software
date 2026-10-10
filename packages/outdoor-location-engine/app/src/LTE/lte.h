#ifndef LTE_H
#define LTE_H

#include <zephyr/kernel.h>

int lte_connect(void);

/* Starts an attach and returns; registration is logged when it lands. */
int lte_start(void);

int lte_stop(void);

/* 0 once lte_start()'s attach registers, -EAGAIN on timeout. */
int lte_wait_registered(k_timeout_t timeout);

int lte_wait_for_time(k_timeout_t timeout);

#endif /* LTE_H */
