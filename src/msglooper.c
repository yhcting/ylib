/******************************************************************************
 * Copyright (C) 2016, 2021
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
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>

#include "lib.h"
#include "common.h"
#include "yerrno.h"
#include "ylog.h"
#include "yut.h"
#include "ymsgq.h"
#include "msg.h"
#include "ymsglooper.h"
#include "msghandler.h"

struct looper_event_data {
	struct ylistl_link lk;
	int fd; /**< File descriptor added to epoll */
	ymsglooper_eventcb_t cb; /**< Callback called  */
	void *data; /**< User data */
};

struct ymsglooper {
	int epfd; /**< epoll fd */
	int ctlfd; /**< control fd */
	struct ylistl_link evds; /**< event datas */
	struct looper_event_data ctled; /* ConTroL Event Data */
	pthread_t thread; /**< Attached thread */
	pthread_mutex_t lock; /**< lock general lock for msglooper */
	volatile enum ymsglooper_state state; /**< state of looper */
};


/**
 * argument of looper thread
 */
struct looper_thread_share {
	pthread_cond_t cond; /**< [IN] input of thread */
	pthread_mutex_t lock; /**< [IN] input of thread */
	int result; /**< [OUT] 0: initial. >0 ready. -errno: err */
	struct ymsglooper *ml; /**< [OUT] */
};

static pthread_key_t _tkey;  /* thread key for msglooper */


/******************************************************************************
 *
 *
 *
 *****************************************************************************/
static void
looper_control(int fd, int events, void *data) {
	/* nothing to do except for consume */
	int64_t v;
	if (unlikely(sizeof(v) != read(fd, &v, sizeof(v)))) {
		/* TOOD: Any thing to do here ? */
		;
	}
}

static struct ymsglooper *
create_msglooper(pthread_t thread) {
	struct epoll_event ev;
	struct ymsglooper *ml = ymalloc(sizeof(*ml));
	if (unlikely(!ml))
		return NULL;
	if (unlikely(0 > (ml->ctlfd = eventfd(0, 0))))
		goto free_ml;
	if (unlikely(0 > (ml->epfd = epoll_create1(EPOLL_CLOEXEC))))
		goto close_ctlfd;
	ylistl_init_link(&ml->evds);
	ev.events = EPOLLIN;
	ev.data.ptr = &ml->ctled;
	ml->ctled.fd = ml->ctlfd;
	ml->ctled.cb = &looper_control;
	ml->ctled.data = NULL;
	epoll_ctl(ml->epfd, EPOLL_CTL_ADD, ml->ctlfd, &ev);
	yassert(thread); /* NOT NULL */
	ml->thread = thread;
	if (unlikely(pthread_mutex_init(&ml->lock, NULL)))
		goto close_epfd;
	return ml;
 close_epfd:
	close(ml->epfd);
 close_ctlfd:
	close(ml->ctlfd);
 free_ml:
	yfree(ml);
	return NULL;
}

static void
destroy_msglooper(void *arg) {
	/* ignore all return value intentionally.
	 * We can't do anything for errors
	 */
	struct looper_event_data *cur, *tmp;
	struct ymsglooper *ml = (struct ymsglooper *)arg;
	ylistl_foreach_item_safe(
		cur, tmp, &ml->evds, struct looper_event_data, lk
	) {
		ylistl_remove(&cur->lk);
		yfree(cur);
	}
	pthread_mutex_destroy(&ml->lock);
	close(ml->ctlfd);
	close(ml->epfd);
	yfree(ml);
}


static inline void
do_lock(struct ymsglooper *ml) {
        fatali0(pthread_mutex_lock(&ml->lock));
}

static inline void
do_unlock(struct ymsglooper *ml) {
        fatali0(pthread_mutex_unlock(&ml->lock));
}

static inline void
set_state(struct ymsglooper *ml, enum ymsglooper_state state) {
	__atomic_store_n(&ml->state, state, __ATOMIC_SEQ_CST);
}

static inline enum ymsglooper_state
get_state(struct ymsglooper *ml) {
	return __atomic_load_n(&ml->state, __ATOMIC_SEQ_CST);
}


static inline struct ymsg *
create_dummy_msg(void) {
	/* initialized dummy message. */
	struct ymsg *m = ymsg_create();
	ymsg_set_data(m, YMSG_PRI_NORMAL, 0, 0, NULL, NULL);
	return m;
}

/*
static inline void
looper_cleanup(void *arg) {
	struct ymsglooper *ml = arg;
}
*/

