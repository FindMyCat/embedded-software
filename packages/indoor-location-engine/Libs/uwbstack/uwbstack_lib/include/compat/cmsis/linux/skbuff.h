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

#ifndef COMPAT_LINUX_SKBUFF_H
#define COMPAT_LINUX_SKBUFF_H

#include <assert.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/errno.h>

#include "uwbmac/uwbmac_buf.h"
#include "uwbmac_error.h"

struct sk_buff_head {
	struct uwbmac_buf_queue queue;
};

struct sk_buff {
	union {
		struct uwbmac_buf buf;
		/* Linux is not using typedefs, so no better solution than to
		 * keep both structures synchronized. */
		struct {
			struct uwbmac_buf *next;
			struct uwbmac_buf *prev;
			uint8_t *data;
			unsigned int len;
			u16 queue_mapping;
			u32 reserved_tailroom;
			uint8_t *head;
			uint8_t *end;
			char cb[UWBMAC_BUF_CB_SIZE]
				__attribute__((__aligned__(8)));
		};
	};
};

static inline struct sk_buff *alloc_skb(unsigned int size, gfp_t flags)
{
	return (struct sk_buff *)uwbmac_buf_alloc(size);
}

static inline void kfree_skb(struct sk_buff *skb)
{
	uwbmac_buf_free(&skb->buf);
}

static inline void skb_reserve(struct sk_buff *skb, unsigned int len)
{
	uwbmac_buf_reserve(&skb->buf, len);
}

static inline unsigned int skb_headroom(const struct sk_buff *skb)
{
	return uwbmac_buf_headroom(&skb->buf);
}

static inline int skb_tailroom(const struct sk_buff *skb)
{
	return uwbmac_buf_tailroom(&skb->buf);
}

static inline unsigned char *skb_tail_pointer(struct sk_buff *skb)
{
	return skb->data + skb->len;
}

static inline void skb_trim(struct sk_buff *skb, unsigned int len)
{
	uwbmac_buf_trim(&skb->buf, len);
}

static inline void *skb_put(struct sk_buff *skb, unsigned int len)
{
	return uwbmac_buf_put(&skb->buf, len);
}

static inline void *skb_put_data(struct sk_buff *skb, const void *data,
				 unsigned int len)
{
	u8 *tmp = skb->data + skb->len;

	uwbmac_buf_put_data(&skb->buf, data, len);
	return tmp;
}

static inline void skb_put_u8(struct sk_buff *skb, u8 v)
{
	uwbmac_buf_put_u8(&skb->buf, v);
}

static inline void *skb_push(struct sk_buff *skb, unsigned int len)
{
	return uwbmac_buf_push(&skb->buf, len);
}

static inline void *skb_pull(struct sk_buff *skb, unsigned int len)
{
	if (len > skb->len)
		return NULL;
	uwbmac_buf_pull(&skb->buf, len);
	return skb->data;
}

static inline void skb_tailroom_reserve(struct sk_buff *skb, unsigned int mtu,
					unsigned int needed_tailroom)
{
	uwbmac_buf_reserve_tailroom(&skb->buf, needed_tailroom);
}

static inline int skb_availroom(const struct sk_buff *skb)
{
	return skb_tailroom(skb);
}

static inline void skb_queue_head_init(struct sk_buff_head *list)
{
	uwbmac_buf_queue_init(&list->queue);
}

static inline int skb_queue_empty(const struct sk_buff_head *list)
{
	return uwbmac_buf_queue_empty(&list->queue);
}

static inline struct sk_buff *skb_peek(struct sk_buff_head *list)
{
	return (struct sk_buff *)uwbmac_buf_queue_peek(&list->queue);
}

static inline void skb_queue_head(struct sk_buff_head *list,
				  struct sk_buff *newsk)
{
	uwbmac_buf_queue_push(&list->queue, &newsk->buf);
}

static inline void skb_queue_tail(struct sk_buff_head *list,
				  struct sk_buff *newsk)
{
	uwbmac_buf_queue_put(&list->queue, &newsk->buf);
}

static inline struct sk_buff *skb_dequeue(struct sk_buff_head *list)
{
	return (struct sk_buff *)uwbmac_buf_queue_pop(&list->queue);
}

static inline void skb_queue_purge(struct sk_buff_head *list)
{
	uwbmac_buf_queue_purge(&list->queue);
}

static inline struct sk_buff *skb_queue_next(const struct sk_buff_head *list,
					     const struct sk_buff *skb)
{
	return (struct sk_buff *)skb->buf.next;
}

static inline bool skb_queue_is_last(const struct sk_buff_head *list,
				     const struct sk_buff *skb)
{
	return skb->buf.next == (struct uwbmac_buf *)&list->queue;
}

static inline int skb_zerocopy(struct sk_buff *to, struct sk_buff *from,
			       int len, int hlen)
{
	if ((int)from->len > skb_availroom(to))
		return -UWBMAC_ENOBUFS;

	skb_put_data(to, from->data, from->len);

	return 0;
}
#endif /* COMPAT_LINUX_SKBUFF_H */
