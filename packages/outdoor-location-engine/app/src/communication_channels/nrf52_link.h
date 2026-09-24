#ifndef NRF52_LINK_H
#define NRF52_LINK_H

#include <stdbool.h>
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
 * Set up the shared request line. Either chip drives it high to ask for a bus
 * session; neither drives it low, and the 52's pull-down holds it low when idle.
 *
 * @param on_request Called from ISR context when the 52 raises the line.
 * @return 0 on success, negative errno otherwise.
 */
int nrf52_link_init(void (*on_request)(void));

/** True while the 52 holds the request line high. */
bool nrf52_link_requested(void);

/**
 * Wake the 52's I2C slave for a transfer. Blocks a few ms if it raises.
 *
 * @return true if we raised the line; pass it to nrf52_session_end().
 */
bool nrf52_session_begin(void);

/** Release a raise from nrf52_session_begin(). */
void nrf52_session_end(bool raised);

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
 * @return 0 on success, -ENODATA if the 52 has never published one,
 *         negative errno otherwise.
 */
int nrf52_read_config(struct fmc_config *out);

/**
 * Push one battery frame to the nRF52, raising the request line first if the
 * 52 is not already holding it. Blocks for a few ms.
 *
 * @return 0 on success, negative errno otherwise.
 */
int nrf52_write_batt(const struct fmc_batt_frame *frame);

#endif /* NRF52_LINK_H */
