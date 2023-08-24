/*
 * Copyright (c) 2020–2021 Qorvo, Inc
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

#ifndef UCI_MESSAGE_H
#define UCI_MESSAGE_H

#include <uci/uci.h>
#include <uci/uci_compat.h>

/**
 * DOC: Message Overview
 *
 * When a message has been received, it is given to a message handler. The
 * message has been reassembled and is given as the payload which may be split
 * in several block buffers.
 *
 * When a user wants to send a message, it gives the payload to the UCI core
 * which handle segmentation and packet generation.
 *
 * The UCI message API allows building message and parsing payload efficiently.
 * For example, the message builder will reserve space for packet header while
 * building the message.
 */

/**
 * struct uci_message_builder - Context to build a message.
 *
 * The message being built must be sent or released by the user.
 */
struct uci_message_builder {
	/**
	 * @uci: UCI context.
	 */
	struct uci *uci;
	/**
	 * @message: Pointer to first buffer block, or NULL if none yet.
	 */
	struct uci_blk *message;
	/**
	 * @current_block: Pointer to last buffer block, the one currently written to,
	 * or NULL if none yet.
	 */
	struct uci_blk *current_block;
	/**
	 * @first_errno: The first error we detected, if any.
	 */
	int first_errno;
	/**
	 * @elems_nr: Pointer to write the number of elements, for nested only.
	 */
	uint8_t *elems_nr;
	/**
	 * @expected_packet_size: The size of the packet we want to build,
	 * used to suggest the correct size to the allocator.
	 */
	uint16_t expected_packet_size;
};

/**
 * struct uci_message_parser - Context to parse a message.
 *
 * The message is not owned by the parser context and must be released by the
 * user.
 */
struct uci_message_parser {
	/**
	 * @current_block: Pointer to block being read.
	 */
	const struct uci_blk *current_block;
	/**
	 * @previous: Pointer to previous block or NULL if still reading the
	 * first block. This is used to break the chain when the remaining
	 * blocks are extracted from a message.
	 */
	const struct uci_blk *previous;
	/**
	 * @offset: Offset in current block.
	 */
	uint16_t offset;
	/**
	 * @current_remaining_len: Total remaining length of message from start
	 * of current block.
	 */
	size_t current_remaining_len;
};

/**
 * UCI_MESSAGE_BUILDER_INITIALIZER_WITH_SIZE() - Initialize on stack message builder.
 * @uci_: UCI context used for allocation.
 * @expected_packet_size_: Size of the packet we want to build.
 */
#define UCI_MESSAGE_BUILDER_INITIALIZER_WITH_SIZE(uci_, expected_packet_size_) \
	{                                                                      \
		.uci = (uci_), .message = NULL, .current_block = NULL,         \
		.first_errno = 0, .elems_nr = NULL,                            \
		.expected_packet_size = expected_packet_size_                  \
	}

/**
 * UCI_MESSAGE_BUILDER_INITIALIZER() - Initialize on stack message builder.
 * @uci_: UCI context used for allocation.
 */
#define UCI_MESSAGE_BUILDER_INITIALIZER(uci_)                          \
	{                                                              \
		.uci = (uci_), .message = NULL, .current_block = NULL, \
		.first_errno = 0, .elems_nr = NULL,                    \
		.expected_packet_size = (uint16_t)UCI_MAX_PACKET_SIZE  \
	}

static inline void uci_message_builder_init(struct uci_message_builder *builder,
					    struct uci *uci)
{
	memset(builder, 0, sizeof(*builder));
	builder->uci = uci;
	builder->expected_packet_size = (uint16_t)UCI_MAX_PACKET_SIZE;
}

/**
 * UCI_MESSAGE_PARSER_INITIALIZER() - Initialize on stack message parser.
 * @message_: UCI message, which can be composed of several packets.
 */
