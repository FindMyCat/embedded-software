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

#ifndef UWBMAC_BUF_NETBUF_H
#define UWBMAC_BUF_NETBUF_H

#include <assert.h>
#include <uwbmac/uwbmac_types.h>
#include <net/buf.h>

#define USER_DATA(x) ((struct uwbmac_buf_userdata *)net_buf_user_data(x))

struct uwbmac_buf_queue {
	/**
	 * @slist: single-linked-list
	 */
	sys_slist_t slist;
};

/**
 * struct uwbmac_buf - Network buffer, used to exchange frames.
 */
struct uwbmac_buf {
	/**
	 * @netbuf: network buffer
	 */
	struct net_buf netbuf;
};

/**
 * struct uwbmac_buf_userdata - User data fields available in uwbmac_buf
 */
struct uwbmac_buf_userdata {
	/**
	 * @frag_next: Fragment next pointer.
	 */
	struct net_buf *frag_next;
	/**
	 * @queue_mapping: Queue id, used by regions supporting multiple queues.
	 */
	uint16_t queue_mapping;
	/**
	 * @data_len: Length of fragments attached to this buffer.
	 *  Same name as Linux sk_buff variable.
	 */
	uint16_t data_len;
	/**
	 * @padding: Extra unused padding used to align cb field align.
	 *
	 * `struct uwbmac_buf_userdata` appears as the payload of Zephyr's `struct net_buf`, which size is
	 * 20 bytes on 32-bit platforms and 40 bytes on 64-bit platforms.
	 * In order to enforce an 8-byte alignment of our own payload, we need
	 * to introduce either 4 bytes if 32-bit or 8 bytes if 64-bit, including
	 * the data_len and queue_mapping fields above.
	 */
	uint16_t padding[2];
	/**
	 * @cb: Control block, used to store related data, depending on the
	 * layer owning the buffer.
	 *
	 * WARNING: Field cb must be aligned on 8-bytes, which is only
	 * guaranteed on a 32-bit platform here.
	 */
	uint8_t cb[UWBMAC_BUF_CB_SIZE];
};
#ifdef __LP64__
#define ZEPHYR_NET_BUF_STRUCT_SIZE 40
#else
#define ZEPHYR_NET_BUF_STRUCT_SIZE 20
#endif
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert((ZEPHYR_NET_BUF_STRUCT_SIZE +
		offsetof(struct uwbmac_buf_userdata, cb)) %
			       8 ==
		       0,
	       "cb[] field must be 8 bytes aligned!");
#endif

struct uwbmac_buf *uwbmac_buf_alloc(unsigned int size);
void uwbmac_buf_free(struct uwbmac_buf *buf);

int uwbmac_buf_put_data(struct uwbmac_buf *buf, const void *data,
			unsigned int len);

static inline void uwbmac_buf_reserve(struct uwbmac_buf *buf, unsigned int len)
{
	net_buf_reserve(&buf->netbuf, len);
}

static inline unsigned int uwbmac_buf_headroom(const struct uwbmac_buf *buf)
{
	return net_buf_headroom(&((struct uwbmac_buf *)buf)->netbuf);
}

static inline unsigned int uwbmac_buf_tailroom(const struct uwbmac_buf *buf)
{
	return net_buf_tailroom(&((struct uwbmac_buf *)buf)->netbuf);
}

static inline void uwbmac_buf_reserve_tailroom(struct uwbmac_buf *buf,
					       unsigned int len)
{
	buf->netbuf.size -= len;
}

static inline void uwbmac_buf_trim(struct uwbmac_buf *buf, unsigned int len)
{
	struct net_buf *frag = USER_DATA(&buf->netbuf)->frag_next;

	if (buf->netbuf.len > len) {
		buf->netbuf.len = len;
	}
	/* TODO: Only remove fragments that must be trimmed. */
	while (frag) {
		struct net_buf *frag_next = USER_DATA(frag)->frag_next;
		USER_DATA(frag)->frag_next = NULL;
		net_buf_unref(frag);
		frag = frag_next;
	}
	USER_DATA(&buf->netbuf)->frag_next = NULL;
	USER_DATA(&buf->netbuf)->data_len = 0;
}

static inline void *uwbmac_buf_put(struct uwbmac_buf *buf, unsigned int len)
{
	return net_buf_add(&buf->netbuf, len);
}

static inline void uwbmac_buf_put_u8(struct uwbmac_buf *buf, uint8_t data)
{
	net_buf_add_u8(&buf->netbuf, data);
}

static inline void *uwbmac_buf_push(struct uwbmac_buf *buf, unsigned int len)
{
	return net_buf_push(&buf->netbuf, len);
}

static inline void uwbmac_buf_pull(struct uwbmac_buf *buf, unsigned int len)
{
	(void)net_buf_pull(&buf->netbuf, len);
}

static inline void uwbmac_buf_queue_init(struct uwbmac_buf_queue *queue)
{
	sys_slist_init(&queue->slist);
}

static inline bool uwbmac_buf_queue_empty(const struct uwbmac_buf_queue *queue)
{
	return sys_slist_is_empty((sys_slist_t *)&queue->slist);
}

static inline void uwbmac_buf_queue_push(struct uwbmac_buf_queue *queue,
					 struct uwbmac_buf *buf)
{
	sys_slist_prepend(&queue->slist, &buf->netbuf.node);
}

static inline void uwbmac_buf_queue_put(struct uwbmac_buf_queue *queue,
					struct uwbmac_buf *buf)
{
	net_buf_slist_put(&queue->slist, &buf->netbuf);
}

static inline struct uwbmac_buf *
uwbmac_buf_queue_peek(struct uwbmac_buf_queue *queue)
{
	return (struct uwbmac_buf *)sys_slist_peek_head(&queue->slist);
}

static inline struct uwbmac_buf *
uwbmac_buf_queue_pop(struct uwbmac_buf_queue *queue)
{
	return (struct uwbmac_buf *)net_buf_slist_get(&queue->slist);
}

static inline void uwbmac_buf_queue_purge(struct uwbmac_buf_queue *queue)
{
	struct uwbmac_buf *buf;
	while ((buf = uwbmac_buf_queue_pop(queue)))
		uwbmac_buf_free(buf);
}

static inline uint8_t *uwbmac_buf_get_data(struct uwbmac_buf *buf)
{
	return buf->netbuf.data;
}

static inline struct uwbmac_buf *
uwbmac_buf_get_next_frag(struct uwbmac_buf *buf)
{
	return (struct uwbmac_buf *)USER_DATA(&buf->netbuf)->frag_next;
}

static inline unsigned int uwbmac_buf_get_len(struct uwbmac_buf *buf)
{
	return (unsigned int)buf->netbuf.len;
}

static inline unsigned int uwbmac_buf_get_total_len(struct uwbmac_buf *buf)
{
	return (unsigned int)USER_DATA(&buf->netbuf)->data_len +
	       buf->netbuf.len;
}

static inline unsigned int uwbmac_buf_get_size(struct uwbmac_buf *buf)
{
	uint16_t cnt = 0;
	struct net_buf *cur = &buf->netbuf;

	while (cur) {
		cnt++;
		cur = USER_DATA(cur)->frag_next;
	}
	/* All net_buf have the same size. */
	return (unsigned int)(cnt * buf->netbuf.size);
}

static inline void uwbmac_buf_set_queue_mapping(struct uwbmac_buf *buf,
						uint16_t value)
{
	USER_DATA(&buf->netbuf)->queue_mapping = value;
}

#endif /* UWBMAC_BUF_NETBUF_H */
