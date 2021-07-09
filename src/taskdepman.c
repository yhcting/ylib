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
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ytaskdepman.h"
#include "ygraph.h"
#include "ymsghandler.h"
#include "yo.h"
#include "task.h"
#include "taskmanager.h"
#include "common.h"

enum tdm_state {
	TDM_READY,
	TDM_STARTED,
	TDM_CANCELLING,
	TDM_CANCELLED,
	TDM_DONE,
};

struct ttg {
	struct ytaskdepman *tdm;
	struct ytask *tsk;
	struct ytask_event_listener_handle *elh;
	struct yvertex v;
	int count; /* Count tasks depending on this, done. */

};

struct ytaskdepman {
	struct ymsghandler *mh;
	void (*on_done)(struct ytaskdepman *, struct ytask *);
	struct ytaskmanager *tm;
	struct ygraph g;
	struct ytask *target;
	int unhandled_task_cnt;
	enum tdm_state state;
	pthread_mutex_t state_lock;
	pthread_mutex_t graph_lock;
	int errcode;
};

/******************************************************************************
 *
 *
 *
 *****************************************************************************/
declare_lock(mutex, struct ytaskdepman, state, NULL)
declare_lock(mutex, struct ytaskdepman, graph, NULL)

static INLINE enum tdm_state
get_state_locked(struct ytaskdepman *tdm) {
	return tdm->state;
}

static INLINE enum tdm_state
get_state(struct ytaskdepman *tdm) {
	enum tdm_state st;
	lock_state(tdm);
	st = get_state_locked(tdm);
	unlock_state(tdm);
	return st;
}

static INLINE void
set_state_locked(struct ytaskdepman *tdm, enum tdm_state st) {
	tdm->state = st;
}

static INLINE void
set_state(struct ytaskdepman *tdm, enum tdm_state st) {
	lock_state(tdm);
	set_state_locked(tdm, st);
	unlock_state(tdm);
}

static INLINE void
set_errcode(struct ytaskdepman *tdm, int ec) {
	tdm->errcode = ec;
}

static INLINE struct ttg *
task_ttg(struct ytask *tsk) {
	return (struct ttg *)tsk->tdmtag;
}

static INLINE struct ttg *
vertex_ttg(struct yvertex *v) {
	return containerof(v, struct ttg, v);
}

static INLINE bool
verify_ttg(struct ytaskdepman *tdm, struct ytask *tsk) __unused;
static INLINE bool
verify_ttg(struct ytaskdepman *tdm, struct ytask *tsk) {
	struct ttg *ttg = task_ttg(tsk);
	return ttg
		&& ttg->tdm == tdm
		&& ttg->tsk == tsk;
}
static INLINE bool
has_empty_ttg(struct ytask *tsk) {
	return !task_ttg(tsk);
}

static struct ttg *
create_ttg(struct ytaskdepman *tdm, struct ytask *tsk) {
	struct ttg *ttg;
	if (unlikely(!(ttg = ymalloc(sizeof(*ttg)))))
		return NULL;
	ttg->tdm = tdm;
	ttg->tsk = tsk;
	tsk->tdmtag = ttg;
	ygraph_init_vertex(&ttg->v);
	ttg->count = 0;
	return ttg;
}

static void
destroy_ttg(struct ttg *ttg) {
	yfree(ttg);
}

static void
destroy_task_ttg(struct ytask *tsk) {
	struct ttg *ttg = task_ttg(tsk);
	yassert(ttg);
	yassert(ttg->tsk == tsk);
	destroy_ttg(ttg);
	tsk->tdmtag = NULL;
}

/******************************************************************************
 *
 *
 *
 *****************************************************************************/
static void
destroy_handled_task_(struct yo *o) {
	int r;
	struct yo *newo;
	struct ymsghandler *mh = o->o0;
	struct ytask *tsk = o->o1;
	r = ytask_destroy(tsk);
	if (!r) return;
	else if (-EPERM != r || ytask_is_active(tsk))
		die2("Unexpected");
	if (unlikely(!(newo = yocreate1(mh, NULL, tsk, NULL))))
		die2("Out of memory!");
	/* event notification may not be delivered completely yet.
	 * Try again at next loop
	 */
	fatali0(ymsghandler_post_exec(
		mh,
		newo,
		(void(*)(void *))&yodestroy,
		(void(*)(void *))&destroy_handled_task_));
}

static void
destroy_handled_task(struct ytaskdepman *tdm, struct ytask *tsk) {
	struct yo *o;
	if (unlikely(!(o = yocreate1(tdm->mh, NULL, tsk, NULL))))
		die2("Out of memory!");
	/* remove vertex from the dependency structure */
	lock_graph(tdm);
	fatali0(ygraph_remove_vertex(&tdm->g, &task_ttg(tsk)->v));
	unlock_graph(tdm);
	destroy_task_ttg(tsk);
	fatali0(ymsghandler_post_exec(
		tdm->mh,
		o,
		(void(*)(void *))&yodestroy,
		(void(*)(void *))&destroy_handled_task_));
}

