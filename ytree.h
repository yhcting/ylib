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
	void*               __item;
	struct ytreel_link  __link;
};

struct ytree {
	void(*              __freecb)(void*);
	struct ytreel_link  __pseudo;  /**< pseudo root */
};

/**
 * We uses abstraction here!
 * Pretty lots of things are done inside 'next' function.
 * So, on-indirect-function-call-cost is not big deal!.
 */
struct ytree_walker {
	struct ytree_node*       (*next)(struct ytree_walker*);
	const struct ytreel_link  *__curr, *__next;
};

/*============================
 * ytree
 *============================*/
static inline void
ytree_free_item(struct ytree* t, void* item) {
	if(NULL != t->__freecb) { t->__freecb(item); }
	else { yfree(item); }
}

#define ytree_item(type, n) (*((type*)((n)->__item)))

/*
  static inline void*
  ytree_item(const struct ytree_node* n) {
  return n->__item;
  }
*/

static inline struct ytree_node*
__ytree_node(const struct ytreel_link* link) {
	return container_of(link, struct ytree_node, __link);
}

static inline void*
__ytree_item(const struct ytreel_link* link) {
	return __ytree_node(link)->__item;
}


static inline void
__ytree_init(struct ytree* t, void(*freecb)(void*)) {
	ytreel_init_link(&t->__pseudo);
	t->__freecb = freecb;
}

static inline struct ytree_node*
ytree_create_node(void* item) {
	struct ytree_node* n;
	n = (struct ytree_node*)ymalloc(sizeof(struct ytree_node));
	ytreel_init_link(&n->__link);
	n->__item = item;
	return n;
}

static inline void*
ytree_free_node(struct ytree_node* n) {
	void*   item;
	item = n->__item;
	yfree(n);
	return item;
}

static inline struct ytree*
ytree_create(void(*freecb)(void*)) {
	struct ytree* t;
	t = (struct ytree*)ymalloc(sizeof(struct ytree));
	__ytree_init(t, freecb);
	return t;
}

/*============================
 * ytree_walker
 *============================*/
/* OT : Order Traversal */
enum {
	/**< pre-order traversal - depth first search(DFS) */
	YTREE_WALKER_PRE_OT,
	/**< level-order traversal - bread first search(BFS) */
	YTREE_WALKER_LEVEL_OT,
	/**< post-order traversal */
	YTREE_WALKER_POST_OT,
	/**< right to left pre-order traversal */
	YTREE_WALKER_R2L_PRE_OT,
	/**< right to left post-order traversal */
	YTREE_WALKER_R2L_POST_OT
};

EXPORT struct ytree_walker*
ytree_walker_create(const struct ytree_node* top_node, int type);

EXPORT void
ytree_walker_destroy(struct ytree_walker* w);

static inline int
ytree_walker_has_next(const struct ytree_walker* w) {
	return NULL != w->__next;
}

/*============================
 * ytree
 *============================*/

static inline struct ytree_node*
ytree_root(const struct ytree* t) {
	/* child(only child) of pseudo root is real root of this tree */
	return __ytree_node(container_of(t->__pseudo._child._next,
					 struct ytreel_link,
					 _sibling));
}


static inline int
ytree_is_leaf(const struct ytree_node* n) {
	return ytreel_is_leaf(&n->__link);
}

static inline int
ytree_is_empty(const struct ytree* t) {
	return ytreel_is_leaf(&t->__pseudo);
}

/**
 * destroy node and it's sub-tree
 * - similar with 'ytree_destroy'
 */
static inline void
ytree_destroy_node_tree(struct ytree_node* n, void(*freecb)(void*)) {
	struct ytree_walker* w;
	/* see 'ytree_free_node_tree' */
	w = ytree_walker_create(n, YTREE_WALKER_LEVEL_OT);
	while(ytree_walker_has_next(w)) {
		n = w->next(w);
		/*
		 * This has additional overhead for comparison.
		 * But it's not big deal
		 */
		if(NULL == freecb) {
			yfree(ytree_free_node(n));
		} else {
			(*freecb)(ytree_free_node(n));
		}
	}
	ytree_walker_destroy(w);
}

