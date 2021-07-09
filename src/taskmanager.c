/******************************************************************************
 * Copyright (C) 2016, 2017
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
#include <unistd.h>
#include <stdint.h>

#include "common.h"
#include "lib.h"
#include "ymsglooper.h"
#include "ymsghandler.h"
#include "ylistl.h"
#include "yo.h"
#include "ygp.h"
#include "task.h"
#include "taskmanager.h"


#define UNLIMITED_SLOTS 99999999

struct ytaskmanager_qevent_listener_handle {
	struct ymsghandler *owner;
        /**
         * Fields used inside task module. This value is initialized inside
         * task module.
         */
        struct ylistl_link lk;
	/**
	 * {@code el} MUST be at the end of this struct, because
	 *   {@code struct ytaskmanager_qevent_listener} has extra bytes at
	 *   the end of struct
	 */
	struct ytaskmanager_qevent_listener el;
};

struct ttg {
	struct ytaskmanager *tm; /* owner task manager */
	struct ytask *tsk; /* task in where this tag belonging to */
	struct ylistl_link lk; /* q link */
	struct ytask_event_listener_handle * elh;
};

/******************************************************************************
 *
 * Lock
 *
 *****************************************************************************/
/* Hash operation is quite expensive. So mutex lock is chosen */
declare_lock(mutex, struct ytaskmanager, tagmap, NULL)
/* List operation is very cheap. But calling listeners requires lock */
declare_lock(mutex, struct ytaskmanager, elh, NULL)
declare_lock(mutex, struct ytaskmanager, q, NULL)

/******************************************************************************
 *
 *
 *
 *****************************************************************************/
static INLINE bool
is_in_owner_thread(struct ytaskmanager *tm) __unused;
static INLINE bool
is_in_owner_thread(struct ytaskmanager *tm) {
	return pthread_self() == ymsglooper_get_thread
		(ymsghandler_get_looper(ytaskmanager_get_owner(tm)));
}

/******************************************************************************
 *
 *
 *
 *****************************************************************************/
static INLINE bool
verify_ttg(struct ytaskmanager *tm, struct ytask *tsk) __unused;
static INLINE bool
verify_ttg(struct ytaskmanager *tm, struct ytask *tsk) {
	struct ttg *ttg = tsk->tmtag;
	return ttg
		&& ttg->tm == tm
		&& ttg->tsk == tsk;
}

static INLINE bool
has_empty_ttg(struct ytask *tsk) {
	return !tsk->tmtag;
}

static INLINE struct ytask *
ttglk_task(struct ylistl_link *tmtaglk) {
	struct ttg *ttg = containerof(tmtaglk, struct ttg, lk);
	return ttg->tsk;
}

static INLINE struct ttg *
task_ttg(struct ytask *tsk) {
	return (struct ttg *)tsk->tmtag;
}

static INLINE struct ylistl_link *
task_lk(struct ytask *tsk) {
	return &task_ttg(tsk)->lk;
}

static struct ttg *
create_ttg(struct ytaskmanager *tm, struct ytask *tsk) {
	struct ttg *ttg = ycalloc(1, sizeof(*ttg));
	yassert(tm && tsk
		&& !tsk->tmtag);
	if (unlikely(!ttg))
		return NULL;
	ttg->tm = tm;
	ttg->tsk = tsk;
	ylistl_init_link(&ttg->lk);

	tsk->tmtag = ttg;
	return ttg;
}

static INLINE void
destroy_ttg(struct ytask *tsk) {
	struct ttg *ttg __unused = tsk->tmtag;
	yassert(ttg);
	yassert(ttg->tsk == tsk);
	yfree(tsk->tmtag);
	tsk->tmtag = NULL;
}

/******************************************************************************
 *
 * Task ready Q
 *
*****************************************************************************/

static INLINE int
readyq_size_locked(struct ytaskmanager *tm) {
	int nrpri = YTHREADEX_NUM_PRIORITY;
	int sz = 0;
	while (nrpri--)
		sz += (int)ylistl_size(&tm->readyq_hd[nrpri]);
	return sz;
}

static INLINE bool
readyq_contains_locked(struct ytaskmanager *tm, struct ytask *tsk) {
	int nrpri = YTHREADEX_NUM_PRIORITY;
	while (nrpri--) {
		if (unlikely(ylistl_contains(&tm->readyq_hd[nrpri],
					     task_lk(tsk))))
			return TRUE;
	}
	return FALSE;
}

