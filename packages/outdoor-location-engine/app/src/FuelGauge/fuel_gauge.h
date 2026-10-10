#ifndef FUEL_GAUGE_H
#define FUEL_GAUGE_H

#include <zephyr/drivers/sensor/npm13xx_charger.h>
static const struct device *npm1300__charger = DEVICE_DT_GET(DT_NODELABEL(npm1300_charger));

/* nPM1300 CHARGER.BCHGCHARGESTATUS bitmasks */
#define NPM13XX_CHG_STATUS_BATTDETECTED_MASK BIT(0)
#define NPM13XX_CHG_STATUS_COMPLETE_MASK     BIT(1)
#define NPM13XX_CHG_STATUS_TRICKLE_MASK      BIT(2)
#define NPM13XX_CHG_STATUS_CC_MASK           BIT(3)
#define NPM13XX_CHG_STATUS_CV_MASK           BIT(4)
#define NPM13XX_CHG_STATUS_RECHARGE_MASK     BIT(5)

int fuel_gauge_read_sensors(float *voltage, float *current, float *temp,
                            int32_t *chg_status);

int fuel_gauge_start(void);
int fuel_gauge_push(void);
int fuel_gauge_get_state_of_charge(float *state_of_charge);
/* Whole percent for the wire, UINT8_MAX before the first sample. */
uint8_t fuel_gauge_battery_percent(void);
int fuel_gauge_send_test_frame(void);

#endif /* FUEL_GAUGE_H */
