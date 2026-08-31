#ifndef FUEL_GAUGE_H
#define FUEL_GAUGE_H

/**
 * Initialise the on-board nPM1300 fuel gauge  and start a
 * background thread that periodically reads V/I/T and prints SoC.
 *
 * @return 0 on success, negative errno otherwise.
 */
int fuel_gauge_start(void);

/**
 * Last state of charge in percent, or -EAGAIN if never sampled.
 */
int fuel_gauge_get_state_of_charge(float *state_of_charge);

/**
 * Send one frame of dummy battery values over the nRF52 I2C link (0x42),
 * without needing the nPM1300. For bench-testing the i2c link on the DK.
 *
 * @return 0 on success, negative errno otherwise.
 */
int fuel_gauge_send_test_frame(void);

#endif /* FUEL_GAUGE_H */
