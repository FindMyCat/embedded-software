#ifndef TRANSPORT_H
#define TRANSPORT_H

struct position_report {
	double latitude;
	double longitude;
	double accuracy_metres;
	int satellites;
};

enum transport_path {
	TRANSPORT_PATH_AUTO,
	TRANSPORT_PATH_LORA,
	TRANSPORT_PATH_CELLULAR,
};

/* Sends one position by whichever link is available, preferring LoRa while the
 * home station is in range so the modem can stay asleep. Falls back to MQTT-SN
 * over LTE-M. */
int transport_publish_position(const struct position_report *report);

/* As above, but over the named link only. LORA fails rather than falling back,
 * and CELLULAR skips LoRa even when the home station is in range. */
int transport_publish_position_via(const struct position_report *report,
				   enum transport_path path);

#endif /* TRANSPORT_H */
