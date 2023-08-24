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

/*
  The way this works is as follows:
  1. Create an array of struct uwbmac_parser_element as large as the
     maximum tag value. For example, if your tags are in an enum which has 5
     entries, 0 to 4, that would be 5.
  2. Fully setup your array using the uwbmac_parser_add_* functions. If
     your array is filled with zeros, only setup the tags you want.
  3. Parse your payload: your data will be automatically filled in.
  */

#ifndef UWBMAC_PAYLOAD_READ_H
#define UWBMAC_PAYLOAD_READ_H

#include <uwbmac/uwbmac.h>
#include <uwbmac/uwbmac_msg.h>
#include <uwbmac/uwbmac_types.h>

/**
 * enum uwbmac_payload_type - UWB MAC serializable types.
 * @UWBMAC_PAYLOAD_TYPE_NONE: No data to recover.
 * @UWBMAC_PAYLOAD_TYPE_FLAG: Flag - no data.
 * @UWBMAC_PAYLOAD_TYPE_BOOL: Boolean.
 * @UWBMAC_PAYLOAD_TYPE_S8: 8 bit signed integer.
 * @UWBMAC_PAYLOAD_TYPE_S16: 16 bit signed integer.
 * @UWBMAC_PAYLOAD_TYPE_S32: 32 bit signed integer.
 * @UWBMAC_PAYLOAD_TYPE_S64: 64 bit signed integer.
 * @UWBMAC_PAYLOAD_TYPE_U8: 8 bit unsigned integer.
 * @UWBMAC_PAYLOAD_TYPE_U16: 16 bit unsigned integer.
 * @UWBMAC_PAYLOAD_TYPE_U32: 32 bit unsigned integer.
 * @UWBMAC_PAYLOAD_TYPE_U64: 64 bit unsigned integer.
 * @UWBMAC_PAYLOAD_TYPE_STRING: NULL terminated character string.
 * @UWBMAC_PAYLOAD_TYPE_BINARY: Binary object.
 * @UWBMAC_PAYLOAD_TYPE_NESTED: Nested payload.
 */
enum uwbmac_payload_type {
	UWBMAC_PAYLOAD_TYPE_NONE,
	UWBMAC_PAYLOAD_TYPE_FLAG,
	UWBMAC_PAYLOAD_TYPE_BOOL,
	UWBMAC_PAYLOAD_TYPE_S8,
	UWBMAC_PAYLOAD_TYPE_S16,
	UWBMAC_PAYLOAD_TYPE_S32,
	UWBMAC_PAYLOAD_TYPE_S64,
	UWBMAC_PAYLOAD_TYPE_U8,
	UWBMAC_PAYLOAD_TYPE_U16,
	UWBMAC_PAYLOAD_TYPE_U32,
	UWBMAC_PAYLOAD_TYPE_U64,
	UWBMAC_PAYLOAD_TYPE_STRING,
	UWBMAC_PAYLOAD_TYPE_BINARY,
	UWBMAC_PAYLOAD_TYPE_NESTED,
};

/* Flags for the element */
/** Tag is mandatory */
static const int UWBMAC_PAYLOAD_FLAG_IS_MANDATORY = (1 << 0);
/** Tag is present */
static const int UWBMAC_PAYLOAD_FLAG_IS_PRESENT = (1 << 1);

/**
 * struct uwbmac_parser_element - Helper to manipulate each UWB MAC elements.
 */
struct uwbmac_parser_element {
	/**
	 * @data: Pointer to data
	 */
	void *data;
	/**
	 * @length: Data length max
	*/
	uint16_t length;
	/**
	 * @rlength: Data length found
	 */
	int *rlength;
	/**
	 * @type: Expected data type
	 */
	uint8_t type;
	/**
	 * @flags: Tag mandatory/present
	 */
	uint8_t flags;
};

/**
 * uwbmac_parser_init_msg() - Initialise on-stack uwbmac_msg.
 * @msg: Message being initialized.
 * @payload: Payload to parse.
 * @length: Length of the payload to parse.
 */
static inline void uwbmac_parser_init_msg(struct uwbmac_msg *msg, void *payload,
					  int length)
{
	uwbmac_msg_init(msg, payload, length, 0);
}

/**
 * uwbmac_parser_read() - Read and parse payload.
 * @msg: Message to parse.
 * @elements: Array of elements.
 * @tag_max: Maximum tag value (number of elements minus one).
 *
 * Return: UWBMAC_SUCCESS or error.
 */
int uwbmac_parser_read(struct uwbmac_msg *msg,
		       struct uwbmac_parser_element elements[], int tag_max);