#define UCI_MESSAGE_PARSER_INITIALIZER(message_)                             \
	{                                                                    \
		.current_block = (message_), .previous = message_,           \
		.offset = 0, .current_remaining_len = (message_)->total_len, \
	}

/**
 * uci_message_failed() - Check whether an operation failed.
 * @builder: Builder context.
 *
 * Return: 0 on success, or errno on failure.
 */
static inline int uci_message_failed(const struct uci_message_builder *builder)
{
	return builder->first_errno;
}

/**
 * uci_message_start_nest() - Start a nested message.
 * @builder: Builder context.
 * @payload_builder: Nested message builder context.
 */
void uci_message_start_nest(struct uci_message_builder *builder,
			    struct uci_message_builder *payload_builder);

/**
 * uci_message_end_nest() - End a nested message.
 * @builder: Builder context.
 * @payload_builder: Nested message builder context.
 */
void uci_message_end_nest(struct uci_message_builder *builder,
			  const struct uci_message_builder *payload_builder);

/**
 * uci_message_nest_get_elems_nr() - Get a nested message's number of elements.
 * @builder: Builder context.
 *
 * Return: 0 if not a nest, number of elements otherwise.
 */
static inline uint8_t
uci_message_nest_get_elems_nr(struct uci_message_builder *builder)
{
	return builder->elems_nr ? *builder->elems_nr : 0;
}

/**
 * uci_message_nest_set_elems_nr() - Set a nested message's number of elements.
 * @builder: Builder context.
 * @elems_nr: Wanted number of elements.
 *
 * NOTE: this function has no effect on non-nested message builders.
 */
static inline void
uci_message_nest_set_elems_nr(struct uci_message_builder *builder,
			      uint8_t elems_nr)
{
	if (!builder->elems_nr)
		return;

	*builder->elems_nr = elems_nr;
}

/**
 * uci_message_put() - Add data to a message.
 * @builder: Builder context.
 * @data: Data to copy to message.
 * @data_len: Length of data.
 *
 * Return: 0 on success, or error on allocation failure.
 */
int uci_message_put(struct uci_message_builder *builder, const uint8_t *data,
		    size_t data_len);

/**
 * uci_message_put_8bit() - Add a 8 bit scalar to a message.
 * @builder: Builder context.
 * @data: Data to copy to message.
 *
 * Return: 0 on success, or error on allocation failure.
 */
static inline int uci_message_put_8bit(struct uci_message_builder *builder,
				       uint8_t data)
{
	return uci_message_put(builder, &data, sizeof(data));
}

/**
 * uci_message_put_16bit() - Add a 16 bit scalar to a message.
 * @builder: Builder context.
 * @data: Data to copy to message.
 *
 * Return: 0 on success, or error on allocation failure.
 */
static inline int uci_message_put_16bit(struct uci_message_builder *builder,
					uint16_t data)
{
	uint8_t buf[] = {
		(uint8_t)data,
		(uint8_t)(data >> 8),
	};
	return uci_message_put(builder, buf, sizeof(buf));
}

/**
 * uci_message_put_32bit() - Add a 32 bit scalar to a message.
 * @builder: Builder context.
 * @data: Data to copy to message.
 *
 * Return: 0 on success, or error on allocation failure.
 */
static inline int uci_message_put_32bit(struct uci_message_builder *builder,
					uint32_t data)
{
	uint8_t buf[] = {
		(uint8_t)data,
		(uint8_t)(data >> 8),
		(uint8_t)(data >> 16),
		(uint8_t)(data >> 24),
	};
	return uci_message_put(builder, buf, sizeof(buf));
}

/**
 * uci_message_put_48bit() - Add a 48 bit scalar to a message.
 * @builder: Builder context.
 * @data: Data to copy to message.
 *
 * Return: 0 on success, or error on allocation failure.
 */
static inline int uci_message_put_48bit(struct uci_message_builder *builder,
					uint64_t data)
{
	uint8_t buf[] = {
		(uint8_t)data,	       (uint8_t)(data >> 8),
		(uint8_t)(data >> 16), (uint8_t)(data >> 24),
		(uint8_t)(data >> 32), (uint8_t)(data >> 40),
	};
	return uci_message_put(builder, buf, 6);
}

