#include "fuel_gauge.h"

#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/sensor/npm13xx_charger.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include <nrf_fuel_gauge.h>

#define BATT_LINK_I2C_NODE DT_NODELABEL(i2c2)
#define BATT_LINK_SLAVE_ADDR 0x42

LOG_MODULE_REGISTER(fuel_gauge, 4);

/* nPM1300 CHARGER.BCHGCHARGESTATUS bitmasks */
#define NPM13XX_CHG_STATUS_COMPLETE_MASK BIT(1)
#define NPM13XX_CHG_STATUS_TRICKLE_MASK  BIT(2)
#define NPM13XX_CHG_STATUS_CC_MASK       BIT(3)
#define NPM13XX_CHG_STATUS_CV_MASK       BIT(4)

#define FUEL_GAUGE_SAMPLE_INTERVAL_MS 10000
#define FUEL_GAUGE_STACK_SIZE 1536
#define FUEL_GAUGE_THREAD_PRIO 7

static const struct device *charger = DEVICE_DT_GET(DT_NODELABEL(npm1300_charger));
static const struct device *batt_link_i2c = DEVICE_DT_GET(BATT_LINK_I2C_NODE);

static int64_t ref_time;

static const struct battery_model battery_model = {
#include "battery_model.inc"
};

static int read_sensors(float *voltage, float *current, float *temp,
                        int32_t *chg_status)
{
    struct sensor_value value;
    int ret;

    ret = sensor_sample_fetch(charger);
    if (ret < 0) {
        return ret;
    }

    sensor_channel_get(charger, SENSOR_CHAN_GAUGE_VOLTAGE, &value);
    *voltage = (float)value.val1 + ((float)value.val2 / 1000000);

    sensor_channel_get(charger, SENSOR_CHAN_GAUGE_TEMP, &value);
    *temp = (float)value.val1 + ((float)value.val2 / 1000000);

    sensor_channel_get(charger, SENSOR_CHAN_GAUGE_AVG_CURRENT, &value);
    *current = (float)value.val1 + ((float)value.val2 / 1000000);

    sensor_channel_get(charger, SENSOR_CHAN_NPM13XX_CHARGER_STATUS, &value);
    *chg_status = value.val1;

    return 0;
}

static int charge_status_inform(int32_t chg_status)
{
    union nrf_fuel_gauge_ext_state_info_data state_info;

    if (chg_status & NPM13XX_CHG_STATUS_COMPLETE_MASK) {
        state_info.charge_state = NRF_FUEL_GAUGE_CHARGE_STATE_COMPLETE;
    } else if (chg_status & NPM13XX_CHG_STATUS_TRICKLE_MASK) {
        state_info.charge_state = NRF_FUEL_GAUGE_CHARGE_STATE_TRICKLE;
    } else if (chg_status & NPM13XX_CHG_STATUS_CC_MASK) {
        state_info.charge_state = NRF_FUEL_GAUGE_CHARGE_STATE_CC;
    } else if (chg_status & NPM13XX_CHG_STATUS_CV_MASK) {
        state_info.charge_state = NRF_FUEL_GAUGE_CHARGE_STATE_CV;
    } else {
        state_info.charge_state = NRF_FUEL_GAUGE_CHARGE_STATE_IDLE;
    }

    return nrf_fuel_gauge_ext_state_update(
        NRF_FUEL_GAUGE_EXT_STATE_INFO_CHARGE_STATE_CHANGE, &state_info);
}

