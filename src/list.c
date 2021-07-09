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
#include <limits.h>

#include "common.h"
#include "ylist.h"

static INLINE struct ylist_node *
lknode(const struct ylistl_link *lk) {
	return containerof(lk, struct ylist_node, lk);
}

static INLINE void *
lkitem(const struct ylistl_link *lk) {
	return lknode(lk)->item;
}

static struct ylist_node *
node_create(void *item) {
	struct ylist_node *n;
	n = (struct ylist_node *)ymalloc(sizeof(*n));
	if (unlikely(!n))
		return NULL;
	n->item = item;
	return n;
}

static void *
node_destroy(struct ylist_node *n) {
	void *item;
	item = n->item;
	yfree(n);
	return item;
}

static void *
remove_node(struct ylist *l, struct ylistl_link *lk, int free) {
	struct ylist_node *n;
	void *item;
	ylistl_remove(lk);
	n = lknode(lk);
	--l->sz;
	item = node_destroy(n);
	if (free)
		ylist_free_item(l, item);
	return item;
}

/****************************************************************************
 *
 * Main Interfaces
 *
 ****************************************************************************/
struct ylist *
ylist_create(u32 max, void (*ifree)(void *)) {
	struct ylist *l = (struct ylist *)ymalloc(sizeof(*l));
	if (unlikely(!l))
		return NULL;
	ylistl_init_link(&l->head);
	l->sz = 0;
	l->max = max;
	l->ifree = ifree;
	if (0 == l->max)
		l->max = UINT_MAX;
	return l;
}

void
ylist_destroy(struct ylist *l) {
	struct ylist_node *n, *p;
	ylistl_foreach_item_safe(
		p, n, &l->head, struct ylist_node, lk
	) {
		ylist_free_item(l, p->item);
		yfree(p);
	}
	yfree(l);
}


/**
 * This doesn't free items.
 */
void
ylist_reset(struct ylist *l) {
	struct ylist_node *n, *p;
	ylistl_foreach_item_safe(
		p, n, &l->head, struct ylist_node, lk
	) { yfree(p); }
	l->sz = 0;
}

bool
ylist_has(const struct ylist *l, void *item) {
	struct ylist_node *p;
	ylistl_foreach_item(p, &l->head, struct ylist_node, lk) {
		if (unlikely(p->item == item))
			return TRUE;
	}
	return FALSE;
}

int
ylist_add_last(struct ylist *l, void *item) {
	struct ylist_node *n;
	if (unlikely(l->max && l->sz >= l->max))
		return -EPERM;
	n = node_create(item);
	if (unlikely(!n))
		return -ENOMEM;
        ylistl_add_prev(&l->head, &n->lk);
	++l->sz;
	return 0;
}

int
ylist_add_first(struct ylist *l, void *item) {
	struct ylist_node *n;
	if (unlikely(l->max && l->sz >= l->max))
		return -EPERM;
	n = node_create(item);
	if (unlikely(!n))
		return -ENOMEM;
        ylistl_add_next(&l->head, &n->lk);
	++l->sz;
	return 0;
}

void *
ylist_peek_last(const struct ylist *l) {
	return ylist_is_empty(l)? NULL: lkitem(l->head.prev);
}

void *
ylist_peek_first(const struct ylist *l) {
	return ylist_is_empty(l)? NULL: lkitem(l->head.next);
}

void *
ylist_remove_last(struct ylist *l, int free) {
	if (unlikely(ylist_is_empty(l)))
		return NULL;
	return remove_node(l, l->head.prev, free);
}

void *
ylist_remove_first(struct ylist *l, int free) {
	if (unlikely(ylist_is_empty(l)))
		return NULL;
	return remove_node(l, l->head.next, free);
}

void *
ylist_remove_current(struct ylist *l, struct ylisti *itr, int free) {
	if (unlikely(ylist_is_empty(l)))
		return NULL;
	return remove_node(l, itr->lcurr, free);
}



/****************************************************************************
 *
 * ylist iterator
 *
 ****************************************************************************/
static void *
inext_forward(struct ylisti *itr) {
	yassert(ylisti_has_next(itr));
	itr->lcurr = itr->lnext;
	itr->lnext = itr->lnext->next;
	return lkitem(itr->lcurr);
}

static void *
inext_backward(struct ylisti *itr) {
	yassert(ylisti_has_next(itr));
	itr->lcurr = itr->lnext;
	itr->lnext = itr->lnext->prev;
	return lkitem(itr->lcurr);
}

struct ylisti *
ylisti_create(struct ylist *l, int type) {
	struct ylisti *itr =
		(struct ylisti *)ymalloc(sizeof(*itr));
	itr->list = l;
	itr->lcurr = &l->head;
	switch(type) {
        case YLISTI_FORWARD:
		itr->next = &inext_forward;
		itr->lnext = l->head.next;
	break;
        case YLISTI_BACKWARD:
		itr->next = &inext_backward;
		itr->lnext = l->head.prev;
	break;
        default:
		yfree(itr);
		itr = NULL;
	}
	return itr;
}

void
ylisti_destroy(struct ylisti *itr) {
	yfree(itr);
}