static void *
looper_thread(void *arg) {
	int r;
	struct looper_thread_share *ts = (struct looper_thread_share *)arg;
	dfpr("looper thread");
	r = ymsglooper_create();
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
	/* Fron now on '*ts' is NOT-safe, anymore! */
	if (unlikely(r))
		return NULL;

	/* pthread_cleanup_push(&looper_cleanup, ymsglooper_get()); */
	ymsglooper_loop();
	/* pthread_cleanup_pop(1); */ /* execute cleanup */
	return NULL;
}

/******************************************************************************
 *
 *
 *
 *****************************************************************************/
int
ymsglooper_create(void) {
	struct ymsglooper *ml;
	ml = ymsglooper_get();
	if (unlikely(ml))
		/* looper is already assigned! */
		return -EPERM;
	ml = create_msglooper(pthread_self());
	if (unlikely(!ml))
		return -EINVAL; /* unknown */
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
ymsglooper_add_fd(struct ymsglooper *ml,
	int fd, int events, ymsglooper_eventcb_t cb, void *data
) {
	int err;
	struct epoll_event epev;
	struct looper_event_data *ed;

	if (unlikely(!cb))
		return -EINVAL;

	ed = ymalloc(sizeof(*ed));
	if (unlikely(!ed))
		return -ENOMEM;
	ed->fd = fd;
	ed->data = data;
	ed->cb = cb;
	epev.events = events;
	epev.data.ptr = ed;
	if (unlikely(epoll_ctl(ml->epfd, EPOLL_CTL_ADD, fd, &epev))) {
		err = errno;
		yfree(ed);
		return -err;
	}
	do_lock(ml);
	ylistl_add_last(&ml->evds, &ed->lk);
	do_unlock(ml);
	return 0;
}

int
ymsglooper_del_fd(struct ymsglooper *ml, int fd) {
	struct looper_event_data *cur;
	do_lock(ml);
	ylistl_foreach_item(cur, &ml->evds, struct looper_event_data, lk) {
		if (fd == cur->fd) {
			ylistl_remove(&cur->lk);
			yfree(cur);
			break;
		}
	}
	do_unlock(ml);
	return 0;
}

int
ymsglooper_loop(void) {
#define MAX_EP_EVENTS 64
	int r;
	struct epoll_event epevs[MAX_EP_EVENTS];
	enum ymsglooper_state expected = YMSGLOOPER_READY;
	struct ymsglooper *ml = ymsglooper_get();
	if (unlikely(!ml))
		return -EPERM;
	r = __atomic_compare_exchange_n(&ml->state, &expected,
		YMSGLOOPER_LOOP, FALSE, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
	if (unlikely(!r))
		goto skip_loop;
	dfpr("start LOOP!");
	while (TRUE) {
		int i, nfds;
		nfds = epoll_wait(ml->epfd, epevs, yut_arrsz(epevs), -1);
		for (i = 0; i < nfds; i++) {
			struct looper_event_data *ed = epevs[i].data.ptr;
			(*ed->cb)(ed->fd, epevs[i].events, ed->data);
		}
		if (YMSGLOOPER_STOPPING == get_state(ml)) {
			dfpr("break!!!! from loop!");
			break;
		}
	}
 skip_loop:
	set_state(ml, YMSGLOOPER_TERMINATED);
	r = pthread_setspecific(_tkey, NULL);
	yassert(!r);
	return 0;
#undef MAX_EP_EVENTS
}

struct ymsglooper *
ymsglooper_start_looper_thread(void) {
	int r, unused r0;
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

	fatali0(pthread_mutex_lock(&ts.lock));
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
	enum ymsglooper_state st;
	st = __atomic_exchange_n(&ml->state,
		YMSGLOOPER_STOPPING, __ATOMIC_SEQ_CST);
	switch (st) {
	case YMSGLOOPER_READY:
	case YMSGLOOPER_LOOP: {
		int64_t v = 1;
		if (unlikely(sizeof(v) != write(ml->ctlfd, &v, sizeof(v)))) {
			set_state(ml, YMSGLOOPER_LOOP);
			return -errno;
		}
	} break;
	case YMSGLOOPER_TERMINATED:
		/* back to terminated state */
		set_state(ml, YMSGLOOPER_TERMINATED);
	break;
	case YMSGLOOPER_STOPPING:
	break; /* do nothing */
	}
	return 0;
}


/******************************************************************************
 *
 *
 *
 *****************************************************************************/
#ifdef CONFIG_TEST
/*
 * This function is used for testing and debugging.
 */
extern void msg_clear(void);

void
msglooper_clear(void) {
	msg_clear();
}
#endif /* CONFIG_TEST */

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
	unused int r;
	r = pthread_key_delete(_tkey);
	yassert(!r);
}

LIB_MODULE(msglooper, minit, mexit) /* @suppress("Unused static function") */