/* This is internal, not documented data. DO NOT TOUCH */
struct uwbmac_parser_read_array_info {
	uintptr_t offset;
	void *pos;
};

/**
 * uwbmac_parser_read_array() - Read and parse array payload.
 * @msg: Message to parse.
 * @elements: Array of elements.
 * @tag_max: Maximum tag value (number of elements minus one).
 * @entry: Array entry.
 * @n: Number of array entries found so far.
 * @info: Internal loop data.
 * @keep_going: Whether there are elements left in the array.
 *
 * Return: UWBMAC_SUCCESS or error.
 */
int uwbmac_parser_read_array(struct uwbmac_msg *msg,
			     struct uwbmac_parser_element elements[],
			     int tag_max, void *entry, int n,
			     struct uwbmac_parser_read_array_info *info,
			     bool *keep_going);

/**
 * uwbmac_parser_init_nested_loop() - Init internal loop data for nested iteration
 * @msg: Message to parse.
 * @info: Internal loop data.
 *
 * This call is once before looping on each element with uwbmac_parser_next_nested_loop_element.
 *
 * Return: UWBMAC_SUCCESS or error.
 */
int uwbmac_parser_init_nested_loop(struct uwbmac_msg *msg,
				   struct uwbmac_parser_read_array_info *info);

/**
 * uwbmac_parser_next_nested_loop_element() - Init nested with the next nested element
 * @msg: Message to parse.
 * @nested: Message to init.
 * @info: Internal loop data.
 * @keep_going: Whether there are elements left in the array.
 *
 * If keep_going is false, you've reache the end of the nested elements and
 * nested was not initialized.
 *
 * If keep_going is true, nested is initialized to point to the next element.
 * You will need to setup your parsing and call uwbmac_parser_read.
 *
 * Return: UWBMAC_SUCCESS or error.
 */
int uwbmac_parser_next_nested_loop_element(
	struct uwbmac_msg *msg, struct uwbmac_msg *nested,
	struct uwbmac_parser_read_array_info *info, bool *keep_going);

/**
 * uwbmac_parser_is_present() - Get tag presence status.
 * @elements: Array of elements.
 * @tag: Tag in the payload.
 *
 * Return: true if present, false otherwise.
 */
static inline bool
uwbmac_parser_is_present(struct uwbmac_parser_element elements[], int tag)
{
	return (elements[tag].flags & UWBMAC_PAYLOAD_FLAG_IS_PRESENT) != 0;
}

/**
 * uwbmac_parser_add() - Set elements.
 * @elements: Array of elements.
 * @tag: Tag in the payload.
 * @type: Expected tag type.
 * @data: Some pointer.
 * @rlength: Actual payload's length.
 * @length: Maximum expected length in the payload.
 * @mandatory: Whether the element is mandatory in the message.
 *
 * NOTE: do not call directly, use one of the provided helpers.
 */
static inline void uwbmac_parser_add(struct uwbmac_parser_element elements[],
				     int tag, enum uwbmac_payload_type type,
				     void *data, int *rlength, int length,
				     bool mandatory)
{
	elements[tag].data = data;
	elements[tag].length = length;
	elements[tag].rlength = rlength;
	elements[tag].type = type;
	elements[tag].flags = mandatory ? UWBMAC_PAYLOAD_FLAG_IS_MANDATORY : 0;
}

/**
 * uwbmac_parser_add_none() - Set element to receive nothing.
 * @elements: Array of elements.
 * @tag: Tag in the payload.
 */
static inline void
uwbmac_parser_add_none(struct uwbmac_parser_element elements[], int tag)
{
	uwbmac_parser_add(elements, tag, UWBMAC_PAYLOAD_TYPE_NONE, NULL, NULL,
			  0, false);
}

/**
 * uwbmac_parser_add_flag() - Set element to receive an empty tag.
 * @elements: Array of elements.
 * @tag: Tag in the payload.
 * @data: Pointer to the boolean to set if present.
 * @mandatory: Whether the element is mandatory in the message.
 */
static inline void
uwbmac_parser_add_flag(struct uwbmac_parser_element elements[], int tag,
		       bool *data, bool mandatory)
{
	*data = false;
	uwbmac_parser_add(elements, tag, UWBMAC_PAYLOAD_TYPE_FLAG, data, NULL,
			  sizeof(*data), mandatory);
}

/**
 * uwbmac_parser_add_bool() - Set element to receive a boolean.
 * @elements: Array of elements.
 * @tag: Tag in the payload.
 * @data: Pointer to the boolean to fill in.
 * @mandatory: Whether the element is mandatory in the message.
 */
