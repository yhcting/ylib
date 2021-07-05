/******************************************************************************
 * Copyright (C) 2011, 2012, 2013, 2014, 2015
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

#include <errno.h>
#include <string.h>

#include "common.h"
#include "ylru.h"
#include "ylistl.h"
#include "yhash.h"

/*
 * Design
 *
 *   Hash
 * +-------+
 * |       |
 * +-------+  <hash value>
 * |       |-------+
 * +-------+       |
 * |  ...  |       v   Doubly Linked List
 *               +---+   +---+   +---+
 *     ^     ... -> <---->  <---->  <- ...
 *     |         +---+   +---+   +---+
 *     |                           |
 *     |  <access with key>        |
 *     +---------------------------+
 */

struct ylru {
	/* members that may be different according to hash contents */
	struct ylistl_link head; /* linked list */
	struct yhash *h;
	u32 sz;
	/* contents-independent members (hash attributes)
	 * 'maxsz' SHOULD be top of 'hash attributes'
	 */
	u32 maxsz;
	void (*dfree)(void *);
	void *(*dcreate)(const void *key);
	u32 (*dsize)(const void *);
};

/* list node */
struct lnode {
	struct ylistl_link lk;
	/* point key memory in internal hash - shallow copy */
	const void *key;
	void *data; /* cached data */
	struct ylru *lru; /* owner lru */
};

static INLINE void
lru_free_default(void *v) {
        if (likely(v))
                yfree(v);
}

static INLINE u32
data_size(struct ylru *l, void *d) {
	return (l->dsize)? (*l->dsize)(d): 1;
}

static INLINE void
data_free(struct ylru *l, void *d) {
	if (l->dfree)
		(*l->dfree)(d);
}

static INLINE void
lnode_free(struct lnode *n) {
	data_free(n->lru, n->data);
	yfree(n);
}

struct ylru *
lru_create(
	struct yhash *h, /* hash used in lru cache */
	u32 maxsz,
	void (*datafree)(void *),
	void *(*datacreate)(const void *key),
	u32 (*datasize)(const void *)
) {
	struct ylru *lru = ymalloc(sizeof(*lru));
	if (unlikely(!lru))
		return NULL;

	if (!maxsz)
		maxsz = 0xffffffff; /* maximum unsigned u32 */
	if (YLRU_PREDEFINED_FREE == datafree)
		datafree = &lru_free_default;

	lru->h = h;
	lru->sz = 0;
	ylistl_init_link(&lru->head);

	lru->maxsz = maxsz;
	lru->dfree = datafree;
	lru->dcreate = datacreate;
	lru->dsize = datasize;

	return lru;

}

/****************************************************************************
 *
 *
 *
 ****************************************************************************/
struct ylru *
ylrui_create(
	u32 maxsz,
	void (*datafree)(void *),
	void *(*datacreate)(const void *key),
	u32 (*datasize)(const void *)
) {
	struct ylru *l;
	struct yhash *h = yhashi_create((void(*)(void *))&lnode_free);
	if (unlikely(!h))
		return NULL;
	if (unlikely(!(l = lru_create(
		h, maxsz, datafree, datacreate, datasize)))
	) { yhash_destroy(h); }
	return l;
}

struct ylru *
ylrus_create(
	u32 maxsz,
	void (*datafree)(void *),
	void *(*datacreate)(const void *key),
	u32 (*datasize)(const void *)
) {
	struct ylru *l;
	struct yhash *h = yhashs_create((void(*)(void *))&lnode_free, TRUE);
	if (unlikely(!h))
		return NULL;
	if (unlikely(!(l = lru_create(
		h, maxsz, datafree, datacreate, datasize)))
	) { yhash_destroy(h); }
	return l;
}

struct ylru *
ylruo_create(
	u32 maxsz,
	void (*datafree)(void *),
	void *(*datacreate)(const void *key),
	u32 (*datasize)(const void *),
	void(*keyfree)(void *),
	int (*keycopy)(void **newkey, const void *),
	int (*keycmp)(const void *, const void *),
	u32 (*hfunc)(const void *key)
) {
	struct ylru *l;
	struct yhash *h;

	if (YLRU_PREDEFINED_FREE == keyfree)
		keyfree = YHASH_PREDEFINED_FREE;

	h = yhasho_create(
		(void(*)(void *))&lnode_free,
		keyfree,
		keycopy,
		keycmp,
		hfunc);
	if (unlikely(!h))
		return NULL;
	if (unlikely(!(l = lru_create(
		h, maxsz, datafree, datacreate, datasize)))
	) { yhash_destroy(h); }
	return l;
}

struct ylru *
ylru_create(const struct ylru *lru) {
	struct ylru *l;
	struct yhash *h;
	if (unlikely(!lru))
		return NULL;
	h = yhash_create(lru->h);
	if (unlikely(!h))
		return NULL;
	if (unlikely(!(l = lru_create(
		h, lru->maxsz, lru->dfree, lru->dcreate, lru->dsize)))
	) { yhash_destroy(h); }
	return l;
}

void
ylru_reset(struct ylru *lru) {
	if (unlikely(!lru))
		return;
	yhash_reset(lru->h);
	/* list node is already destroied in yhash_clean */
	ylistl_init_link(&lru->head);
	lru->sz = 0;
}

void
ylru_destroy(struct ylru *lru) {
	if (unlikely(!lru))
		return;
	yhash_destroy(lru->h);
	/* list node is already destroied in yhash_destroy */
	yfree(lru);
}

int
ylru_put(struct ylru *lru, const void *key, void *data) {
	struct lnode *n, *tmp;
	u32 dsz;
	if (unlikely(!lru))
		return -EINVAL;
	dsz = data_size(lru, data);
	if (unlikely(dsz > lru->maxsz))
		/* too large data to be in the cache */
		return -EINVAL;

	/*
	 * shrink cache if cache becomes too large
	 * the 'last' in the list is the 'oldest'.
	 */
	ylistl_foreach_item_removal_safe_backward(
		n, tmp, &lru->head, struct lnode, lk
	) {
		if (unlikely(lru->sz + dsz > lru->maxsz)) {
			ylistl_remove(&n->lk);
			lru->sz -= data_size(lru, n->data);
			yhash_remove(lru->h, n->key);
		} else
			break;
	}
	if (unlikely(!(n = ymalloc(sizeof(*n)))))
		return -ENOMEM;
	n->data = data;
	n->lru = lru;
	if (unlikely(-1 == yhash_add3(lru->h, &n->key, (void *)key, n, TRUE))) {
		yfree(n);
		return -ENOMEM;
	}

	/* put at the first (newlest) */
	ylistl_add_first(&lru->head, &n->lk);
	lru->sz += dsz;
	return 0;
}

int
ylru_get(struct ylru *lru, void **data, const void *key) {
	struct lnode *n;
	int  r = 0; /* See comment for 'return' at ylru.h */
	void *nd = NULL;
	if (unlikely(!data))
		return -EINVAL;
	if (0 < yhash_remove2(lru->h, (void **)&n, key)) {
		/* found */
		ylistl_remove(&n->lk);
		lru->sz -= data_size(lru, n->data);
		nd = n->data;
		/* free only 'node' structure. */
		yfree(n);
	} else {
		/* Fail to find in the cache */
		if (lru->dcreate)
			nd = lru->dcreate(key);
		else
			r = 1;
	}
	if (!r)
		*data = nd;
	return r;
}

u32
ylru_sz(struct ylru *lru) {
	return lru->sz;
}
