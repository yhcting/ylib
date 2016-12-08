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
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "common.h"
#include "lib.h"
#include "ythreadex.h"
#include "ylog.h"
#include "ymsghandler.h"

static long _id;
static pthread_spinlock_t _id_lock;

#define MAX_THREAD_NAME 32


struct ythreadex {
	/* ---------- READ ONLY values(set only once ----------*/
	pthread_t thread;
	long id;
	char name[MAX_THREAD_NAME];
	struct ymsghandler *owner;
	enum ythreadex_priority priority;
	struct ythreadex_listener listener;
	void (*free_arg)(void *);
	void (*free_result)(void *);
	int (*run)(struct ythreadex *, void **result);

	/* ---------- Dynamically updated values ----------*/
	enum ythreadex_state state;
	pthread_mutex_t state_lock;
	int errcode; /**< error value returned from 'run' */
	void *arg; /**< argument passed to thread */
	void *result; /**< result from thread(run) */

	/* ---------- used as internal arguments ----------*/
	bool cancel_started;
	bool cancel_pthdcancel;
	long progress_maxprog;
	long progress_prog;
#ifdef CONFIG_DEBUG
	volatile bool proginit;
#endif /* CONFIG_DEBUG */
};

/******************************************************************************
 *
 * Debug
 *
 *****************************************************************************/
#ifdef CONFIG_DEBUG
#else /* CONFIG_DEBUG */
#endif /* CONFIG_DEBUG */


/******************************************************************************
 *
 * Lock
 *
 *****************************************************************************/
#define declare_locker_unlocker(tYPE, nAME)                             \
        static inline void                                              \
        lock_##nAME(struct ythreadex *threadex) {                       \
                fatali0(pthread_##tYPE##_lock(&threadex->nAME##_lock)); \
        }                                                               \
        static inline void                                              \
        unlock_##nAME(struct ythreadex *threadex) {                     \
                fatali0(pthread_##tYPE##_unlock(&threadex->nAME##_lock)); \
        }

declare_locker_unlocker(mutex, state)

#undef declare_locker_unlocker

/******************************************************************************
 *
 * Getters / Setters
 *
 *****************************************************************************/
static long
gen_unique_id(void) {
	long id;
        fatali0(pthread_spin_lock(&_id_lock));
	id = ++_id;
        fatali0(pthread_spin_unlock(&_id_lock));
	return id;
}

/* ------------------------------------------------------------------------- */
#define declare_locked_getter_setter(tYPE, fIELD)                       \
        static inline tYPE                                              \
        get_##fIELD##_locked(struct ythreadex *threadex) {              \
                return threadex->fIELD;                                 \
        }                                                               \
        static inline void                                              \
        set_##fIELD##_locked(struct ythreadex *threadex, tYPE value) {  \
                threadex->fIELD = value;                                \
        }

declare_locked_getter_setter(enum ythreadex_state, state)

#undef declare_locked_getter_setter

/* ------------------------------------------------------------------------- */
#define declare_with_lock_getter_setter(tYPE, fIELD)		\
        static inline tYPE                                      \
        get_##fIELD(struct ythreadex *threadex) {		\
		tYPE value;					\
		lock_##fIELD(threadex);				\
		value = get_##fIELD##_locked(threadex);		\
		unlock_##fIELD(threadex);			\
		return value;					\
        }                                                       \
        static inline void                                      \
        set_##fIELD(struct ythreadex *threadex, tYPE value) {	\
		lock_##fIELD(threadex);				\
		set_##fIELD##_locked(threadex, value);		\
		unlock_##fIELD(threadex);			\
        }

declare_with_lock_getter_setter(enum ythreadex_state, state)

#undef declare_with_lock_getter_setter

/* ------------------------------------------------------------------------- */
#define declare_simple_getter_setter(tYPE, fIELD)          \
        static inline tYPE                                      \
        get_##fIELD(struct ythreadex *threadex) {                \
                return threadex->fIELD;                          \
        }                                                       \
        static inline void                                      \
        set_##fIELD(struct ythreadex *threadex, tYPE value) {    \
                threadex->fIELD = value;                         \
        }

declare_simple_getter_setter(int, errcode) /* @suppress("Unused static function") */
declare_simple_getter_setter(void *, result) /* @suppress("Unused static function") */


#undef declare_simple_field_getter_setter


/******************************************************************************
 *
 * Message notifier functions
 *
 *****************************************************************************/
#define post_to_owner(threadex, callback)			\
	fatali0(ymsghandler_post_exec((threadex)->owner,	\
				      threadex, NULL,		\
				      &callback))

