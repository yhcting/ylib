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


/**
 *************************************
 * !!! DRAFT !!!
 *************************************
 *
 * ytree
 * This is easy-to-use tree compared with 'ytreel'.
 *  But, in performance point of view, this is slower.
 * Interface of 'ytree' may return not-essential-value for easy-to-use;
 * For example, parameter value itself.
 *
 * Operation to the empty tree is not defined. It's consumer's responsibility!
 *
 */

#ifndef __YTREe_h__
#define __YTREe_h__

#include <malloc.h>

#include "ytreel.h"

/*============================
 * Types
 *============================*/

/**
 * If possible DO NOT access struct directly!.
 */
struct ytree_node {
	void               *item;
	struct ytreel_link  link;
};

struct ytree {
	void              (*freecb)(void *);
	struct ytreel_link  pseudo;  /**< pseudo root */
};

/**
 * We uses abstraction here!
 * Pretty lots of things are done inside 'next' function.
 * So, on-indirect-function-call-cost is not big deal!.
 */
struct ytreei {
	struct ytree_node         *(*next)(struct ytreei *);
	const struct ytreel_link  *lcurr, *lnext;
};

/*============================
 * ytree
 *============================*/
static inline void
ytree_free_item(struct ytree *t, void *item) {
	if (t->freecb)
		t->freecb(item);
	else
		yfree(item);
}

#define ytree_item(type, n) (*((type *)((n)->item)))

/*
  static inline void *
  ytree_item(const struct ytree_node *n) {
  return n->item;
  }
*/

static inline struct ytree_node *
__ytree_node(const struct ytreel_link* link) {
	return container_of(link, struct ytree_node, link);
}

static inline void *
__ytree_item(const struct ytreel_link* link) {
	return __ytree_node(link)->item;
}


static inline void
__ytree_init(struct ytree *t, void(*freecb)(void *)) {
	ytreel_init_link(&t->pseudo);
	t->freecb = freecb;
}

static inline struct ytree_node *
ytree_create_node(void *item) {
	struct ytree_node *n;
	n = (struct ytree_node *)ymalloc(sizeof(*n));
	ytreel_init_link(&n->link);
	n->item = item;
	return n;
}

static inline void *
ytree_free_node(struct ytree_node *n) {
	void  *item;
	item = n->item;
	yfree(n);
	return item;
}

static inline struct ytree *
ytree_create(void(*freecb)(void *)) {
	struct ytree *t;
	t = (struct ytree *)ymalloc(sizeof(*t));
	__ytree_init(t, freecb);
	return t;
}

/*============================
 * ytree_walker
 *============================*/
/* OT : Order Traversal */
enum {
	/**< pre-order traversal - depth first search(DFS) */
	YTREEI_PRE_OT,
	/**< level-order traversal - bread first search(BFS) */
	YTREEI_LEVEL_OT,
	/**< post-order traversal */
	YTREEI_POST_OT,
	/**< right to left pre-order traversal */
	YTREEI_R2L_PRE_OT,
	/**< right to left post-order traversal */
	YTREEI_R2L_POST_OT
};

/**
 * return : NULL for error.
 */
EXPORT struct ytreei *
ytreei_create(const struct ytree_node *top_node, int type);

/**
 * return  : -1 for error(ex. invalid param)
 */
EXPORT int
ytreei_destroy(struct ytreei *);

static inline int
ytreei_has_next(const struct ytreei *itr) {
	return !!itr->lnext;
}

/*============================
 * ytree
 *============================*/

static inline struct ytree_node *
ytree_root(const struct ytree *t) {
	/* child(only child) of pseudo root is real root of this tree */
	return __ytree_node(container_of(t->pseudo.child.next,
					 struct ytreel_link,
					 sibling));
}


static inline int
ytree_is_leaf(const struct ytree_node *n) {
	return ytreel_is_leaf(&n->link);
}

static inline int
ytree_is_empty(const struct ytree *t) {
	return ytreel_is_leaf(&t->pseudo);
}

/**
 * destroy node and it's sub-tree
 * - similar with 'ytree_destroy'
 */