static int
inject_task(struct ytaskdepman *tdm, struct ytask *tsk) {
	fatali0(ytaskmanager_add_task(tdm->tm, tsk));
	return 0;
}

/******************************************************************************
 *
 *
 *
 *****************************************************************************/
static void
on_task_finished(struct ytask_event_listener *el, struct ytask *tsk) {
	enum tdm_state state, next_state;
	struct yedge *e;
	struct ttg *ttg = task_ttg(tsk);
	struct ytaskdepman *tdm = ttg->tdm;
	struct ytask *result = tsk;

	lock_state(tdm);
	state = get_state_locked(tdm);
	next_state = TDM_DONE;
	switch (state) {
	case TDM_READY:
	case TDM_DONE:
	case TDM_CANCELLED:
		unlock_state(tdm);
		die();
	case TDM_CANCELLING:
		next_state = TDM_CANCELLED;
		result = NULL;
	case TDM_STARTED:
		;
	}

	/* TDM_STARTED */
	tdm->unhandled_task_cnt--;
	if (!tdm->unhandled_task_cnt) {
		destroy_handled_task(tdm, tsk);
		set_state_locked(tdm, next_state);
		(*tdm->on_done)(tdm, result);
		unlock_state(tdm);
		return;
	}
	unlock_state(tdm);

	if (TDM_CANCELLING == state)
		goto done;

	lock_graph(tdm);
	ygraph_foreach_oedge(&ttg->v, e) {
		struct ttg *t = vertex_ttg(e->vt);
		t->count--;
		if (unlikely(!t->count)) {
			/* All prerequsite tasks are done.
			 * Start target task
			 */
			if (unlikely(inject_task(tdm, t->tsk))) {
				unlock_graph(tdm);
				goto done;
			}

		}
	}
	unlock_graph(tdm);
 done:
	destroy_handled_task(tdm, tsk);
}

static void
task_on_done(
	struct ytask_event_listener *el,
	struct ytask *tsk,
	void *result __unused,
	int errcode __unused
) {
	on_task_finished(el, tsk);
}

static void
task_on_cancelled(
	struct ytask_event_listener *el,
	struct ytask *tsk,
	int errcode __unused
) {
	on_task_finished(el, tsk);
}


static struct ytask_event_listener _task_el = {
	.on_done = &task_on_done,
	.on_cancelled = &task_on_cancelled
};

/******************************************************************************
 *
 *
 *
 *****************************************************************************/
int
ytaskdepman_add_task(struct ytaskdepman *tdm, struct ytask *tsk) {
	struct ttg *ttg;
	if (unlikely(TDM_READY != get_state(tdm)))
		return -EPERM;
	if (unlikely(!(ttg = create_ttg(tdm, tsk))))
		return -ENOMEM;
	ttg->elh = ytask_add_event_listener2(tsk, tdm->mh, &_task_el, FALSE);
	if (unlikely(!ttg->elh)) {
		destroy_ttg(ttg);
		return -ENOMEM;
	}
	ygraph_add_vertex(&tdm->g, &ttg->v);
	tdm->unhandled_task_cnt++;
	return 0;
}

int
ytaskdepman_remove_task(struct ytaskdepman *tdm, struct ytask *tsk) {
	int r;
	struct ttg *ttg = task_ttg(tsk);
	yassert(verify_ttg(tdm, tsk));
	if (unlikely(!ttg))
		return -EINVAL;
	if (unlikely(TDM_READY != get_state(tdm)))
		return -EPERM;
	if (unlikely(r = ygraph_remove_vertex(&tdm->g, &ttg->v)))
		return r;
	fatali0(ytask_remove_event_listener(tsk, ttg->elh));
	destroy_task_ttg(tsk);
	tdm->unhandled_task_cnt--;
	return 0;
}

int
ytaskdepman_add_dependency(
	struct ytaskdepman *tdm,
	struct ytask *target,
	struct ytask *prereq
) {
	/* represented as incoming edge of target */
	int r;
	struct ttg *tt = task_ttg(target);
	struct ttg *pt = task_ttg(prereq);
	yassert(verify_ttg(tdm, target)
		&& verify_ttg(tdm, prereq));
	if (unlikely(TDM_READY != get_state(tdm)))
		return -EPERM;
	if (unlikely(r = ygraph_add_edge(&tdm->g, NULL, &pt->v, &tt->v)))
		return r;
	tt->count++;
	return 0;
}

int
ytaskdepman_remove_dependency(
	struct ytaskdepman *tdm,
	struct ytask *target,
	struct ytask *prereq
) {
	int r;
	struct ttg *tt = task_ttg(target);
	struct ttg *pt = task_ttg(prereq);
	yassert(verify_ttg(tdm, target)
		&& verify_ttg(tdm, prereq));
	if (unlikely(TDM_READY != get_state(tdm)))
		return -EPERM;
	if (unlikely(r = ygraph_remove_edge(&tdm->g, &pt->v, &tt->v)))
		return r;
	tt->count--;
	yassert(tt->count >= 0);
	return 0;
}

