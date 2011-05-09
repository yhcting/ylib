/**
 * Operation to the empty list is not defined. It's consumer's responsibility!
 */

#ifndef __YQUEUe_h__
#define __YQUEUe_h__

#include "ylist.h"

struct yqueue {
	struct ylist*   __l;
	unsigned int    __max; /**< maximun queue size allowed. */
	unsigned int    __sz;  /**< current queue size */
};

static inline struct yqueue*
yqueue_create(void(*freecb)(void*)) {
	struct yqueue* q = ymalloc(sizeof(*q));
	q->__l = ylist_create(freecb);
	/* maximum value at 2's complement system */
	q->__max = (unsigned int)((int)-1);
	q->__sz = 0;
	return q;
}

static inline struct yqueue*
yqueue_reset(struct yqueue* q) {
	ylist_reset(q->__l);
	q->__sz = 0;
	return q;
}

static inline void
yqueue_destroy(struct yqueue* q) {
	ylist_destroy(q->__l);
	yfree(q);
}

static inline struct ylist*
yqueue_list(const struct yqueue* q) {
	/* to make gcc happy from warning */
	return q->__l;
}

static inline int
yqueue_en(struct yqueue* q, void* item) {
	if (q->__sz < q->__max) {
		ylist_add_last(q->__l, ylist_create_node(item));
		q->__sz++;
		return 0;
	}
	else
		return -1;
}

static inline void*
yqueue_de(struct yqueue* q) {
	if (q->__sz > 0) {
		q->__sz--;
		return ylist_free_node(ylist_del_first(q->__l));
	} else
		return NULL;
}

static inline void*
yqueue_peek(const struct yqueue* q) {
	return ylist_peek_first(q->__l);
}

static inline int
yqueue_is_empty(const struct yqueue* q) {
	return ylist_is_empty(q->__l);
}

static inline unsigned int
yqueue_size(const struct yqueue* q) {
	return q->__sz;
}

#endif /* __YQUEUe_h__ */
