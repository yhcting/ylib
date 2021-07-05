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

#include <string.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>

#include "lib.h"
#include "common.h"
#include "yerrno.h"
#include "ylog.h"
#include "ymsgq.h"
#include "msg.h"
#include "ymsglooper.h"
#include "msghandler.h"

/**
 * [NOTE]: 'stop' field.
 * 'stop' field is NOT multithreading-safe(May NOT atomic. But, it's ok.
 * It's just boolean value. And if it's dirty due to any-reason
 *   (including race-condition), we can tell as 'stop == TRUE'.
 * So, we don't need to lock 'read' operation of 'stop'.
 * ('stop' field is read very frequently(every loop).
 * Therefore we can get good performance benefits.
 * But, in case of write, things are different.
 * Concurrent-wrting may result-in unexpected value.
 * 'write' operation should be synchronized.
 */
struct ymsglooper {
	struct ymsgq *mq; /**< Message Q. */
	pthread_t thread; /**< Attached thread */
	bool destroy_on_exit; /**< destroy instance on exiting thread */
	pthread_mutex_t state_lock; /**< lock for {@code state} */
	volatile enum ymsglooper_state state; /**< state of looper */
};


/**
 * argument of looper thread
 */
struct looper_thread_share {
	pthread_cond_t cond; /**< [IN] input of thread */
	pthread_mutex_t lock; /**< [IN] input of thread */
	bool destroy_on_exit; /**< [IN] destroy instance on exiting thread */
	int result; /**< [OUT] 0: initial. >0 ready. -errno: err */
	struct ymsglooper *ml; /**< [OUT] */
};

static pthread_key_t _tkey;  /* thread key for msglooper */


/******************************************************************************
 *
 *
 *
 *****************************************************************************/
static struct ymsglooper *
create_msglooper(pthread_t thread, int msgq_capacity) {
	struct ymsglooper *ml = ymalloc(sizeof(*ml));
	if (unlikely(!ml))
		return NULL;
	ml->mq = ymsgq_create(msgq_capacity);
	if (unlikely(!ml->mq))
		goto free_ml;
	yassert(thread); /* NOT NULL */
	ml->thread = thread;
	if (unlikely(pthread_mutex_init(&ml->state_lock, NULL)))
		goto free_mq;
	return ml;

 free_mq:
	ymsgq_destroy(ml->mq);
 free_ml:
	yfree(ml);
	return NULL;
}

static void
dummy_do_nothing(void *arg __unused) { }

static void
destroy_msglooper(void *arg) {
	/* ignore all return value intentionally.
	 * We can't do anything for errors
	 */
	struct ymsglooper *ml = (struct ymsglooper *)arg;
	pthread_mutex_destroy(&ml->state_lock);
	ymsgq_destroy(ml->mq);
	yfree(ml);
}

static inline void
lock_state(struct ymsglooper *ml) {
        fatali0(pthread_mutex_lock(&ml->state_lock));
}

static inline void
unlock_state(struct ymsglooper *ml) {
        fatali0(pthread_mutex_unlock(&ml->state_lock));
}

static void
set_state_locked(struct ymsglooper *ml, enum ymsglooper_state state) {
	ml->state = state;
}

static void
set_state(struct ymsglooper *ml, enum ymsglooper_state state) {
	lock_state(ml);
	set_state_locked(ml, state);
	unlock_state(ml);
}

static enum ymsglooper_state
get_state_locked(struct ymsglooper *ml) {
	return ml->state;
}

static enum ymsglooper_state
get_state(struct ymsglooper *ml) {
	enum ymsglooper_state st;
	lock_state(ml);
	st = get_state_locked(ml);
	unlock_state(ml);
	return st;
}


static struct ymsg *
create_dummy_msg(void) {
	/* initialized dummy message. */
	struct ymsg *m = ymsg_create();
	ymsg_set_data(m, YMSG_PRI_NORMAL, 0, 0, NULL, NULL);
	return m;
}

static void *
looper_thread(void *arg) {
	int r;
	void (*cleanup)(void *);
	struct looper_thread_share *ts = (struct looper_thread_share *)arg;
	dfpr("looper thread");
	r = ymsglooper_create(0, ts->destroy_on_exit);
        fatali0(pthread_mutex_lock(&ts->lock));
	if (unlikely(r)) {
		ts->result = r;
		goto cond_done;
	}
	ts->result = 1; /* success */
	ts->ml = ymsglooper_get();

 cond_done:
        fatali0(pthread_cond_broadcast(&ts->cond));
        fatali0(pthread_mutex_unlock(&ts->lock));
	if (unlikely(r))
	    return NULL;

	cleanup = ts->destroy_on_exit ? &destroy_msglooper : &dummy_do_nothing;
	pthread_cleanup_push(cleanup, ts->ml);
	ymsglooper_loop();
	pthread_cleanup_pop(1); /* execute cleanup */
	return NULL;
}

/******************************************************************************
 *
 *
 *
 *****************************************************************************/
int
ymsglooper_create(int msgq_capacity, bool destroy_on_exit) {
	struct ymsglooper *ml;
	ml = ymsglooper_get();
	if (unlikely(ml))
		/* looper is already assigned! */
		return -EPERM;
	ml = create_msglooper(pthread_self(), msgq_capacity);
	if (unlikely(!ml))
		return -EINVAL; /* unknown */
	ml->destroy_on_exit = destroy_on_exit;
	fatali0(pthread_setspecific(_tkey, ml));
	set_state(ml, YMSGLOOPER_READY);
	return 0;
}

