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

#ifndef UCI_H
#define UCI_H

#include <stddef.h>

#include <uci/uci_compat.h>
#include <uwbmac/uwbmac_error.h>

/**
 * DOC: Overview
 *
 * The UCI core implements packets parsing, packets generation, error
 * processing, segmentation, reassembly and routing. It can be used to implement
 * a UCI server (which receives commands) or UCI client (which receives
 * responses and notifications).
 *
 * The packets are sent and received using a transport channel which is
 * responsible for framing, and communication with the transport driver.
 *
 * Received commands are given to message handlers which decode the payload.
 */

/**
 * DOC: Memory handling
 *
 * To allow zero copy, buffers used to store packet data are allocated
 * dynamically. This can be backed with simple heap allocation, or using
 * a limited pool of memory blocks.
 *
 * To limit memory fragmentation, packet buffers can be split in smaller
 * blocks. This is decided by the memory allocator. In this case, a packet can
 * be composed of several blocks which are chained together. The first block
 * contains the information of the full packet length.
 */

/*
 * UCI_BLK_SIZE_MIN - Minimal size of a buffer block.
 *
 * The minimal size of a block is 20 bytes. This is needed for segmentation of
 * a block over several UCI packets, 4 bytes for the header, and 16 bytes for
 * a block descriptor.
 *
 * This minimal size ought to be respected by allocator
 * unless provided size_hint specified less.
 */
#define UCI_BLK_SIZE_MIN 20

/**
 * UCI_MAX_PAYLOAD_SIZE - Maximum size of a UCI payload.
 */
#define UCI_MAX_PAYLOAD_SIZE 255

/**
 * UCI_PACKET_HEADER_SIZE - Size of a UCI packet header.
 *
 * Note: this also the minimum packet size.
 */
#define UCI_PACKET_HEADER_SIZE 4

/**
 * UCI_MAX_PACKET_SIZE - Maximum size of a UCI packet.
 */
#define UCI_MAX_PACKET_SIZE (UCI_PACKET_HEADER_SIZE + UCI_MAX_PAYLOAD_SIZE)

/**
 * UCI_STATUS_PACKET_SIZE - Size of a UCI status packet.
 *
 */
#define UCI_STATUS_PACKET_SIZE 5

/**
 * UCI_MT_GID_OID() - Macro to build &uci_message_handler.mt_gid_oid.
 * @mt: Message type.
 * @gid: Group identifier.
 * @oid: Opcode identifier.
 *
 * Return: Integer suitable for use in &uci_message_handler.mt_gid_oid.
 */
#define UCI_MT_GID_OID(mt, gid, oid) ((mt) << 13 | (gid) << 8 | (oid))

/**
 * UCI_GID_OID() - Macro to build @gid_oid parameter.
 * @gid: Group identifier.
 * @oid: Opcode identifier.
 *
 * Return: Integer suitable for use as @gid_oid parameter.
 */
#define UCI_GID_OID(gid, oid) UCI_MT_GID_OID(0, (gid), (oid))

/**
 * UCI_MT() - Macro to extract message type from a @mt_gid_oid parameter.
 * @mt_gid_oid: Union of message type, group identifier and opcode identifier.
 *
 * Return: Message type.
 */
#define UCI_MT(mt_gid_oid) (((mt_gid_oid) >> 13) & 0x7)

/**
 * UCI_GID() - Macro to extract group identifier from a @mt_gid_oid parameter.
 * @mt_gid_oid: Union of message type, group identifier and opcode identifier.
 *
 * Return: Group identifier.
 */
#define UCI_GID(mt_gid_oid) (((mt_gid_oid) >> 8) & 0xf)

/**
 * UCI_OID() - Macro to extract opcode identifier from a @mt_gid_oid parameter.
 * @mt_gid_oid: Union of message type, group identifier and opcode identifier.
 *
 * Return: Opcode identifier.
 */
#define UCI_OID(mt_gid_oid) ((mt_gid_oid)&0x3f)

/**
 * UCI_CONTAINER_OF() - Get a struct pointer from one of its members pointer.
 * @ptr: Pointer to the member.
 * @type: Type of the parent struct.
 * @member: Name to the member in the parent struct.
 *
 * Return: Pointer to the parent struct.
 */
#define UCI_CONTAINER_OF(ptr, type, member)                  \
	({                                                   \
		char *__mptr = (char *)(ptr);                \
		((type *)(__mptr - offsetof(type, member))); \
	})

