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
#include <limits.h>

#include "common.h"
#include "lib.h"
#include "yhash.h"
#include "yo.h"
#include "ygp.h"
#include "yut.h"
#include "ymsghandler.h"
#include "threadex.h"
#include "task.h"



static const u64 PROG_DEFAULT_INTERVAL = 500; /* ms */

/******************************************************************************
 *
 * Lock
 *
 *****************************************************************************/
/* Hash operation is quite expensive. So mutex lock is chosen */
declare_lock(mutex, struct ytask, tagmap, NULL)
/* List operation is very cheap. But calling listeners requires lock */
declare_lock(mutex, struct ytask, elh, NULL)

/******************************************************************************
 *
 *
 *
 *****************************************************************************/
static void
task_put_runnable(void *arg) {
	struct ytask *tsk = (struct ytask *)arg;
	task_put(tsk);
}

/******************************************************************************
 *
 *
 *
 *****************************************************************************/
static void
on_started_runnable(void *arg) {
	struct yo *o = (struct yo *)arg;
	struct ytask *tsk = o->o0;
	struct ytask_event_listener *el = o->o1;
	/* arg will be freed at out-side */
	(*el->on_started)(el, tsk);
}

static void
on_done_runnable(void *arg) {
	struct yo *o = (struct yo *)arg;
	struct ytask *tsk = o->o0;
	struct ytask_event_listener *el = o->o1;
	void *result = o->o2;
	int errcode = (int)(intptr_t)o->o3;
	/* arg will be freed at out-side */
	(*el->on_done)(el, tsk, result, errcode);
}

static void
on_cancelling_runnable(void *arg) {
	struct yo *o = (struct yo *)arg;
	struct ytask *tsk = o->o0;
	struct ytask_event_listener *el = o->o1;
	bool started = (bool)(intptr_t)o->o2;
	/* arg will be freed at out-side */
	(*el->on_cancelling)(el, tsk, started);
}

static void
on_cancelled_runnable(void *arg) {
	struct yo *o = (struct yo *)arg;
	struct ytask *tsk = o->o0;
	struct ytask_event_listener *el = o->o1;
	int errcode = (int)(intptr_t)o->o2;
	/* arg will be freed at out-side */
	(*el->on_cancelled)(el, tsk, errcode);
}

static void
on_progress_init_runnable(void *arg) {
	struct yo *o = (struct yo *)arg;
	struct ytask *tsk = o->o0;
	struct ytask_event_listener *el = o->o1;
	long max_prog = (long)(intptr_t)o->o2;
	/* arg will be freed at out-side */
	(*el->on_progress_init)(el, tsk, max_prog);
}

static void
on_progress_runnable(void *arg) {
	struct yo *o = (struct yo *)arg;
	struct ytask *tsk = o->o0;
	struct ytask_event_listener *el = o->o1;
	long prog = (long)(intptr_t)o->o2;
	/* arg will be freed at out-side */
	(*el->on_progress)(el, tsk, prog);
}

/*****************************************************************************/