static inline void
uwbmac_parser_add_bool(struct uwbmac_parser_element elements[], int tag,
		       bool *data, bool mandatory)
{
	uwbmac_parser_add(elements, tag, UWBMAC_PAYLOAD_TYPE_BOOL, data, NULL,
			  sizeof(*data), mandatory);
}

/**
 * uwbmac_parser_add_s8() - Set element to receive a signed 8-bit integer.
 * @elements: Array of elements.
 * @tag: Tag in the payload.
 * @data: Pointer to the integer to fill in.
 * @mandatory: Whether the element is mandatory in the message.
 */
static inline void uwbmac_parser_add_s8(struct uwbmac_parser_element elements[],
					int tag, int8_t *data, bool mandatory)
{
	uwbmac_parser_add(elements, tag, UWBMAC_PAYLOAD_TYPE_S8, data, NULL,
			  sizeof(*data), mandatory);
}

/**
 * uwbmac_parser_add_s16() - Set element to receive a signed 16-bit integer.
 * @elements: Array of elements.
 * @tag: Tag in the payload.
 * @data: Pointer to the integer to fill in.
 * @mandatory: Whether the element is mandatory in the message.
 */
static inline void
uwbmac_parser_add_s16(struct uwbmac_parser_element elements[], int tag,
		      int16_t *data, bool mandatory)
{
	uwbmac_parser_add(elements, tag, UWBMAC_PAYLOAD_TYPE_S16, data, NULL,
			  sizeof(*data), mandatory);
}

/**
 * uwbmac_parser_add_s32() - Set element to receive a signed 32-bit integer.
 * @elements: Array of elements.
 * @tag: Tag in the payload.
 * @data: Pointer to the integer to fill in.
 * @mandatory: Whether the element is mandatory in the message.
 */
static inline void
uwbmac_parser_add_s32(struct uwbmac_parser_element elements[], int tag,
		      int32_t *data, bool mandatory)
{
	uwbmac_parser_add(elements, tag, UWBMAC_PAYLOAD_TYPE_S32, data, NULL,
			  sizeof(*data), mandatory);
}

/**
 * uwbmac_parser_add_s64() - Set element to receive a signed 64-bit integer.
 * @elements: Array of elements.
 * @tag: Tag in the payload.
 * @data: Pointer to the integer to fill in.
 * @mandatory: Whether the element is mandatory in the message.
 */
static inline void
uwbmac_parser_add_s64(struct uwbmac_parser_element elements[], int tag,
		      int64_t *data, bool mandatory)
{
	uwbmac_parser_add(elements, tag, UWBMAC_PAYLOAD_TYPE_S64, data, NULL,
			  sizeof(*data), mandatory);
}

/**
 * uwbmac_parser_add_u8() - Set element to receive an unsigned 8-bit integer.
 * @elements: Array of elements.
 * @tag: Tag in the payload.
 * @data: Pointer to the integer to fill in.
 * @mandatory: Whether the element is mandatory in the message.
 */
static inline void uwbmac_parser_add_u8(struct uwbmac_parser_element elements[],
					int tag, uint8_t *data, bool mandatory)
{
	uwbmac_parser_add(elements, tag, UWBMAC_PAYLOAD_TYPE_U8, data, NULL,
			  sizeof(*data), mandatory);
}

/**
 * uwbmac_parser_add_u16() - Set element to receive an unsigned 16-bit integer.
 * @elements: Array of elements.
 * @tag: Tag in the payload.
 * @data: Pointer to the integer to fill in.
 * @mandatory: Whether the element is mandatory in the message.
 */
static inline void
uwbmac_parser_add_u16(struct uwbmac_parser_element elements[], int tag,
		      uint16_t *data, bool mandatory)
{
	uwbmac_parser_add(elements, tag, UWBMAC_PAYLOAD_TYPE_U16, data, NULL,
			  sizeof(*data), mandatory);
}

/**
 * uwbmac_parser_add_u32() - Set element to receive an unsigned 32-bit integer.
 * @elements: Array of elements.
 * @tag: Tag in the payload.
 * @data: Pointer to the integer to fill in.
 * @mandatory: Whether the element is mandatory in the message.
 */
static inline void
uwbmac_parser_add_u32(struct uwbmac_parser_element elements[], int tag,
		      uint32_t *data, bool mandatory)
{
	uwbmac_parser_add(elements, tag, UWBMAC_PAYLOAD_TYPE_U32, data, NULL,
			  sizeof(*data), mandatory);
}

/**
 * uwbmac_parser_add_u64() - Set element to receive an unsigned 64-bit integer.
 * @elements: Array of elements.
 * @tag: Tag in the payload.
 * @data: Pointer to the integer to fill in.
 * @mandatory: Whether the element is mandatory in the message.
 */