/**
 * enum uci_blk_flags - Packet buffer block flag.
 *
 * @UCI_BLK_FLAGS_STATIC:
 *     Do not release this block.
 * @UCI_BLK_FLAGS_DESTRUCTIBLE:
 *     Do not release this block with the free method from the given
 *     uci_allocator, but use a custom destructor.
 *     In this case the block should be cast to &struct uci_blk_destructible
 *     to access the destructor.
 * @UCI_BLK_FLAGS_HEADER_RESERVED:
 *     Set if there are 4 reserved bytes for header before payload at the
 *     beginning of the data buffer.
 */
enum uci_blk_flags {
	UCI_BLK_FLAGS_STATIC = 1,
	UCI_BLK_FLAGS_DESTRUCTIBLE = 2,
	UCI_BLK_FLAGS_HEADER_RESERVED = 4,
};

/**
 * struct uci_blk - Packet buffer block.
 */
struct uci_blk {
	/**
	 * @next: Pointer to next block, or NULL if last one.
	 */
	struct uci_blk *next;
	/**
	 * @data: Pointer to data in this block. This can be changed during the
	 * lifetime of the buffer and should not be used for memory release.
	 */
	uint8_t *data;
	/**
	 * @len: Length of data in this block.
	 */
	uint16_t len;
	/**
	 * @total_len: Length of the full packet or payload. This is set in the
	 * first block only, and zero for other blocks.
	 */
	uint16_t total_len;
	/**
	 * @size: Allocation size for data in this block.
	 */
	uint16_t size;
	/**
	 * @flags: Block flags, see &enum uci_blk_flags.
	 */
	uint8_t flags;
};

/**
 * struct uci_blk_destructible - Packet buffer block with a destructor.
 *
 * This is not allocated using the UCI allocator. It can be used by an external
 * code to embed data inside a packet and avoid copy.
 */
struct uci_blk_destructible {
	/**
	 * @blk: Block common part.
	 */
	struct uci_blk blk;
	/**
	 * @destructor: Function to call instead of &uci_allocator_ops.free.
	 */
	void (*destructor)(void *arg, struct uci_blk_destructible *blk);
	/**
	 * @destructor_arg: First argument of &uci_blk_destructible.destructor.
	 */
	void *destructor_arg;
};

struct uci_allocator;

/**
 * struct uci_allocator_ops - UCI allocator operations.
 */
struct uci_allocator_ops {
	/**
	 * @alloc: Allocate one block. The returned block can be smaller or
	 * larger than the size_hint. Return NULL if memory is exhausted.
	 */
	struct uci_blk *(*alloc)(struct uci_allocator *allocator,
				 size_t size_hint);
	/**
	 * @free: Release one block.
	 */
	void (*free)(struct uci_allocator *allocator, struct uci_blk *blk);
};

/**
 * struct uci_allocator - UCI allocator instance.
 */
struct uci_allocator {
	/**
	 * @ops: Allocator operations.
	 */
	const struct uci_allocator_ops *ops;
};

struct uci;

/**
 * uci_init() - Initialize UCI core.
 * @uci: UCI context.
 * @allocator: Allocator for buffers.
 * @is_client: True if this is a client.
 *
 * Return: UWBMAC_SUCCESS on success, or an error code.
 */
uwbmac_error uci_init(struct uci *uci, struct uci_allocator *allocator,
		      bool is_client);

/**
 * uci_uninit() - Uninitialize UCI core.
 * @uci: UCI context.
 *
 * Note that uninit does not cleanup the handlers linked list, which should
 * no longer be used outside of the library.
 */
void uci_uninit(struct uci *uci);

/**
 * uci_blk_alloc() - Allocate buffer block.
 * @uci: UCI context.
 * @size_hint: Hint on the required size.
 *
 * Allocate a buffer, usually used to send a message. The size hint can give an
 * indication of the required size. The returned buffer can be smaller or larger
 * due to allocation constrains. In all cases, only one block is returned.
 *
 * The UCI message API should be preferred to build a message to send.
 *
 * Return: The allocated buffer or NULL if no memory is available.
 */
struct uci_blk *uci_blk_alloc(struct uci *uci, size_t size_hint);

/**
 * uci_blk_free_all() - Release a chain of buffer blocks.
 * @uci: UCI context.
 * @blks: Chain of buffer blocks, or NULL.
 *
 * Release a chain of buffer blocks which were allocated from uci_blk_alloc() or
 * received by a message handler.
 *
 * Does nothing if called with a NULL pointer.
 */
void uci_blk_free_all(struct uci *uci, struct uci_blk *blks);

/**
 * typedef uci_message_handler_function - Handle a received message.
 * @uci: UCI context.
 * @mt_gid_oid: Union of message type, group identifier and opcode identifier.
 * @payload: Reassembled message payload, can span several buffer blocks.
 * @user_data: User data given at registration.
 *
 * If the handler returns 0, it musts take care of the response if needed,
 * during its execution, or later.
 *
 * If the message is a command and the return value signals an error, a response
 * with status FAILED will be sent.
 *
 * Return: UWBMAC_SUCCESS on success, or an uwbmac error code.
 *	   The special value UWBMAC_UCI_MSG_HANDLED forces the packet receiver to stop
 *	   calling after this handler.
 */