int
ymsglooper_destroy(struct ymsglooper *ml) {
	if (unlikely(YMSGLOOPER_TERMINATED != get_state(ml)))
		return -EPERM;
	destroy_msglooper(ml);
	return 0;
}

int
ymsglooper_loop(void) {
	int r __unused;
	struct ymsglooper *ml = ymsglooper_get();
	if (unlikely(!ml))
		return -EPERM;
	dfpr("start LOOP!");
	lock_state(ml);
	if (YMSGLOOPER_READY != get_state_locked(ml)) {
		unlock_state(ml);
		goto skip_loop;
	}
	set_state_locked(ml, YMSGLOOPER_LOOP);
	unlock_state(ml);
	while (TRUE) {
		struct ymsg *m = ymsgq_de(ml->mq);
		struct ymsg_ *m_ = msg_mutate(m);
		/* Operation is NOT locked intentionally.
		 * See comment for 'struct ymsglooper'.
		 */
		if (YMSGLOOPER_STOPPING == get_state(ml)) {
			if (likely(m))
				ymsg_destroy(m);
			dfpr("break!!!! from loop!");
			break;
		}
		if (unlikely(!m))
			continue;
		/* Handler code here! */
		yassert(m_->handler && m_->handler->handle);
		(*m_->handler->handle)(m_->handler, m);
		ymsg_destroy(m);
	}
 skip_loop:
	r = pthread_setspecific(_tkey, NULL);
	yassert(!r);
	set_state(ml, YMSGLOOPER_TERMINATED);
	return 0;
}

struct ymsglooper *
ymsglooper_start_looper_thread(bool destroy_on_exit) {
	int r __unused;
	int r0 __unused;
	pthread_t thread;
	struct looper_thread_share ts;

	dfpr("Enter");
	memset(&ts, 0, sizeof(ts));

	r = pthread_cond_init(&ts.cond, NULL);
	if (unlikely(r))
		goto done;

	r = pthread_mutex_init(&ts.lock, NULL);
	if (unlikely(r))
		goto free_cond;
	ts.destroy_on_exit = destroy_on_exit;
	r = pthread_create(
		&thread,
		NULL,
		&looper_thread,
		&ts);
	if (unlikely(r)) {
		yassert(0);
		ylogf("Fail to create thread: %s\n", yerrno_str(r));
		goto free_cond;
	}

	r = pthread_mutex_lock(&ts.lock);
	if (unlikely(r))
		goto free_lock;
	if (!ts.result) {
		r = pthread_cond_wait(&ts.cond, &ts.lock);
		if (unlikely(r)) {
			r0 = pthread_mutex_unlock(&ts.lock);
			yassert(!r0);
			goto free_lock;
		}
	}
	yassert(ts.result);
	r = pthread_mutex_unlock(&ts.lock);
	yassert(!r);

	if (ts.result < 0)
		r = -ts.result; /* error ! */
	else
		yassert(ts.ml);

 free_lock:
	r0 = pthread_mutex_destroy(&ts.lock);
	yassert(!r0);
 free_cond:
	r0 = pthread_cond_destroy(&ts.cond);
	yassert(!r0);
 done:
	if (unlikely(r)) {
		dfpr("%s", yerrno_str(r));
		return NULL;
	}
	return ts.ml;
}

struct ymsgq *
ymsglooper_get_msgq(struct ymsglooper *ml) {
	/* ml->mq is read only. So, lock is NOT required */
	return ml->mq;
}

pthread_t
ymsglooper_get_thread(const struct ymsglooper *ml) {
	/* ml->thread is read only. So, lock is NOT required */
	return ml->thread;
}

struct ymsglooper *
ymsglooper_get(void) {
	return pthread_getspecific(_tkey);
}

enum ymsglooper_state
ymsglooper_get_state(struct ymsglooper *ml) {
	return get_state(ml);
}

int
ymsglooper_stop(struct ymsglooper *ml) {
	int r __unused;
	if (ymsgq_sz(ml->mq) > 0)
		return -EPERM; /* There is not-handled-message. */
	lock_state(ml);
	switch (get_state_locked(ml)) {
	case YMSGLOOPER_LOOP:
		r = ymsgq_en(ml->mq, create_dummy_msg());
		if (unlikely(r)) {
			unlock_state(ml);
			return r;
		}
		/* missing break in intention */
		/* @suppress("No break at end of case") */
	case YMSGLOOPER_READY:
		set_state_locked(ml, YMSGLOOPER_STOPPING);
		/* @suppress("No break at end of case") */
	default: /* do nothing */
		;
	}
	unlock_state(ml);
	return 0;
}


/******************************************************************************
 *
 *
 *
 *****************************************************************************/
#ifdef CONFIG_DEBUG
/*
 * This function is used for testing and debugging.
 */
extern void msg_clear(void);
void
msglooper_clear(void) {
	msg_clear();
}
#endif /* CONFIG_DEBUG */

static int
minit(const struct ylib_config *cfg) {
	/*struct ymsglooper object instance may be refered after exiting
	 * thread.
	 */
	int r = pthread_key_create(&_tkey, NULL);
	if (unlikely(r))
		return -r;
	return 0;
}

static void
mexit(void) {
	int r __unused;
	r = pthread_key_delete(_tkey);
	yassert(!r);
}

LIB_MODULE(msglooper, minit, mexit) /* @suppress("Unused static function") */
