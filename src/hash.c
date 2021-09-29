/******************************************************************************
 * Copyright (C) 2011, 2012, 2013, 2014, 2015, 2021
 * Younghyung Cho. <yhcting77@gmail.com>
 * All rights reserved.
 *
 * This file is part of ylib
 *
 * This program is licensed under the FreeBSD license
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * official policies, either expressed or implied, of the FreeBSD Project.
 *****************************************************************************/


#include <memory.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include "common.h"
#include "yhash.h"
#include "yhashl.h"
/* crc is used as hash function */
#include "ycrc.h"


#define MAX_HBITS 32
#define MIN_HBITS 4

/* hash node */
struct hn {
	struct yhashl_node hn;
	void *v; /* data value */
};

typedef int (*kcp_func_t)(const void **, const void *);
typedef void (*free_func_t)(void *);

/* order of struct member has meaning */
struct yhash {
	struct yhashl h;
	void (*vfree)(void *); /* free for hash value */
        void (*kfree)(void *); /* free for hash key */
        kcp_func_t kcp; /* copy hash key */
};


/****************************************************************************
 *
 * Predefined functions for hash
 *
 ****************************************************************************/
/*---------------------------------------------------------------------------
 * Common
 *--------------------------------------------------------------------------*/
static INLINE void
free_default(void *v) {
        if (likely(v))
                yfree(v);
}

static INLINE void
free_noop(void *v __unused) {
	return;
}

static INLINE int
keq_default(const void *s0, const void *s1) {
	return s0 == s1 ? 0 : 1;
}


static INLINE int
kcp_shallow(const void **out, const void *k) {
	*out = k;
	return 0;
}


static INLINE int
kcp_i(const void **ni, const void *i) {
        /* return address value itself */
	*ni = i;
        return 0;
}

static INLINE u32
hfunc_i(const void *k) {
        return (intptr_t)k & 0xffffffff;
}

/**
 * @return NULL for errors or '0' is returned.
 */
static int
kcp_s(const void **news, const void *s) {
	void *ns = ystrdup(s);
	if (unlikely(!ns))
		return -ENOMEM;
        *news = ns;
	return 0;
}

static INLINE void
kfree_s(void *s) {
        if (likely(s))
                yfree(s);
}

static INLINE int
keq_s(const void *s0, const void *s1) {
        return strcmp((const char *)s0, (const char *)s1);
}

static INLINE u32
hfunc_s(const void *k) {
        return ycrc32(0, (const u8 *)k, (u32)strlen((const char *)k) + 1);
}

/****************************************************************************
 *
 *
 *
 ****************************************************************************/
static INLINE u32
hmapsz(const struct yhash *h) {
	return yhashl_hmapsz(&h->h);
}

static INLINE u32
hv__(u32 mapbits, u32 hv32) {
	return hv32 >> (32 - mapbits);
}

static INLINE u32
hv_(const struct yhash *h, u32 hv32) {
	return hv__(h->h.mapbits, hv32);
}

static INLINE u32
hv(const struct yhash *h, const struct hn *n) {
	return hv_(h, n->hn.hv32);
}


/****************************************************************************
 *
 *
 *
 ****************************************************************************/
static free_func_t
hfree_func(free_func_t f) {
	return f ? YHASH_MEM_FREE == f
			? &free_default
			: f
		: &free_noop;
}

static void
hdestroy_nodes(struct yhash *h) {
	struct hn *n;
	struct yhashl_node *cur, *tmp;
	yhashl_foreach_safe(&h->h, cur, tmp) {
		n = containerof(cur, struct hn, hn);
		(*h->kfree)(yhashl_node_key(cur));
		(*h->vfree)(n->v);
		yhashl_node_remove(&h->h, cur);
		yfree(n);
	}
}

static struct yhash *
hash_create_internal(
	void (*vfree)(void *),
	void (*kfree)(void *),
	int (*kcp)(const void **, const void *),
	int (*keq)(const void *, const void *),
	u32 (*hfunc)(const void *)
) {
	int r;
	if (unlikely(!hfunc))
		return NULL;

	struct yhash *h = ymalloc(sizeof(*h));
	if (unlikely(!h))
		return NULL;
	r = yhashl_init2(&h->h, hfunc, keq, MIN_HBITS);
	if (unlikely(r)) {
		yfree(h);
		return NULL;
	}
        h->vfree = vfree;
        h->kfree = kfree;
        h->kcp = kcp;
	return h;
}

