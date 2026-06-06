#ifndef PERSISTENT_CONFIG_H
#define PERSISTENT_CONFIG_H

#include "i2c_nrf9151.h"   /* struct fmc_config */

/* Register the FDS handler. Call before pm_init() runs fds_init() (in ble_init),
 * so this receives FDS_EVT_INIT. On init, any saved config is loaded from flash,
 * pushed to the nRF9151, and restored into the BLE read-back value. */
void persistent_config_init(void);

/* Persist the given config to flash (call on a new config from BLE). */
void persistent_config_save(const struct fmc_config *cfg);

#endif /* PERSISTENT_CONFIG_H */