static inline void
ytree_destroy_node_tree(struct ytree_node *n, void(*freecb)(void *)) {
	struct ytreei *itr;
	/* see 'ytree_free_node_tree' */
	itr = ytreei_create(n, YTREEI_LEVEL_OT);
	while (ytreei_has_next(itr)) {
		n = itr->next(itr);
		/*
		 * This has additional overhead for comparison.
		 * But it's not big deal
		 */
		if (freecb)
			(*freecb)(ytree_free_node(n));
		else
			yfree(ytree_free_node(n));
	}
	ytreei_destroy(itr);
}

/**
 * see 'ylist_destroy'
 */
static inline void
ytree_destroy(struct ytree *t) {
	ytree_destroy_node_tree(ytree_root(t), t->freecb);
	yfree(t);
}

/**
 * free node and it's sub-tree (this don't free item).
 * - similar with 'ytree_tree'
 */
static inline void
ytree_free_node_tree(struct ytree_node *n) {
	struct ytreei *itr;
	/*
	 * NOTE! :
	 *  - Depends on implementation of YTREEI_LEVEL_OT.
	 *
	 * level order traverse doesn't use visited node.
	 * So, it is suitable for this function.
	 */
	itr = ytreei_create(n, YTREEI_LEVEL_OT);
	while (ytreei_has_next(itr)) {
		n = itr->next(itr);
		/*
		 * This has additional overhead for comparison.
		 * But it's not big deal
		 */
		ytree_free_node(n);
	}
	ytreei_destroy(itr);
}

/**
 * see 'ylist_free'
 */
static inline void
ytree_free(struct ytree *t) {
	ytree_free_node_tree(ytree_root(t));
	yfree(t);
}


/**
 * @t should be empty. @nnew becomes root.
 */
static inline void
ytree_add_to_empty(struct ytree *t, struct ytree_node *nnew) {
	/* yassert(ytree_is_empty(t)); */
	ytreel_add_first_child(&t->pseudo, &nnew->link);
}

static inline void
ytree_add_next(struct ytree_node *node, struct ytree_node *nnew) {
	ytreel_add_next(&node->link, &nnew->link);
}

static inline void
ytree_add_prev(struct ytree_node *node, struct ytree_node *nnew) {
	ytreel_add_prev(&node->link, &nnew->link);
}

static inline void
ytree_add_first_child(struct ytree_node *node, struct ytree_node *nnew) {
	ytreel_add_first_child(&node->link, &nnew->link);
}

static inline void
ytree_add_last_child(struct ytree_node *node, struct ytree_node *nnew) {
	ytreel_add_last_child(&node->link, &nnew->link);
}

static inline struct ytree_node *
ytree_del(struct ytree_node *n) {
	ytreel_del(&n->link);
	return n;
}

/*===================
 * Node walking
 *===================*/
static inline struct ytree_node *
ytree_parent(const struct ytree_node *n) {
	return container_of(n->link.parent, struct ytree_node, link);
}

static inline struct ytree_node *
ytree_next(const struct ytree_node *n) {
	return container_of(ytreel_next(&n->link),
			    struct ytree_node,
			    link);
}

static inline struct ytree_node *
ytree_prev(const struct ytree_node *n) {
	return container_of(ytreel_prev(&n->link),
			    struct ytree_node,
			    link);
}

static inline int
ytree_has_next(const struct ytree_node *n) {
	return ytreel_has_next(&n->link);
}

static inline int
ytree_has_prev(const struct ytree_node *n) {
	return ytreel_has_prev(&n->link);
}

static inline struct ytree_node *
ytree_first_child(const struct ytree_node *n) {
	return container_of(ytreel_first_child(&n->link),
			    struct ytree_node,
			    link);
}

static inline struct ytree_node *
ytree_last_child(const struct ytree_node *n) {
	return container_of(ytreel_last_child(&n->link),
			    struct ytree_node,
			    link);
}

/**
 * return number of siblings of this tree link.
 * (including itself)
 */
static inline unsigned int
ytree_sibling_size(const struct ytree_node *n) {
	return ytreel_sibling_size(&n->link);
}

/**
 * return number of children of this tree link.
 */
static inline unsigned int
ytree_child_size(const struct ytree_node *n) {
	return ytreel_child_size(&n->link);
}

#endif /* __YTREe_h__ */