/**
 * see 'ylist_destroy'
 */
static inline void
ytree_destroy(struct ytree* t) {
	ytree_destroy_node_tree(ytree_root(t), t->__freecb);
	yfree(t);
}

/**
 * free node and it's sub-tree (this don't free item).
 * - similar with 'ytree_tree'
 */
static inline void
ytree_free_node_tree(struct ytree_node* n) {
	struct ytree_walker* w;
	/*
	 * NOTE! :
	 *  - Depends on implementation of YTREE_WALKER_LEVEL_OT.
	 *
	 * level order traverse doesn't use visited node.
	 * So, it is suitable for this function.
	 */
	w = ytree_walker_create(n, YTREE_WALKER_LEVEL_OT);
	while(ytree_walker_has_next(w)) {
		n = w->next(w);
		/*
		 * This has additional overhead for comparison.
		 * But it's not big deal
		 */
		ytree_free_node(n);
	}
	ytree_walker_destroy(w);
}

/**
 * see 'ylist_free'
 */
static inline void
ytree_free(struct ytree* t) {
	ytree_free_node_tree(ytree_root(t));
	yfree(t);
}


/**
 * @t should be empty. @nnew becomes root.
 */
static inline void
ytree_add_to_empty(struct ytree* t, struct ytree_node* nnew) {
	/* yassert(ytree_is_empty(t)); */
	ytreel_add_first_child(&t->__pseudo, &nnew->__link);
}

static inline void
ytree_add_next(struct ytree_node* node, struct ytree_node* nnew) {
	ytreel_add_next(&node->__link, &nnew->__link);
}

static inline void
ytree_add_prev(struct ytree_node* node, struct ytree_node* nnew) {
	ytreel_add_prev(&node->__link, &nnew->__link);
}

static inline void
ytree_add_first_child(struct ytree_node* node, struct ytree_node* nnew) {
	ytreel_add_first_child(&node->__link, &nnew->__link);
}

static inline void
ytree_add_last_child(struct ytree_node* node, struct ytree_node* nnew) {
	ytreel_add_last_child(&node->__link, &nnew->__link);
}

static inline struct ytree_node*
ytree_del(struct ytree_node* n) {
	ytreel_del(&n->__link);
	return n;
}

/*===================
 * Node walking
 *===================*/
static inline struct ytree_node*
ytree_parent(const struct ytree_node* n) {
	return container_of(n->__link._parent, struct ytree_node, __link);
}

static inline struct ytree_node*
ytree_next(const struct ytree_node* n) {
	return container_of(ytreel_next(&n->__link),
			    struct ytree_node,
			    __link);
}

static inline struct ytree_node*
ytree_prev(const struct ytree_node* n) {
	return container_of(ytreel_prev(&n->__link),
			    struct ytree_node,
			    __link);
}

static inline int
ytree_has_next(const struct ytree_node* n) {
	return ytreel_has_next(&n->__link);
}

static inline int
ytree_has_prev(const struct ytree_node* n) {
	return ytreel_has_prev(&n->__link);
}

static inline struct ytree_node*
ytree_first_child(const struct ytree_node* n) {
	return container_of(ytreel_first_child(&n->__link),
			    struct ytree_node,
			    __link);
}

static inline struct ytree_node*
ytree_last_child(const struct ytree_node* n) {
	return container_of(ytreel_last_child(&n->__link),
			    struct ytree_node,
			    __link);
}

/**
 * return number of siblings of this tree link.
 * (including itself)
 */
static inline unsigned int
ytree_sibling_size(const struct ytree_node* n) {
	return ytreel_sibling_size(&n->__link);
}

/**
 * return number of children of this tree link.
 */
static inline unsigned int
ytree_child_size(const struct ytree_node* n) {
	return ytreel_child_size(&n->__link);
}

#endif /* __YTREe_h__ */
