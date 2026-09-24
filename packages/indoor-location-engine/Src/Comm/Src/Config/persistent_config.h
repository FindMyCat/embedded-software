#ifndef PERSISTENT_CONFIG_H
#define PERSISTENT_CONFIG_H

#include "i2c_nrf9151.h"   /* struct fmc_config */

/* Registers the FDS handler; call before ble_init runs fds_init(). */
void persistent_config_init(void);

/* Persist config to flash. */
void persistent_config_save(const struct fmc_config *cfg);

#endif /* PERSISTENT_CONFIG_H */
