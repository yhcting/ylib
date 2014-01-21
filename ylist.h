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


EXPORT void
ylist_init(struct ylist *l,
	   unsigned int  max,
	   void(*freecb)(void *));

EXPORT struct ylist *
ylist_create(unsigned int max,
	     void (*freecb)(void *));

/* Items are freed too. */
EXPORT void
ylist_clean(struct ylist *l);

static inline void
ylist_destroy(struct ylist *l) {
	ylist_clean(l);
	yfree(l);
}

/**
 * This doesn't free items. (cf. ylist_clean)
 */
/* items are not freed. (cf. ylist_clean) */
EXPORT void
ylist_free(struct ylist *l);

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

EXPORT int
ylist_add_last(struct ylist *l, void *item);

EXPORT int
ylist_add_first(struct ylist *l, void *item);

EXPORT void *
ylist_peek_last(const struct ylist *l);

EXPORT void *
ylist_peek_first(const struct ylist *l);

EXPORT void *
ylist_remove_last(struct ylist *l, int free);

EXPORT void *
ylist_remove_first(struct ylist *l, int free);

EXPORT void *
ylist_remove_current(struct ylist *l, struct ylisti *itr, int free);

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

static inline int
ylisti_has_next(const struct ylisti *itr) {
	return &itr->list->head != itr->lnext;
}

EXPORT struct ylisti *
ylisti_create(struct ylist *l, int type);

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
