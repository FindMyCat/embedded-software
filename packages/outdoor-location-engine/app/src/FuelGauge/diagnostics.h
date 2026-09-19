#ifndef FUEL_GAUGE_DIAGNOSTICS_H
#define FUEL_GAUGE_DIAGNOSTICS_H

#include <zephyr/drivers/sensor/npm13xx_charger.h>

#define CHG_STATUS_BATTDETECTED BIT(0)
#define CHG_STATUS_COMPLETE BIT(1)
#define CHG_STATUS_TRICKLE BIT(2)
#define CHG_STATUS_CC BIT(3)
#define CHG_STATUS_CV BIT(4)
#define CHG_STATUS_RECHARGE BIT(5)

#define TEMP_MIN_C 0.f
#define TEMP_MAX_C 60.f

static const struct device *charger =
    DEVICE_DT_GET(DT_NODELABEL(npm1300_charger));
int fuel_gauge_diagnostics(void);

#endif /* FUEL_GAUGE_DIAGNOSTICS_H */
