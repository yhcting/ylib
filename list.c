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
