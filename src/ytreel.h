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
 * @file ytreel.h
 * @brief \b [DRAFT] Header to use low-level interface of tree data structure.
 *
 * Note that this is still DRAFT version!\n
 * This module defines primitive operation of tree.
 * So, before using this, user should understand algorithm of this module.
 */

#ifndef __YTREEl_h__
#define __YTREEl_h__

#include "ydef.h"
#include "ylistl.h"

/******************************************************************************
 *
 * Tree
 *
 *****************************************************************************/
/**
 * Tree link node structrue.
 * DO NOT access struct directly, except that you have to!.
 */
struct ytreel_link {
	/* @cond */
	struct ytreel_link *parent; /* parent link */
	/* head of children list - see ylistl for details */
	struct ylistl_link child;
	struct ylistl_link sibling; /* list link for siblings */
	/* @endcond */
};

/**
 * Initialize tree link.
 *
 * @param lk Tree link
 */
static YYINLINE void
ytreel_init_link(struct ytreel_link *lk) {
	lk->parent = NULL; /* parent of root is itself */
	ylistl_init_link(&lk->child);
	ylistl_init_link(&lk->sibling);
}

/**
 * Get parent link
 *
 * @param lk Tree link
 * @return Parent link.
 */
static YYINLINE struct ytreel_link *
ytreel_parent(const struct ytreel_link *lk) {
	return lk->parent;
}

/**
 * Does tree link has parent?
 *
 * @param lk Tree link
 * @return TRUE or FALSE
 */
static YYINLINE bool
ytreel_has_parent(const struct ytreel_link *lk) {
	return !!lk->parent;
}

/**
 * Does tree link has child?
 *
 * @param lk Tree link
 * @return TRUE or FALSE
 */
static YYINLINE bool
ytreel_has_child(const struct ytreel_link *lk) {
	return !ylistl_is_empty(&lk->child);
}

/**
 * Add new link as next sibling of {@code lk}.
 *
 * @param lk Base tree link
 * @param lnew New tree link
 */
static YYINLINE void
ytreel_add_next(struct ytreel_link *lk, struct ytreel_link *lnew) {
	ylistl_add_next(&lk->sibling, &lnew->sibling);
	lnew->parent = lk->parent;
}

/**
 * Add new link as next sibling of {@code lk}.
 *
 * @param lk Base tree link
 * @param lnew New tree link
 */
static YYINLINE void
ytreel_add_prev(struct ytreel_link *lk, struct ytreel_link *lnew) {
	ylistl_add_prev(&lk->sibling, &lnew->sibling);
	lnew->parent = lk->parent;
}

/**
 * Add new link as the first child of {@code lk}.
 *
 * @param lk Base tree link
 * @param lnew New tree link
 */
static YYINLINE void
ytreel_add_first_child(struct ytreel_link *lk, struct ytreel_link *lnew) {
	ylistl_add_next(&lk->child, &lnew->sibling);
	lnew->parent = lk;
}

/**
 * Add new link as the last child of {@code lk}.
 *
 * @param lk Base tree link
 * @param lnew New tree link
 */
static YYINLINE void
ytreel_add_last_child(struct ytreel_link *lk, struct ytreel_link *lnew) {
	ylistl_add_prev(&lk->child, &lnew->sibling);
	lnew->parent = lk;
}

/**
 * Remove link and all its subtree from current tree link.
 * {@code lk} becomes new root of subtree.
 *
 * @param lk Tree link
 */
static YYINLINE void
ytreel_remove(struct ytreel_link *lk) {
	ylistl_remove(&lk->sibling);   /* separate from container tree link */
	lk->parent = NULL;
}

/**
 * Replace tree link with new one (includes all sub tree).
 * {@code lold} is removed from the tree.
 *
 * @param lold Existing tree link
 * @param lnew New tree link
 */
static YYINLINE void
ytreel_replace(struct ytreel_link *lold, struct ytreel_link *lnew) {
	lnew->parent = lold->parent;
	ylistl_replace(&lold->sibling, &lnew->sibling);
}

/**
 * Get next tree link.
 *
 * @param lk Tree link
 * @return Next tree link
 */
static YYINLINE struct ytreel_link *
ytreel_next(const struct ytreel_link *lk) {
	return YYcontainerof(lk->sibling.next, struct ytreel_link, sibling);
}

