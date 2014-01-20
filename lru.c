/******************************************************************************
 *    Copyright (C) 2011, 2012, 2013, 2014
 *    Younghyung Cho. <yhcting77@gmail.com>
 *
 *    This file is part of ylib
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as
 *    published by the Free Software Foundation either version 3 of the
 *    License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License
 *    (<http://www.gnu.org/licenses/lgpl.html>) for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.	If not, see <http://www.gnu.org/licenses/>.
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
	struct yhash *h;
	unsigned int maxsz;
	unsigned int sz;
	struct ylru_cb cbs;
};

/* list node */
struct lnode {
	struct ylistl_link lk;
	const void *key; /* point key memory in internal hash - shallow copy */
	unsigned int keysz;
	void *data; /* cached data */
	unsigned int data_size;
	void (*free)(void *); /* free cached data */
};

static inline void
lnode_free(struct lnode *n) {
	if (n->free)
		(*n->free)(n->data);
	yfree(n);
}

struct ylru *
ylru_create(unsigned int maxsz,
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
	 const void *key, unsigned int keysz,
	 void *data, unsigned int data_size) {
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
	 unsigned int *data_size, /* returned data size */
	 const void *key, unsigned int keysz) {
	struct lnode *n;
	unsigned int sztmp;
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

unsigned int
ylru_sz(struct ylru *lru) {
	return lru->sz;
}
