#include "nrf52_link.h"

#include <errno.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>

LOG_MODULE_REGISTER(nrf52_link, CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

/* The 52 enables TWIS from its edge ISR, which the SoftDevice can delay. */
#define RAISE_SETTLE_MS 2
#define WRITE_ATTEMPTS  3

static const struct device *i2c = DEVICE_DT_GET(NRF52_I2C_NODE);
static const struct gpio_dt_spec line =
    GPIO_DT_SPEC_GET(DT_ALIAS(nrf52_config_ready), gpios);

static struct gpio_callback line_cb;
static void (*request_cb)(void);
static atomic_t raising; /* our own edge, not a request */

static void line_isr(const struct device *port, struct gpio_callback *cb,
                     uint32_t pins)
{
    ARG_UNUSED(port);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);

    LOG_INF("DBG line edge, self=%d", (int)atomic_get(&raising));
    if (!atomic_get(&raising) && request_cb != NULL) {
        request_cb();
    }
}

int nrf52_link_init(void (*on_request)(void))
{
    int ret;

    if (!gpio_is_ready_dt(&line)) {
        return -ENODEV;
    }

    request_cb = on_request;

    ret = gpio_pin_configure_dt(&line, GPIO_INPUT | GPIO_OUTPUT_INACTIVE |
                                           GPIO_OPEN_SOURCE);
    if (ret < 0) {
        return ret;
    }

    ret = gpio_pin_interrupt_configure_dt(&line, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret < 0) {
        return ret;
    }

    gpio_init_callback(&line_cb, line_isr, BIT(line.pin));
    ret = gpio_add_callback_dt(&line, &line_cb);
    if (ret < 0) {
        return ret;
    }

    LOG_INF("DBG line at init=%d", gpio_pin_get_dt(&line));
    if (nrf52_link_requested()) {
        on_request();
    }
    return 0;
}

bool nrf52_link_requested(void)
{
    return gpio_pin_get_dt(&line) > 0;
}

bool nrf52_session_begin(void)
{
    /* Already high means the 52 is listening; otherwise wake it. */
    if (nrf52_link_requested()) {
        LOG_INF("DBG session: line already high");
        return false;
    }

    atomic_set(&raising, 1);
    gpio_pin_set_dt(&line, 1);
    k_msleep(RAISE_SETTLE_MS);
    return true;
}

void nrf52_session_end(bool raised)
{
    if (!raised) {
        return;
    }

    gpio_pin_set_dt(&line, 0);
    k_busy_wait(10); /* let the 52's pull-down bring the line down */
    atomic_set(&raising, 0);

    /* The 52 may have raised while we held the line, hiding its edge. */
    if (nrf52_link_requested() && request_cb != NULL) {
        request_cb();
    }
}

int nrf52_probe(void)
{
    if (!device_is_ready(i2c)) {
        return -ENODEV;
    }

    uint8_t ver;
    return i2c_reg_read_byte(i2c, NRF52_I2C_ADDR, NRF52_REG_CONFIG_VERSION, &ver);
}

int nrf52_read_config(struct fmc_config *out)
{
    if (!device_is_ready(i2c)) {
        LOG_ERR("Error: I2C bus not ready");
        return -ENODEV;
    }

    uint8_t ver = 0;
    uint8_t ver_after = 0;
    int ret;

    /* seqlock: read version, blob, re-read version; retry if it moved mid-read */
    for (int attempt = 0; attempt < 3; attempt++) {
        ret = i2c_reg_read_byte(i2c, NRF52_I2C_ADDR, NRF52_REG_CONFIG_VERSION, &ver);
        if (ret < 0) {
            LOG_ERR("Error: read config version (%d)", ret);
            return ret;
        }

        if (ver == 0) {
            break;
        }

        ret = i2c_burst_read(i2c, NRF52_I2C_ADDR, NRF52_REG_CONFIG,
                             (uint8_t *)out, sizeof(*out));
        if (ret < 0) {
            LOG_ERR("Error: read config blob (%d)", ret);
            return ret;
        }

        ret = i2c_reg_read_byte(i2c, NRF52_I2C_ADDR, NRF52_REG_CONFIG_VERSION,
                                &ver_after);
        if (ret < 0) {
            LOG_ERR("Error: re-read config version (%d)", ret);
            return ret;
        }

        if (ver == ver_after) {
            break;
        }
    }

    /* ack the version we ingested; the 52 releases the line on it */
    ret = i2c_reg_write_byte(i2c, NRF52_I2C_ADDR, NRF52_REG_CONFIG_APPLIED, ver);
    if (ret < 0) {
        LOG_ERR("Error: write config_applied (%d)", ret);
        return ret;
    }

    /* 0: the 52 has no config yet, so the blob is zeros */
    if (ver == 0) {
        return -ENODATA;
    }

    /* keep string fields safe to print/use even if the blob is junk */
    out->host[sizeof(out->host) - 1] = '\0';
    out->device_id[sizeof(out->device_id) - 1] = '\0';

    LOG_INF("config v%u: host=%s port=%u id=%s", ver, out->host, out->port,
            out->device_id);
    return 0;
}

int nrf52_write_batt(const struct fmc_batt_frame *frame)
{
    if (!device_is_ready(i2c)) {
        LOG_ERR("Error: I2C bus not ready");
        return -ENODEV;
    }

    uint8_t buf[1 + sizeof(*frame)];
    buf[0] = NRF52_REG_BATT;
    memcpy(&buf[1], frame, sizeof(*frame));

    bool raised = nrf52_session_begin();

    int ret = -EIO;
    for (int attempt = 0; attempt < WRITE_ATTEMPTS; attempt++) {
        ret = i2c_write(i2c, buf, sizeof(buf), NRF52_I2C_ADDR);
        if (ret == 0) {
            break;
        }
        k_msleep(RAISE_SETTLE_MS);
    }

    nrf52_session_end(raised);

    if (ret < 0) {
        LOG_ERR("Error: batt frame write to 0x%02x failed (%d)", NRF52_I2C_ADDR, ret);
    }
    return ret;
}