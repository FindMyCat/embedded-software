#ifndef LORA_FRAME_H
#define LORA_FRAME_H

#include <stddef.h>
#include <stdint.h>
#include <zephyr/sys/util.h>


/* Mirrors homestation-esp32 modules/lora/lora_frame.h; keep the two identical.
 *
 * Wire format for the collar <-> home station LoRa link.
 *
 *   0    magic      1
 *   1    version    1
 *   2    type       1
 *   3    flags      1
 *   4    counter    4, little endian
 *   8    id length  1
 *   9    device id  n, ASCII
 *   9+n  body       per type
 *   tail MAC        8, only when LORA_FRAME_FLAG_MAC is set
 *
 */

#define LORA_FRAME_MAGIC   0xFCU
#define LORA_FRAME_VERSION 1U

/* RAK3172 P2P payload cap. */
#define LORA_FRAME_MAX_BYTES 127

/* Matches fmc_config.device_id. */
#define LORA_FRAME_MAX_DEVICE_ID 36

#define LORA_FRAME_FLAG_MAC BIT(0)

/* Test heartbeat, collar to station: this one byte and nothing else. Distinct
 * from the frame magic and the WiFi scan type byte. */
#define LORA_HEARTBEAT_BYTE 0xFDU

enum lora_frame_type {
	LORA_FRAME_TYPE_PING = 1,
	LORA_FRAME_TYPE_POSITION = 2,
	/* Periodic "still here" from the station. No body: the collar's module
	 * clears its absence timer on the CAD detection alone. */
	LORA_FRAME_TYPE_BEACON = 3,
	/* Collar to station, the moment a ping decodes. Tells the half-duplex
	 * station to stop pinging and listen. No body. */
	LORA_FRAME_TYPE_ACK = 4,
};

struct lora_frame_position {
	int32_t latitude_microdegrees;
	int32_t longitude_microdegrees;
	/* Decimetres, UINT16_MAX when unknown. */
	uint16_t accuracy_decimetres;
	uint8_t satellites;
	/* Percent, UINT8_MAX when unknown. */
	uint8_t battery_percent;
};

struct lora_frame {
	enum lora_frame_type type;
	uint32_t counter;
	char device_id[LORA_FRAME_MAX_DEVICE_ID + 1];
	/* Valid only for LORA_FRAME_TYPE_POSITION. */
	struct lora_frame_position position;
};

/* Returns bytes written, or a negative errno. */
int lora_frame_encode_position(uint8_t *out, size_t out_len,
			       const char *device_id, uint32_t counter,
			       const struct lora_frame_position *position);

int lora_frame_encode_ping(uint8_t *out, size_t out_len, const char *device_id,
			   uint32_t counter);

int lora_frame_encode_beacon(uint8_t *out, size_t out_len,
			     const char *device_id, uint32_t counter);

/* Echoes the acknowledged ping's counter, so the station knows which landed. */
int lora_frame_encode_ack(uint8_t *out, size_t out_len, const char *device_id,
			  uint32_t counter);

/* Returns 0, -EBADMSG for a non-frame, -ENOTSUP for an unknown type. */
int lora_frame_decode(const uint8_t *in, size_t in_len, struct lora_frame *out);

#endif /* LORA_FRAME_H */
