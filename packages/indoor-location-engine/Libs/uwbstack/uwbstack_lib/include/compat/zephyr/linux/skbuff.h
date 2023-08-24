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

#ifndef COMPAT_LINUX_SKBUFF_H
#define COMPAT_LINUX_SKBUFF_H

#include <assert.h>
#include <linux/slab.h>
#include <linux/types.h>

#include "uwbmac/uwbmac_buf.h"

struct sk_buff_head {
	struct uwbmac_buf_queue queue;
};

struct sk_buff {
	union {
		struct uwbmac_buf buf;
		struct {
			struct net_buf *__frags;
			uint8_t __ref;
			uint8_t __flags;
			uint8_t __pool_id;
			uint8_t __user_data_size;
			uint8_t *data;
			uint16_t len;
			uint16_t __size;
			uint8_t *__buf;
			/* All the fields above this line must match the layout
			 * of Zephyr's 'struct net_buf'.
			 */
			struct net_buf *frag_next;
			uint16_t queue_mapping;
			uint16_t data_len;
			/*
			 * cb[] field must be aligned on an 8 byte boundary on 32- and 64-bit architectures.
			 */
			uint16_t padding[2];

			uint8_t cb[];
		};
	};
};
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(offsetof(struct sk_buff, cb) % 8 == 0,
	       "cb[] field must be 8 bytes aligned!");
#endif

static inline struct sk_buff *alloc_skb(unsigned int size, gfp_t flags)
{
	return (struct sk_buff *)uwbmac_buf_alloc(size);
}

static inline void kfree_skb(struct sk_buff *skb)
{
	if (skb)
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
	return net_buf_tail(&skb->buf.netbuf);
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
	/* TODO: This function should do a simple memcpy to match linux'
	 * implementation. */
	uint8_t *tail =
		uwbmac_buf_get_data(&skb->buf) + uwbmac_buf_get_len(&skb->buf);
	uwbmac_buf_put_data(&skb->buf, data, len);
	return tail;
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
	return net_buf_pull(&skb->buf.netbuf, len);
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
	return (struct sk_buff *)skb->buf.netbuf.node.next;
}

static inline bool skb_queue_is_last(const struct sk_buff_head *list,
				     const struct sk_buff *skb)
{
	return &skb->buf.netbuf.node == list->queue.slist.tail;
}

static inline int skb_zerocopy(struct sk_buff *to, struct sk_buff *from,
			       int len, int hlen)
{
	struct sk_buff *last_frag = to;

	while (last_frag->frag_next) {
		last_frag = (struct sk_buff *)last_frag->frag_next;
	}

	last_frag->frag_next = net_buf_ref(&from->buf.netbuf);
	to->data_len += from->len + from->data_len;
	USER_DATA(last_frag->frag_next)->frag_next = NULL;

	return 0;
}
#endif /* COMPAT_LINUX_SKBUFF_H */
