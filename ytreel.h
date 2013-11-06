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
 *************************************
 * !!! DRAFT !!!
 *************************************
 *
 * This is not 100%-performance-oriented. But, almost focusing on performance. 
 * It uses list internally (This may drop performance.)
 */


#ifndef __YTREEl_h__
#define __YTREEl_h__

#include "ylistl.h"

/*================================
 * Define primitive operation of tree!
 * User should understand algorithm fully before using this!
 *================================*/

struct ytreel_link {
	struct ytreel_link  *parent;     /**< parent link */
	/**< head of children list - see ylistl for details */
	struct ylistl_link   child;
	struct ylistl_link   sibling;    /**< list link for siblings */
};


/**
 * @link    : &struct ytreel_link pointer
 * @type    : type of the struct @link is embedded in.
 * @member  : the name of @link with in the 'type' struct.
 */
#define ytreel_item(link, type, member)		\
        container_of(link, type, member)

static inline void
ytreel_init_link(struct ytreel_link *link) {
	link->parent = link;    /* parent of root is itself */
	ylistl_init_link(&link->child);
	ylistl_init_link(&link->sibling);
}

static inline int
ytreel_is_leaf(const struct ytreel_link *link) {
	return ylistl_is_empty(&link->child);
}

static inline void
ytreel_add_next(struct ytreel_link *link, struct ytreel_link *lnew) {
	ylistl_add_next(&link->sibling, &lnew->sibling);
	lnew->parent = link->parent;
}

static inline void
ytreel_add_prev(struct ytreel_link *link, struct ytreel_link *lnew) {
	ylistl_add_prev(&link->sibling, &lnew->sibling);
	lnew->parent = link->parent;
}

static inline void
ytreel_add_first_child(struct ytreel_link *link, struct ytreel_link *lnew) {
	ylistl_add_next(&link->child, &lnew->sibling);
	lnew->parent = link;
}

static inline void
ytreel_add_last_child(struct ytreel_link *link, struct ytreel_link *lnew) {
	ylistl_add_prev(&link->child, &lnew->sibling);
	lnew->parent = link;
}

/**
 * Delete link and all its subtree from the container tree link.
 */
static inline void
ytreel_del(struct ytreel_link *link) {
	ylistl_del(&link->sibling);   /* separate from container tree link */
}

/**
 * replace tree link (includes all sub tree)
 */
static inline void
ytreel_replace(struct ytreel_link *old, struct ytreel_link *lnew) {
	lnew->parent = old->parent;
	ylistl_replace(&old->sibling, &lnew->sibling);
}

static inline struct ytreel_link *
ytreel_next(const struct ytreel_link *l) {
	return container_of(l->sibling.next, struct ytreel_link, sibling);
}

static inline struct ytreel_link *
ytreel_prev(const struct ytreel_link *l) {
	return container_of(l->sibling.prev, struct ytreel_link, sibling);
}

static inline int
ytreel_has_next(const struct ytreel_link *l) {
	/*
	 * All links have parent except for pseudo node.
	 * But pseudo node is inside blackbox.
	 * So, we can assume that @l is not pseudo node.
	 */
	yassert(l->parent != l);
	return &l->parent->child != l->sibling.next;
}

static inline int
ytreel_has_prev(const struct ytreel_link *l) {
	yassert(l->parent != l); /* see 'ytreel_has_next' */
	return &l->parent->child != l->sibling.prev;
}

static inline struct ytreel_link *
ytreel_first_child(const struct ytreel_link *l) {
	return container_of(l->child.next, struct ytreel_link, sibling);
}

static inline struct ytreel_link *
ytreel_last_child(const struct ytreel_link *l) {
	return container_of(l->child.prev, struct ytreel_link, sibling);
}

/**
 * return number of siblings of this tree link.
 * (including itself)
 */
static inline unsigned int
ytreel_sibling_size(const struct ytreel_link *l) {
	/* even if &l->sibling is not head of list, counting size is ok */
	return ylistl_size(&l->sibling);
}

/**
 * return number of children of this tree link.
 */
static inline unsigned int
ytreel_child_size(const struct ytreel_link *l) {
	return ylistl_size(&l->child);
}

#endif /* __YTREEl_h__ */
