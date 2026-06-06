#ifndef I2C_NRF9151_H
#define I2C_NRF9151_H

#include <stdint.h>
#include <stddef.h>

#define I2C_NRF9151_BUF_SIZE        256
#define I2C_NRF9151_SLAVE_ADDR      0x42

/* Register map (offsets into the shared buffer). Mirrors nrf52_link.h on the 91. */
#define I2C_NRF9151_REG_BATT            0x00 /* battery frame, 91 -> 52 */
#define I2C_NRF9151_REG_CONFIG_VERSION  0x20 /* bumped on each new config */
#define I2C_NRF9151_REG_CONFIG_APPLIED  0x21 /* 91 writes back the version it ingested */
#define I2C_NRF9151_REG_CONFIG          0x40 /* fmc_config blob, 52 -> 91 */

/* Config the iOS app sets over BLE. Layout MUST match struct fmc_config
 * in the nRF9151 comm/nrf52_link.h. */
struct __attribute__((packed)) fmc_config {
    char     host[64];
    uint16_t port;
    char     device_id[37];
};

void i2c_nrf9151_init(void);

int i2c_nrf9151_read(uint8_t offset, uint8_t *dst, size_t len);
int i2c_nrf9151_write(uint8_t offset, const uint8_t *src, size_t len);

/* Publish config to the 91: store the blob, bump the version, raise CONFIG_RDY.
 * Call this from the BLE write handler once the iOS app sends new settings. */
void i2c_nrf9151_publish_config(const struct fmc_config *cfg);

/* Publish a fixed dummy config (bench test, stands in for a BLE write). */
void i2c_nrf9151_publish_test_config(void);

#endif
