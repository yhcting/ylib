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
#include <unistd.h>
#include <stdint.h>

#include "common.h"
#include "lib.h"
#include "ytaskmanager.h"
#include "ymsglooper.h"
#include "ymsghandler.h"
#include "ylistl.h"
#include "yo.h"
#include "ygp.h"
#include "task.h"


#define UNLIMITED_SLOTS 99999999
#define TASK_TAG_NAME "___ytaskmanager___"

struct ytaskmanager {
        /* -------- READ ONLY values(set only once) --------*/
	struct ymsghandler *owner;
	int slots;
        /* ---------- Dynamically updated values ----------*/
        struct yhash *tagmap;
	pthread_mutex_t tagmap_lock;

	pthread_mutex_t el_lock;
        struct ylistl_link elhd; /* head of event listener */

	pthread_mutex_t q_lock;
        struct ylistl_link readyq_hd[YTHREADEX_NUM_PRIORITY];
        struct ylistl_link runq_hd; /* head of run Q */

};

struct taskmanager_qevent_listener {
	struct ytaskmanager_qevent_listener el;
	struct ymsghandler *owner;
        /**
         * Fields used inside task module. This value is initialized inside
         * task module.
         */
        struct ylistl_link lk;
};

struct ttg {
	void *tm; /* owner task manager */
	struct ytask *tsk; /* task in where this tag belonging to */
	struct ylistl_link lk; /* q link */
	void *task_event_listener_handle;
};

/******************************************************************************
 *
 * Lock
 *
 *****************************************************************************/
/* Hash operation is quite expensive. So mutex lock is chosen */
declare_lock(mutex, struct ytaskmanager, tagmap, NULL)
/* List operation is very cheap. But calling listeners requires lock */
declare_lock(mutex, struct ytaskmanager, el, NULL)
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
	struct ttg *ttg = tsk->tm.tag;
	return ttg
		&& ttg->tm == tm
		&& ttg->tsk == tsk;
}

static INLINE bool
verify_empty_ttg(struct ytask *tsk) {
	return !tsk->tm.tag && !tsk->tm.tagfree;
}

static INLINE struct ytask *
ttglk_task(struct ylistl_link *tmtaglk) {
	struct ttg *ttg = containerof(tmtaglk, struct ttg, lk);
	return ttg->tsk;
}

static INLINE struct ttg *
task_ttg(struct ytask *tsk) {
	return (struct ttg *)tsk->tm.tag;
}

static INLINE struct ylistl_link *
task_lk(struct ytask *tsk) {
	return &task_ttg(tsk)->lk;
}

static struct ttg *
create_ttg(struct ytaskmanager *tm, struct ytask *tsk) {
	struct ttg *ttg = ycalloc(1, sizeof(*ttg));
	yassert(tm && tsk
		&& !tsk->tm.tag
		&& !tsk->tm.tagfree);
	if (unlikely(!ttg))
		return NULL;
	ttg->tm = tm;
	ttg->tsk = tsk;
	ylistl_init_link(&ttg->lk);

	tsk->tm.tag = ttg;
	tsk->tm.tagfree = &yfree;
	return ttg;
}