/**
 * uci_message_put_64bit() - Add a 64 bit scalar to a message.
 * @builder: Builder context.
 * @data: Data to copy to message.
 *
 * Return: 0 on success, or error on allocation failure.
 */
static inline int uci_message_put_64bit(struct uci_message_builder *builder,
					uint64_t data)
{
	uint8_t buf[] = {
		(uint8_t)data,	       (uint8_t)(data >> 8),
		(uint8_t)(data >> 16), (uint8_t)(data >> 24),
		(uint8_t)(data >> 32), (uint8_t)(data >> 40),
		(uint8_t)(data >> 48), (uint8_t)(data >> 56),
	};
	return uci_message_put(builder, buf, sizeof(buf));
}

/**
 * uci_message_put_nocopy() - Reserve space to write data to a message.
 * @builder: Builder context.
 * @size: Size of the reservation.
 * @data: Where to store the data pointer.
 *
 * This can be used to write to a message using external mechanism.
 *
 * .. note::
 *    The returned size can be smaller than the requested size. If needed, the
 *    caller can call this function several times.
 *
 * .. note::
 *    The returned memory space has no particular alignment.
 *
 * Return: Size of reserved space, or error on allocation failure.
 */
ssize_t uci_message_put_nocopy(struct uci_message_builder *builder, size_t size,
			       uint8_t **data);

/**
 * uci_message_put_blk() - Add a buffer block to a message, without copy.
 * @builder: Builder context.
 * @blk: Buffer to add, ownership is transferred to message builder.
 */
void uci_message_put_blk(struct uci_message_builder *builder,
			 struct uci_blk *blk);

/**
 * uci_message_reserve_8bit() - Add a 8bit scalar to message and return pointer to it.
 * @builder: Builder context.
 * @data: Initial value to set.
 *
 * NOTE: This is useful when constructing TLV message, when status and number of
 *
 * Return: pointer to the data written.
 */
static inline uint8_t *
uci_message_reserve_8bit(struct uci_message_builder *builder, uint8_t data)
{
	uci_message_put_8bit(builder, data);
	if (!uci_message_failed(builder)) {
		return &(builder->current_block
				 ->data[builder->current_block->len - 1]);
	}
	return NULL;
}

/**
 * uci_message_get() - Get data from a message.
 * @parser: Parser context.
 * @data: Where to write read data.
 * @size: Maximum size to get.
 *
 * Return: Length of data read from buffer.
 */
size_t uci_message_get(struct uci_message_parser *parser, uint8_t *data,
		       size_t size);

/**
 * uci_message_remaining() - Get remaining length of a message.
 * @parser: Parser context.
 *
 * Return: Remaining data in message.
 */
static inline size_t uci_message_remaining(struct uci_message_parser *parser)
{
	return parser->current_remaining_len - parser->offset;
}

/**
 * uci_message_get_8bit() - Get a 8 bit scalar from a message.
 * @parser: Parser context.
 *
 * .. note::
 *    Size of remaining data must be checked before this call.
 *
 * Return: Data read from message.
 */
static inline uint8_t uci_message_get_8bit(struct uci_message_parser *parser)
{
	uint8_t data;
	__attribute__((unused))
	size_t size = uci_message_get(parser, &data, sizeof(data));
	UCI_ASSERT(size == sizeof(data));
	return data;
}

/**
 * uci_message_get_16bit() - Get a 16 bit scalar from a message.
 * @parser: Parser context.
 *
 * .. note::
 *    Size of remaining data must be checked before this call.
 *
 * Return: Data read from message.
 */
static inline uint16_t uci_message_get_16bit(struct uci_message_parser *parser)
{
	uint8_t data[sizeof(uint16_t)];
	__attribute__((unused))
	size_t size = uci_message_get(parser, data, sizeof(data));
	UCI_ASSERT(size == sizeof(data));
	return data[0] | (uint16_t)data[1] << 8;
}

