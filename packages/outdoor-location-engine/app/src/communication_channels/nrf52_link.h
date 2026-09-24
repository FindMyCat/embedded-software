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

/* Wire layout at NRF52_REG_BATT; must match struct batt_frame on the 52. */
struct __attribute__((packed)) fmc_batt_frame {
    float   voltage;
    float   current;
    float   temp;
    float   soc;
    float   tte;
    float   ttf;
    int32_t chg_status;
};

/**
 * Silently check whether the nRF52 answers, i.e. whether a bus window is open.
 *
 * @return 0 if it answered, negative errno otherwise. Logs nothing, so it is
 *         safe to call in a poll loop.
 */
int nrf52_probe(void);

/**
 * Read the config block the nRF52 published, ack the version
 *
 * @return 0 on success, negative errno otherwise.
 */
int nrf52_read_config(struct fmc_config *out);

/**
 * Push one battery frame to the nRF52.
 *
 * Only valid while the 52 holds its config-ready line high: outside that
 * window its I2C slave is powered down and the write NACKs.
 *
 * @return 0 on success, negative errno otherwise.
 */
int nrf52_write_batt(const struct fmc_batt_frame *frame);

#endif /* NRF52_LINK_H */
