#ifndef NRF52_LINK_H
#define NRF52_LINK_H

#include <stdint.h>
#include <zephyr/devicetree.h>

/*
 * I2C link from the nRF9151 (master) to the nRF52 / DWM indoor-location-engine.
 * Shares the i2c2 bus with the nPM1300 PMIC (0x6b); the nRF52 answers at 0x42
 * as an i2c-target-eeprom style 256-byte register buffer.
 */
#define NRF52_I2C_NODE DT_NODELABEL(i2c2)
#define NRF52_I2C_ADDR 0x42

/* nRF52 shared-memory register map (offsets into its 256-byte buffer). */
#define NRF52_REG_BATT           0x00 /* battery frame, 91 -> 52 */
#define NRF52_REG_CONFIG_VERSION 0x20 /* 52 bumps on each new config */
#define NRF52_REG_CONFIG_APPLIED 0x21 /* 91 writes back the version it ingested */
#define NRF52_REG_CONFIG         0x40 /* fmc_config blob, 52 -> 91 */

/* Config the iOS app sets over BLE on the nRF52, read here by the nRF9151.
 * Layout MUST match struct fmc_config in the nRF52 i2c_nrf9151.h. */
struct __attribute__((packed)) fmc_config {
    char     host[64];
    uint16_t port;
    char     device_id[37];
};

/**
 * Read the config block the nRF52 published, ack the version, copy into *out.
 *
 * @return 0 on success, negative errno otherwise.
 */
int nrf52_read_config(struct fmc_config *out);

#endif /* NRF52_LINK_H */
