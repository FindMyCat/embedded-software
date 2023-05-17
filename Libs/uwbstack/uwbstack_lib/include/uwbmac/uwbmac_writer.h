/*
 * Copyright (c) 2021 Qorvo, Inc
 *
 * All rights reserved.
 *
 * NOTICE: All information contained herein is, and remains the property
 * of Qorvo, Inc. and its suppliers, if any. The intellectual and technical
 * concepts herein are proprietary to Qorvo, Inc. and its suppliers, and
 * may be covered by patents, patent applications, and are protected by
 * trade secret and/or copyright law. Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written
 * permission is obtained from Qorvo, Inc.
 *
 */

#ifndef UWBMAC_PAYLOAD_WRITE_H
#define UWBMAC_PAYLOAD_WRITE_H

#include <uwbmac/uwbmac.h>
#include <uwbmac/uwbmac_msg.h>
#include <uwbmac/uwbmac_types.h>

#define UWBMAC_WRITER_DEFINE_MSG(params, size) \
	char payload[size];                    \
	struct uwbmac_msg params;              \
	uwbmac_writer_init_msg(&params, payload, sizeof(payload))

/**
 * uwbmac_writer_init_msg() - Initialise on-stack uwbmac_msg.
 * @msg: Message being initialized.
 * @payload: Payload buffer to fill in.
 * @size: Size of the payload buffer.
 */
static inline void uwbmac_writer_init_msg(struct uwbmac_msg *msg, void *payload,
					  int size)
{
	uwbmac_msg_init(msg, payload, 0, size);
}

/**
 * uwbmac_writer_success() - Check that all 'add' operations succeeded.
 * @msg: Message being written.
 *
 * Return: true on success, false otherwise.
 */
static inline bool uwbmac_writer_success(struct uwbmac_msg *msg)
{
	return !msg->add_failed;
}

/**
 * uwbmac_writer_add_flag() - Add an empty tag to the message.
 * @msg: Message being written.
 * @tag: Tag in the payload.
 *
 * Return: UWBMAC_SUCCESS or error.
 */
int uwbmac_writer_add_flag(struct uwbmac_msg *msg, int tag);

/**
 * uwbmac_writer_add_bool() - Add a boolean to the message.
 * @msg: Message being written.
 * @tag: Tag in the payload.
 * @value: Value to add.
 *
 * Return: UWBMAC_SUCCESS or error.
 */
int uwbmac_writer_add_bool(struct uwbmac_msg *msg, int tag, bool value);

/**
 * uwbmac_writer_add_s8() - Add a signed 8-bit integer to the message.
 * @msg: Message being written.
 * @tag: Tag in the payload.
 * @value: Value to add.
 *
 * Return: UWBMAC_SUCCESS or error.
 */
int uwbmac_writer_add_s8(struct uwbmac_msg *msg, int tag, int8_t value);

/**
 * uwbmac_writer_add_s16() - Add a signed 16-bit integer to the message.
 * @msg: Message being written.
 * @tag: Tag in the payload.
 * @value: Value to add.
 *
 * Return: UWBMAC_SUCCESS or error.
 */
int uwbmac_writer_add_s16(struct uwbmac_msg *msg, int tag, int16_t value);

/**
 * uwbmac_writer_add_s32() - Add a signed 32-bit integer to the message.
 * @msg: Message being written.
 * @tag: Tag in the payload.
 * @value: Value to add.
 *
 * Return: UWBMAC_SUCCESS or error.
 */
int uwbmac_writer_add_s32(struct uwbmac_msg *msg, int tag, int32_t value);

/**
 * uwbmac_writer_add_s64() - Add a signed 64-bit integer to the message.
 * @msg: Message being written.
 * @tag: Tag in the payload.
 * @value: Value to add.
 *
 * Return: UWBMAC_SUCCESS or error.
 */
int uwbmac_writer_add_s64(struct uwbmac_msg *msg, int tag, int64_t value);

/**
 * uwbmac_writer_add_u8() - Add a unsigned 8-bit integer to the message.
 * @msg: Message being written.
 * @tag: Tag in the payload.
 * @value: Value to add.
 *
 * Return: UWBMAC_SUCCESS or error.
 */
int uwbmac_writer_add_u8(struct uwbmac_msg *msg, int tag, uint8_t value);

/**
 * uwbmac_writer_add_u16() - Add a unsigned 16-bit integer to the message.
 * @msg: Message being written.
 * @tag: Tag in the payload.
 * @value: Value to add.
 *
 * Return: UWBMAC_SUCCESS or error.
 */
int uwbmac_writer_add_u16(struct uwbmac_msg *msg, int tag, uint16_t value);

/**
 * uwbmac_writer_add_u32() - Add a unsigned 32-bit integer to the message.
 * @msg: Message being written.
 * @tag: Tag in the payload.
 * @value: Value to add.
 *
 * Return: UWBMAC_SUCCESS or error.
 */
int uwbmac_writer_add_u32(struct uwbmac_msg *msg, int tag, uint32_t value);

/**
 * uwbmac_writer_add_u64() - Add a unsigned 64-bit integer to the message.
 * @msg: Message being written.
 * @tag: Tag in the payload.
 * @value: Value to add.
 *
 * Return: UWBMAC_SUCCESS or error.
 */
int uwbmac_writer_add_u64(struct uwbmac_msg *msg, int tag, uint64_t value);

/**
 * uwbmac_writer_add_string() - Add a string to the message.
 * @msg: Message being written.
 * @tag: Tag in the payload.
 * @value: Pointer to the string to add.
 *
 * Return: UWBMAC_SUCCESS or error.
 */
int uwbmac_writer_add_string(struct uwbmac_msg *msg, int tag,
			     const char *value);

/**
 * uwbmac_writer_add_binary() - Add a binary object to the message.
 * @msg: Message being written.
 * @tag: Tag in the payload.
 * @data: Pointer to the object to add.
 * @length: Length of the object to add.
 *
 * Return: UWBMAC_SUCCESS or error.
 */
int uwbmac_writer_add_binary(struct uwbmac_msg *msg, int tag, const void *data,
			     int length);

/**
 * uwbmac_writer_start_nested() - Start adding a nested payload to the message.
 * @msg: Message being written.
 * @tag: Tag in the payload.
 * @nested: Pointer to the structure to keep internal data.
 *
 * Return: UWBMAC_SUCCESS or error.
 */
int uwbmac_writer_start_nested(struct uwbmac_msg *msg, int tag,
			       struct uwbmac_msg *nested);

/**
 * uwbmac_writer_end_nested() - Stop adding a nested payload to the message.
 * @msg: Message being written.
 * @nested: Pointer to the structure to keep internal data.
 *
 * Return: UWBMAC_SUCCESS or error.
 */
int uwbmac_writer_end_nested(struct uwbmac_msg *msg, struct uwbmac_msg *nested);

#endif // UWBMAC_PAYLOAD_WRITE_H