#define post_listener_event(threadex, listener_name)		\
	post_to_owner(threadex, on_##listener_name)


static void
on_started(void *arg) {
        struct ythreadex *threadex = (struct ythreadex *)arg;
	if (likely(threadex->listener.on_started))
		(*threadex->listener.on_started)(threadex);
}

static void
on_done(void *arg) {
        struct ythreadex *threadex = (struct ythreadex *)arg;
	if (likely(threadex->listener.on_done))
		(*threadex->listener.on_done)
			(threadex,
			 ythreadex_get_result(threadex),
			 ythreadex_get_errcode(threadex));
	set_state(threadex, YTHREADEX_TERMINATED);
}

static void
on_cancelled(void *arg) {
        struct ythreadex *threadex = (struct ythreadex *)arg;
	if (likely(threadex->listener.on_cancelled))
		(*threadex->listener.on_cancelled)
			(threadex, ythreadex_get_errcode(threadex));
	set_state(threadex, YTHREADEX_TERMINATED_CANCELLED);
}

static void
on_cancelling(void *arg) {
        struct ythreadex *threadex = (struct ythreadex *)arg;
	bool started = threadex->cancel_started;
	bool pthdcancel = threadex->cancel_pthdcancel;
	if (likely(threadex->listener.on_cancelling))
		(*threadex->listener.on_cancelling)(threadex, started);
	if (likely(started)) {
		/* threadex->thread is NULL in case ythreadex_start_sync */
		if (pthdcancel && threadex->thread)
			/* return value is ignored in intention.
			 * Thread may be already finised.
			 */
			pthread_cancel(threadex->thread);
	} else {
		set_state(threadex, YTHREADEX_CANCELLED);
                post_listener_event(threadex, cancelled);
	}
}

static void
on_progress_init(void *arg) {
        struct ythreadex *threadex = (struct ythreadex *)arg;
	if (threadex->listener.on_progress_init)
		(*threadex->listener.on_progress_init)
			(threadex, threadex->progress_maxprog);
}

static void
on_progress(void *arg) {
        struct ythreadex *threadex = (struct ythreadex *)arg;
	if (threadex->listener.on_progress)
		(*threadex->listener.on_progress)
			(threadex, threadex->progress_prog);
}


/******************************************************************************
 *
 *
 *
 *****************************************************************************/
static void
thread_main_cleanup(void *arg) {
        struct ythreadex *threadex = (struct ythreadex *)arg;
        bool cancel = FALSE;

        lock_state(threadex);
        if (unlikely(YTHREADEX_CANCELLING == get_state_locked(threadex))) {
                set_state_locked(threadex, YTHREADEX_CANCELLED);
                cancel = TRUE;
        } else {
                yassert(YTHREADEX_STARTED == get_state_locked(threadex));
                set_state_locked(threadex, YTHREADEX_DONE);
        }
        unlock_state(threadex);
        if (unlikely(cancel))
                post_listener_event(threadex, cancelled);
        else
                post_listener_event(threadex, done);

}

static void *

thread_main(void *arg) {
	enum ythreadex_state state;
        struct ythreadex *threadex = (struct ythreadex *)arg;

        pthread_cleanup_push(&thread_main_cleanup, threadex);
	/* READY, STARTED or CANCELLING states are possible at this moment */
	switch (state = ythreadex_get_state(threadex)) {
	/* STARTED or CANCELLING states are possible here */
	case YTHREADEX_STARTED:
		/* Execute user function
		 *
		 * Note that, there is NO locks for errcode and result.
		 * The reason why &threadex->result is directly passed, is
		 * 'run' may be exited by 'pthread_cancel'.
		 * In this case, we don't have any chance to set threadex->result
		 *   by calling set_result() after calling-'run'.
		 * result and errcode doesn't require 'lock', we can pass it
		 *   directly to 'run' function here.
		 */
		threadex->errcode = (*threadex->run)(threadex,
						     &threadex->result);
		break;

	case YTHREADEX_CANCELLING:
		break;
	default:
		/* This MUST NOT happend. this thread MUST wait at
		 * ythreadex_get_state() until thread state is changed
		 * to STARTED!
		 */
		yassert(FALSE);
		ylogf("Threadex state internal error!: %d", state);
		exit(1);
	}

	pthread_cleanup_pop(1);
        return NULL;
}

/******************************************************************************
 *
 *
 *
 *****************************************************************************/

struct ythreadex *
ythreadex_create(const char *name,
		 struct ymsghandler *owner,
		 enum ythreadex_priority priority,
		 const struct ythreadex_listener *listener,
		 void *arg,
		 void (*free_arg)(void *),
		 void (*free_result)(void *),
		 int (*run)(struct ythreadex *, void **result)) {
	struct ythreadex *threadex;
	if (unlikely(!run))
		return NULL;
	threadex = ycalloc(sizeof(*threadex), 1);
	if (unlikely(!threadex)) {
		ylogf("Out of memory!");
		return NULL;
	}
	/* -1 to preserve terminating 0 */
	strncpy(threadex->name, name, sizeof(threadex->name) - 1);
	threadex->owner = owner;
	threadex->priority = priority;
	threadex->state = YTHREADEX_READY; /* initial state */
        fatali0(pthread_mutex_init(&threadex->state_lock, NULL));
	threadex->listener = *listener;
	threadex->errcode = 0;
	threadex->arg = arg;
	threadex->free_arg = free_arg;
	threadex->result = NULL;
	threadex->free_result = free_result;
	threadex->run = run;
	threadex->id = gen_unique_id();

	return threadex;
}


int
ythreadex_destroy(struct ythreadex *threadex) {
	enum ythreadex_state state;
	state = ythreadex_get_state(threadex);
	if (unlikely(!ythreadex_is_terminated(state)))
		return -EPERM;
	if (threadex->arg && threadex->free_arg)
		(*threadex->free_arg)(threadex->arg);
	if (threadex->result && threadex->free_result)
		(*threadex->free_result)(threadex->result);
        fatali0(pthread_mutex_destroy(&threadex->state_lock));
	yfree(threadex);
	return 0;
}

enum ythreadex_state
ythreadex_get_state(struct ythreadex *threadex) {
	return get_state(threadex);
}

/* ------------------------------------------------------------------------- */

#define declare_interface_field_getter(tYPE, nAME)		\
        tYPE ythreadex_get_##nAME(struct ythreadex *threadex) { \
                return threadex->nAME;                          \
        }

