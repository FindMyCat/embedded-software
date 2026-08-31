#ifndef LTE_H
#define LTE_H

#include <zephyr/kernel.h>

int lte_connect(void);

int lte_wait_for_time(k_timeout_t timeout);

#endif /* LTE_H */
