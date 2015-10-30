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

/**
 * @file ylist.h
 * @brief Easy-to-use list module compared with 'ylistl'.
 *
 * In performance point of view, this is slower.
 * Interface of \a ylist may return not-essential-value for easy-to-use;
 * For example, parameter value itself.
 *
 * Operation to the empty list is not defined. It's user's responsibility!
 */

#ifndef __YLISt_h__
#define __YLISt_h__

#include <errno.h>
#include <malloc.h>

#include "ydef.h"
#include "ylistl.h"

/****************************************************************************
 *
 * Types
 *
 ****************************************************************************/
/** List node object. */
struct ylist_node {
	/* \cond */
        /* This MUST be on top of struct members */
	void *item; /* node data */
	struct ylistl_link lk; /* node for list link */
	/* \endcond */
};

/** List object */
struct ylist {
	/* \cond */
	void (*ifree)(void *); /* callback to free list item(Item FREE) */
	struct ylistl_link head; /* head of list */
	uint32_t sz; /* current list size */
	uint32_t max; /* maximum list size allowed, 0 means 'no limit' */
	/* \endcond */
};

/** List iterator object */
struct ylisti {
	/* \cond */
	struct ylist *list;
	struct ylistl_link *lnext, *lcurr;
	void *(*next)(struct ylisti *);
	/* \endcond */
};


/****************************************************************************
 *
 * Main Interfaces
 *
 ****************************************************************************/
/**
 * Free \a item.
 *
 * @param l List object.
 * @param item List item to free.
 */
static YYINLINE void
ylist_free_item(struct ylist *l, void *item) {
	if (l->ifree && item)
		l->ifree(item);
}


/**
 * Create list object
 *
 * @param max Maximum number of items can be contained at the list
 * @param ifree Function to free list items
 * @return NULL if fails. Otherwise new list object.
 */
YYEXPORT struct ylist *
ylist_create(uint32_t max,
	     void (*ifree)(void *));

/**
 * Destroy list object.
 * List object itself is destroied.
 */
YYEXPORT void
ylist_destroy(struct ylist *);

/**
 * This reset list to empty without freeing list items in it.
 */
YYEXPORT void
ylist_reset(struct ylist *);

/**
 * Get free-callback in the list object.
 *
 * @param l List object
 * @return Function used to free list item.
 */
static YYINLINE void(*
ylist_ifree(struct ylist *l))(void *) {
	return l->ifree;
}

/**
 * Is list is empty?
 *
 * @return TRUE or FALSE
 */
static YYINLINE bool
ylist_is_empty(const struct ylist *l) {
	return !l->sz;
}

/**
 * Get list size(number of items in the list)
 *
 * @return list size
 */
static YYINLINE uint32_t
ylist_size(const struct ylist *l) {
	return l->sz;
}

/**
 * Is \a item in the list?
 *
 * @param item Item to check
 * @return TRUE or FALSE
 */
YYEXPORT bool
ylist_has(const struct ylist *, void *item);

/**
 * Add item to the end of list.
 *
 * @param item new item too add
 * @return 0 if success. Otherwise -errno. (ex. -ENOMEM)
 */
YYEXPORT int
ylist_add_last(struct ylist *, void *item);

/**
 * Add item at front of list.
 *
 * @param item new item too add
 * @return 0 if success. Otherwise -errno (ex. -ENOMEM)
 */
YYEXPORT int
ylist_add_first(struct ylist *, void *item);

/**
 * Return last item in the list without removing it.
 *
 * @return NULL if list is empty. Otherwise requested item.
 */
YYEXPORT void *
ylist_peek_last(const struct ylist *);

/**
 * Return last item in the list without removing it.
 *
 * @return NULL if fails(ex. list is empty). Otherwise requested item.
 */
YYEXPORT void *
ylist_peek_first(const struct ylist *);

/**
 * Remove last item from list.
 *
 * @param free TRUE to free item after removing it from list.
 *             FALSE to remove item from list and preserve it.
 * @return
 *     NULL if fails(ex. list is empty) Otherwise pointer to requested item.
 *     In case that free == TRUE, returned pointer is invalid one.
 */
YYEXPORT void *
ylist_remove_last(struct ylist *, int free);

/** See {@link ylist_remove_last} */
YYEXPORT void *
ylist_remove_first(struct ylist *, int free);

/**
 * @param itr Iterator
 * @param free See {@link ylist_remove_last}
 * @return See {@link ylist_remove_last}
 */
YYEXPORT void *
ylist_remove_current(struct ylist *, struct ylisti *itr, int free);

/****************************************************************************
 *
 * Alias
 *
 ****************************************************************************/
/** Alias to {@link ylist_add_last} */
static YYINLINE int
ylist_push(struct ylist *l, void *item) {
	return ylist_add_last(l, item);
}

/** Alias to {@link ylist_remove_last} with free == FALSE */
static YYINLINE void *
ylist_pop(struct ylist *l) {
	return ylist_remove_last(l, FALSE);
}

/** Alias to {@link ylist_add_last} */
static YYINLINE int
ylist_enq(struct ylist *l, void *item) {
	return ylist_add_last(l, item);
}

/** Alias to {@link ylist_remove_first} with free == FALSE */
static YYINLINE void *
ylist_deq(struct ylist *l) {
	return ylist_remove_first(l, FALSE);
}

/****************************************************************************
 *
 * ylist iterator
 *
 ****************************************************************************/
enum {
	YLISTI_FORWARD, /**< Iterates from the first to the last */
	YLISTI_BACKWARD /**< Iterates from the last to the first */
};

/**
 * Does iterator has next item?
 *
 * @return boolean
 */
static YYINLINE bool
ylisti_has_next(const struct ylisti *itr) {
	return &itr->list->head != itr->lnext;
}

/**
 * Create iterater object.
 *
 * @param type iterator type. {@link YLISTI_FORWARD} or {@link YLISTI_BACKWARD}
 * @return NULL if fails. Otherwise iterator object.
 */
YYEXPORT struct ylisti *
ylisti_create(struct ylist *, int type);

/**
 * Destroy iterator object
 */
YYEXPORT void
ylisti_destroy(struct ylisti *itr);

/**
 * Get next list item and set it as current.
 *
 * @return NULL if fails. Otherwise next iterator item.
 */
static YYINLINE void *
ylisti_next(struct ylisti *itr) {
	if (YYunlikely(!ylisti_has_next(itr)))
		return NULL;
	return (*itr->next)(itr);
}

#endif /* __YLISt_h__ */
