/**
 * ylistl => ylist Low level.
 * Low level list.
 */


#ifndef _YLISTl_h_
#define _YLISTl_h_

#include "ydef.h"

#ifdef __cplusplus
extern "C" {
#endif


/*================================
 * Define primitive operation of linked list!
 * User should understand algorithm fully before using this!
 * (From "include/linux/list.h" (with a little modification - 'GCC' dependent factors are removed))
 *================================*/

/**
 * If possible DO NOT access struct directly!.
 */
struct ylistl_link {
    struct ylistl_link *_next, *_prev;
};

/**
 * initialize list head.
 */
static inline void
ylistl_init_link(struct ylistl_link* link) {
    link->_next = link->_prev = link;
}

static inline int
ylistl_is_empty(const struct ylistl_link* head) {
    return head->_next == head;
}

static inline void
ylistl_add(struct ylistl_link* prev, struct ylistl_link* next, struct ylistl_link* anew) {
    next->_prev = prev->_next = anew;
    anew->_next = next; anew->_prev = prev;
}

static inline void
ylistl_add_next(struct ylistl_link* link, struct ylistl_link* anew) {
    ylistl_add(link, link->_next, anew);
}

static inline void
ylistl_add_prev(struct ylistl_link* link, struct ylistl_link* anew) {
    ylistl_add(link->_prev, link, anew);
}

static inline void
ylistl_add_first(struct ylistl_link* head, struct ylistl_link* anew) {
    ylistl_add_next(head, anew);
}

static inline void
ylistl_add_last(struct ylistl_link* head, struct ylistl_link* anew) {
    ylistl_add_prev(head, anew);
}

static inline void
__ylistl_del(struct ylistl_link* prev, struct ylistl_link* next) {
    prev->_next = next;
    next->_prev = prev;
}

static inline void
ylistl_del(struct ylistl_link* link) {
    __ylistl_del(link->_prev, link->_next);
}

static inline void
ylistl_replace(struct ylistl_link* old, struct ylistl_link* anew) {
    anew->_next = old->_next;
    anew->_next->_prev = anew;
    anew->_prev = old->_prev;
    anew->_prev->_next = anew;
}

/**
 * @pos     : the &struct ylistl_link to use as a loop cursor
 * @head    : head of list (&struct ylistl_link)
 */
#define ylistl_foreach(pos, head) \
        for((pos) = (head)->_next; (pos) != (head); (pos) = (pos)->_next)

#define ylistl_foreach_backward(pos, head) \
        for((pos) = (head)->_prev; (pos) != (head); (pos) = (pos)->_prev)

/**
 * @pos     : the &struct ylistl_link to use as a loop cursor
 * @n       : another &struct ylistl_link to use as temporary storage
 * @head    : head of list (&struct ylistl_link)
 */
#define ylistl_foreach_removal_safe(pos, n, head) \
        for((pos) = (head), (n) = (pos)->_next; (pos) != (head); (pos) = (n), (n) = (pos)->_next)

#define ylistl_foreach_removal_safe_backward(pos, n, head) \
        for((pos) = (head), (n) = (pos)->_prev; (pos) != (head); (pos) = (n), (n) = (pos)->_prev)
/**
 * @pos     : the @type* to use as a loop cursor.
 * @head    : the head for list (&struct ylistl_link)
 * @type    : the type of the struct of *@pos
 * @member  : the name of the ylistl_link within the struct.
 */
#define ylistl_foreach_item(pos, head, type, member)            \
        for((pos) = container_of((head)->_next, type, member);   \
            &(pos)->member != (head);                           \
            (pos) = container_of((pos)->member._next, type, member))

#define ylistl_foreach_item_backward(pos, head, type, member)   \
        for((pos) = container_of((head)->_prev, type, member);   \
            &(pos)->member != (head);                           \
            (pos) = container_of((pos)->member._prev, type, member))

/**
 * @type    : the type of the struct of *@pos
 * @pos     : the @type* to use as a loop cursor.
 * @n       : another @type* to use as temporary storage.
 * @head    : the head for list (&struct ylistl_link)
 * @member  : the name of the ylistl_link within the struct.
 */
#define ylistl_foreach_item_removal_safe(pos, n, head, type, member)    \
        for((pos) = container_of((head)->_next, type, member),           \
                (n) = container_of((pos)->member._next, type, member);   \
            &(pos)->member != (head);                                   \
            (pos) = (n), (n) = container_of((pos)->member._next, type, member))

#define ylistl_foreach_item_removal_safe_backward(pos, n, head, type, member)   \
        for((pos) = container_of((head)->_prev, type, member),                   \
                (n) = container_of((pos)->member._prev, type, member);           \
            &(pos)->member != (head);                                           \
            (pos) = (n), (n) = container_of((pos)->member._prev, type, member))

static inline unsigned int
ylistl_size(const struct ylistl_link* head) {
    struct ylistl_link*   pos;
    unsigned int size = 0;
    ylistl_foreach(pos, head) { size++; }
    return size;
}




#ifdef __cplusplus
}
#endif

#endif /* _YLISTl_h_ */
