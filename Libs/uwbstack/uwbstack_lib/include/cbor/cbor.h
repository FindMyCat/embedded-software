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

#ifndef QORVO_CBOR_H
#define QORVO_CBOR_H

#include <cbor/cbor_compat.h>

/* TODO(Guillaume): basic types utf8, simple and tags are not implemented yet.
We don't need them for uwbmac_uci. */
/**
 * enum cbor_type - Type of data encoded.
 *
 * @CBOR_PINT:
 *  Positive integer of maximum size 64 bits.
 * @CBOR_NINT:
 *  Negative integer of maximum size 64 bits.
 * @CBOR_BINARY:
 *  Array of bytes.
 * @CBOR_UTF8:
 *  utf-8 encoded string.
 * @CBOR_ARRAY:
 *  Array of cbor elements.
 * @CBOR_MAP:
 *  Table of cbor elements.
 * @CBOR_TAG:
 *  Self describing type.
 * @CBOR_SIMPLE:
 *  Simple type is used for small ints, floating point and the break code.
 * @CBOR_LAST:
 *  Internal use or to express decoding error.
 */
enum cbor_type {
	CBOR_PINT = 0,
	CBOR_NINT,
	CBOR_BINARY,
	CBOR_UTF8,
	CBOR_ARRAY,
	CBOR_MAP,
	CBOR_TAG,
	CBOR_SIMPLE,
	CBOR_LAST,
};

/**
 * enum cbor_type_arg - Additional information contained in type byte.
 *
 * @CBOR_8B:
 *  One byte length.
 * @CBOR_16B:
 *  Two bytes length.
 * @CBOR_32B:
 *  Four bytes length.
 * @CBOR_64B:
 *  Eight bytes length.
 * @CBOR_BREAK:
 *  End of indefinite length array or map.
 * @CBOR_LAST_ARG:
 *  Internal use or to express decoding error.
 */
enum cbor_type_arg {
	CBOR_8B = 24,
	CBOR_16B = 25,
	CBOR_32B = 26,
	CBOR_64B = 27,
	CBOR_BREAK = 31,
	CBOR_LAST_ARG,
};

/**
 * enum cbor_error_code - Error codes definition.
 *
 * @CBOR_OK:
 *  No error.
 * @CBOR_ERROR:
 *  Unidentified error.
 * @CBOR_OVERFLOW:
 *  Encoding or decoding too far in memory.
 * @CBOR_INVALID_TYPE:
 *  Try to decode the wrong kind of data.
 */
enum cbor_error_code {
	CBOR_OK = 0,
	CBOR_ERROR,
	CBOR_OVERFLOW,
	CBOR_INVALID_TYPE,
};

/**
 * struct cbor_state - State of encoding and/or decoding.
 */
struct cbor_state {
	/**
	 * @payload: Byte string representing the message.
	 */
	uint8_t *payload;
	/**
	 * @offset: Position of the next operation in the message.
	 */
	int offset;
	/**
	 * @size: Size of the message in bytes.
	 */
	int size;
	/**
	 * @error: Containing the error raised during processing.
	 */
	int error;
};

/**
 * cbor_init() - Initialise state with a data buffer.
 *
 * @state: State to be initialised/provided by the client.
 * @payload: Byte string of the message already allocated.
 * @size: Size of the payload.
 */
void cbor_init(struct cbor_state *state, void *payload, int size);

/**
 * cbor_get_error() - Get the state error flag.
 *
 * @state: State of the process.
 *
 * NOTE: This is used to check if any operation failed and raised an error flag.
 *
 * Return: CBOR_OK or an cbor_error_code.
 */
enum cbor_error_code cbor_get_error(struct cbor_state *state);

/**
 * cbor_get_remaining() - Initialise state with a data buffer.
 *
 * @state: State of the process.
 *
 * Return: int representing the space left in the message to encode or decode values.
 */
int cbor_get_remaining(struct cbor_state *state);