static INLINE int
readyq_enq_locked(struct ytaskmanager *tm, struct ytask *tsk) {
	int pri = ytask_get_priority(tsk);
	yassert(verify_ttg(tm, tsk));
	yassert(0 <= pri && pri < YTHREADEX_NUM_PRIORITY);
	ylistl_add_last(&tm->readyq_hd[pri],
			task_lk(tsk));
	return 0;
}

static INLINE int
readyq_remove_locked(struct ytaskmanager *tm __unused,
		     struct ytask *tsk) {
	struct ylistl_link *lk = task_lk(tsk);
	yassert(verify_ttg(tm, tsk));
	yassert(readyq_contains_locked(tm, tsk));
	if (unlikely(!(lk->prev && lk->next)
		     || lk == lk->prev
		     || lk == lk->next))
		return -EINVAL;
	ylistl_remove(lk);
	return 0;
}

static struct ytask *
readyq_deq_locked(struct ytaskmanager *tm) {
	struct ylistl_link *lk;
	struct ylistl_link *hd;
	int nrpri = YTHREADEX_NUM_PRIORITY;
	while (nrpri--) {
		hd = &tm->readyq_hd[nrpri];
		if (unlikely(ylistl_is_empty(hd)))
			continue;
		lk = ylistl_remove_first(hd);
		return ttglk_task(lk);
	}
	return NULL;
}

static INLINE int
runq_size_locked(struct ytaskmanager *tm) {
	return (int)ylistl_size(&tm->runq_hd);
}

static INLINE bool
runq_contains_locked(struct ytaskmanager *tm, struct ytask *tsk) {
	yassert(verify_ttg(tm, tsk));
	return ylistl_contains(&tm->runq_hd, task_lk(tsk));
}

static INLINE int
runq_enq_locked(struct ytaskmanager *tm, struct ytask *tsk) {
	yassert(verify_ttg(tm, tsk));
	ylistl_add_last(&tm->runq_hd, task_lk(tsk));
	return 0;
}

static INLINE int
runq_remove_locked(struct ytaskmanager *tm, struct ytask *tsk) {
	struct ylistl_link *lk = task_lk(tsk);
	yassert(runq_contains_locked(tm, tsk));
	if (unlikely(!(lk->prev && lk->next)))
		return -EINVAL;
	ylistl_remove(lk);
	return 0;
}

static INLINE struct ytask *
runq_deq_locked(struct ytaskmanager *tm) {
	struct ylistl_link *lk;
	if (unlikely(ylistl_is_empty(&tm->runq_hd)))
		return NULL;
	lk = ylistl_remove_last(&tm->runq_hd);
	return ttglk_task(lk);
}

/******************************************************************************
 *
 * Task listeners
 *
 *****************************************************************************/
static void
remove_task_from_runq(struct ytaskmanager *tm, struct ytask *tsk);

static void
task_event_on_finished(struct ytask *tsk) {
	struct ttg *ttg = task_ttg(tsk);
	yassert(!has_empty_ttg(tsk)
		&& is_in_owner_thread(ttg->tm));
	remove_task_from_runq(ttg->tm, tsk);
}

static void
task_event_on_done(
	struct ytask_event_listener *el __unused,
	struct ytask *tsk,
	void *result __unused,
	int errcode __unused
) {
	task_event_on_finished(tsk);
}

static void
task_event_on_cancelled(
	struct ytask_event_listener *el __unused,
	struct ytask *tsk,
	int errcode
) {
	task_event_on_finished(tsk);
}

const static struct ytask_event_listener _tsk_event_listener = {
	.on_cancelled = &task_event_on_cancelled,
	.on_done = &task_event_on_done,
};


/******************************************************************************
 *
 *
 *
 *****************************************************************************/
struct qevent_arg {
 	struct ytaskmanager *tm;
	enum ytaskmanager_qevent ev;
	int readyq_sz;
	int runq_sz;
	struct ytask *tsk;
	/* DO NOT USE pointer for 'el'!. Memory may be freed at other context
	 *   before referencing it.
	 * 'el' may have extra data at the end of it.
	 */
	struct ytaskmanager_qevent_listener el;
};

