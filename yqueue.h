/**
 * Operation to the empty list is not defined. It's consumer's responsibility!
 */

#ifndef __YQUEUe_h__
#define __YQUEUe_h__

#include "ylist.h"

struct yqueue {
	struct ylist    __l;
};

static inline struct yqueue*
yqueue_create(void(*freecb)(void*)) {
	return (struct yqueue*)ylist_create(freecb);
}

static inline struct yqueue*
yqueue_reset(struct yqueue* q) {
	ylist_reset(&q->__l);
	return q;
}

static inline void
yqueue_destroy(struct yqueue* q) {
	ylist_destroy(&q->__l);
}

static inline struct ylist*
yqueue_list(const struct yqueue* q) {
	/* to make gcc happy from warning */
	return &((struct yqueue*)q)->__l;
}

static inline struct yqueue*
yqueue_en(struct yqueue* q, void* item) {
	ylist_add_last(&q->__l, ylist_create_node(item));
	return q;
}

static inline void*
yqueue_de(struct yqueue* q) {
	return ylist_free_node(ylist_del_first(&q->__l));
}

static inline void*
yqueue_peek(const struct yqueue* q) {
	return ylist_peek_first(&q->__l);
}

static inline int
yqueue_is_empty(const struct yqueue* q) {
	return ylist_is_empty(&q->__l);
}

static inline unsigned int
yqueue_size(const struct yqueue* q) {
	return ylist_size(&q->__l);
}

#endif /* __YQUEUe_h__ */