int
hash_set(
	struct yhash *h,
	const void ** const phkey,
	void *key,
	void **oldv,
	void *v
) {
	int r;
	struct hn *hn;
	void *key_copied;
	struct yhashl_node *hln;

	/* We need to expand hash map size if hash seems to be full */
	if (yhashl_sz(&h->h) > yhashl_hmapsz(&h->h) * 2)
		/* return value is ignored intentionally.
		 * Actually, even if hmodify fails, hash can still
		 *   continue to add new value.
		 */
		yhashl_hremap(&h->h, h->h.mapbits + 1);

	hn = ymalloc(sizeof(*hn));
	if (unlikely(!hn))
		return -ENOMEM;
	r = (*h->kcp)((const void **)&key_copied, key);
	if (unlikely(r)) {
		yfree(hn);
		return -ENOMEM;
	}
	hn->v = v;
	hln = yhashl_set(&h->h, key_copied, &hn->hn);
	if (phkey)
		*phkey = yhashl_node_key(&hn->hn);
	if (hln) {
		/* get replaced hash item */
		hn = containerof(hln, struct hn, hn);
		if (oldv) *oldv = hn->v;
		else (*h->vfree)(hn->v);
		(*h->kfree)(key_copied);
		yfree(hn);
	}
	return hln ? 0 : 1;
}

/****************************************************************************
 *
 *
 *
 ****************************************************************************/
struct yhash *
yhashi_create(void (*vfree)(void *)) {
        return hash_create_internal(
		hfree_func(vfree),
		&free_noop, /* kfree */
		&kcp_i, /* kcp */
		&keq_default, /* keq */
		&hfunc_i);
}

struct yhash *
yhashs_create(void (*vfree)(void *), bool key_deepcopy) {
        return hash_create_internal(
		hfree_func(vfree),
		&kfree_s, /* kfree */
		key_deepcopy ? &kcp_s : &kcp_shallow, /* kcp */
		&keq_s, /* keq */
		&hfunc_s);
}

struct yhash *
yhasho_create(
	void (*vfree)(void *),
	void (*keyfree)(void *),
	kcp_func_t keycopy,
	int (*keyeq)(const void *, const void *),
	u32 (*hfunc)(const void *key)
) {
	/* Invalid request */
	if (unlikely(!hfunc)) return NULL;
        return hash_create_internal(
		hfree_func(vfree),
		hfree_func(keyfree),
		keycopy ? keycopy : &kcp_shallow, /* kcp */
		keyeq ? keyeq : &keq_default, /* keq */
		hfunc);
}

/*---------------------------------------------------------------------------
 *
 *--------------------------------------------------------------------------*/
struct yhash *
yhash_create(const struct yhash *h) {
	return hash_create_internal(
		h->vfree,
		h->kfree,
		h->kcp,
		h->h.keq,
		h->h.h);
}

int
yhash_reset(struct yhash *h) {
	hdestroy_nodes(h);
	yhashl_clean(&h->h);
	return yhashl_init(&h->h, h->h.h, h->h.keq);
}

void
yhash_destroy(struct yhash *h) {
	hdestroy_nodes(h);
	yhashl_clean(&h->h);
	yfree(h);
}

u32
yhash_sz(const struct yhash *h) {
	return yhashl_sz(&h->h);
}

bool
yhash_is_sametype(const struct yhash *h0, const struct yhash *h1) {
	return h0->h.keq == h1->h.keq
		&& h0->h.h == h1->h.h
		&& h0->kcp == h1->kcp
		&& h0->kfree == h1->kfree
		&& h0->vfree == h1->vfree;
}

u32
yhash_keys(const struct yhash *h, const void **keysbuf, u32 bufsz) {
	struct yhashl_node *cur;
	u32 i = 0;
	yhashl_foreach(&h->h, cur) {
		if (i >= bufsz) goto done;
		keysbuf[i++] = yhashl_node_key(cur);
	}
 done:
	return i;
}

int
yhash_set(struct yhash *h, void *key, void *v) {
	return hash_set(h, NULL, key, NULL, v);
}

int
yhash_set2(struct yhash *h, void *key, void **oldv, void *v) {
	return hash_set(h, NULL, key, oldv, v);
}

int
yhash_set3(
	struct yhash *h,
	const void ** const phkey,
	void *key,
	void *v
) {
	return hash_set(h, phkey, key, NULL, v);
}

int
yhash_remove2(struct yhash *h, const void *key, void **value) {
	struct hn *hn;
	struct yhashl_node *hln = yhashl_remove(&h->h, key);
	if (!hln) return 0;

	hn = containerof(hln, struct hn, hn);
	if (value)
		*value = hn->v;
	else
		(*h->vfree)(hn->v);
	(*h->kfree)(yhashl_node_key(hln));
	yfree(hn);
	if (yhashl_sz(&h->h) < yhashl_hmapsz(&h->h) / 2)
		/* return value is ignored intentionally.
		 * Failure is not harmful.
		 */
		yhashl_hremap(&h->h, h->h.mapbits - 1);
	return 1;
}

int
yhash_remove(struct yhash *h, const void *key) {
	return yhash_remove2(h, key, NULL);
}

int
yhash_get(const struct yhash *h, const void *key, void **value) {
	struct yhashl_node *hln = yhashl_get(&h->h, key);
	if (likely(hln)) {
		if (likely(value))
			*value = containerof(hln, struct hn, hn)->v;
		return 0;
	} else return -ENOENT;
}