/* Value of \a result are \a errcode may be updated at other thread.
 * And reading and updating value may not be atomic.
 * But, It is caller's responsibility, getting stable value.
 * (Reading after thread is DONE of CANCELLED)
 */
declare_interface_field_getter(void *, result)
declare_interface_field_getter(int, errcode)
declare_interface_field_getter(void *, arg)
declare_interface_field_getter(enum ythreadex_priority, priority)
declare_interface_field_getter(struct ymsghandler *, owner)
declare_interface_field_getter(const char *, name)
declare_interface_field_getter(long, id)

#undef declare_interface_field_getter

/* ------------------------------------------------------------------------- */

int
ythreadex_start(struct ythreadex *threadex) {
	int r;
	lock_state(threadex);
	if (unlikely(YTHREADEX_READY != get_state_locked(threadex))) {
		unlock_state(threadex);
		return -EPERM;
	}
	if (unlikely(r = pthread_create(&threadex->thread,
					NULL,
					&thread_main,
					threadex))) {
		unlock_state(threadex);
		return -r;
	}
	set_state_locked(threadex, YTHREADEX_STARTED);
	post_listener_event(threadex, started);
	unlock_state(threadex);
	return 0;
}

int
ythreadex_start_sync(struct ythreadex *threadex) {
        lock_state(threadex);
        if (unlikely(YTHREADEX_READY != threadex->state)) {
		unlock_state(threadex);
                return -EPERM;
        }
        set_state_locked(threadex, YTHREADEX_STARTED);
        post_listener_event(threadex, started);
        unlock_state(threadex);
        thread_main(threadex);
        return 0;
}

int
ythreadex_join(struct ythreadex *threadex, void **retval) {
	/* in case of threadex_start_sync, join is impossible. */
	if (unlikely(YTHREADEX_READY == ythreadex_get_state(threadex)
		     || !threadex->thread))
		return -EPERM;
	return pthread_join(threadex->thread, retval);
}

int
ythreadex_cancel(struct ythreadex *threadex, bool pthdcancel) {
	bool started = FALSE;
	lock_state(threadex);
	switch (get_state_locked(threadex)) {
	case YTHREADEX_READY:
		break;
	case YTHREADEX_STARTED:
		started = TRUE;
		break;
	default:
		unlock_state(threadex);
		return -EPERM;
	}
	threadex->cancel_started = started;
	threadex->cancel_pthdcancel = pthdcancel;
	set_state_locked(threadex, YTHREADEX_CANCELLING);
	post_listener_event(threadex, cancelling);
	unlock_state(threadex);
	return 0;
}

int
ythreadex_publish_progress_init(struct ythreadex *threadex, long maxprog) {
#ifdef CONFIG_DEBUG
	yassert(!threadex->proginit);
	threadex->proginit = TRUE;
#endif /* CONFIG_DEBUG */
	threadex->progress_maxprog = maxprog;
	post_listener_event(threadex, progress_init);
	return 0;
}

int
ythreadex_publish_progress(struct ythreadex *threadex, long prog) {
	yassert(threadex->proginit);
	threadex->progress_prog = prog;
	post_listener_event(threadex, progress);
	return 0;
}


/******************************************************************************
 *
 *
 *
 *****************************************************************************/
static int
minit(const struct ylib_config *cfg) {
	int r __unused;
	r = pthread_spin_init(&_id_lock, PTHREAD_PROCESS_PRIVATE);
	if (unlikely(r))
		return -r;
	return 0;
}

static void
mexit(void) {
        fatali0(pthread_spin_destroy(&_id_lock));
}

LIB_MODULE(threadex, minit, mexit) /* @suppress("Unused static function") */
