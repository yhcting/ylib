/*****************************************************************************
 *    Copyright (C) 2011 Younghyung Cho. <yhcting77@gmail.com>
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

#include <malloc.h>

#include "ylistl.h"

/*============================
 * Types
 *============================*/
/**
 * If possible DO NOT access struct directly!.
 */
struct ylist_node {
	void*               __item;   /**< This MUST be top of members. */
	struct ylistl_link  __link;
};

struct ylist {
	void(*              __freecb)(void*);
	struct ylistl_link  __head;
};

/**
 * Trade off : performance vs. abstraction..
 * We may use 'ylist_node* (*next)()'.
 * But in this implementation, we don't do this.
 * Instead of that,
 *  we uses 'ylist_walker_next_forward/ylist_walker_next_backward'
 *  to save one-indirect-function-call-cost,
 *  because next_forward/next_backward doesn't do much.
 * (on-indirect-function-call-cost is pretty high compare with
 *   what are done inside function.)
 */
struct ylist_walker {
	struct ylist*           __list;
	struct ylistl_link     *__next, *__curr;
};


/*============================
 * ylist
 *============================*/

/**
 * Free only item itself!
 */
static inline void
ylist_free_item(struct ylist* l, void* item) {
	if (NULL != l->__freecb)
		l->__freecb(item);
	else
		yfree(item);
}

static inline void*
ylist_item(const struct ylist_node* n) {
	return n->__item;
}


static inline void*
__ylist_item(const struct ylistl_link* lk) {
	return ylist_item(container_of(lk, struct ylist_node, __link));
}

static inline struct ylist_node*
__ylist_node(const struct ylistl_link* lk) {
	return container_of(lk, struct ylist_node, __link);
}

static inline struct ylistl_link*
__ylist_first_link(const struct ylist* l) {
	return l->__head._next;
}

static inline struct ylistl_link*
__ylist_last_link(const struct ylist* l) {
	return l->__head._prev;
}


/**
 * link should be deleted by 'ylist_del.
 */
static inline struct ylist_node*
ylist_create_node(void* item) {
	struct ylist_node* n = (struct ylist_node*)ymalloc(sizeof(*n));
	n->__item = item;
	return n;
}

static inline struct ylist_node*
ylist_del(struct ylist_node* n) {
	ylistl_del(&n->__link);
	return n;
}

static inline void*
ylist_free_node(struct ylist_node* n) {
	void*   item;
	item = n->__item;
	yfree(n);
	return item;
}

static inline void
__ylist_init(struct ylist* l, void(*freecb)(void*)) {
	ylistl_init_link(&l->__head);
	l->__freecb = freecb;
}

static inline struct ylist*
ylist_create(void(*freecb)(void*)) {
	struct ylist* l = (struct ylist*)ymalloc(sizeof(*l));
	__ylist_init(l, freecb);
	return l;
}

static inline void
ylist_reset(struct ylist* l) {
	struct ylist_node *n, *p;
	ylistl_foreach_item_removal_safe(p,
					 n,
					 &l->__head,
					 struct ylist_node,
					 __link) {
		ylist_free_item(l, p->__item);
		yfree(p);
	}
}

/**
 * Free item too.
 */
static inline void
ylist_destroy(struct ylist* l) {
	ylist_reset(l);
	yfree(l);
}

/**
 * This doesn't free items. (cf. destory)
 */
static inline void
ylist_free(struct ylist* l) {
	struct ylist_node *n, *p;
	ylistl_foreach_item_removal_safe(p,
					 n,
					 &l->__head,
					 struct ylist_node,
					 __link)
		yfree(p);
	yfree(l);
}

static inline void(*ylist_freecb(struct ylist* l))(void*) {
	return l->__freecb;
}

static inline struct ylist_node*
ylist_find_node(const struct ylist* l, void* item) {
	struct ylist_node*   p;
	ylistl_foreach_item(p, &l->__head, struct ylist_node, __link)
		if (p->__item == item)
			return p;

	return NULL;
}

static inline int
ylist_is_empty(const struct ylist* l) {
	return ylistl_is_empty(&l->__head);
}

static inline unsigned int
ylist_size(const struct ylist* l) {
	return ylistl_size(&l->__head);
}

/**
 *
 */
static inline void
ylist_add_next(struct ylist_node* node, struct ylist_node* nnew) {
	ylistl_add_next(&node->__link, &nnew->__link);
}

static inline void
ylist_add_prev(struct ylist_node* node, struct ylist_node* nnew) {
	ylistl_add_prev(&node->__link, &nnew->__link);
}

static inline void
ylist_add_last(struct ylist* l, struct ylist_node* nnew) {
	ylistl_add_last(&l->__head, &nnew->__link);
}

static inline void
ylist_add_first(struct ylist* l, struct ylist_node* nnew) {
	ylistl_add_first(&l->__head, &nnew->__link);
}

static inline struct ylist_node*
ylist_del_last(struct ylist* l) {
	return ylist_del( __ylist_node( __ylist_last_link(l)) );
}

static inline struct ylist_node*
ylist_del_first(struct ylist* l) {
	return ylist_del( __ylist_node(__ylist_first_link(l)) );
}

static inline void*
ylist_peek_last(const struct ylist* l) {
	return ylist_is_empty(l)? NULL: __ylist_item(__ylist_last_link(l));
}

static inline void*
ylist_peek_first(const struct ylist* l) {
	return ylist_is_empty(l)? NULL: __ylist_item(__ylist_first_link(l));
}


/*=====================
 * ylist walker
 *=====================*/
enum {
	YLIST_WALKER_FORWARD,
	YLIST_WALKER_BACKWARD
};

static inline struct ylist_walker*
ylist_walker_create(struct ylist* l, int type) {
	struct ylist_walker* w =
		(struct ylist_walker*)ymalloc(sizeof(*w));
	w->__list = l;
	w->__curr = &l->__head;
	switch(type) {
        case YLIST_WALKER_FORWARD:  w->__next = __ylist_first_link(l);  break;
        case YLIST_WALKER_BACKWARD: w->__next = __ylist_last_link(l);   break;
        default: yfree(w); w = NULL;
	}
	return w;
}

static inline void
ylist_walker_destroy(struct ylist_walker* w) {
	yfree(w);
}

/**
 *
 */
static inline int
ylist_walker_has_next(const struct ylist_walker* w) {
	return &w->__list->__head != w->__next;
}

/**
 * Error check should be done by caller.
 *
 */
static inline void*
ylist_walker_next_forward(struct ylist_walker* w) {
	yassert(ylist_walker_has_next(w));
	w->__curr = w->__next;
	w->__next = w->__next->_next;
	return __ylist_item(w->__curr);
}

/**
 *
 */
static inline void*
ylist_walker_next_backward(struct ylist_walker* w) {
	yassert(ylist_walker_has_next(w));
	w->__curr = w->__next;
	w->__next = w->__next->_prev;
	return __ylist_item(w->__curr);
}

static inline struct ylist*
ylist_walker_list(const struct ylist_walker* w) {
	return w->__list;
}

static inline struct ylist_node*
ylist_walker_node(const struct ylist_walker* w) {
	return __ylist_node(w->__curr);
}

/**
 * doesn't free memory for item! it just delete node!
 */
static inline struct ylist_node*
ylist_walker_del(struct ylist_walker* w) {
	struct ylist_node* n;
	n = __ylist_node(w->__curr);
	ylist_del(n);
	w->__curr = &w->__list->__head;
	return n;
}

#endif /* __YLISt_h__ */