static INLINE void
destroy_ttg(struct ytask *tsk) {
	struct ttg *ttg __unused = tsk->tm.tag;
	yassert(ttg
		&& tsk->tm.tagfree);
	yassert(ttg->tsk == tsk);

	(*tsk->tm.tagfree)(tsk->tm.tag);
	tsk->tm.tag = NULL;
	tsk->tm.tagfree = NULL;
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
remove_task_from_runq(struct ytaskmanager *tm,
		      struct ytask *tsk);

static void
task_event_on_finished(struct ytask *tsk) {
	struct ttg *ttg = task_ttg(tsk);
	yassert(!verify_empty_ttg(tsk)
		&& is_in_owner_thread(ttg->tm));
	remove_task_from_runq(ttg->tm, tsk);
}

static void
task_event_on_done(struct ytask *tsk,
		   void *result __unused,
		   int errcode __unused) {
	task_event_on_finished(tsk);
}

static void
task_event_on_cancelled(struct ytask *tsk,
			int errcode) {
	task_event_on_finished(tsk);
}

const static struct ytask_event_listener _tsk_event_listener = {
	.on_cancelled = &task_event_on_cancelled,
	.on_done = &task_event_on_done
};


/******************************************************************************
 *
 *
 *
 *****************************************************************************/
struct qevent_arg {
	/* DO NOT USE pointer for 'el'!. Memory may be freed at other context
	 *   before referencing it.
	 */
	struct ytaskmanager_qevent_listener el;
 	struct ytaskmanager *tm;
	enum ytaskmanager_qevent ev;
	int readyq_sz;
	int runq_sz;
	struct ytask *tsk;
};

static void
notify_qevent_to_listener(void *arg) {
	struct qevent_arg *a = arg;
	a->el.on_event(a->tm, a->ev, a->readyq_sz, a->runq_sz, a->tsk);
}

static void
notify_qevent_locked(struct ytaskmanager *tm,
		     enum ytaskmanager_qevent ev,
		     struct ytask *tsk) {
	struct qevent_arg *a;
	struct taskmanager_qevent_listener *el;
	int readyq_sz = readyq_size_locked(tm);
	int runq_sz = runq_size_locked(tm);
	ylistl_foreach_item(el,
			    &tm->elhd,
			    struct taskmanager_qevent_listener,
			    lk) {
		if (unlikely(!(a = ymalloc(sizeof(*a)))))
			die2("Out-of-memory");
		a->el = el->el; /* deep copy */
		a->tm = tm;
		a->ev = ev;
		a->readyq_sz = readyq_sz;
		a->runq_sz = runq_sz;
		a->tsk = tsk;
		fatali0(ymsghandler_post_exec
			(el->owner, a, &yfree,
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
		ttg->task_event_listener_handle =
			ytask_add_event_listener(tsk,
						 tm->owner,
						 &_tsk_event_listener,
						 FALSE);
		yassert(ttg->task_event_listener_handle);
		fatali0(runq_enq_locked(tm, tsk));
		notify_qevent_locked(tm, YTASKMANAGERQ_MOVED_TO_RUN, tsk);
	}
	unlock_q(tm);
	/* TODO: Improvement(More tolerable). */
	fatal2(!tsk || !ytask_start(tsk), "May be OOM?");
	return !!tsk;
}

static int
add_task_to_readyq(struct ytaskmanager *tm,
		   struct ytask *tsk) {
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
	notify_qevent_locked(tm,
			     YTASKMANAGERQ_ADDED_TO_READY,
			     tsk);
	unlock_q(tm);
	balance_taskq(tm);
	return 0;
}

/*
 * This means task is removed from taskmanager.
 */
static void
remove_task_from_runq(struct ytaskmanager *tm,
		      struct ytask *tsk) {
	struct ttg *ttg = task_ttg(tsk);
	yassert(is_in_owner_thread(tm)
		&& verify_ttg(tm, tsk)
		&& ttg->task_event_listener_handle);
	fatali0(ytask_remove_event_listener
		(tsk, ttg->task_event_listener_handle));
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
	init_el_lock(tm);
	init_q_lock(tm);
	init_tagmap_lock(tm);
	int nrpri = YTHREADEX_NUM_PRIORITY;
	while (nrpri--)
		ylistl_init_link(&tm->readyq_hd[nrpri]);
	ylistl_init_link(&tm->runq_hd);
	ylistl_init_link(&tm->elhd);
	return tm;
}

int
ytaskmanager_destroy(struct ytaskmanager *tm) {
	if (ytaskmanager_size(tm) > 0)
		return -EPERM;
	destroy_q_lock(tm);
	destroy_tagmap_lock(tm);
	destroy_el_lock(tm);
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
ytaskmanager_add_tag(struct ytaskmanager *tm,
		     const char *key,
		     void *tag,
		     void (*tagfree)(void *)) {
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
ytaskmanager_get_tag(struct ytaskmanager *tm,
		     const char *key) {
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
ytaskmanager_remove_tag(struct ytaskmanager *tm,
			const char *key) {
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
	if (unlikely(!verify_empty_ttg(tsk)
		     || YTHREADEX_READY != ytask_get_state(tsk)))
		return -EINVAL;
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
		notify_qevent_locked(tm,
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

struct ytask *
ytaskmanager_find_task(struct ytaskmanager *tm,
		       void *arg,
		       bool (*match)(struct ytask *, void *arg)) {
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

void *
ytaskmanager_add_qevent_listener
(struct ytaskmanager *tm,
 struct ymsghandler *owner,
 const struct ytaskmanager_qevent_listener *qel) {
	struct taskmanager_qevent_listener *elh;
	if (unlikely(!(elh = ymalloc(sizeof(*elh)))))
		return NULL;
	elh->el = *qel;
	elh->owner = owner;
	ylistl_init_link(&elh->lk);
	lock_el(tm);
	ylistl_add_last(&tm->elhd, &elh->lk);
	unlock_el(tm);
	return elh;
}

int
ytaskmanager_remove_qevent_listener
(struct ytaskmanager *tm, void *el_handle) {
	bool r;
	struct taskmanager_qevent_listener *elh = el_handle;
	lock_el(tm);
	r = ylistl_remove2(&tm->elhd, &elh->lk);
	unlock_el(tm);
	return r? 0: -EINVAL;
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
