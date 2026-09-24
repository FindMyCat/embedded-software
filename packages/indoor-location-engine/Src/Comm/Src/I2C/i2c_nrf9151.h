#ifndef I2C_NRF9151_H
#define I2C_NRF9151_H

#include <stdint.h>
#include <stddef.h>

#define I2C_NRF9151_BUF_SIZE        256
#define I2C_NRF9151_SLAVE_ADDR      0x42

/* Offsets into the shared buffer. Mirrors nrf52_link.h on the 91. */
#define I2C_NRF9151_REG_BATT            0x00 /* battery frame, 91 -> 52 */
#define I2C_NRF9151_REG_CONFIG_VERSION  0x20 /* bumped on each new config */
#define I2C_NRF9151_REG_CONFIG_APPLIED  0x21 /* 91 writes back the version it ingested */
#define I2C_NRF9151_REG_CONFIG          0x40 /* fmc_config blob, 52 -> 91 */

/* Layout MUST match struct fmc_config in the 91's comm/nrf52_link.h. */
struct __attribute__((packed)) fmc_config {
    char     host[64];
    uint16_t port;
    char     device_id[37];
};

void i2c_nrf9151_init(void);

int i2c_nrf9151_read(uint8_t offset, uint8_t *dst, size_t len);
int i2c_nrf9151_write(uint8_t offset, const uint8_t *src, size_t len);

/* Store the blob, bump the version, raise CONFIG_RDY. */
void i2c_nrf9151_publish_config(const struct fmc_config *cfg);

/* Fixed dummy config, bench test. */
void i2c_nrf9151_publish_test_config(void);

#endif
