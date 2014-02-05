/******************************************************************************
 *    Copyright (C) 2011, 2012, 2013, 2014
 *    Younghyung Cho. <yhcting77@gmail.com>
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

#include <pthread.h>
#include <time.h>
#include <errno.h>

#include "ycommon.h"
#include "ymsgq.h"

/* Android NDK doesn't define CLOCK_MONOTONIC_RAW */
#ifndef CLOCK_MONOTONIC_RAW
#define CLOCK_MONOTONIC_RAW 4
#endif

struct ymq {
	struct ylistl_link q[YMSG_PRI_NR];
	unsigned int sz;
	pthread_mutex_t m;
	pthread_cond_t cond;
};


/* ============================================================================
 *
 * Y Message
 *
 * ==========================================================================*/
/*
 * return 0 if error otherwise CLOCK_MONOTONIC_RAW is returned.
 * (nano-seconds - OVERFLOW is NOT considered)
 */
static inline uint64_t
monotonic_time(void) {
	struct timespec ts = {0, 0};
	/* ignore return value intentionally */
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	return sec2ns(ts.tv_sec) + ts.tv_nsec;
}



/* ============================================================================
 *
 * Y Message Queue
 *
 * ==========================================================================*/
struct ymq *
ymq_create(void) {
	int i;
	struct ymq *q = ymalloc(sizeof(*q));
	if (unlikely(!q))
		return NULL; /* OOM */
	for (i = 0; i < arrsz(q->q); i++)
		ylistl_init_link(&q->q[i]);
	q->sz = 0;
	pthread_mutex_init(&q->m, NULL);
	pthread_cond_init(&q->cond, NULL);
	return q;
}

void
ymq_destroy(struct ymq *q) {
	int i;
	struct ymsg *pos, *tmp;
	for (i = 0; i < arrsz(q->q); i++) {
		ylistl_foreach_item_removal_safe(pos,
						 tmp,
						 &q->q[i],
						 struct ymsg,
						 lk) {
			ymsg_destroy(pos);
		}
	}
	pthread_cond_destroy(&q->cond);
	pthread_mutex_destroy(&q->m);
	yfree(q);
}

int
ymq_en(struct ymq *q, struct ymsg *m) {
	if (unlikely(!q || !m
		     || YMSG_TYP_INVALID == m->type
		     || YMSG_PRI_NR <= m->pri))
		return EINVAL;
	m->when = monotonic_time();
	pthread_mutex_lock(&q->m);
	ylistl_add_last(&q->q[m->pri], &m->lk);
	++q->sz;
	pthread_cond_broadcast(&q->cond);
	pthread_mutex_unlock(&q->m);
	return 0;
}


/**
 * NULL for error
 */
struct ymsg *
ymq_de(struct ymq *q) {
	int i;
	struct ylistl_link *lk;

	pthread_mutex_lock(&q->m);
	if (!q->sz)
		/* queue is empty. wait! */
		pthread_cond_wait(&q->cond, &q->m);
	yassert(q->sz > 0);
	lk = NULL;
	/* Highest priority = index 0 */
	for (i = 0 ; i < arrsz(q->q); i++) {
		if (!ylistl_is_empty(&q->q[i])) {
			/* dequeue */
			lk = q->q[i].next;
			ylistl_del(lk);
			--q->sz;
			break;
		}
	}
	pthread_mutex_unlock(&q->m);
	yassert(lk);
	return container_of(lk, struct ymsg, lk);
}

unsigned int
ymq_sz(const struct ymq *q) {
	/* In most platform, read int is atomic operation.
	 * That's why 'lock' is NOT used here.
	 */
	return q->sz;
}
