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


/**
 * ylist
 * This is easy-to-use list compared with 'ylistl'.
 * But, in performance point of view, this is slower.
 * Interface of 'ylist' may return not-essential-value for easy-to-use;
 * For example, parameter value itself.
 *
 * Operation to the empty list is not defined. It's consumer's responsibility!
 */

#ifndef __YLISt_h__
#define __YLISt_h__

#include <errno.h>
#include <malloc.h>

#include "ylistl.h"

/*============================
 * Types
 *============================*/
/**
 * If possible DO NOT access struct directly!.
 */
struct ylist_node {
	void               *item;   /**< This MUST be top of members. */
	struct ylistl_link  lk;
};

struct ylist {
	void              (*freecb)(void *);
	struct ylistl_link  head;
	unsigned int        sz;  /**< current list size */
	unsigned int        max; /**< maximum list size allowed,
				      0 means 'no limit' */
};

struct ylisti {
	struct ylist       *list;
	struct ylistl_link *lnext, *lcurr;
	void *            (*next)(struct ylisti *);
};


/*============================
 * ylist
 *============================*/

static inline struct ylist_node *
__ylist_node(const struct ylistl_link *lk) {
	return container_of(lk, struct ylist_node, lk);
}

static inline void *
__ylist_item(const struct ylistl_link *lk) {
	return __ylist_node(lk)->item;
}

static inline struct ylist_node *
__ylist_create_node(void *item) {
	struct ylist_node *n;
	n = (struct ylist_node *)ymalloc(sizeof(*n));
	if (unlikely(!n))
		return NULL;
	n->item = item;
	return n;
}

static inline struct ylist_node *
__ylist_del_node(struct ylist_node *n) {
	ylistl_del(&n->lk);
	return n;
}

static inline void *
__ylist_destroy_node(struct ylist_node *n) {
	void *item;
	item = n->item;
	yfree(n);
	return item;
}

/* ============================================================================
 *
 * Main Interfaces
 *
 * ==========================================================================*/
/**
 * Free only item itself!
 */
static inline void
ylist_free_item(struct ylist *l, void *item) {
	if (l->freecb)
		l->freecb(item);
	else
		yfree(item);
}


static inline void
ylist_init(struct ylist *l,
	   unsigned int  max,
	   void(*freecb)(void *)) {
	ylistl_init_link(&l->head);
	l->sz = 0;
	l->max = max;
	l->freecb = freecb;
}

static inline struct ylist *
ylist_create(unsigned int max,
	     void (*freecb)(void *)) {
	struct ylist *l = (struct ylist *)ymalloc(sizeof(*l));
	if (unlikely(!l))
		return NULL;
	ylist_init(l, max, freecb);
	return l;
}

static inline void
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
 * Free item too.
 */
static inline void
ylist_destroy(struct ylist *l) {
	ylist_clean(l);
	yfree(l);
}

/**
 * This doesn't free items. (cf. ylist_clean)
 */
static inline void
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

static inline void(*
ylist_freecb(struct ylist *l))(void *) {
	return l->freecb;
}

static inline int
ylist_is_empty(const struct ylist *l) {
	return !l->sz;
}

static inline unsigned int
ylist_size(const struct ylist *l) {
	return l->sz;
}

static inline int
ylist_add_last(struct ylist *l, void *item) {
	struct ylist_node *n;
	if (l->max
	    && l->sz >= l->max)
		return EPERM;
	n = __ylist_create_node(item);
	if (unlikely(!n))
		return ENOMEM;
        ylistl_add_prev(&l->head, &n->lk);
	++l->sz;
	return 0;
}

static inline int
ylist_add_first(struct ylist *l, void *item) {
	struct ylist_node *n;
	if (l->max
	    && l->sz >= l->max)
		return EPERM;
	n = __ylist_create_node(item);
	if (unlikely(!n))
		return ENOMEM;
        ylistl_add_next(&l->head, &n->lk);
	++l->sz;
	return 0;
}

static inline void *
ylist_peek_last(const struct ylist *l) {
	return ylist_is_empty(l)? NULL: __ylist_item(l->head.prev);
}

static inline void *
ylist_peek_first(const struct ylist *l) {
	return ylist_is_empty(l)? NULL: __ylist_item(l->head.next);
}

static inline void *
ylist_remove_last(struct ylist *l, int free) {
	struct ylist_node *n;
	void *item;
	yassert(!ylist_is_empty(l));
	n = __ylist_node(l->head.prev);
	__ylist_del_node(n);
	--l->sz;
	item = __ylist_destroy_node(n);
	if (!free)
		return item;
	ylist_free_item(l, item);
	return NULL;
}

static inline void *
ylist_remove_first(struct ylist *l, int free) {
	struct ylist_node *n;
	void *item;
	yassert(!ylist_is_empty(l));
	n = __ylist_node(l->head.next);
	__ylist_del_node(n);
	--l->sz;
	item = __ylist_destroy_node(n);
	if (!free)
		return item;
	ylist_free_item(l, item);
	return NULL;
}

static inline void *
ylist_remove_current(struct ylist *l, struct ylisti *itr, int free) {
	void *item;
	struct ylist_node *n;
	yassert(!ylist_is_empty(l)
		&& l == itr->list);
	n = __ylist_node(itr->lcurr);
	__ylist_del_node(n);
	--l->sz;
	item = __ylist_destroy_node(n);
	if (!free)
		return item;
	ylist_free_item(l, item);
	return NULL;
}

/* ==================================
 * Alias
 * ==================================*/
static inline int
ylist_push(struct ylist *l, void *item) {
	return ylist_add_last(l, item);
}

static inline void *
ylist_pop(struct ylist *l) {
	return ylist_remove_last(l, FALSE);
}

static inline int
ylist_enq(struct ylist *l, void *item) {
	return ylist_add_last(l, item);
}

static inline void *
ylist_deq(struct ylist *l) {
	return ylist_remove_first(l, FALSE);
}

/*=============================================================================
 *
 * ylist iterator
 *
 *===========================================================================*/

enum {
	YLISTI_FORWARD,
	YLISTI_BACKWARD
};

/**
 *
 */
static inline int
ylisti_has_next(const struct ylisti *itr) {
	return &itr->list->head != itr->lnext;
}

static inline void *
__ylisti_next_forward(struct ylisti *itr) {
	yassert(ylisti_has_next(itr));
	itr->lcurr = itr->lnext;
	itr->lnext = itr->lnext->next;
	return __ylist_item(itr->lcurr);
}

static inline void *
__ylisti_next_backward(struct ylisti *itr) {
	yassert(ylisti_has_next(itr));
	itr->lcurr = itr->lnext;
	itr->lnext = itr->lnext->prev;
	return __ylist_item(itr->lcurr);
}

static inline struct ylisti *
ylisti_create(struct ylist *l, int type) {
	struct ylisti *itr =
		(struct ylisti *)ymalloc(sizeof(*itr));
	itr->list = l;
	itr->lcurr = &l->head;
	switch(type) {
        case YLISTI_FORWARD:
		itr->next = &__ylisti_next_forward;
		itr->lnext = l->head.next;
		break;
        case YLISTI_BACKWARD:
		itr->next = &__ylisti_next_backward;
		itr->lnext = l->head.prev;
		break;
        default:
		yfree(itr);
		itr = NULL;
	}
	return itr;
}

static inline void
ylisti_destroy(struct ylisti *itr) {
	yfree(itr);
}

static inline void *
ylisti_next(struct ylisti *itr) {
	if (unlikely(!ylisti_has_next(itr)))
		return NULL;
	return (*itr->next)(itr);
}

#endif /* __YLISt_h__ */