/**
 * Get previous tree link.
 * Prerequisite : {@code lk} has parent.
 *
 * @param lk Tree link
 * @return Previous tree link
 */
static YYINLINE struct ytreel_link *
ytreel_prev(const struct ytreel_link *lk) {
	return YYcontainerof(lk->sibling.prev, struct ytreel_link, sibling);
}

/**
 * Does it have next sibling?
 * Prerequisite : {@code lk} has parent.
 *
 * @param lk Tree link
 * @return TRUE or FALSE
 */
static YYINLINE bool
ytreel_has_next(const struct ytreel_link *lk) {
	return ytreel_has_parent(lk)
		&& ylistl_has_next(&ytreel_parent(lk)->child, &lk->sibling);
}

/**
 * Does it have previous sibling?
 * Prerequisite : {@code lk} has parent.
 *
 * @param lk Tree link
 * @return TRUE or FALSE
 */
static YYINLINE bool
ytreel_has_prev(const struct ytreel_link *lk) {
	return ytreel_has_parent(lk)
		&& ylistl_has_prev(&ytreel_parent(lk)->child, &lk->sibling);
}

/**
 * Get the first child.
 * Prerequisite : {@code lk} has child.
 *
 * @param lk Tree link
 * @return First child.
 */
static YYINLINE struct ytreel_link *
ytreel_first_child(const struct ytreel_link *lk) {
	return YYcontainerof(lk->child.next, struct ytreel_link, sibling);
}

/**
 * Get the last child.
 * Prerequisite : {@code lk} has child.
 *
 * @param lk Tree link
 * @return First child.
 */
static YYINLINE struct ytreel_link *
ytreel_last_child(const struct ytreel_link *lk) {
	return YYcontainerof(lk->child.prev, struct ytreel_link, sibling);
}

/**
 * Get number children.
 *
 * @param lk Link
 * @return Number of children of this tree link.
 */
static YYINLINE uint32_t
ytreel_child_size(const struct ytreel_link *lk) {
	return ylistl_size(&lk->child);
}

/**
 * Number of siblings of this tree link.(including itself)
 * Therefore return value is at least 1.
 *
 * @param lk Link
 * @return Number of siblings.
 */
static YYINLINE uint32_t
ytreel_sibling_size(const struct ytreel_link *lk) {
	return ytreel_has_parent(lk)?
		ytreel_child_size(ytreel_parent(lk)):
		1;
}


/******************************************************************************
 *
 * Iterators
 *
 *****************************************************************************/
struct ytreeli;

/** Type of OT : Order Traversal */
enum ytreeli_ot_type {
	/**< pre-order traversal - depth first search(DFS) */
	YTREELI_PRE_OT = 0,
	/**< right to left pre-order traversal */
	YTREELI_R2L_PRE_OT,
	/**< level-order traversal - bread first search(BFS) */
	YTREELI_LEVEL_OT,
	/**< post-order traversal */
	YTREELI_POST_OT,
	/**< right to left post-order traversal */
	YTREELI_R2L_POST_OT,
	/**< Number of tree order-traversal */
	YTREELI_OT_TYPE_COUNT,
};

/**
 * Create iterator object for tree link.
 * This is removal-safe iterator.
 *
 * @param toplk Root link of iteration
 * @param type Iteration type
 * @return Iterator object
 */
YYEXPORT struct ytreeli *
ytreeli_create(const struct ytreel_link *toplk,
	       enum ytreeli_ot_type type);

/**
 * Destroy iterator object.
 */
YYEXPORT void
ytreeli_destroy(struct ytreeli *);

/**
 * Move current link of the iterator to the next;
 * Return code:\n
 * - -ENOENT: Iterator reaches to the end.
 * - -ENOMEM: Not enough memory.
 * - -EINVAL: Something wrong.
 *
 * @return 0 if success. Otherwise -errno.
 */
YYEXPORT int
ytreeli_next(struct ytreeli *);

/**
 * Get current link of the iterator.
 */
YYEXPORT struct ytreel_link *
ytreeli_get(struct ytreeli *);

/**
 * Does iterator have next link to go?
 *
 * @return TRUE or FALSE
 */
YYEXPORT bool
ytreeli_has_next(struct ytreeli *);

#endif /* __YTREEl_h__ */
