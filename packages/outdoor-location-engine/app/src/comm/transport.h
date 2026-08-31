#ifndef TRANSPORT_H
#define TRANSPORT_H

struct position_report {
	double latitude;
	double longitude;
	double accuracy_metres;
	int satellites;
};

/* Sends one position by whichever link is available, preferring LoRa while the
 * home station is in range so the modem can stay asleep. Falls back to MQTT-SN
 * over LTE-M. */
int transport_publish_position(const struct position_report *report);

#endif /* TRANSPORT_H */
