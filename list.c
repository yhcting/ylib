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

#include "ylist.h"

static inline struct ylist_node *
node(const struct ylistl_link *lk) {
	return container_of(lk, struct ylist_node, lk);
}

static inline void *
item(const struct ylistl_link *lk) {
	return node(lk)->item;
}

static inline struct ylist_node *
create_node(void *item) {
	struct ylist_node *n;
	n = (struct ylist_node *)ymalloc(sizeof(*n));
	if (unlikely(!n))
		return NULL;
	n->item = item;
	return n;
}

static inline void *
destroy_node(struct ylist_node *n) {
	void *item;
	item = n->item;
	yfree(n);
	return item;
}

static inline void *
remove_node(struct ylist *l, struct ylistl_link *lk, int free) {
	struct ylist_node *n;
	void *item;
	ylistl_del(lk);
	n = node(lk);
	--l->sz;
	item = destroy_node(n);
	if (!free)
		return item;
	ylist_free_item(l, item);
	return NULL;
}

/* ============================================================================
 *
 * Main Interfaces
 *
 * ==========================================================================*/
void
ylist_init(struct ylist *l,
	   unsigned int  max,
	   void(*freecb)(void *)) {
	ylistl_init_link(&l->head);
	l->sz = 0;
	l->max = max;
	l->freecb = freecb;
}

struct ylist *
ylist_create(unsigned int max,
	     void (*freecb)(void *)) {
	struct ylist *l = (struct ylist *)ymalloc(sizeof(*l));
	if (unlikely(!l))
		return NULL;
	ylist_init(l, max, freecb);
	return l;
}

void
ylist_clean(struct ylist *l) {
	struct ylist_node *n, *p;
	ylistl_foreach_item_removal_safe(p,
					 n,
					 &l->head,
					 struct ylist_node,
					 lk) {
		ylist_free_item(l, p->item);
		yfree(p);
	}
	l->sz = 0;
}

/**
 * This doesn't free items. (cf. ylist_clean)
 */
void
ylist_free(struct ylist *l) {
	struct ylist_node *n, *p;
	ylistl_foreach_item_removal_safe(p,
					 n,
					 &l->head,
					 struct ylist_node,
					 lk)
		yfree(p);
	yfree(l);
	l->sz = 0;
}

int
ylist_add_last(struct ylist *l, void *item) {
	struct ylist_node *n;
	if (unlikely(l->max
		     && l->sz >= l->max))
		return EPERM;
	n = create_node(item);
	if (unlikely(!n))
		return ENOMEM;
        ylistl_add_prev(&l->head, &n->lk);
	++l->sz;
	return 0;
}

int
ylist_add_first(struct ylist *l, void *item) {
	struct ylist_node *n;
	if (unlikely(l->max
		     && l->sz >= l->max))
		return EPERM;
	n = create_node(item);
	if (unlikely(!n))
		return ENOMEM;
        ylistl_add_next(&l->head, &n->lk);
	++l->sz;
	return 0;
}

void *
ylist_peek_last(const struct ylist *l) {
	return ylist_is_empty(l)? NULL: item(l->head.prev);
}

void *
ylist_peek_first(const struct ylist *l) {
	return ylist_is_empty(l)? NULL: item(l->head.next);
}

void *
ylist_remove_last(struct ylist *l, int free) {
	yassert(!ylist_is_empty(l));
	return remove_node(l, l->head.prev, free);
}

void *
ylist_remove_first(struct ylist *l, int free) {
	yassert(!ylist_is_empty(l));
	return remove_node(l, l->head.next, free);
}

void *
ylist_remove_current(struct ylist *l, struct ylisti *itr, int free) {
	yassert(!ylist_is_empty(l)
		&& l == itr->list);
	return remove_node(l, itr->lcurr, free);
}



/*=============================================================================
 *
 * ylist iterator
 *
 *===========================================================================*/
static inline void *
inext_forward(struct ylisti *itr) {
	yassert(ylisti_has_next(itr));
	itr->lcurr = itr->lnext;
	itr->lnext = itr->lnext->next;
	return item(itr->lcurr);
}

static inline void *
inext_backward(struct ylisti *itr) {
	yassert(ylisti_has_next(itr));
	itr->lcurr = itr->lnext;
	itr->lnext = itr->lnext->prev;
	return item(itr->lcurr);
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
