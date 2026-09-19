#include "diagnostics.h"

#include <errno.h>
#include <zephyr/device.h>
#include <zephyr/drivers/led.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include <stdlib.h>

LOG_MODULE_REGISTER(fuel_gauge_diag,
                    CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

static void report_charge_status(int32_t status) {
  LOG_INF("  charge status 0x%02x:%s%s%s%s%s%s", (unsigned)status,
          (status & CHG_STATUS_BATTDETECTED) ? " battery-detected" : "",
          (status & CHG_STATUS_COMPLETE) ? " complete" : "",
          (status & CHG_STATUS_TRICKLE) ? " trickle" : "",
          (status & CHG_STATUS_CC) ? " constant-current" : "",
          (status & CHG_STATUS_CV) ? " constant-voltage" : "",
          (status & CHG_STATUS_RECHARGE) ? " recharge" : "");

  if ((status & CHG_STATUS_BATTDETECTED) == 0) {
    LOG_WRN("  no battery detected: check the cell and its NTC");
  }
}

static void report_charge_input(void) {
  struct sensor_value present, cur_limit;
  int ret;

  ret = sensor_attr_get(charger, SENSOR_CHAN_NPM13XX_CHARGER_VBUS_STATUS,
                        SENSOR_ATTR_NPM13XX_CHARGER_VBUS_PRESENT, &present);
  if (ret < 0) {
    LOG_ERR("  FAIL USB power status read (%d)", ret);
    return;
  }

  (void)sensor_attr_get(charger, SENSOR_CHAN_NPM13XX_CHARGER_VBUS_STATUS,
                        SENSOR_ATTR_NPM13XX_CHARGER_VBUS_CUR_LIMIT, &cur_limit);

  LOG_INF("  USB power %s, input current limit %s",
          present.val1 ? "connected, charging" : "not connected, on battery",
          cur_limit.val1 ? "reached" : "not reached");
}

int fuel_gauge_diagnostics(void) {
  struct sensor_value value;
  float voltage, current, temp;
  int ret;

  LOG_INF("nPM1300:");

  if (!device_is_ready(charger)) {
    LOG_ERR("  FAIL charger device not ready; the PMIC never answered on I2C");
    return -ENODEV;
  }

  ret = sensor_sample_fetch(charger);
  if (ret < 0) {
    LOG_ERR(
        "  FAIL sample fetch (%d); check SDA/SCL and that the PMIC is powered",
        ret);
    return ret;
  }
  LOG_INF("  PASS PMIC answers on I2C");

  sensor_channel_get(charger, SENSOR_CHAN_GAUGE_VOLTAGE, &value);
  voltage = (float)value.val1 + ((float)value.val2 / 1000000);
  sensor_channel_get(charger, SENSOR_CHAN_GAUGE_AVG_CURRENT, &value);
  current = (float)value.val1 + ((float)value.val2 / 1000000);
  sensor_channel_get(charger, SENSOR_CHAN_GAUGE_TEMP, &value);
  temp = (float)value.val1 + ((float)value.val2 / 1000000);

  LOG_INF("  battery %.3f V, %.3f A, NTC %.2f C", (double)voltage,
          (double)current, (double)temp);

  if (temp < TEMP_MIN_C || temp > TEMP_MAX_C) {
    LOG_WRN("  NTC outside %d..%d C: thermistor likely absent or miswired",
            (int)TEMP_MIN_C, (int)TEMP_MAX_C);
  }

  sensor_channel_get(charger, SENSOR_CHAN_NPM13XX_CHARGER_STATUS, &value);
  report_charge_status(value.val1);

  sensor_channel_get(charger, SENSOR_CHAN_NPM13XX_CHARGER_ERROR, &value);
  if (value.val1 != 0) {
    LOG_ERR("  FAIL charger error 0x%02x, decode with BCHGERRREASON in the "
            "product spec",
            (unsigned)value.val1);
  }

  sensor_channel_get(charger, SENSOR_CHAN_DIE_TEMP, &value);
  LOG_INF("  die %d.%02d C", value.val1, abs(value.val2) / 10000);

  report_charge_input();

  return 0;
}
