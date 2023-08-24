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

#ifndef COMPAT_NET_NETLINK_H
#define COMPAT_NET_NETLINK_H

#include <linux/skbuff.h>
#include <linux/types.h>
#include <asm/unaligned.h>
#include <string.h>

#define NLE_SUCCESS 0
#define NLE_INVAL 7
#define NLE_RANGE 8
#define NLE_MSGSIZE 9

#define NLMSG_DEFAULT_SIZE 3072

#define NLA_HDRLEN (sizeof(struct nlattr))

struct nlattr {
	u16 nla_len;
	u16 nla_type;
};

struct netlink_ext_ack {
};

struct netlink_range_validation {
	u64 min, max;
};

enum nla_policy_validation {
	NLA_VALIDATE_NONE,
	NLA_VALIDATE_RANGE,
	NLA_VALIDATE_MIN,
	NLA_VALIDATE_MAX,
	NLA_VALIDATE_RANGE_PTR,
};

struct nla_policy {
	const struct nla_policy *nested_policy;
	u16 len;
	union {
		struct {
			s16 min, max;
		};
		struct netlink_range_validation *range;
	};
	u8 type;
	u8 validation_type;
};

enum { NLA_UNSPEC,
       NLA_U8,
       NLA_U16,
       NLA_U32,
       NLA_U64,
       NLA_STRING,
       NLA_FLAG,
       NLA_NESTED,
       NLA_NESTED_ARRAY,
       NLA_BINARY,
       NLA_S8,
       NLA_S16,
       NLA_S32,
       NLA_S64,
       __NLA_TYPE_MAX,
};

#define NLA_TYPE_MAX (__NLA_TYPE_MAX - 1)

#ifndef NLA_ALIGNTO
#define NLA_ALIGNTO 4
#endif
#define NLA_ALIGN(len) (((len) + NLA_ALIGNTO - 1) & ~(NLA_ALIGNTO - 1))
#define NLA_HEADER_LEN ((int)NLA_ALIGN(sizeof(struct nlattr)))

#define NLA_COMPAT_PADDING (0xFFFF)

#define NLA_F_NESTED (1 << 15)
#define NLA_F_NET_BYTEORDER (1 << 14)
#define NLA_TYPE_MASK ~(NLA_F_NESTED | NLA_F_NET_BYTEORDER)

#define NLA_POLICY_EXACT_LEN(_len)                                         \
	{                                                                  \
		.type = NLA_BINARY, .validation_type = NLA_VALIDATE_RANGE, \
		.min = (_len), .max = (_len)                               \
	}

#define NLA_POLICY_RANGE(tp, _min, _max)                             \
	{                                                            \
		.type = (tp), .validation_type = NLA_VALIDATE_RANGE, \
		.min = (_min), .max = (_max),                        \
	}

#define NLA_POLICY_FULL_RANGE(tp, _range)                                \
	{                                                                \
		.type = (tp), .validation_type = NLA_VALIDATE_RANGE_PTR, \
		.range = (_range),                                       \
	}

#define NLA_POLICY_MIN(tp, _min)                                   \
	{                                                          \
		.type = (tp), .validation_type = NLA_VALIDATE_MIN, \
		.min = (_min),                                     \
	}

#define NLA_POLICY_MAX(tp, _max)                                   \
	{                                                          \
		.type = (tp), .validation_type = NLA_VALIDATE_MAX, \
		.max = (_max),                                     \
	}

#define nla_for_each_attr(pos, head, len, rem)        \
	for (pos = head, rem = len; nla_ok(pos, rem); \
	     pos = nla_next(pos, &(rem)))

#define nla_for_each_nested(pos, nla, rem) \
	nla_for_each_attr (pos, nla_data(nla), nla_len(nla), rem)

struct nlattr *nla_nest_start(struct sk_buff *skb, int attrtype);
int nla_nest_end(struct sk_buff *skb, struct nlattr *start);
int nla_put(struct sk_buff *skb, int attrtype, int attrlen, const void *data);
int __nla_parse_nested(const struct nlattr *head, int len, int maxtype,
		       const struct nla_policy *policy, struct nlattr **tb);
int nla_parse_nested(struct nlattr *tb[], int maxtype, const struct nlattr *nla,
		     const struct nla_policy *policy,
		     struct netlink_ext_ack *extack);

static inline int nla_attr_size(int payload)
{
	return NLA_HEADER_LEN + payload;
}

static inline int nla_total_size(int payload)
{
	return NLA_ALIGN(nla_attr_size(payload));
}

static inline int nla_padlen(int payload)
{
	return nla_total_size(payload) - nla_attr_size(payload);
}

static inline struct nlattr *nla_next(const struct nlattr *nla, int *remaining)
{
	unsigned int totlen = NLA_ALIGN(nla->nla_len);

	*remaining -= totlen;
	return (struct nlattr *)((char *)nla + totlen);
}