#define listener_stmt(nAME, yOnUM, yOaRG, cALLaRG)			\
	do {								\
		struct ytask *tsk = (struct ytask *)threadex;		\
		struct ytask_event_listener_handle *elh;		\
		if (tsk->listener.on_early_##nAME) {			\
			(*tsk->listener.on_early_##nAME) cALLaRG;	\
		}							\
                lock_elh(tsk);						\
		ylistl_foreach_item(elh,				\
				    &tsk->elhhd,			\
				    struct ytask_event_listener_handle,	\
				    lk) {				\
			if (elh->el.on_##nAME) {			\
				struct yo *o = yocreate##yOnUM yOaRG;	\
				if (unlikely(!o)) {			\
					die2("Out of memory!");		\
				}					\
				/* To prevent task to be destroied before \
				 * handling listener message		\
				 */					\
				task_get(tsk);				\
				fatali0(ymsghandler_post_exec		\
					(elh->owner,			\
					 o, (void(*)(void *))&yodestroy, \
					 &on_##nAME##_runnable));	\
				/* Put tsk object after handling listener */ \
				fatali0(ymsghandler_post_exec		\
					(elh->owner,			\
					 tsk, NULL,			\
					 &task_put_runnable));		\
			}						\
		}							\
                unlock_elh(tsk);					\
		if (tsk->listener.on_late_##nAME) {			\
			(*tsk->listener.on_late_##nAME) cALLaRG;	\
		}							\
	} while (0)

static void
on_started(struct ythreadex *threadex) {
	listener_stmt(started, 1,
		      (tsk, NULL, &elh->el, NULL),
		      (tsk));
}

static void
on_done(struct ythreadex *threadex, void *result, int errcode) {
	listener_stmt(done, 3,
		      (tsk, NULL,
		       &elh->el,
		       NULL, result,
		       NULL, (void *)(intptr_t)errcode, NULL),
		      (tsk, result, errcode));
}

static void
on_cancelling(struct ythreadex *threadex, bool started) {
	listener_stmt(cancelling, 2,
		      (tsk, NULL,
		       &elh->el, NULL,
		       (void *)(intptr_t)started, NULL),
		      (tsk, started));
}

static void
on_cancelled(struct ythreadex *threadex, int errcode) {
	listener_stmt(cancelled, 2,
		      (tsk, NULL,
		       &elh->el, NULL,
		       (void *)(intptr_t)errcode, NULL),
		      (tsk, errcode));
}

static void
on_progress_init(struct ythreadex *threadex, long max_prog) {
	listener_stmt(progress_init, 2,
		      (tsk, NULL,
		       &elh->el, NULL,
		       (void *)(intptr_t)max_prog, NULL),
		      (tsk, max_prog));
}

static void
on_progress(struct ythreadex *threadex, long prog) {
	listener_stmt(progress, 2,
		      (tsk, NULL,
		       &elh->el, NULL,
		       (void *)(intptr_t)prog, NULL),
		      (tsk, prog));
}

#undef listener_stmt

static const struct ythreadex_listener _threadex_listener = {
        .on_started = &on_started,
        .on_done = &on_done,
        .on_cancelling = &on_cancelling,
        .on_cancelled = &on_cancelled,
        .on_progress_init = &on_progress_init,
        .on_progress = &on_progress
};


/******************************************************************************
 *
 *
 *
 *****************************************************************************/
static void
free_event_listener_handle(struct ytask_event_listener_handle *elh) {
	if (elh->el.free_extra)
		(*elh->el.free_extra)(&elh->el.extra, elh->el.extrasz);
	yfree(elh);
}

static void
task_destroy(struct ytask *tsk) {
	task_clean(tsk);
	yfree(tsk);
}

int
task_init(struct ytask *tsk,
          const char *name,
          struct ymsghandler *owner,
          enum ythreadex_priority priority,
          const struct ytask_listener *listener,
          void *arg,
          void (*free_arg)(void *),
          void (*free_result)(void *),
          int (*run)(struct ytask *, void **result),
          bool pthdcancel) {
	int r;
        if (unlikely(r = threadex_init
		     (&tsk->t,
		      name,
		      owner,
		      priority,
		      &_threadex_listener,
		      arg,
		      free_arg,
		      free_result,
		      (int(*)(struct ythreadex *, void **))run)))
		return r;
        tsk->pthdcancel = pthdcancel;
        ylistl_init_link(&tsk->elhhd);
        tsk->prog.interval = PROG_DEFAULT_INTERVAL;
        if (listener)
                tsk->listener = *listener;
        tsk->tagmap = yhashs_create((void (*)(void *))&yodestroy, TRUE);
        if (unlikely(!tsk->tagmap)) {
                r = -ENOMEM;
		goto clean_threadex;
	}
	init_tagmap_lock(tsk);
	init_elh_lock(tsk);
	ygpinit(&tsk->gp, tsk, (void(*)(void *))&task_destroy);
	/* Initialize library internal values */
	tsk->tmtag = NULL;
	tsk->tdmtag = NULL;
	return 0;

 clean_threadex:
	threadex_clean(&tsk->t);
	return r;
}

int
task_clean(struct ytask *tsk) {
	int r;
	struct ytask_event_listener_handle *pos, *n;
	if (unlikely(r = threadex_clean(&tsk->t)))
		return r;
	/* clean up listeners */
	ylistl_foreach_item_removal_safe(pos,
					 n,
					 &tsk->elhhd,
					 struct ytask_event_listener_handle,
					 lk) {
		free_event_listener_handle(pos);
	}
        yhash_destroy(tsk->tagmap);
	destroy_tagmap_lock(tsk);
	destroy_elh_lock(tsk);
        return 0;
}

static bool
task_has_event_listener(struct ytask *tsk,
			struct ytask_event_listener_handle *el) {
	struct ylistl_link *n;
	bool ret = TRUE;
	lock_elh(tsk);
	ylistl_foreach(n, &tsk->elhhd) {
		if (n == &el->lk)
			goto done;
	}
	ret = FALSE;
 done:
	unlock_elh(tsk);
	return ret;
}

static void
notify_current_progress_to_new_listener(void *arg) {
	/* Runs at tsk owner's context. */
	struct yo *yo = (struct yo *)arg;
	struct ytask *tsk = yo->o0;
	struct ytask_event_listener_handle *el = yo->o1;
	yassert(tsk->prog.init);
        if (unlikely(YTHREADEX_STARTED != ythreadex_get_state(&tsk->t)))
		return; /* ignore notifying progress requrest */
	(*el->el.on_progress_init)(&el->el, tsk, tsk->prog.max);
	(*el->el.on_progress)(&el->el, tsk, tsk->prog.prog);
	task_put(tsk);
}

/******************************************************************************
 *
 *
 *
 *****************************************************************************/
struct ytask *
ytask_create3(const char *name,
	      struct ymsghandler *owner,
	      enum ythreadex_priority priority,
	      const struct ytask_listener *listener,
	      void *arg,
	      void (*free_arg)(void *),
	      void (*free_result)(void *),
	      int (*run)(struct ytask *, void **result),
	      bool pthdcancel) {
	struct ytask *tsk;
	tsk = ycalloc(sizeof(*tsk), 1);
	if (unlikely(!tsk)) {
		ylogf("Out of memory!");
		return NULL;
	}
        if (unlikely(task_init(tsk,
                               name,
                               owner,
                               priority,
                               listener,
                               arg,
                               free_arg,
                               free_result,
                               run,
                               pthdcancel))) {
                yfree(tsk);
                return NULL;
        }
	task_get(tsk);
        return tsk;
}

int
ytask_destroy(struct ytask *tsk) {
	if (unlikely(!ythreadex_is_terminated(ythreadex_get_state(&tsk->t))))
		return -EPERM;
	task_put(tsk);
        return 0;
}

int
ytask_cancel(struct ytask *tsk) {
        return ythreadex_cancel(&tsk->t, tsk->pthdcancel);
}

int
ytask_publish_progress_init(struct ytask *tsk, long maxprog) {
#ifdef CONFIG_DEBUG
	yassert(!tsk->prog.init);
	tsk->prog.init = TRUE;
#endif /* CONFIG_DEBUG */
	if (unlikely(!maxprog))
		return -EINVAL;
	tsk->prog.pubtm = 0;
	tsk->prog.prog = 0;
	tsk->prog.max = maxprog;
	return ythreadex_publish_progress_init(&tsk->t, maxprog);
}

int
ytask_publish_progress(struct ytask *tsk, long prog) {
	u64 now;
	yassert(tsk->prog.init);
	if (unlikely(prog > tsk->prog.max))
		prog = tsk->prog.max;
	now = yut_current_time_millis();
	if (now - tsk->prog.pubtm < tsk->prog.interval
	    || prog == tsk->prog.prog)
		return -EPERM;
	tsk->prog.pubtm = now;
	tsk->prog.prog = prog;
	return ythreadex_publish_progress(&tsk->t, prog);
}

int
ytask_add_tag(struct ytask *tsk,
              const char *name,
              void *tag,
              void (*tagfree)(void *)) {
        int r;
        struct yo *o = yocreate0(tag, tagfree);
        if (unlikely(!o))
                return -ENOMEM;
	lock_tagmap(tsk);
        r = yhash_add(tsk->tagmap, name, o, TRUE);
	unlock_tagmap(tsk);
	return r;
}

void *
ytask_get_tag(struct ytask *tsk, const char *name) {
	int r;
        struct yo *o;
	lock_tagmap(tsk);
	r = yhash_find(tsk->tagmap, (void **)&o, name);
	unlock_tagmap(tsk);
        if (unlikely(r))
                return NULL;
        return o->o0;
}

int
ytask_remove_tag(struct ytask *tsk, const char *name) {
	int r;
	lock_tagmap(tsk);
        r = yhash_remove(tsk->tagmap, name);
	unlock_tagmap(tsk);
	return r;
}

struct ytask_event_listener_handle *
ytask_add_event_listener2(struct ytask *tsk,
			  struct ymsghandler *event_listener_owner,
			  const struct ytask_event_listener *yel,
			  bool progress_notice) {
	struct yo *yo;
	struct ytask_event_listener_handle *elh;
	yassert(tsk && event_listener_owner && yel);
	if (unlikely(!(elh = ymalloc(sizeof(*elh) + yel->extrasz))))
		return NULL;
	memcpy(&elh->el, yel, sizeof(*yel) + yel->extrasz);
	elh->owner = event_listener_owner;
	lock_elh(tsk);
	ylistl_add_last(&tsk->elhhd, &elh->lk);
	unlock_elh(tsk);
	if (!(tsk->prog.max > 0
	      && YTHREADEX_STARTED == ythreadex_get_state(&tsk->t)
	      && progress_notice))
		return elh;
	if (unlikely(!(yo = yocreate1(tsk, NULL, elh, NULL)))) {
		/* At failure, extra data SHOULD be preserved.
		 * So, yfree is used instead of free_event_listener_handle
		 */
		yfree(elh);
		return NULL;
	}
	task_get(tsk);
	fatali0(ymsghandler_post_exec
		(event_listener_owner, yo, (void(*)(void *))&yodestroy,
		 &notify_current_progress_to_new_listener));
	return elh; /* el is used as event-listener-handler */
}

int
ytask_remove_event_listener(struct ytask *tsk,
                            struct ytask_event_listener_handle *elh) {
	yassert(tsk && elh);
	if (!task_has_event_listener(tsk, elh))
		return -EINVAL;
	lock_elh(tsk);
	ylistl_remove(&elh->lk);
	unlock_elh(tsk);
	free_event_listener_handle(elh);
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
extern void o_clear(void);
extern void msghandler_clear(void);
extern void threadex_clear(void);
void
task_clear(void) {
	o_clear();
	msghandler_clear();
	threadex_clear();
}
#endif /* CONFIG_DEBUG */
