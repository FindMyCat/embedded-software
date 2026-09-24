#include "nrf52_link.h"

#include <errno.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(nrf52_link, CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

static const struct device *i2c = DEVICE_DT_GET(NRF52_I2C_NODE);

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

    /* keep string fields safe to print/use even if the blob is junk */
    out->host[sizeof(out->host) - 1] = '\0';
    out->device_id[sizeof(out->device_id) - 1] = '\0';

    /* ack the version we ingested */
    ret = i2c_reg_write_byte(i2c, NRF52_I2C_ADDR, NRF52_REG_CONFIG_APPLIED, ver);
    if (ret < 0) {
        LOG_ERR("Error: write config_applied (%d)", ret);
        return ret;
    }

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

    int ret = i2c_write(i2c, buf, sizeof(buf), NRF52_I2C_ADDR);
    if (ret < 0) {
        LOG_ERR("Error: batt frame write to 0x%02x failed (%d)", NRF52_I2C_ADDR, ret);
    }
    return ret;
}