static void
taskmanager_qevent_listener_free(
	struct ytaskmanager_qevent_listener_handle *elh
) {
	if (elh->el.free_extra)
		(*elh->el.free_extra)(&elh->el.extra, elh->el.extrasz);
	yfree(elh);
}

static void
notify_qevent_to_listener(void *arg) {
	struct qevent_arg *a = arg;
	a->el.on_event(&a->el, a->tm, a->ev, a->readyq_sz, a->runq_sz, a->tsk);
}


static void
notify_qevent_locked(
	struct ytaskmanager *tm,
	enum ytaskmanager_qevent ev,
	struct ytask *tsk
) {
	struct qevent_arg *a;
	struct ytaskmanager_qevent_listener_handle *elh;
	int readyq_sz = readyq_size_locked(tm);
	int runq_sz = runq_size_locked(tm);
	ylistl_foreach_item(
		elh,
		&tm->elhhd,
		struct ytaskmanager_qevent_listener_handle,
		lk
	) {
		if (unlikely(!(a = ymalloc(sizeof(*a) + elh->el.extrasz))))
			die2("Out-of-memory");
		/* Copied value is passed because handle may be freed before
		 *   msg is executed.
		 */
		memcpy(&a->el, &elh->el, sizeof(elh->el) + elh->el.extrasz);
		a->tm = tm;
		a->ev = ev;
		a->readyq_sz = readyq_sz;
		a->runq_sz = runq_sz;
		a->tsk = tsk;
		fatali0(ymsghandler_post_exec(
			elh->owner,
			a,
			&yfree,
			&notify_qevent_to_listener));
	}

}

static bool
balance_taskq(struct ytaskmanager *tm) {
	/* There are two jobs.
	 * - Updating task Q
	 * - Notifying to listeners
	 *
	 * Important point is that these two jobs should be protected in the same block,
	 *   because order of above two jobs should be same!
	 * If above two jobs are protected by different lock, following issue can be seen.
	 * +------------------------------------------------
	 * |   < Thread1 >              < Thread2 >
	 * |   addToReady
	 * |                             moveToRun
	 * |                             notifyMoveToRun
	 * |  notifyAddToReady
	 * |
	 * v
	 */
	struct ytask *tsk = NULL;
	lock_q(tm);
	if (readyq_size_locked(tm) > 0
	       && runq_size_locked(tm) < tm->slots) {
		struct ttg *ttg;
		tsk = readyq_deq_locked(tm);
		ttg = task_ttg(tsk);
		yassert(YTHREADEX_READY == ytask_get_state(tsk));
		yassert(verify_ttg(tm, tsk));
		ttg->elh = ytask_add_event_listener2(
			tsk,
			tm->owner,
			&_tsk_event_listener,
			FALSE);
		yassert(ttg->elh);
		fatali0(runq_enq_locked(tm, tsk));
		notify_qevent_locked(tm, YTASKMANAGERQ_MOVED_TO_RUN, tsk);
	}
	unlock_q(tm);
	/* TODO: Improvement(More tolerable). */
	fatal2(!tsk || !ytask_start(tsk), "May be OOM?");
	return !!tsk;
}

static int
add_task_to_readyq(struct ytaskmanager *tm, struct ytask *tsk) {
	int r;
	lock_q(tm);
	if (unlikely(r = readyq_enq_locked(tm, tsk)))
		return r;
	/* 'task_get' prevents task from being free outside tm unexpectedly.
	 * This should be called before notifying to listeners.
	 * Because, 'ADDED_TO_READY' means "it's ready to being managed by
	 *   taskmanager.
	 * And this includes "task cannot be destroied inforce outside
	 *   taskmanager".
	 */
	task_get(tsk);
	notify_qevent_locked(
		tm, YTASKMANAGERQ_ADDED_TO_READY, tsk);
	unlock_q(tm);
	balance_taskq(tm);
	return 0;
}

/*
 * This means task is removed from taskmanager.
 */
static void
remove_task_from_runq(struct ytaskmanager *tm, struct ytask *tsk) {
	struct ttg *ttg = task_ttg(tsk);
	yassert(is_in_owner_thread(tm)
		&& verify_ttg(tm, tsk)
		&& ttg->elh);
	fatali0(ytask_remove_event_listener(tsk, ttg->elh));
	lock_q(tm);
	yassert(ylistl_contains(&tm->runq_hd, &ttg->lk));
	ylistl_remove(&ttg->lk);
	destroy_ttg(tsk);
	notify_qevent_locked(tm, YTASKMANAGERQ_REMOVED_FROM_RUN, tsk);
	unlock_q(tm);
	/* 'task_put' is different from 'task_get' used when adding task.
	 * Putting task is NOT time-critical. That is, we don't need to call
	 *   it inside lock.
	 */
	task_put(tsk);
	balance_taskq(tm);
}


