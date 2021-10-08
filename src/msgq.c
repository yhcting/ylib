/******************************************************************************
 * Copyright (C) 2011, 2012, 2013, 2014, 2015, 2021
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
#include <sys/eventfd.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

#include "common.h"
#include "yut.h"
#include "msg.h"
#include "ymsgq.h"

/* Android NDK doesn't define CLOCK_MONOTONIC_RAW */
#ifndef CLOCK_MONOTONIC_RAW
#define CLOCK_MONOTONIC_RAW 4
#endif

#define sec2ns(x) ((x) * 1000 * 1000 * 1000)

struct ymsgq {
	struct ylistl_link q[YMSG_PRI_NR];
	u32 sz;
	/* This SHOULD NOT be changed once it is set at ymsgq_create()
	 * This module assumes that this value is constant and only readable.
	 */
	u32 capacity;
	pthread_mutex_t lock;
	int evfd; /* event fd */
};


/******************************************************************************
 *
 *
 *
 *****************************************************************************/
#ifdef CONFIG_DEBUG

/*
 * return 0 if error otherwise CLOCK_MONOTONIC_RAW is returned.
 * (nano-seconds - OVERFLOW is NOT considered)
 */
static INLINE u64
monotonic_time(void) {
	struct timespec ts = {0, 0};
	/* ignore return value intentionally */
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	return sec2ns(ts.tv_sec) + ts.tv_nsec;
}



static INLINE void
set_time_stamp(struct ymsg_ *m) {
	m->when = monotonic_time();
}

#else /* CONFIG_DEBUG */

static INLINE void set_time_stamp(struct ymsg_ *m) { }

#endif /* CONFIG_DEBUG */



/******************************************************************************
 *
 *
 *
 *****************************************************************************/
static int
qen(struct ymsgq *q, struct ymsg_ *m) {
	int r;
	int64_t v = 1;
	r = pthread_mutex_lock(&q->lock);
	yassert(!r);
	if (unlikely(q->sz >= q->capacity)) {
		r = pthread_mutex_unlock(&q->lock);
		yassert(!r);
		return -EAGAIN;
	}
	set_time_stamp(m);
	ylistl_add_last(&q->q[m->m.pri], &m->lk);
	++q->sz;
	r = pthread_mutex_unlock(&q->lock);
	yassert(!r);
	r = write(q->evfd, &v, sizeof(v));
	yassert(r == sizeof(v));
	return 0;
}

/******************************************************************************
 *
 * Y Message Queue
 *
 *****************************************************************************/
struct ymsgq *
ymsgq_create(int capacity) {
	int i;
	int r __unused;
	struct ymsgq *q = ymalloc(sizeof(*q));
	if (unlikely(!q))
		return NULL; /* OOM */
	for (i = 0; i < yut_arrsz(q->q); i++)
		ylistl_init_link(&q->q[i]);
	q->capacity = capacity <= 0 ? 0xffffffff : (u32)capacity;
	q->sz = 0;
	if (unlikely(pthread_mutex_init(&q->lock, NULL)))
		goto free_q;
	if (unlikely(0 > (q->evfd = eventfd(0, EFD_SEMAPHORE | EFD_CLOEXEC))))
		goto free_lock;
	return q;

 free_lock:
	r = pthread_mutex_destroy(&q->lock);
	yassert(!r);
 free_q:
	yfree(q);
	return NULL;
}

void
ymsgq_destroy(struct ymsgq *q) {
	int i;
	int r __unused;
	struct ymsg_ *pos, *tmp;
	r = close(q->evfd);
	yassert(!r);
	r = pthread_mutex_lock(&q->lock);
	yassert(!r);
	for (i = 0; i < yut_arrsz(q->q); i++) {
		ylistl_foreach_item_safe(
			pos,
			tmp,
			&q->q[i],
			struct ymsg_,
			lk
		) {
			ymsg_destroy(&pos->m);
		}
	}
	r = pthread_mutex_unlock(&q->lock);
	yassert(!r);
	r = pthread_mutex_destroy(&q->lock);
	yassert(!r);
	yfree(q);
}

int
ymsgq_evfd(const struct ymsgq *q) {
	return q->evfd;
}

int
ymsgq_en(struct ymsgq *q, struct ymsg *ym) {
	if (unlikely(!q
		|| !ym
		|| YMSG_TYP_INVALID == ym->type
		|| YMSG_PRI_NR <= ym->pri)
	) { return -EINVAL; }
	return qen(q, msg_mutate(ym));
}


/**
 * NULL for error
 */
struct ymsg *
ymsgq_de(struct ymsgq *q) {
	int i, r;
	int64_t v;
	struct ylistl_link *lk;

	/* read with block */
	r = read(q->evfd, &v, sizeof(v));
	yassert(r == sizeof(v));
	r = pthread_mutex_lock(&q->lock);
	yassert(!r);
	lk = NULL;
	/* Highest priority = index 0 */
	for (i = 0 ; i < yut_arrsz(q->q); i++) {
		if (!ylistl_is_empty(&q->q[i])) {
			/* dequeue */
			lk = q->q[i].next;
			ylistl_remove(lk);
			--q->sz;
			break;
		}
	}
	r = pthread_mutex_unlock(&q->lock);
	yassert(!r);
	return lk ? &containerof(lk, struct ymsg_, lk)->m : NULL;
}

u32
ymsgq_sz(const struct ymsgq *q) {
	/* In most platform, read int is atomic operation.
	 * That's why 'lock' is NOT used here.
	 */
	return q->sz;
}
