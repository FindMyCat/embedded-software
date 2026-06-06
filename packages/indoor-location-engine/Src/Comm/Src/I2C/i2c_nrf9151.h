#ifndef I2C_NRF9151_H
#define I2C_NRF9151_H

#include <stdint.h>
#include <stddef.h>

#define I2C_NRF9151_BUF_SIZE        256
#define I2C_NRF9151_SLAVE_ADDR      0x42

void i2c_nrf9151_init(void);

int i2c_nrf9151_read(uint8_t offset, uint8_t *dst, size_t len);
int i2c_nrf9151_write(uint8_t offset, const uint8_t *src, size_t len);

#endif