static inline void *nla_data(const struct nlattr *nla)
{
	return (char *)nla + NLA_HEADER_LEN;
}

static inline int nla_len(const struct nlattr *nla)
{
	return nla->nla_len - NLA_HEADER_LEN;
}

static inline int nla_ok(const struct nlattr *nla, int remaining)
{
	return remaining >= (int)sizeof(*nla) && nla->nla_len >= sizeof(*nla) &&
	       nla->nla_len <= remaining;
}

static inline int nla_type(const struct nlattr *nla)
{
	return nla->nla_type & NLA_TYPE_MASK;
}

static inline const char *nla_get_string(const struct nlattr *nla)
{
	return (const char *)nla_data(nla);
}

static inline int nla_put_string(struct sk_buff *skb, int attrtype,
				 const char *str)
{
	return nla_put(skb, attrtype, strlen(str) + 1, str);
}

static inline int nla_strcmp(const struct nlattr *nla, const char *str)
{
	char *data = (char *)nla_data(nla);

	return strncmp(str, data, strlen(str));
}

static inline s64 nla_get_s64(const struct nlattr *nla)
{
#if NLA_ALIGNTO >= 8
	return *(s64 *)nla_data(nla);
#else
	s64 tmp;
	memcpy(&tmp, nla_data(nla), sizeof(s64));
	return tmp;
#endif
}

static inline int nla_put_s64(struct sk_buff *skb, int attrtype, s64 value)
{
	return nla_put(skb, attrtype, sizeof(s64), &value);
}

static inline s32 nla_get_s32(const struct nlattr *nla)
{
#if NLA_ALIGNTO >= 4
	return *(s32 *)nla_data(nla);
#else
	s32 tmp;
	memcpy(&tmp, nla_data(nla), sizeof(s32));
	return tmp;
#endif
}

static inline int nla_put_s32(struct sk_buff *skb, int attrtype, s32 value)
{
	return nla_put(skb, attrtype, sizeof(s32), &value);
}

static inline s16 nla_get_s16(const struct nlattr *nla)
{
#if NLA_ALIGNTO >= 2
	return *(s16 *)nla_data(nla);
#else
	s16 tmp;
	memcpy(&tmp, nla_data(nla), sizeof(s16));
	return tmp;
#endif
}

static inline int nla_put_s16(struct sk_buff *skb, int attrtype, s16 value)
{
	return nla_put(skb, attrtype, sizeof(s16), &value);
}

static inline s8 nla_get_s8(const struct nlattr *nla)
{
	return *(s8 *)nla_data(nla);
}

static inline int nla_put_s8(struct sk_buff *skb, int attrtype, s8 value)
{
	return nla_put(skb, attrtype, sizeof(s8), &value);
}

static inline u64 nla_get_u64(const struct nlattr *nla)
{
#if NLA_ALIGNTO >= 8
	return *(u64 *)nla_data(nla);
#else
	s64 tmp;
	memcpy(&tmp, nla_data(nla), sizeof(s64));
	return tmp;
#endif
}

static inline int nla_put_u64(struct sk_buff *skb, int attrtype, u64 value)
{
	return nla_put(skb, attrtype, sizeof(u64), &value);
}

static inline u32 nla_get_u32(const struct nlattr *nla)
{
#if NLA_ALIGNTO >= 4
	return *(u32 *)nla_data(nla);
#else
	u32 tmp;
	memcpy(&tmp, nla_data(nla), sizeof(u32));
	return tmp;
#endif
}

static inline int nla_put_u32(struct sk_buff *skb, int attrtype, u32 value)
{
	return nla_put(skb, attrtype, sizeof(u32), &value);
}

static inline u16 nla_get_u16(const struct nlattr *nla)
{
#if NLA_ALIGNTO >= 2
	return *(u16 *)nla_data(nla);
#else
	u16 tmp;
	memcpy(&tmp, nla_data(nla), sizeof(u16));
	return tmp;
#endif
}

static inline __le16 nla_get_le16(const struct nlattr *nla)
{
	return (__le16)get_unaligned_le16(nla_data(nla));
}

static inline int nla_put_u16(struct sk_buff *skb, int attrtype, u16 value)
{
	return nla_put(skb, attrtype, sizeof(u16), &value);
}

static inline int nla_put_le16(struct sk_buff *skb, int attrtype, __le16 value)
{
	return nla_put(skb, attrtype, sizeof(__le16), &value);
}

static inline u8 nla_get_u8(const struct nlattr *nla)
{
	return *(u8 *)nla_data(nla);
}

static inline int nla_put_u8(struct sk_buff *skb, int attrtype, u8 value)
{
	return nla_put(skb, attrtype, sizeof(u8), &value);
}

static inline int nla_get_flag(const struct nlattr *nla)
{
	return !!nla;
}

static inline int nla_put_flag(struct sk_buff *skb, int attrtype)
{
	return nla_put(skb, attrtype, 0, NULL);
}

#endif /* COMPAT_NET_NETLINK_H */
