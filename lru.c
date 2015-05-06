/******************************************************************************
 * Copyright (C) 2011, 2012, 2013, 2014
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

#include "ycommon.h"
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
	struct ylistl_link head; /* linked list */
	struct yhash      *h;
	u32                maxsz;
	u32                sz;
	struct ylru_cb     cbs;
};

/* list node */
struct lnode {
	struct ylistl_link lk;
	/* point key memory in internal hash - shallow copy */
	const void        *key;
	u32                keysz;
	void              *data; /* cached data */
	u32                data_size;
	void             (*free)(void *); /* free cached data */
};

static inline void
lnode_free(struct lnode *n) {
	if (n->free)
		(*n->free)(n->data);
	yfree(n);
}

struct ylru *
ylru_create(u32 maxsz,
	    const struct ylru_cb *cbs) {
	struct ylru *lru = ymalloc(sizeof(*lru));
	if (unlikely(!lru))
		return NULL;
	lru->h = yhash_create((void(*)(void *))&lnode_free);
	if (unlikely(!lru->h)) {
		yfree(lru);
		return NULL;
	}

	/* Allocation is done - initialize it.*/
	ylistl_init_link(&lru->head);
	lru->maxsz = maxsz;
	memcpy(&lru->cbs, cbs, sizeof(*cbs));
	lru->sz = 0;
	return lru;
}

void
ylru_clean(struct ylru *lru) {
	yhash_clean(lru->h);
	/* list node is already destroied in yhash_clean */
	ylistl_init_link(&lru->head);
	lru->sz = 0;
}

void
ylru_destroy(struct ylru *lru) {
	yhash_destroy(lru->h);
	/* list node is already destroied in yhash_destroy */
	yfree(lru);
}

int
ylru_put(struct ylru *lru,
	 const void *key, u32 keysz,
	 void *data, u32 data_size) {
	struct lnode *n, *tmp;
	if (unlikely(data_size > lru->maxsz / 2))
		/* too large data to be in the cache */
		return EINVAL;

	/*
	 * shrink cache if cache becomes too large
	 * the 'last' in the list is the 'oldest'.
	 */
	ylistl_foreach_item_removal_safe_backward(n,
						  tmp,
						  &lru->head,
						  struct lnode,
						  lk) {
		if (unlikely(lru->sz + data_size > lru->maxsz)) {
			ylistl_del(&n->lk);
			lru->sz -= n->data_size;
			yhash_del(lru->h, n->key, n->keysz);
		} else
			break;
	}
	yassert(lru->sz >= 0);

	if (unlikely(!(n = ymalloc(sizeof(*n)))))
		return ENOMEM;
	n->keysz = keysz;
	n->data = data;
	n->data_size = data_size;
	n->free = lru->cbs.free;
	if (unlikely(-1 == yhash_add2(lru->h, &n->key, key, keysz, n))) {
		yfree(n);
		return ENOMEM;
	}

	/* put at the first (newlest) */
	ylistl_add_first(&lru->head, &n->lk);
	lru->sz += data_size;
	return 0;
}

void *
ylru_get(struct ylru *lru,
	 u32 *data_size, /* returned data size */
	 const void *key, u32 keysz) {
	struct lnode *n;
	u32 sztmp = 0;
	void *data = NULL;
	if (0 < yhash_del2(lru->h, (void **)&n, key, keysz)) {
		/* found */
		ylistl_del(&n->lk);
		lru->sz -= n->data_size;
		yassert(lru->sz >= 0);
		sztmp = n->data_size;
		data = n->data;
		/* free only 'node' structure. */
		yfree(n);
	} else {
		/* Fail to find in the cache */
		if (lru->cbs.create)
			data = lru->cbs.create(&sztmp,
					       key,
					       keysz);
	}

	if (likely(data_size))
		*data_size = sztmp;

	return data;
}

u32
ylru_sz(struct ylru *lru) {
	return lru->sz;
}