static inline void
uwbmac_parser_add_u64(struct uwbmac_parser_element elements[], int tag,
		      uint64_t *data, bool mandatory)
{
	uwbmac_parser_add(elements, tag, UWBMAC_PAYLOAD_TYPE_U64, data, NULL,
			  sizeof(*data), mandatory);
}

/**
 * uwbmac_parser_add_string() - Set element to receive a string.
 * @elements: Array of elements.
 * @tag: Tag in the payload.
 * @data: Pointer to the string to fill in.
 * @max_length: Length available, including terminating NUL character.
 * @mandatory: Whether the element is mandatory in the message.
 *
 * NOTE: The payload string is copied from the payload to the provided string.
 */
static inline void
uwbmac_parser_add_string(struct uwbmac_parser_element elements[], int tag,
			 char *data, int max_length, bool mandatory)
{
	uwbmac_parser_add(elements, tag, UWBMAC_PAYLOAD_TYPE_STRING, data, NULL,
			  max_length, mandatory);
}

/**
 * uwbmac_parser_add_binary() - Set element to receive a binary object.
 * @elements: Array of elements.
 * @tag: Tag in the payload.
 * @data: Pointer to the object to fill in.
 * @length: Actual length.
 * @max_length: Length available.
 * @mandatory: Whether the element is mandatory in the message.
 *
 * NOTE: The payload object is copied from the payload to the provided object.
 */
static inline void
uwbmac_parser_add_binary(struct uwbmac_parser_element elements[], int tag,
			 void *data, int *length, int max_length,
			 bool mandatory)
{
	uwbmac_parser_add(elements, tag, UWBMAC_PAYLOAD_TYPE_BINARY, data,
			  length, max_length, mandatory);
}

/**
 * uwbmac_parser_add_nested() - Set element to receive a nested message.
 * @elements: Array of elements.
 * @tag: Tag in the payload.
 * @nested: Pointer to the structure to keep internal data.
 * @mandatory: Whether the element is mandatory in the message.
 */
static inline void
uwbmac_parser_add_nested(struct uwbmac_parser_element elements[], int tag,
			 struct uwbmac_msg *nested, bool mandatory)
{
	uwbmac_parser_add(elements, tag, UWBMAC_PAYLOAD_TYPE_NESTED, nested,
			  NULL, 0, mandatory);
}

/**
 * uwbmac_msg_read_tag() - Tell current message element tag and data type.
 * @msg: message in its current reading state
 * @tag: output value
 * @is_nested: output value telling whether data is a nested message to binary data.
 * @rem: in/out remaining size to parse in message
 *
 * Set message state to the next element.
 * Used to serialize the message without knowing the meaning of its elements.
 *
 * NB: `rem` parameter must be updated synchronously to msg `position` pointer.
 *
 * Return: status UWBMAC_SUCCESS or UWBMAC_EINVAL.
 */
int uwbmac_msg_read_tag(struct uwbmac_msg *msg, int *tag, bool *is_nested,
			int *rem);

/**
 * uwbmac_msg_read_nested() - Get the current element as a nested msg
 * @msg: message in its current reading state
 * @nested: messge to be initialized to point to the nested part of msg
 * @rem: in/out remaining size to parse in message
 *
 * Actual type not checked, uwbmac_msg_read_tag() should be used before.
 * Set message state to the next element.
 * Used to serialize the message without knowing the meaning of its elements.
 *
 * NB: `rem` parameter must be updated synchronously to msg `position` pointer.
 *
 * Return: status UWBMAC_SUCCESS or UWBMAC_EINVAL.
 */
int uwbmac_msg_read_nested(struct uwbmac_msg *msg, struct uwbmac_msg *nested,
			   int *rem);

/**
 * uwbmac_msg_read_data() - Get the current element as binary data
 * @msg: message in its current reading state
 * @data: output, set to point to the data
 * @length: output, set to the data length
 * @rem: in/out remaining size to parse in message
 *
 * Actual type not checked, uwbmac_msg_read_tag() should be used before.
 * Set message state to the next element.
 * Used to serialize the message without knowing the meaning of its elements.
 *
 * NB: `rem` parameter must be updated synchronously to msg `position` pointer.
 *
 * Return: status UWBMAC_SUCCESS or UWBMAC_EINVAL.
 */
int uwbmac_msg_read_data(struct uwbmac_msg *msg, uint8_t **data, size_t *length,
			 int *rem);

#endif // UWBMAC_PAYLOAD_READ_H
