/******************************************************************************
 * Copyright (C) 2016
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
#include <string.h>

#include "common.h"
#include "msg.h"



#define MSG_POOL_SIZE 100  /* Should this be configurable? */


struct pool {
	pthread_mutex_t lock;
	int sz;
	struct ylistl_link hd;
};

static struct pool _pool;


/******************************************************************************
 *
 *
 *
 *****************************************************************************/
static INLINE void
mclean(struct ymsg_ *m) {
	memset(m, 0, sizeof(*m));
}

static void
mdestroy(struct ymsg_ *m) {
	if (unlikely(!m))
		return;
	if (likely(YMSG_TYP_INVALID != m->m.type
		   && m->m.dfree
		   && m->m.data))
		(*m->m.dfree)(m->m.data);
	yfree(m);
}


/******************************************************************************
 *
 * Message pool
 *
 *****************************************************************************/
static INLINE void
pool_init(void) {
	int r __unused;
	r = pthread_mutex_init(&_pool.lock, NULL);
	yassert(!r);
	ylistl_init_link(&_pool.hd);
}

static INLINE int
pool_lock(void) {
	int r = pthread_mutex_lock(&_pool.lock);
	yassert(!r);
	return r;
}

static INLINE int
pool_unlock(void) {
	int r = pthread_mutex_unlock(&_pool.lock);
	yassert(!r);
	return r;
}

static struct ymsg_ *
pool_get(void) {
	struct ylistl_link *lk;
	struct ymsg_ *m = NULL;
	pool_lock(); /* return value is ignored intentionally */
	if (unlikely(ylistl_is_empty(&_pool.hd)))
		goto done_unlock;
	lk = ylistl_remove_first(&_pool.hd);
	yassert(_pool.sz > 0);
	_pool.sz--;
	m = containerof(lk, struct ymsg_, lk);
 done_unlock:
	pool_unlock();
	if (likely(m))
		mclean(m);
	return m;
}

static bool
pool_put(struct ymsg_ *m) {
	bool r = FALSE;
	pool_lock();
	if (likely(_pool.sz < MSG_POOL_SIZE)) {
		ylistl_add_last(&_pool.hd, &m->lk);
		_pool.sz++;
		r = TRUE;
	}
	pool_unlock();
	return r;
}

static void
pool_clear(void) {
	struct ylistl_link *lk;
	pool_lock();
	while (!ylistl_is_empty(&_pool.hd)) {
		lk = ylistl_remove_first(&_pool.hd);
		yfree(containerof(lk, struct ymsg_, lk));
	}
	_pool.sz = 0;
	pool_unlock();
}


/******************************************************************************
 *
 *
 *
 *****************************************************************************/
struct ymsg *
ymsg_create(void) {
	struct ymsg_ *m = pool_get();
	if (unlikely(!m))
		m = (struct ymsg_ *)ycalloc(1, sizeof(*m));
	if (unlikely(!m))
		return NULL;
	ylistl_init_link(&m->lk);
	_msg_magic_set(m);
	return &m->m;
}

void
ymsg_destroy(struct ymsg *ym) {
	struct ymsg_ *m = msg_mutate(ym);
	if (unlikely(!pool_put(m)))
		mdestroy(m);
}


/******************************************************************************
 *
 *
 *
 *****************************************************************************/
/*
 * This function is used for testing and debugging.
 */
void
msg_clear_pool(void) {
	pool_clear();
}


static void init(void) __attribute__ ((constructor));
static void
init(void) {
	pool_init();
}