/******************************************************************************
 *
 * Public APIs
 *
 *****************************************************************************/
struct ytaskmanager *
ytaskmanager_create(struct ymsghandler *owner, int slots) {
	struct ytaskmanager *tm = ycalloc(1, sizeof(*tm));
	if (unlikely(!tm))
		return NULL;
	tm->tagmap = yhashs_create((void (*)(void *))&yodestroy, TRUE);
	if (unlikely(!tm->tagmap)) {
		yfree(tm);
		return NULL;
	}
	yassert(owner);
	if (slots <= 0)
		slots = 999999999; /* large enough value */
	tm->owner = owner;
	tm->slots = slots;
	init_elh_lock(tm);
	init_q_lock(tm);
	init_tagmap_lock(tm);
	int nrpri = YTHREADEX_NUM_PRIORITY;
	while (nrpri--)
		ylistl_init_link(&tm->readyq_hd[nrpri]);
	ylistl_init_link(&tm->runq_hd);
	ylistl_init_link(&tm->elhhd);
	return tm;
}

int
ytaskmanager_destroy(struct ytaskmanager *tm) {
	struct ytaskmanager_qevent_listener_handle *elh, *elhtmp;
	if (ytaskmanager_size(tm) > 0)
		return -EPERM;
	ylistl_foreach_item_safe(
		elh,
		elhtmp,
		&tm->elhhd,
		struct ytaskmanager_qevent_listener_handle,
		lk
	) {
		yfree(elh);
	}
	destroy_q_lock(tm);
	destroy_tagmap_lock(tm);
	destroy_elh_lock(tm);
        yhash_destroy(tm->tagmap);
	yfree(tm);
	return 0;
}

struct ymsghandler *
ytaskmanager_get_owner(struct ytaskmanager *tm) {
	return tm->owner;
}

int
ytaskmanager_get_slots(struct ytaskmanager *tm) {
	return tm->slots;
}

int
ytaskmanager_add_tag(
	struct ytaskmanager *tm,
	const char *key,
	void *tag,
	void (*tagfree)(void *)
) {
        int r;
        struct yo *o = yocreate0(tag, tagfree);
        if (unlikely(!o))
                return -ENOMEM;
	lock_tagmap(tm);
        r = yhash_add(tm->tagmap, key, o, TRUE);
	unlock_tagmap(tm);
	return r;
}

void *
ytaskmanager_get_tag(struct ytaskmanager *tm, const char *key) {
	int r;
        struct yo *o;
	lock_tagmap(tm);
	r = yhash_find(tm->tagmap, (void **)&o, key);
	unlock_tagmap(tm);
        if (unlikely(r))
                return NULL;
        return o->o0;
}

int
ytaskmanager_remove_tag(struct ytaskmanager *tm, const char *key) {
	int r;
	lock_tagmap(tm);
        r = yhash_remove(tm->tagmap, key);
	unlock_tagmap(tm);
	return r;
}

enum ytaskmanager_qtype
ytaskmanager_contains(struct ytaskmanager *tm, struct ytask *tsk) {
	enum ytaskmanager_qtype qt = YTASKMANAGERQ_INVALIDQ;
	lock_q(tm);
	if (readyq_contains_locked(tm, tsk))
		qt = YTASKMANAGERQ_READY;
	else if (runq_contains_locked(tm, tsk))
		qt = YTASKMANAGERQ_RUN;
	unlock_q(tm);
	return qt;
}

int
ytaskmanager_size(struct ytaskmanager *tm) {
	int sz;
	lock_q(tm);
	sz = runq_size_locked(tm) + readyq_size_locked(tm);
	unlock_q(tm);
	return sz;
}

int
ytaskmanager_add_task(struct ytaskmanager *tm, struct ytask *tsk) {
	int r = 0;
	if (unlikely(!has_empty_ttg(tsk)
		|| YTHREADEX_READY != ytask_get_state(tsk))
	) { return -EINVAL; }
	if (unlikely(!create_ttg(tm, tsk)))
		return -ENOMEM;
	if (unlikely(r = add_task_to_readyq(tm, tsk))) {
		destroy_ttg(tsk);
		return r;
	}
	return 0;
}