typedef uwbmac_error (*uci_message_handler_function)(
	struct uci *uci, uint16_t mt_gid_oid, const struct uci_blk *payload,
	void *user_data);

/**
 * struct uci_message_handler - Definition of a message handler.
 */
struct uci_message_handler {
	/**
	 * @mt_gid_oid: Union of message type, group identifier and opcode
	 * identifier. Use the UCI_MT_GID_OID() macro.
	 */
	uint16_t mt_gid_oid;
	/**
	 * @handler: Function called to handle a received message.
	 */
	uci_message_handler_function handler;
};

/**
 * struct uci_message_handlers - Definition of several message handlers
 * belonging to the same module.
 */
struct uci_message_handlers {
	/**
	 * @next: Pointer to next message handlers definition, or NULL if this
	 * is the last one. Filled at registration.
	 */
	struct uci_message_handlers *next;
	/**
	 * @handlers: Pointer to array of message handler definitions.
	 *
	 * NOTE: **This must be sorted** by message type, then group identifier, then
	 * opcode identifier.
	 */
	const struct uci_message_handler *handlers;
	/**
	 * @n_handlers: Number of message handlers.
	 */
	size_t n_handlers;
	/**
	 * @user_data: User data given to message handlers when a message is
	 * received.
	 */
	void *user_data;
};

// TODO(cyril): in unit test handlers memory is discarded before uci_uninit()
// TODO(cyril): implement an unregister() to losen the memory requirement
/**
 * uci_message_handlers_register() - Register message handlers.
 * @uci: UCI context.
 * @handlers: Information on message handlers.
 *
 * Be sure that the uci_message_handlers provided only contains handlers for
 * only one GID.
 * The method will not accept it otherwise and will register nothing.
 *
 * NOTE: Handlers memory is managed by the client, but its representation
 * is managed as a linked list by the library. Its memory is assumed to be
 * usable &until uci_uninit().
 *
 */
void uci_message_handlers_register(struct uci *uci,
				   struct uci_message_handlers *handlers);

/**
 * uci_packet_recv_alloc() - Allocate buffer for reception.
 * @uci: UCI context.
 * @size_hint: Hint on the required size.
 *
 * This function should be called by the transport channel to build packets for
 * received data. The size hint can give an indication of the required size in
 * case the packet size is known. The returned buffer can be smaller or larger
 * due to allocation constrains. In all cases, only one block is returned.
 *
 * Return: The allocated buffer or NULL if no memory is available.
 */
struct uci_blk *uci_packet_recv_alloc(struct uci *uci, uint16_t size_hint);

/**
 * uci_packet_recv_free_all() - Release unused reception buffer.
 * @uci: UCI context.
 * @blks: Chain of buffer blocks to release, or NULL.
 *
 * This function should be called by the transport channel with unused buffers
 * when a packet reception was aborted due to an error, or when the channel is
 * shut down.
 *
 * Does nothing if called with a NULL pointer.
 */
void uci_packet_recv_free_all(struct uci *uci, struct uci_blk *blks);

/**
 * uci_packet_recv() - Hand off valid received packet.
 * @uci: UCI context.
 * @packet: Received packet (can be composed of several blocks).
 *
 * This function should be called by the transport channel when a packet has
 * been successfully received. The UCI core takes ownership of the associated
 * memory.
 *
 */
void uci_packet_recv(struct uci *uci, struct uci_blk *packet);

struct uci_transport;

/**
 * struct uci_transport_ops - UCI transport channel callbacks.
 */
struct uci_transport_ops {
	/**
	 * @attach: Callback invoked when the transport channel is attached.
	 */
	void (*attach)(struct uci_transport *uci_tr, struct uci *uci);
	/**
	 * @detach: Callback invoked when the transport channel is detached. All
	 * lent buffers should be released (using uci_packet_recv_free() and
	 * uci_packet_send_done()).
	 */
	void (*detach)(struct uci_transport *uci_tr);
	/**
	 * @packet_send_ready: Callback invoked when UCI wants to send a packet
	 * initially or after &uci_packet_get_ready() returned NULL. It allows to
	 * restart the sending data pump.
	 */
	void (*packet_send_ready)(struct uci_transport *uci_tr);
};

/**
 * struct uci_transport - UCI generic transport channel.
 */
struct uci_transport {
	/**
	 * @ops: Transport channel callbacks.
	 */
	const struct uci_transport_ops *ops;
};

