#include <modem/location.h>
#ifndef GPS_DOT_H    /* This is an "include guard" */
#define GPS_DOT_H    /* prevents the file from being included twice. */
                     /* Including a header file twice causes all kinds */
                     /* of interesting problems.*/

#include <stdbool.h>
#include <stdint.h>

#include "../comm/transport.h"

struct location_result {
	bool has_fix;
	/** "fix", "timeout", "error", "rejected", "unknown" or "none". */
	const char *reason;
	enum location_method method;
	uint8_t num_satellites;
	uint32_t elapsed_ms;
};

const struct location_result *location_last_result(void);

int location_ensure_init(void);

int location_publish(double latitude, double longitude, int satellites, double accuracy);
int location_publish_spoofed(enum transport_path path);
void location_default_get(void);
void location_gnss_low_accuracy_get(void);
void location_gnss_high_accuracy_get(void);
void location_gnss_periodic_get(int period);
void location_event_handler(const struct location_event_data *);

#endif