static int fuel_gauge_init(void)
{
    struct sensor_value value;
    struct nrf_fuel_gauge_init_parameters parameters = {
        .model = &battery_model,
        .opt_params = NULL,
        .state = NULL,
    };
    float max_charge_current;
    float term_charge_current;
    int32_t chg_status;
    int ret;

    LOG_INF("nRF Fuel Gauge version: %s", nrf_fuel_gauge_version);

    ret = read_sensors(&parameters.v0, &parameters.i0, &parameters.t0, &chg_status);
    if (ret < 0) {
        LOG_ERR("Error: read_sensors failed (%d)", ret);
        return ret;
    }
    /* Zephyr convention: negative current = discharging.
     * nrf_fuel_gauge expects negative = charging so we have to flip the sign.
     */
    parameters.i0 = -parameters.i0;

    sensor_channel_get(charger, SENSOR_CHAN_GAUGE_DESIRED_CHARGING_CURRENT, &value);
    max_charge_current = (float)value.val1 + ((float)value.val2 / 1000000);
    term_charge_current = max_charge_current / 10.f;

    ret = nrf_fuel_gauge_init(&parameters, NULL);
    if (ret < 0) {
        LOG_ERR("Error: nrf_fuel_gauge_init failed (%d)", ret);
        return ret;
    }

    ret = nrf_fuel_gauge_ext_state_update(
        NRF_FUEL_GAUGE_EXT_STATE_INFO_CHARGE_CURRENT_LIMIT,
        &(union nrf_fuel_gauge_ext_state_info_data){
            .charge_current_limit = max_charge_current});
    if (ret < 0) {
        return ret;
    }

    ret = nrf_fuel_gauge_ext_state_update(
        NRF_FUEL_GAUGE_EXT_STATE_INFO_TERM_CURRENT,
        &(union nrf_fuel_gauge_ext_state_info_data){
            .charge_term_current = term_charge_current});
    if (ret < 0) {
        return ret;
    }

    ret = charge_status_inform(chg_status);
    if (ret < 0) {
        return ret;
    }

    ref_time = k_uptime_get();
    return 0;
}

static int fuel_gauge_sample(void)
{
    static int32_t chg_status_prev;

    float voltage, current, temp, soc, tte, ttf, delta;
    int32_t chg_status;
    int ret;

    ret = read_sensors(&voltage, &current, &temp, &chg_status);
    if (ret < 0) {
        LOG_ERR("Error: read_sensors failed (%d)", ret);
        return ret;
    }

    if (chg_status != chg_status_prev) {
        chg_status_prev = chg_status;
        (void)charge_status_inform(chg_status);
    }

    delta = (float)k_uptime_delta(&ref_time) / 1000.f;

    soc = nrf_fuel_gauge_process(voltage, -current, temp, delta, NULL);
    tte = nrf_fuel_gauge_tte_get();
    ttf = nrf_fuel_gauge_ttf_get();

    LOG_INF("V: %.3f V, I: %.3f A, T: %.2f C, SoC: %.2f %%, TTE: %.0f, TTF: %.0f",
            (double)voltage, (double)current, (double)temp,
            (double)soc, (double)tte, (double)ttf);

    struct __attribute__((packed)) batt_snapshot {
        float   voltage;
        float   current;
        float   temp;
        float   soc;
        float   tte;
        float   ttf;
        int32_t chg_status;
    } snap = { voltage, current, temp, soc, tte, ttf, chg_status };

    int i2c_ret = i2c_write(batt_link_i2c, (const uint8_t *)&snap, sizeof(snap),
                            BATT_LINK_SLAVE_ADDR);
    if (i2c_ret < 0) {
        LOG_ERR("Error: i2c_write to 0x%02x failed (%d)", BATT_LINK_SLAVE_ADDR, i2c_ret);
    }

    return 0;
}

static void fuel_gauge_thread(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    while (true) {
        fuel_gauge_sample();
        k_sleep(K_MSEC(FUEL_GAUGE_SAMPLE_INTERVAL_MS));
    }
}

K_THREAD_STACK_DEFINE(fuel_gauge_stack, FUEL_GAUGE_STACK_SIZE);
static struct k_thread fuel_gauge_tdata;
static bool started;

int fuel_gauge_start(void)
{
    int ret;

    if (started) {
        return 0;
    }

    if (!device_is_ready(charger)) {
        LOG_ERR("Error: nPM1300 charger device not ready");
        return -ENODEV;
    }

    ret = fuel_gauge_init();
    if (ret < 0) {
        return ret;
    }

    k_thread_create(&fuel_gauge_tdata, fuel_gauge_stack,
                    K_THREAD_STACK_SIZEOF(fuel_gauge_stack),
                    fuel_gauge_thread, NULL, NULL, NULL,
                    FUEL_GAUGE_THREAD_PRIO, 0, K_NO_WAIT);
    k_thread_name_set(&fuel_gauge_tdata, "fuel_gauge");

    started = true;
    LOG_INF("nPM1300 fuel gauge started");
    return 0;
}
