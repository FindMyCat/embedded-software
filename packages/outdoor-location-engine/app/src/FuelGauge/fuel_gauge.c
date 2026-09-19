#include "fuel_gauge.h"

#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include <nrf_fuel_gauge.h>

#include "../comm/nrf52_link.h"

LOG_MODULE_REGISTER(fuel_gauge,
                    CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

static int64_t ref_time;
static float last_state_of_charge = -1.f;

static struct fmc_batt_frame last_frame;
static bool started;

static const struct battery_model battery_model = {
#include "battery_model.inc"
};

int fuel_gauge_read_sensors(float *voltage, float *current, float *temp,
                            int32_t *chg_status) {
  struct sensor_value value;
  int ret;

  ret = sensor_sample_fetch(npm1300__charger);
  if (ret < 0) {
    return ret;
  }

  sensor_channel_get(npm1300__charger, SENSOR_CHAN_GAUGE_VOLTAGE, &value);
  *voltage = (float)value.val1 + ((float)value.val2 / 1000000);

  sensor_channel_get(npm1300__charger, SENSOR_CHAN_GAUGE_TEMP, &value);
  *temp = (float)value.val1 + ((float)value.val2 / 1000000);

  sensor_channel_get(npm1300__charger, SENSOR_CHAN_GAUGE_AVG_CURRENT, &value);
  *current = (float)value.val1 + ((float)value.val2 / 1000000);

  sensor_channel_get(npm1300__charger, SENSOR_CHAN_NPM13XX_CHARGER_STATUS,
                     &value);
  *chg_status = value.val1;

  return 0;
}

static int charge_status_inform(int32_t chg_status) {
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

static int fuel_gauge_init(void) {
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

  ret = fuel_gauge_read_sensors(&parameters.v0, &parameters.i0, &parameters.t0,
                                &chg_status);
  if (ret < 0) {
    LOG_ERR("Error: fuel_gauge_read_sensors failed (%d)", ret);
    return ret;
  }

  parameters.i0 = -parameters.i0;

  sensor_channel_get(npm1300__charger,
                     SENSOR_CHAN_GAUGE_DESIRED_CHARGING_CURRENT, &value);
  max_charge_current = (float)value.val1 + ((float)value.val2 / 1000000);
  term_charge_current = max_charge_current / 10.f;

  ret = nrf_fuel_gauge_init(&parameters, NULL);
  if (ret < 0) {
    LOG_ERR("Error: nrf_fuel_gauge_init failed (%d)", ret);
    return ret;
  }

  ret = nrf_fuel_gauge_ext_state_update(
      NRF_FUEL_GAUGE_EXT_STATE_INFO_CHARGE_CURRENT_LIMIT,
      &(union nrf_fuel_gauge_ext_state_info_data){.charge_current_limit =
                                                      max_charge_current});
  if (ret < 0) {
    return ret;
  }

  ret = nrf_fuel_gauge_ext_state_update(
      NRF_FUEL_GAUGE_EXT_STATE_INFO_TERM_CURRENT,
      &(union nrf_fuel_gauge_ext_state_info_data){.charge_term_current =
                                                      term_charge_current});
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

static int fuel_gauge_sample(void) {
  static int32_t chg_status_prev;

  float voltage, current, temp, soc, tte, ttf, delta;
  int32_t chg_status;
  int ret;

  ret = fuel_gauge_read_sensors(&voltage, &current, &temp, &chg_status);
  if (ret < 0) {
    LOG_ERR("Error: fuel_gauge_read_sensors failed (%d)", ret);
    return ret;
  }

  if (chg_status != chg_status_prev) {
    chg_status_prev = chg_status;
    (void)charge_status_inform(chg_status);
  }

  delta = (float)k_uptime_delta(&ref_time) / 1000.f;

  soc = nrf_fuel_gauge_process(voltage, -current, temp, delta, NULL);
  last_state_of_charge = soc;
  tte = nrf_fuel_gauge_tte_get();
  ttf = nrf_fuel_gauge_ttf_get();

  LOG_INF("V: %.3f V, I: %.3f A, T: %.2f C, SoC: %.2f %%, TTE: %.0f, TTF: %.0f",
          (double)voltage, (double)current, (double)temp, (double)soc,
          (double)tte, (double)ttf);

  last_frame = (struct fmc_batt_frame){voltage, current, temp,      soc,
                                       tte,     ttf,     chg_status};
  return 0;
}

int fuel_gauge_sample_and_push(void) {
  if (!started) {
    return -EAGAIN;
  }

  int ret = fuel_gauge_sample();
  if (ret < 0) {
    return ret;
  }
  return nrf52_write_batt(&last_frame);
}

int fuel_gauge_start(void) {
  int ret;

  if (started) {
    return 0;
  }

  if (!device_is_ready(npm1300__charger)) {
    LOG_ERR("Error: nPM1300 charger device not ready");
    return -ENODEV;
  }

  ret = fuel_gauge_init();
  if (ret < 0) {
    return ret;
  }

  /* Seed the gauge so SoC is available before the first bus window. */
  (void)fuel_gauge_sample();

  started = true;
  LOG_INF("nPM1300 fuel gauge started");
  return 0;
}

int fuel_gauge_get_state_of_charge(float *state_of_charge) {
  if (last_state_of_charge < 0.f) {
    return -EAGAIN;
  }

  *state_of_charge = last_state_of_charge;
  return 0;
}

int fuel_gauge_send_test_frame(void) {
  static const struct fmc_batt_frame snap = {3.70f,   0.10f, 25.0f, 88.5f,
                                             3600.0f, 0.0f,  1};

  int ret = nrf52_write_batt(&snap);
  if (ret < 0) {
    return ret;
  }

  LOG_INF("Sent dummy batt frame to 0x%02x (%u bytes)", NRF52_I2C_ADDR,
          (unsigned)sizeof(snap));
  return 0;
}
