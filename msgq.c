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