int
ytaskmanager_cancel_task(struct ytaskmanager *tm, struct ytask *tsk) {
	ylogd("cancel task: %s(%ld)",
	      ytask_get_name(tsk),
	      ytask_get_id(tsk));
	lock_q(tm);
	if (readyq_contains_locked(tm, tsk)) {
		fatali0(readyq_remove_locked(tm, tsk));
		notify_qevent_locked(
			tm,
			YTASKMANAGERQ_REMOVED_FROM_READY,
			tsk);
	} else if (!runq_contains_locked(tm, tsk)) {
		unlock_q(tm);
		return -EINVAL;
	}
	unlock_q(tm);
	ytask_cancel(tsk);
	return 0;
}

int
ytaskmanager_cancel_all(struct ytaskmanager *tm) {
	struct ylistl_link *p, *n;
	int nrpri = YTHREADEX_NUM_PRIORITY;
	lock_q(tm);
	/* Cancel all tasks at the ready Q */
	while (nrpri--) {
		ylistl_foreach_safe(p, n, &tm->readyq_hd[nrpri]) {
			struct ytask *tsk = ttglk_task(p);
			fatali0(readyq_remove_locked(tm, tsk));
			notify_qevent_locked(
				tm,
				YTASKMANAGERQ_REMOVED_FROM_READY,
				tsk);
			ytask_cancel(tsk);
		}
	}
	/* Cancel all tasks at the run Q */
	ylistl_foreach(p, &tm->runq_hd) {
		ytask_cancel(ttglk_task(p));
	}
	unlock_q(tm);
	return 0;
}


struct ytask *
ytaskmanager_find_task(
	struct ytaskmanager *tm,
	void *arg,
	bool (*match)(struct ytask *, void *arg)
) {
	struct ylistl_link *p;
	int nrpri = YTHREADEX_NUM_PRIORITY;
	struct ytask *ret_tsk = NULL;
	/* NOTE: if this function is heavily used, please consider using
	 *   rwlock instead of mutex
	 */
	lock_q(tm);
	while (nrpri--) {
		ylistl_foreach(p, &tm->readyq_hd[nrpri]) {
			struct ytask *tsk = ttglk_task(p);
			if (unlikely((*match)(tsk, arg))) {
				ret_tsk = tsk;
				goto done;
			}
		}
	}

	ylistl_foreach(p, &tm->runq_hd) {
		struct ytask *tsk = ttglk_task(p);
		if (unlikely((*match)(tsk, arg))) {
			ret_tsk = tsk;
			goto done;
		}
	}
 done:
	unlock_q(tm);
	return ret_tsk;
}

struct ytaskmanager_qevent_listener_handle *
ytaskmanager_add_qevent_listener2(
	struct ytaskmanager *tm,
	struct ymsghandler *owner,
	const struct ytaskmanager_qevent_listener *qel
) {
	struct ytaskmanager_qevent_listener_handle *elh;
	if (unlikely(!(elh = ymalloc(sizeof(*elh) + qel->extrasz))))
		return NULL;
	memcpy(&elh->el, qel, sizeof(*qel) + qel->extrasz);
	elh->owner = owner;
	ylistl_init_link(&elh->lk);
	lock_elh(tm);
	ylistl_add_last(&tm->elhhd, &elh->lk);
	unlock_elh(tm);
	return elh;
}

int
ytaskmanager_remove_qevent_listener(
	struct ytaskmanager *tm,
	struct ytaskmanager_qevent_listener_handle *elh
) {
	bool r;
	lock_elh(tm);
	r = ylistl_remove2(&tm->elhhd, &elh->lk);
	unlock_elh(tm);
	if (likely(r)) {
		taskmanager_qevent_listener_free(elh);
		return 0;
	} else
		return -EINVAL;
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
extern void task_clear(void);
extern void o_clear(void);
extern void msg_clear(void);
extern void msghandler_clear(void);
extern void msglooper_clear(void);
void
taskmanager_clear(void) {
	task_clear();
	o_clear();
	msg_clear();
	msghandler_clear();
	msglooper_clear();
}
#endif /* CONFIG_DEBUG */