/**
 * uci_message_get_32bit() - Get a 32 bit scalar from a message.
 * @parser: Parser context.
 *
 * .. note::
 *    Size of remaining data must be checked before this call.
 *
 * Return: Data read from message.
 */
static inline uint32_t uci_message_get_32bit(struct uci_message_parser *parser)
{
	uint8_t data[sizeof(uint32_t)];
	__attribute__((unused))
	size_t size = uci_message_get(parser, data, sizeof(data));
	UCI_ASSERT(size == sizeof(data));
	return data[0] | (uint32_t)data[1] << 8 | (uint32_t)data[2] << 16 |
	       (uint32_t)data[3] << 24;
}

/**
 * uci_message_get_48bit() - Get a 48 bit scalar from a message.
 * @parser: Parser context.
 *
 * .. note::
 *    Size of remaining data must be checked before this call.
 *
 * Return: Data read from message.
 */
static inline uint64_t uci_message_get_48bit(struct uci_message_parser *parser)
{
	uint8_t data[6];
	__attribute__((unused))
	size_t size = uci_message_get(parser, data, sizeof(data));
	uint32_t lsb, msb;
	UCI_ASSERT(size == sizeof(data));
	lsb = data[0] | (uint32_t)data[1] << 8 | (uint32_t)data[2] << 16 |
	      (uint32_t)data[3] << 24;
	msb = data[4] | (uint32_t)data[5] << 8 | (uint32_t)0 << 16 |
	      (uint32_t)0 << 24;
	return lsb | (uint64_t)msb << 32;
}

/**
 * uci_message_get_64bit() - Get a 64 bit scalar from a message.
 * @parser: Parser context.
 *
 * .. note::
 *    Size of remaining data must be checked before this call.
 *
 * Return: Data read from message.
 */
static inline uint64_t uci_message_get_64bit(struct uci_message_parser *parser)
{
	uint8_t data[sizeof(uint64_t)];
	__attribute__((unused))
	size_t size = uci_message_get(parser, data, sizeof(data));
	uint32_t lsb, msb;
	UCI_ASSERT(size == sizeof(data));
	lsb = data[0] | (uint32_t)data[1] << 8 | (uint32_t)data[2] << 16 |
	      (uint32_t)data[3] << 24;
	msb = data[4] | (uint32_t)data[5] << 8 | (uint32_t)data[6] << 16 |
	      (uint32_t)data[7] << 24;
	return lsb | (uint64_t)msb << 32;
}

/**
 * uci_message_get_nocopy() - Get a pointer to message data.
 * @parser: Parser context.
 * @size: Size of data to get.
 * @data: Where to store the data pointer.
 *
 * This can be used to read from a message using external mechanism.
 *
 * .. note::
 *    The returned size can be smaller than the requested size even if enough
 *    data is available because the message can be split in several blocks. If
 *    needed, the caller can call this function several times.
 *
 * .. note::
 *    The returned memory space has no particular alignment.
 *
 * .. note::
 *    The returned memory is valid until message is released.
 *
 * Return: Size of data, or 0 if no data left.
 */
size_t uci_message_get_nocopy(struct uci_message_parser *parser, size_t size,
			      uint8_t **data);

//TODO(Guillaume): missing implementation
/**
 * uci_message_get_blk_all() - Extract all remaining data from the message.
 * @parser: Parser context.
 *
 * This extracts the remaining blocks form the parsed message to be consumed
 * later.
 *
 * .. note::
 *    If the returned pointer is different from the original message pointer,
 *    you must release the blocks which are part of the original message.
 *
 * Return: Pointer to first block of remaining data, or NULL if no data left.
 */
struct uci_blk *uci_message_get_blk_all(struct uci_message_parser *parser);

#endif /* UCI_MESSAGE_H */
