/*
 * Copyright (c) 2022 Qorvo, Inc
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

#ifndef UWBMAC_BUF_MALLOC_H
#define UWBMAC_BUF_MALLOC_H

#include <uwbmac/uwbmac_types.h>
#include "uwbmac/uwbmac_buf.h"

struct uwbmac_buf_queue {
	/**
	 * @next: Pointer to the first buffer, or pointer to self if the queue
	 * is empty.
	 */
	struct uwbmac_buf *next;
	/**
	 * @prev: Pointer to the last buffer, or pointer to self if the queue is
	 * empty.
	 */
	struct uwbmac_buf *prev;
};

/**
 * struct uwbmac_buf - Network buffer, used to exchange frames.
 */
struct uwbmac_buf {
	/**
	 * @next: Pointer to the next buffer when used in a queue.
	 */
	struct uwbmac_buf *next;
	/**
	 * @prev: Pointer to the previous buffer when used in a queue.
	 */
	struct uwbmac_buf *prev;
	/**
	 * @data: Pointer to the start of data. There can be some room between
	 * the start of allocated data and this pointer.
	 */
	uint8_t *data;
	/**
	 * @len: Length of used data in the buffer.
	 */
	unsigned int len;
	/**
	 * @queue_mapping: Queue id, used by regions supporting multiple queues.
	 */
	uint16_t queue_mapping;
	/**
	 * @reserved_tailroom: Unused.
	 */
	uint32_t reserved_tailroom;
	/**
	 * @head: Pointer to the start of allocated buffer.
	 */
	uint8_t *head;
	/**
	 * @end: Pointer to the end of the allocated buffer.
	 */
	uint8_t *end;
	/**
	 * @cb: Control block, used to store related data, depending on the
	 * layer owning the buffer.
	 */
	char cb[UWBMAC_BUF_CB_SIZE] __attribute__((__aligned__(8)));
};

static inline void uwbmac_buf_reserve(struct uwbmac_buf *buf, unsigned int len)
{
	buf->data += len;
}

static inline unsigned int uwbmac_buf_headroom(const struct uwbmac_buf *buf)
{
	return buf->data - buf->head;
}

static inline unsigned int uwbmac_buf_tailroom(const struct uwbmac_buf *buf)
{
	return buf->end - (buf->data + buf->len);
}

static inline void uwbmac_buf_reserve_tailroom(struct uwbmac_buf *buf,
					       unsigned int len)
{
	buf->end -= len;
}

static inline void uwbmac_buf_queue_init(struct uwbmac_buf_queue *queue)
{
	queue->next = queue->prev = (struct uwbmac_buf *)queue;
}

static inline bool uwbmac_buf_queue_empty(const struct uwbmac_buf_queue *queue)
{
	return queue->next == (const struct uwbmac_buf *)queue;
}

static inline uint8_t *uwbmac_buf_get_data(struct uwbmac_buf *buf)
{
	return buf->data;
}

static inline struct uwbmac_buf *
uwbmac_buf_get_next_frag(struct uwbmac_buf *buf)
{
	/* No fragments on Linux. */
	return NULL;
}

static inline unsigned int uwbmac_buf_get_len(struct uwbmac_buf *buf)
{
	return buf->len;
}

static inline unsigned int uwbmac_buf_get_size(struct uwbmac_buf *buf)
{
	return buf->end - buf->head;
}

static inline unsigned int uwbmac_buf_get_total_len(struct uwbmac_buf *buf)
{
	/* No fragments on Linux. */
	return buf->len;
}

static inline void uwbmac_buf_set_queue_mapping(struct uwbmac_buf *buf,
						uint16_t value)
{
	buf->queue_mapping = value;
}

#endif /* UWBMAC_BUF_MALLOC_H */
