#include "lora_frame.h"

#include <errno.h>
#include <string.h>
#include <zephyr/sys/byteorder.h>

#define HEADER_BYTES 9
#define POSITION_BYTES 12

#define OFFSET_MAGIC 0
#define OFFSET_VERSION 1
#define OFFSET_TYPE 2
#define OFFSET_FLAGS 3
#define OFFSET_COUNTER 4
#define OFFSET_ID_LENGTH 8
#define OFFSET_ID 9

/* Writes the fixed header and device id. Returns the offset the body starts
 * at, or a negative errno. */
static int encode_header(uint8_t *out, size_t out_len,
                         enum lora_frame_type type, const char *device_id,
                         uint32_t counter, size_t body_bytes) {
  if (!out || !device_id) {
    return -EINVAL;
  }

  size_t id_len = strlen(device_id);

  if (id_len == 0 || id_len > LORA_FRAME_MAX_DEVICE_ID) {
    return -EINVAL;
  }

  size_t total = HEADER_BYTES + id_len + body_bytes;

  if (total > out_len || total > LORA_FRAME_MAX_BYTES) {
    return -ENOSPC;
  }

  out[OFFSET_MAGIC] = LORA_FRAME_MAGIC;
  out[OFFSET_VERSION] = LORA_FRAME_VERSION;
  out[OFFSET_TYPE] = (uint8_t)type;
  out[OFFSET_FLAGS] = 0;
  sys_put_le32(counter, &out[OFFSET_COUNTER]);
  out[OFFSET_ID_LENGTH] = (uint8_t)id_len;
  memcpy(&out[OFFSET_ID], device_id, id_len);

  return (int)(OFFSET_ID + id_len);
}

int lora_frame_encode_ping(uint8_t *out, size_t out_len, const char *device_id,
                           uint32_t counter) {
  int body_offset =
      encode_header(out, out_len, LORA_FRAME_TYPE_PING, device_id, counter, 0);

  if (body_offset < 0) {
    return body_offset;
  }

  return body_offset;
}

int lora_frame_encode_beacon(uint8_t *out, size_t out_len,
                             const char *device_id, uint32_t counter) {
  return encode_header(out, out_len, LORA_FRAME_TYPE_BEACON, device_id, counter,
                       0);
}

int lora_frame_encode_ack(uint8_t *out, size_t out_len, const char *device_id,
                          uint32_t counter) {
  return encode_header(out, out_len, LORA_FRAME_TYPE_ACK, device_id, counter,
                       0);
}

int lora_frame_encode_position(uint8_t *out, size_t out_len,
                               const char *device_id, uint32_t counter,
                               const struct lora_frame_position *position) {
  if (!position) {
    return -EINVAL;
  }

  int body_offset = encode_header(out, out_len, LORA_FRAME_TYPE_POSITION,
                                  device_id, counter, POSITION_BYTES);

  if (body_offset < 0) {
    return body_offset;
  }

  uint8_t *body = &out[body_offset];

  sys_put_le32((uint32_t)position->latitude_microdegrees, &body[0]);
  sys_put_le32((uint32_t)position->longitude_microdegrees, &body[4]);
  sys_put_le16(position->accuracy_decimetres, &body[8]);
  body[10] = position->satellites;
  body[11] = position->battery_percent;

  return body_offset + POSITION_BYTES;
}

int lora_frame_decode(const uint8_t *in, size_t in_len,
                      struct lora_frame *out) {
  if (!in || !out) {
    return -EINVAL;
  }

  if (in_len < HEADER_BYTES || in[OFFSET_MAGIC] != LORA_FRAME_MAGIC ||
      in[OFFSET_VERSION] != LORA_FRAME_VERSION) {
    return -EBADMSG;
  }

  size_t id_len = in[OFFSET_ID_LENGTH];

  if (id_len == 0 || id_len > LORA_FRAME_MAX_DEVICE_ID ||
      in_len < HEADER_BYTES + id_len) {
    return -EBADMSG;
  }

  memset(out, 0, sizeof(*out));
  out->counter = sys_get_le32(&in[OFFSET_COUNTER]);
  memcpy(out->device_id, &in[OFFSET_ID], id_len);

  const uint8_t *body = &in[OFFSET_ID + id_len];
  size_t body_len = in_len - HEADER_BYTES - id_len;

  switch (in[OFFSET_TYPE]) {
  case LORA_FRAME_TYPE_PING:
    out->type = LORA_FRAME_TYPE_PING;
    return 0;
  case LORA_FRAME_TYPE_BEACON:
    out->type = LORA_FRAME_TYPE_BEACON;
    return 0;
  case LORA_FRAME_TYPE_ACK:
    out->type = LORA_FRAME_TYPE_ACK;
    return 0;
  case LORA_FRAME_TYPE_POSITION:
    if (body_len < POSITION_BYTES) {
      return -EBADMSG;
    }
    out->type = LORA_FRAME_TYPE_POSITION;
    out->position.latitude_microdegrees = (int32_t)sys_get_le32(&body[0]);
    out->position.longitude_microdegrees = (int32_t)sys_get_le32(&body[4]);
    out->position.accuracy_decimetres = sys_get_le16(&body[8]);
    out->position.satellites = body[10];
    out->position.battery_percent = body[11];
    return 0;
  default:
    return -ENOTSUP;
  }
}
