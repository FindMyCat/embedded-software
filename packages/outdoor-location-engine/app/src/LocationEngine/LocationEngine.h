#include <modem/location.h>
#ifndef GPS_DOT_H    /* This is an "include guard" */
#define GPS_DOT_H    /* prevents the file from being included twice. */
                     /* Including a header file twice causes all kinds */
                     /* of interesting problems.*/



void location_default_get(void);
void location_gnss_low_accuracy_get(void);
void location_gnss_high_accuracy_get(void);
void location_gnss_periodic_get(int period);
void location_event_handler(const struct location_event_data *);

#endif
