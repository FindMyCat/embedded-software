#ifndef NRF52_LINK_H
#define NRF52_LINK_H

#include <zephyr/devicetree.h>

/*
 * I2C link from the nRF9151 (master) to the nRF52 / DWM indoor-location-engine.
 * Shares the i2c2 bus with the nPM1300 PMIC (0x6b); the nRF52 answers at 0x42
 * as an i2c-target-eeprom style 256-byte register buffer.
 */
#define NRF52_I2C_NODE DT_NODELABEL(i2c2)
#define NRF52_I2C_ADDR 0x42

/* Register offset of the battery frame in the nRF52 shared-memory map. */
#define NRF52_REG_BATT 0x00

#endif /* NRF52_LINK_H */
