#ifndef NRF52_LINK_H
#define NRF52_LINK_H

#include <stdint.h>
#include <zephyr/devicetree.h>

#define NRF52_I2C_NODE DT_NODELABEL(i2c2)
#define NRF52_I2C_ADDR 0x42

#define NRF52_REG_BATT           0x00 /* battery frame, 91 -> 52 */
#define NRF52_REG_CONFIG_VERSION 0x20 /* 52 bumps on each new config */
#define NRF52_REG_CONFIG_APPLIED 0x21 /* 91 writes back the version it ingested */
#define NRF52_REG_CONFIG         0x40 /* fmc_config blob, 52 -> 91 */

struct __attribute__((packed)) fmc_config {
    char     host[64];
    uint16_t port;
    char     device_id[37];
};

/**
 * Read the config block the nRF52 published, ack the version
 *
 * @return 0 on success, negative errno otherwise.
 */
int nrf52_read_config(struct fmc_config *out);

#endif /* NRF52_LINK_H */
