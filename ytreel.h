/**
 *************************************
 * !!! DRAFT !!!
 *************************************
 *
 * This is not 100%-performance-oriented. But, almost focusing on performance. 
 * It uses list internally (This may drop performance.)
 */


#ifndef _YTREEl_h_
#define _YTREEl_h_

#include "ylistl.h"

#ifdef __cplusplus
extern "C" {
#endif


/*================================
 * Define primitive operation of tree!
 * User should understand algorithm fully before using this!
 *================================*/

struct ytreel_link {
    struct ytreel_link*  _parent;     /**< parent link */
    struct ylistl_link   _child;      /**< head of children list - see ylistl for details */
    struct ylistl_link   _sibling;    /**< list link for siblings */
};


/**
 * @link    : &struct ytreel_link pointer
 * @type    : type of the struct @link is embedded in.
 * @member  : the name of @link with in the 'type' struct.
 */
#define ytreel_item(link, type, member) \
        container_of(link, type, member)

static inline void
ytreel_init_link(struct ytreel_link* link) {
    link->_parent = link;    /* parent of root is itself */
    ylistl_init_link(&link->_child);
    ylistl_init_link(&link->_sibling);
}

static inline int
ytreel_is_leaf(const struct ytreel_link* link) {
    return ylistl_is_empty(&link->_child);
}

static inline void
ytreel_add_next(struct ytreel_link* link, struct ytreel_link* lnew) {
    ylistl_add_next(&link->_sibling, &lnew->_sibling);
    lnew->_parent = link->_parent;
}

static inline void
ytreel_add_prev(struct ytreel_link* link, struct ytreel_link* lnew) {
    ylistl_add_prev(&link->_sibling, &lnew->_sibling);
    lnew->_parent = link->_parent;
}

static inline void
ytreel_add_first_child(struct ytreel_link* link, struct ytreel_link* lnew) {
    ylistl_add_next(&link->_child, &lnew->_sibling);
    lnew->_parent = link;
}

static inline void
ytreel_add_last_child(struct ytreel_link* link, struct ytreel_link* lnew) {
    ylistl_add_prev(&link->_child, &lnew->_sibling);
    lnew->_parent = link;
}

/**
 * Delete link and all its subtree from the container tree link.
 */
static inline void
ytreel_del(struct ytreel_link* link) {
    ylistl_del(&link->_sibling);   /* separate from container tree link */
}

/**
 * replace tree link (includes all sub tree)
 */
static inline void
ytreel_replace(struct ytreel_link* old, struct ytreel_link* lnew) {
    lnew->_parent = old->_parent;
    ylistl_replace(&old->_sibling, &lnew->_sibling);
}

static inline struct ytreel_link*
ytreel_next(const struct ytreel_link* l) {
    return container_of(l->_sibling._next, struct ytreel_link, _sibling);
}

static inline struct ytreel_link*
ytreel_prev(const struct ytreel_link* l) {
    return container_of(l->_sibling._prev, struct ytreel_link, _sibling);
}

static inline int
ytreel_has_next(const struct ytreel_link* l) {
    /*
     * All links have parent except for pseudo node. 
     * But pseudo node is inside blackbox.
     * So, we can assume that @l is not pseudo node.
     */
    yassert(l->_parent != l);
    return &l->_parent->_child != l->_sibling._next;
}

static inline int
ytreel_has_prev(const struct ytreel_link* l) {
    yassert(l->_parent != l); /* see 'ytreel_has_next' */
    return &l->_parent->_child != l->_sibling._prev;
}

static inline struct ytreel_link*
ytreel_first_child(const struct ytreel_link* l) {
    return container_of(l->_child._next, struct ytreel_link, _sibling);
}

static inline struct ytreel_link*
ytreel_last_child(const struct ytreel_link* l) {
    return container_of(l->_child._prev, struct ytreel_link, _sibling);
}

/**
 * return number of siblings of this tree link.
 * (including itself)
 */
static inline unsigned int
ytreel_sibling_size(const struct ytreel_link* l) {
    /* even if &l->sibling is not head of list, counting size is ok */
    return ylistl_size(&l->_sibling);
}

/**
 * return number of children of this tree link.
 */
static inline unsigned int
ytreel_child_size(const struct ytreel_link* l) {
    return ylistl_size(&l->_child);
}



#ifdef __cplusplus
}
#endif

#endif /* _YTREEl_h_ */
