#include <modem/location.h>
#ifndef GPS_DOT_H    /* This is an "include guard" */
#define GPS_DOT_H    /* prevents the file from being included twice. */
                     /* Including a header file twice causes all kinds */
                     /* of interesting problems.*/

#include <stdbool.h>
#include <stdint.h>

#include "../comm/transport.h"

/**
 * @brief How the last finished location request ended.
 */
struct location_result {
	/** True only when a position was obtained. */
	bool has_fix;
	/** "fix", "timeout", "error", "rejected", "unknown" or "none". */
	const char *reason;
	/** Method that produced the outcome. */
	enum location_method method;
	/** Satellites tracked when the request ended. */
	uint8_t satellites;
	/** Time the method spent on the request. */
	uint32_t elapsed_ms;
};

/**
 * @brief Outcome of the last finished request, for status reporting.
 */
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