/**
 * uci_transport_attach() - Attach a transport channel.
 * @uci: UCI context.
 * @uci_tr: Transport channel to attach.
 *
 * Return: UWBMAC_SUCCESS on success, or UWBMAC_EBUSY if a transport is already attached.
 */
uwbmac_error uci_transport_attach(struct uci *uci,
				  struct uci_transport *uci_tr);

/**
 * uci_transport_detach() - Detach a transport channel.
 * @uci: UCI context.
 *
 * Return: UWBMAC_SUCCESS on success, or UWBMAC_EINVAL if no transport is attached.
 */
uwbmac_error uci_transport_detach(struct uci *uci);

/**
 * uci_send_message() - Send a message.
 * @uci: UCI context.
 * @mt_gid_oid: Union of message type, group identifier and opcode identifier.
 *              Use the UCI_MT_GID_OID() macro.
 * @payload: Message payload which can be composed of several buffer blocks, or NULL if there is none.
 *           Ownership is transferred to UCI core.
 *
 * The message payload is segmented, packets are built and queued to be sent. If
 * you use the UCI message API, this is done efficiently as room is reserved
 * during message construction for headers.
 */
void uci_send_message(struct uci *uci, uint16_t mt_gid_oid,
		      struct uci_blk *payload);

/**
 * uci_send_status() - Send a status response.
 * @uci: UCI context.
 * @gid_oid: Union of group identifier and opcode identifier. Use the
 *           UCI_GID_OID() macro, message type is ignored.
 * @status: Status code.
 */
void uci_send_status(struct uci *uci, uint16_t gid_oid, uint8_t status);

/**
 * uci_packet_send_get_ready() - Retrieve the packet ready to be sent.
 * @uci: UCI context.
 *
 * Packet buffers are lent to the transport channel, they should be returned
 * once transmission is done using uci_packet_send_done().
 *
 * If there is no packet to send, NULL is returned and UCI core will signal any
 * new pending packet using the &uci_tr_ops.packet_send_ready callback.
 *
 * Return: The first block of the packet to send, or NULL if no packet to send.
 */
struct uci_blk *uci_packet_send_get_ready(struct uci *uci);

/**
 * uci_packet_send_done() - Signal the packet has been transmitted.
 * @uci: UCI context.
 * @packet: Packet that was sent, or that failed to be sent.
 * @status: 0 if transmission was successful, or a negative error code.
 *
 * Once a packet has been transmitted, or after a non recoverable failure, packet
 * should be returned by the transport channel using this function
 * (see &uci_packet_get_ready and &uci_tr_ops.packet_send_ready).
 */
void uci_packet_send_done(struct uci *uci, struct uci_blk *packet, int status);

//TODO(Guillaume): not implemented
/**
 * uci_packet_response_expire() - Signal the response timer expired.
 * @uci: UCI context.
 *
 * This is only needed for UCI client.
 *
 * The transport channel should reset a timer each time a packet is sent on the
 * transport channel. If the timer expires, it should call this function. This
 * is used to identify problem with a missing response.
 *
 * The timer expiration will be ignored if UCI core has a pending packet which
 * was not given back using uci_packet_send_done().
 *
 * The timer duration is specific to the transport channel.
 */
void uci_packet_response_expire(struct uci *uci);

/**
 * struct uci - UCI core context.
 */
struct uci {
	/**
	 * @allocator: Allocator to use to allocate buffer blocks.
	 */
	struct uci_allocator *allocator;
	/**
	 * @handlers_head: Head of the list of handlers.
	 */
	struct uci_message_handlers *handlers_head;
	/**
	 * @rx: Used to collect segmented messages before processing.
	 */
	struct uci_blk *rx;
	/**
	 * @rx_last: Last received segmented message before processing.
	 */
	struct uci_blk *rx_last;
	/**
	 * @tx: Pointer to first message to send of TX queue.
	 */
	struct uci_blk *tx;
	/**
	 * @tx_last: Pointer to last message to send of TX queue.
	 */
	struct uci_blk *tx_last;
	/**
	 * @tr: Transport channel attached to this context or NULL.
	 */
	struct uci_transport *tr;
	/**
	 * @status_blk: Buffer block reserved for status signaling. This should
	 * not fail to signal error status so it does not depend on dynamic allocation.
	 */
	struct uci_blk status_blk;
	/**
	 * @status_data: Data for error buffer block.
	 */
	uint8_t status_data[UCI_STATUS_PACKET_SIZE];
	/**
	 * @is_client: True if this is a client.
	 */
	bool is_client;
	/**
	 * @known_gid: Bitfield of known GID in message handlers list.
	 */
	uint16_t known_gid;
};

#endif /* UCI_H */