struct ytaskdepman *
ytaskdepman_create(
	struct ymsghandler *mh,
	void (*on_done)(struct ytaskdepman *, struct ytask *),
	int slots
) {
	struct ytaskdepman *tdm;
	yassert(mh && on_done);
	if (unlikely(!(tdm = ycalloc(1, sizeof(*tdm)))))
		return NULL;
	tdm->mh = mh;
	if (unlikely(!(tdm->tm = ytaskmanager_create(mh, slots))))
		goto free_taskdepman;
	init_state_lock(tdm);
	init_graph_lock(tdm);
	tdm->on_done = on_done;
	ygraph_init(&tdm->g);
	tdm->state = TDM_READY;

	return tdm;

 free_taskdepman:
	yfree(tdm);
	return NULL;
}

int
ytaskdepman_destroy(struct ytaskdepman *tdm) {
	enum tdm_state state = get_state(tdm);
	if (!(TDM_CANCELLED == state || TDM_DONE == state))
		return -EPERM;
	/* all tasks are done.
	 * So, we can sure that in short time, ytaskmanager will be empty
	 */
	while (ytaskmanager_size(tdm->tm))
		usleep(1 * 1000);
	fatali0(ytaskmanager_destroy(tdm->tm));
	lock_graph(tdm);
	ygraph_clean(&tdm->g);
	unlock_graph(tdm);
	destroy_state_lock(tdm);
	destroy_graph_lock(tdm);
	yfree(tdm);
	return 0;
}

int
ytaskdepman_verify(struct ytaskdepman *tdm, struct ytask **roottsk) {
	int r, n, nr_vertices;
	struct yvertex *v, *root;

	if (unlikely(TDM_READY != get_state(tdm)))
		return -EPERM;

	nr_vertices = 0;
	/* reset count flag */
	ygraph_foreach_vertex(&tdm->g, v) {
		nr_vertices++;
	}

	if (unlikely(!nr_vertices))
		return YTASKDEPMAN_EMPTY;

	/* find root */
	root = NULL;
	ygraph_foreach_vertex(&tdm->g, v) {
		if (0 == ygraph_oedge_size(&tdm->g, v)) {
			if (unlikely(root))
				return YTASKDEPMAN_MULTI_ROOT;
			root = v;
		}
	}
	if (roottsk)
		*roottsk = vertex_ttg(root)->tsk;
	/* single root is found. Check circular depenency and dangling node */
	r = ygraph_has_cycle(&tdm->g, root, &n);
	switch (r) {
	case 1:
		return YTASKDEPMAN_CIRCULAR_DEP;
	case 0:
		return n < nr_vertices
			? YTASKDEPMAN_ISOLATED_TASK
			: YTASKDEPMAN_OK;
	default:
		return -r;
	}
}

int
ytaskdepman_start(struct ytaskdepman *tdm) {
	int r;
	struct yvertex *v;
	yassert(tdm);
	if (YTASKDEPMAN_OK != (r = ytaskdepman_verify(tdm, &tdm->target)))
		return r;

	lock_state(tdm);
	if (unlikely(TDM_READY != get_state_locked(tdm))) {
		unlock_state(tdm);
		return -EPERM;
	}
	set_state_locked(tdm, TDM_STARTED);
	unlock_state(tdm);

	/* start all leaf tasks */
	ygraph_foreach_vertex(&tdm->g, v) {
		if (0 == ygraph_iedge_size(&tdm->g, v)) {
			if (unlikely(r = inject_task(tdm, vertex_ttg(v)->tsk)))
				return r;
		}
	}
	return 0;
}

int
ytaskdepman_cancel(struct ytaskdepman *tdm) {
	struct yvertex *v, *vtmp;
	struct ytask *tsk;
	enum tdm_state state;

	lock_state(tdm);
	state = get_state_locked(tdm);
	if (!(TDM_READY == state || TDM_STARTED == state)) {
		unlock_state(tdm);
		return -EPERM;
	}

	if (TDM_READY == state
		&& !tdm->unhandled_task_cnt
	) {
		set_state_locked(tdm, TDM_CANCELLED);
		unlock_state(tdm);
		return 0;
	}
	set_state_locked(tdm, TDM_CANCELLING);
	unlock_state(tdm);

	lock_graph(tdm);
	/* TODO: Improve fault tolerance here! */
	ygraph_foreach_vertex_safe(&tdm->g, v, vtmp) {
		tsk = vertex_ttg(v)->tsk;
		yassert(YTHREADEX_READY == ytask_get_state(tsk));
		/* Ignore return value. Try to cancel all tasks */
		ytask_cancel(tsk);
	}
	unlock_graph(tdm);
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
extern void task_clear(void);
extern void o_clear(void);
extern void msghandler_clear(void);
void
taskdepman_clear(void) {
	task_clear();
	o_clear();
	msghandler_clear();
}
#endif /* CONFIG_DEBUG */