/**
 * cbor_move() - Move offset without reading the value.
 *
 * @state: State to be moved.
 * @move: The amount of values by which position is moved.
 */
void cbor_move(struct cbor_state *state, int move);

/**
 * cbor_peek_type() - Decode major type.
 *
 * @state: State.
 *
 * NOTE: Does not move the state.
 *
 * Return: An enum cbor_type, CBOR_LAST on error.
 */
enum cbor_type cbor_peek_type(struct cbor_state *state);

/**
 * cbor_peek_type_arg() - Decode additional information in type byte.
 *
 * @state: State.
 *
 * NOTE: Does not move the state
 *
 * Return: An enum cbor_type_arg, CBOR_LAST_ARG on error.
 */
enum cbor_type_arg cbor_peek_type_arg(struct cbor_state *state);

/**
 * cbor_get_binary_no_cpy() - Give back a pointer to start of byte string and its length.
 *
 * @state: State
 * @result: The pointer that will be set to the correct place in the message.
 * @length: The length of the the resulting array.
 */
void cbor_get_binary_no_cpy(struct cbor_state *state, void **result,
			    int *length);

/**
 * cbor_get_binary() - Copy the byte string into result.
 *
 * @state: State
 * @result: The array that will be fill with data from the message.
 * @length: The length of the the resulting array.
 * @max_length: The maximum length accepted.
 */
void cbor_get_binary(struct cbor_state *state, void *result, int *length,
		     int max_length);

/**
 * cbor_get_length() - Read the length of map, array, string or byte string type.
 *
 * @state: State
 *
 * When decoding, use this to read the length or array types (binary, map, string, array).
 * If reading is not possible, an error is raised.
 *
 * Return: the length or -1 in case of indefinite length and error.
 */
int cbor_get_length(struct cbor_state *state);

/**
 * cbor_get_u64() - Read an unsigned integer of size 64bits.
 *
 * @state: State
 *
 * When decoding, use this to read a type CBOR_PINT.
 * If reading is not possible, an error is raised.
 *
 * Return: the integer read.
 */
uint64_t cbor_get_u64(struct cbor_state *state);

/**
 * cbor_get_s64() - Read a signed integer of size 64bits.
 *
 * @state: State
 *
 * When decoding, use this to read a type CBOR_SINT.
 * If reading is not possible, an error is raised.
 *
 * Return: the integer read.
 */
int64_t cbor_get_s64(struct cbor_state *state);

/**
 * cbor_put_u64() - Put a value of CBOR_PINT type.
 *
 * @state: State
 * @val: Positiv int to put in.
 */
void cbor_put_u64(struct cbor_state *state, uint64_t val);

/**
 * cbor_put_s64() - Put a value of CBOR_NINT or CBOR_PINT type.
 *
 * @state: State
 * @val: Signed int to put in.
 */
void cbor_put_s64(struct cbor_state *state, int64_t val);

/**
 * cbor_start_map() - Start a type map.
 *
 * @state: State
 * @length: Number of items in the map. If negative then indefinite length.
 */
void cbor_start_map(struct cbor_state *state, int length);

/**
 * cbor_start_array() - Start an array type.
 *
 * @state: State
 * @length: Number of items in the map. If negative then indefinite length.
 */
void cbor_start_array(struct cbor_state *state, int length);

/**
 * cbor_put_break() - End an indefinite length map or array by adding the break code.
 *
 * @state: State.
 */
void cbor_put_break(struct cbor_state *state);

/**
 * cbor_put_binary() - Copy an array into a cbor byte string encoded value.
 *
 * @state: State.
 * @array: The byte string to copy.
 * @length: Length of array.
 */
void cbor_put_binary(struct cbor_state *state, const void *array, int length);

/**
 * cbor_next() - Move state to next value.
 *
 * @state: State.
 *
 * When decoding, use this to ignore a value being read or to jump over
 * a map or array header.
 */
void cbor_next(struct cbor_state *state);

#endif /* QORVO_CBOR_H */
