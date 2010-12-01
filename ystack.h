/**
 * Operation to the empty list is not defined. It's consumer's responsibility!
 */

#ifndef __YSTACk_h__
#define __YSTACk_h__

#include "ylist.h"

struct ystack {
    struct ylist    __l;
};

static inline struct ystack*
ystack_create(void(*freecb)(void*)) {
    return (struct ystack*)ylist_create(freecb);
}

static inline struct ystack*
ystack_reset(struct ystack* s) {
    ylist_reset(&s->__l);
    return s;
}

static inline void
ystack_destroy(struct ystack* s) {
    ylist_destroy(&s->__l);
}

static inline struct ylist*
ystack_list(const struct ystack* s) {
    return &((struct ystack*)s)->__l;
}

static inline struct ystack*
ystack_push(struct ystack* s, void* item) {
    ylist_add_last(&s->__l, ylist_create_node(item));
    return s;
}

static inline void*
ystack_pop(struct ystack* s) {
    return ylist_free_node(ylist_del_last(&s->__l));
}

static inline void*
ystack_peek(const struct ystack* s) {
    return ylist_peek_last(&s->__l);
}

static inline int
ystack_is_empty(const struct ystack* s) {
    return ylist_is_empty(&s->__l);
}

static inline unsigned int
ystack_size(const struct ystack* s) {
    return ylist_size(&s->__l);
}

#endif /* __YSTACk_h__ */
