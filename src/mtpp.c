/******************************************************************************
 * Copyright (C) 2014, 2015
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

#include <limits.h>
#include <pthread.h>
#include <stdio.h>

#include "common.h"
#include "ygraph.h"
#include "ymtpp.h"
#include "ymsg.h"
#include "ymsgq.h"
#include "ylist.h"

/******************************************************************************
 *
 *
 *
 *****************************************************************************/

/******************************************************************************
 *
 * Data structures
 *
 *****************************************************************************/
#define MCODE_DEF_LIST					\
	/* start running */				\
	_MCODE_DEF(START,     YMSG_PRI_HIGHER)		\
	/* internal error */				\
	_MCODE_DEF(ERROR,     YMSG_PRI_HIGHER)		\
	/* all jobs are done */				\
	_MCODE_DEF(ALL_DONE,  YMSG_PRI_NORMAL)		\
	/* one job is done.				\
	 * Message data is 'return value from job'.	\
	 * data : struct job_routine_ret.		\
	 */						\
	_MCODE_DEF(JOB_END,   YMSG_PRI_NORMAL)		\
	_MCODE_DEF(JOB_ERROR, YMSG_PRI_HIGH)

#define _MCODE_DEF(x, y) MCODE_##x,
enum {
	MCODE_DEF_LIST
	MCODE_ENUM_LIMIT
};
#undef _MCODE_DEF

struct ymtpp {
	struct ygraph *g;
	struct yvertex *targetv; /* target vertex */
	struct ymsgq *mq;
	u32 maxjobs; /* # jobs can be run in parallel */
	/* Below this line : variables foro runtime information.
	 * That is, they become dirty while running - ymtpp_run()
	 */
	struct ylistl_link readyQ;
	u32 runningjobs; /* # jobs now running */
	int err; /* error value. 0 means "OK" */
};

struct job {
	struct ymtppjob j; /* This MUST BE top of this structure */
	struct ylistl_link lk; /* for Queue */
	pthread_t thread; /* Thread dedicated to this job */
	int wjcnt; /* Waiting Job CouNT. -1 if NOT initialized. */
	void *jobout; /* 'out' value of this job */
	/* job Output REFerence CouNT.
	 * Output value of the job is used as input value of next jobs.
	 * And Output value should be freed after all next jobs are finished.
	 * Like above, output of ccjob is used several places simultaneously.
	 * So, reference count is requred to know when this object can be
	 *   freed safely.
	 */
	int jorefcnt;
	int flag; /* used as flag */
};

struct job_thread_arg {
	struct ymtpp *m;
	struct job *j;
};

#ifdef CONFIG_DEBUG
#define _MCODE_DEF(x, y) #x,
static const char *_mcode_name[] __unused = {
	MCODE_DEF_LIST
};
#undef _MCODE_DEF
#endif /* CONFIG_DEBUG */

#define _MCODE_DEF(x, y) y,
static const u8 _mcode_pri[] = {
	MCODE_DEF_LIST
};
#undef _MCODE_DEF

/******************************************************************************
 *
 * Accessing data structure.
 *
 *****************************************************************************/



/******************************************************************************
 *
 * Message utils
 *
 *****************************************************************************/
/*
 * return
 *     0 : success
 *    <0 : -errno
 */
static int
post_message(struct ymtpp *m,
	     int code,
	     void *data,
	     void (*dfree)(void *)) {
	struct ymsg *msg;
	int r = 0;
	if (unlikely(code < 0
		     || code >= MCODE_ENUM_LIMIT))
		return -EINVAL;

	dfpr("code=%s", _mcode_name[code]);
	if (unlikely(!(msg = ymsg_create())))
		return -ENOMEM;
	ymsg_set_data(msg, _mcode_pri[code], 0, code, data, dfree);
	msg->dfree = dfree;
	if (unlikely(r = ymsgq_en(m->mq, msg))) {
		ymsg_destroy(msg);
		return r;
	}
	return 0;
}


/******************************************************************************
 *
 * Access job.
 *
 *****************************************************************************/
static INLINE struct job *
v2job(struct yvertex *v) {
	return (struct job *)&v->d;
}

static INLINE struct yvertex *
job2v(struct job *j) {
	return containerof(j, struct yvertex, d);
}

static void
job_init(struct job *j, const struct ymtppjob *job) {
	memmove(&j->j, job, sizeof(j->j));
	ylistl_init_link(&j->lk);
	j->thread = 0;
	j->wjcnt = -1;
	j->jobout = NULL;
	j->jorefcnt = 0;
	j->flag = 0;
}

static INLINE void
job_get_jobout(struct job *j) {
	++j->jorefcnt;
}

static void
job_put_jobout(struct job *j) {
	if (unlikely(0 >= j->jorefcnt))
		return;
	--j->jorefcnt;
	dfpr("%s: %d", j->j.name, j->jorefcnt);
	if (unlikely(!j->jorefcnt
		     && j->jobout && j->j.ofree)) {
		dfpr("[%s] out freed", j->j.name);
		(*j->j.ofree)(j->jobout);
		j->jobout = NULL;
	}
}

/* put data that are used as arguments for this job */
static void
job_put_arguments(struct job *j) {
	struct yedge *e;
	struct yvertex *v = job2v(j);
	ygraph_foreach_iedge(v, e) {
		job_put_jobout(v2job(e->vf));
	}
}

static void
job_clean(struct job *j) {
	/* free additional argument */
	if (j->j.arg && j->j.afree)
		(*j->j.afree)(j->j.arg);
	if (j->jobout && j->j.ofree)
		(*j->j.ofree)(j->jobout);
}


/******************************************************************************
 *
 *
 *
 *****************************************************************************/
/*
 * return
 *     1 : success and iteration is done.
 *     0 : success but iteration is interrupted by callback's return
 *    <0 : -errno
 */
static int
iterate_depending_vertices_dfs(struct ymtpp *m,
                               struct yvertex *basev,
                               void *tag,
                               /* callback called per each vertex.
                                * return
                                *     1 : keep iteration
                                *     0 : stop iteration
                                *    <0 : -errno
                                */
                               int (*cb)(struct yvertex *, void *)) {
	struct ylist *vs;
	struct yvertex *v;
	struct yedge* e;
	int r = -EINVAL;

#define mark_discovered__(v) do { v2job(v)->flag = 1; } while (0)
#define mark_undiscovered__(v) do { v2job(v)->flag = 0; } while (0)
#define is_discovered__(v) (!!v2job(v)->flag)

	/* DFS */
	if (unlikely(!(vs = ylist_create(0, NULL))))
		return -ENOMEM;

	/* clear all flag of vertex */
	ygraph_foreach_vertex(m->g, v) {
		mark_undiscovered__(v);
	}

	if (unlikely(r = ylist_push(vs, basev)))
		goto done;

	while (ylist_size(vs)) {
		v = ylist_pop(vs);
		if (is_discovered__(v))
			/* It's already discovered. Skip next steps! */
			continue;

		/* This is first visit */
		if (unlikely(0 > (r = (*cb)(v, tag))))
			goto done;
		/* This would better to be called after callback, because
		 *   at some callbacks, flag may be reset in it
		 */
		mark_discovered__(v);

		if (unlikely(!r))
			goto done;

		ygraph_foreach_iedge(v, e) {
			if (unlikely(r = ylist_push(vs, e->vf)))
				goto done;
		}
	}
	r = 1; /* iteration is completely done */

 done:
	ylist_destroy(vs);
	return r;

#undef is_discovered__
#undef mark_undiscovered__
#undef mark_discovered__
}

static void *
job_thread_routine(void *arg) {
	int r __unused;
	int sz;
	struct job *j;
	struct ymtpp *m;
	struct yvertex *v;
	struct yedge *e;
	struct ymtppjobarg *jargs = NULL;
	struct job_thread_arg *jta = arg;

	/* get data from thread argument */
	m = jta->m;
	j = jta->j;
	yfree(jta);

	v = job2v(j);

	dfpr("%s : %d", j->j.name, j->wjcnt);
	yassert(!j->wjcnt);

	/* Check unrecoverable Out Of Memory! */
	sz = ylistl_size(&v->ie);
	/* + 1 for terminating empty arg */
	if (unlikely(!(jargs = ymalloc(sizeof(*jargs) * (sz + 1)))))
		goto err;
	/* fill terminiating empty arg with 0 */
	memset(jargs + sz, 0, sizeof(*jargs));

	ygraph_foreach_iedge(v, e) {
		struct job *ij = v2job(e->vf);
		--sz;
		jargs[sz].name = ij->j.name;
		jargs[sz].data = ij->jobout;
	}

	if (unlikely(0 > (*j->j.run)(&j->jobout, j->j.arg, jargs)))
		goto err;

	if (unlikely(post_message(m, MCODE_JOB_END, j, NULL)))
		goto err;

	yfree(jargs);
	return NULL;

 err:
	if (jargs)
		yfree(jargs);
	if (unlikely(post_message(m, MCODE_JOB_ERROR, j, NULL)))
		yassert(0);

	return NULL;
}

/*
 * return
 *    <0 : -errno
 *     Otherwise # of jobs started.
 */
static int
run_ready_jobs(struct ymtpp *m) {
	static pthread_attr_t __thdattr;
	static pthread_attr_t *__pthdattr;
	int count;
	int r = 0;
	if (unlikely(!__pthdattr)) {
		if (unlikely(r = pthread_attr_init(&__thdattr)))
			return -r;
		/* pthread_join() is NOT used!.
		 * So, thread should be detached by default.
		 */
		if (unlikely(r = pthread_attr_setdetachstate
			         (&__thdattr, PTHREAD_CREATE_DETACHED))) {
			pthread_attr_destroy(&__thdattr);
			return -r;
		}
		__pthdattr = &__thdattr;
	}

	if (unlikely(m->err))
		/* In case of error, new job SHOULD NOT be launched */
		return 0;

	count = 0;
	while (m->maxjobs > m->runningjobs
	       && ylistl_size(&m->readyQ) > 0) {
		struct job_thread_arg *jta;
		struct job *ccj;
		struct ylistl_link *plk = m->readyQ.next;
		/* remove first - dequeue from the readyQ */
		ylistl_remove(plk);
		ccj = containerof(plk, struct job, lk);
		yassert(!ccj->wjcnt);
		if (unlikely(!(jta = ymalloc(sizeof(*jta)))))
			return -ENOMEM;
		jta->m = m;
		jta->j = ccj;
		if (unlikely(r = pthread_create(&ccj->thread,
						__pthdattr,
						&job_thread_routine,
						jta)))
			return -r;
		dfpr("Thread Created");
		++m->runningjobs;
		++count;
	}
	return count;
}

static int
cleanup_jobout_cb(struct yvertex *v, void *tag __unused) {
	struct job *j = v2job(v);
	if (j->jobout && j->j.ofree) {
		(*j->j.ofree)(j->jobout);
		j->jobout = NULL;
	}
	return 1;
}

/******************************************************************************
 *
 * Message Handlers
 *
 *****************************************************************************/
static int
mhndr_start_enq_leaf_cb(struct yvertex *v, void *tag) {
	struct ymtpp *m = tag;
	/* Is this leaf vertex? */
	if (unlikely(!ylistl_size(&v->ie))) {
		/* This is leaf vertex. Add to the ready Queue */
		dfpr("*** Leaf : %s", v2job(v)->j.name);
		ylistl_add_last(&m->readyQ, &v2job(v)->lk);
	}
	return 1;
}

/* return 0 if success. Otherwise -errno */
static int
mhndr_start(struct ymtpp *m, struct yvertex *targetv) {
	int r = iterate_depending_vertices_dfs
		(m, targetv, (void *)m, &mhndr_start_enq_leaf_cb);
	if (unlikely(0 >= r))
		return r? r: -EINTR;
	if (unlikely(0 > (r = run_ready_jobs(m))))
		return r;
	return 0;
}

static void
mhndr_error(struct ymtpp *m) {
	m->err = 1;
}

/* return 0 if success. Otherwise -errno */
static int
mhndr_job_end(struct ymtpp *m,
	      struct job *j) {
	struct yedge *e;
	int r;
	struct yvertex *v = job2v(j);

	--m->runningjobs; /* one job is endded */

	ygraph_foreach_oedge(v, e) {
		struct job *oj = v2job(e->vt);
		if (0 > oj->wjcnt)
			/* This is NOT participatns */
			continue;
		yassert(oj->wjcnt > 0);
		/* Job is done.
		 * And, output of this job will be used by next jobs.
		 * So, reference count for out data of this job should be
		 *   increased by number of next jobs waiting this job.
		 */
		job_get_jobout(j);
		--oj->wjcnt;
		if (unlikely(!oj->wjcnt))
			/* add to ready Q */
			ylistl_add_last(&m->readyQ, &oj->lk);
	}
	job_put_arguments(j);
	if (unlikely(0 > (r = run_ready_jobs(m))))
		return r;
	return 0;
}

/* return 0 if success. Otherwise -errno */
static int
mhndr_job_error(struct ymtpp *m, struct job *j) {
	--m->runningjobs; /* one job is endded with error*/
	m->err = 1;
	job_put_arguments(j);
	return 0;
}

/******************************************************************************
 *
 * ymtpp_run helper functions
 *
 *****************************************************************************/
/* Initialize mtpp RunTime INFOrmation */
static void
init_rtinfo(struct ymtpp *m) {
	m->runningjobs = 0;
	m->err = 0;
	ylistl_init_link(&m->readyQ);
}

/*
 * Check cyclic link
 *
 * return
 *     1 : cyclic link exists
 *     0 : cyclic link DOESN'T exists
 *    <0 : -errno
 */
static int
check_cyclic_link(struct ymtpp *m, struct yvertex *basev) {
	struct ylist *vs; /* Vertex Stack */
	struct ylist *es; /* Edge Stack */
	struct yvertex *v;
	struct yedge* e;
	int r = 0;

#define __mark_discovered(v) do { v2job(v)->flag = 1; } while (0)
#define __mark_undiscovered(v) do { v2job(v)->flag = 0; } while (0)
#define __is_discovered(v) (!!v2job(v)->flag)

	/* clear all flag of vertex */
	ygraph_foreach_vertex(m->g, v) {
		__mark_undiscovered(v);
	}

	/* DFS */
	vs = ylist_create(0, NULL);
	es = ylist_create(0, NULL);

	if (unlikely(!vs || !es)) {
		r = -ENOMEM;
		goto done;
	}

	if (unlikely(r = ylist_push(vs, basev)))
		goto done;
	ygraph_foreach_iedge(basev, e) {
		if (unlikely(r = ylist_push(es, e)))
			goto done;
	}

	r = 0;
	while (ylist_size(es)) {
		e = ylist_pop(es);
		yassert(e);
		v = e->vf;

		/* Adjust DFS vertex history.
		 * Top of vertex stack SHOULD be same with target vertex
		 *   of current visiting edge.
		 * That is, vertex stack has right DFS visit-history.
		 */
		while (e->vt != ylist_peek_last(vs)
		       && ylist_size(vs) > 0)
			ylist_pop(vs);

		if (__is_discovered(v)) {
			if (ylist_has(vs, v)) {
				r = 1;
				goto done;
			}
			/* It's already discovered. Skip next steps! */
			continue;
		}

		/* This is first visit */

		/* save to history stack */
		__mark_discovered(v);
		if (unlikely(r = ylist_push(vs, v)))
			goto done;

		ygraph_foreach_iedge(v, e) {
			if (unlikely(r = ylist_push(es, e)))
				goto done;
		}
	}

 done:
	if (likely(vs))
		ylist_destroy(vs);
	if (likely(es))
		ylist_destroy(es);
	dfpr("returns : %d\n", r);
	return r;

#undef __is_discovered
#undef __mark_undiscovered
#undef __mark_discovered

}

static int
prepare_pp_run_cb(struct yvertex *v,
		      void *tag __unused) {
	struct job *j = v2job(v);
	job_init(j, &j->j); /* initialize with existing job info */
	v2job(v)->wjcnt = ylistl_size(&v->ie);
	return 1;
}

/*
 * return
 *     0 : success
 *    <0 : -errno
 */
static int
prepare_pp_run(struct ymtpp *m, struct yvertex *targetv) {
	int r;
	struct yvertex *v;
	m->targetv = targetv;

	/* set wjcnt to -1 which means "NOT participants" */
	ygraph_foreach_vertex(m->g, v) {
		struct job *j = v2job(v);
		j->wjcnt = -1;
	}

	init_rtinfo(m);

	/* initialize participant jobs */
	if (unlikely(0 > (r = iterate_depending_vertices_dfs
			      (m, targetv, NULL, &prepare_pp_run_cb))))
		return r;
	return r? 0: -EINTR;
}

/******************************************************************************
 *
 * Interface helper functions
 *
 *****************************************************************************/

/******************************************************************************
 *
 * Interfaces
 *
 *****************************************************************************/
struct ymtppjob *
ymtpp_task_job(struct yvertex *v) {
	return (struct ymtppjob *)v2job(v);
}

ymtpp_task_t
ymtpp_create_task(struct ymtpp *m) {
	return ygraph_create_vertex(m->g);
}

void
ymtpp_destroy_task(struct ymtpp *m, struct yvertex *v) {
	ygraph_destroy_vertex(m->g, v);
}

struct ymtpp *
ymtpp_create(u32 maxjobs) {
	struct ymtpp *m;
	if (unlikely(!(m = ymalloc(sizeof(*m)))))
		return NULL;
	m->targetv = NULL;
	if (unlikely(!(m->g = ygraph_create(sizeof(struct job),
					    (void(*)(void *))&job_clean,
					    0,
					    NULL))))
		goto fail_mtpp;

	if (unlikely(!(m->mq = ymsgq_create(0))))
		goto fail_graph;

	m->maxjobs = !maxjobs? UINT_MAX: maxjobs;
	return m;

 fail_graph:
	ygraph_destroy(m->g);
 fail_mtpp:
	yfree(m);

	return NULL;
}

void
ymtpp_destroy(struct ymtpp *m) {
	if (unlikely(!m))
		return;
	ymsgq_destroy(m->mq);
	ygraph_destroy(m->g);
	yfree(m);
}

int
ymtpp_add_dependency(struct ymtpp *m,
		     struct yvertex *target,
		     struct yvertex *dependent) {
	if (unlikely(!(m && target && dependent)))
		return -EINVAL;
	return ygraph_add_edge(m->g, NULL, dependent, target);
}

int
ymtpp_remove_dependency(struct ymtpp *m,
			struct yvertex *target,
			struct yvertex *dependent) {
	if (unlikely(!(m && target && dependent)))
		return -EINVAL;
	return ygraph_remove_edge(m->g, dependent, target);
}

int
ymtpp_add_task(struct ymtpp *m, struct yvertex *v) {
	if (unlikely(!(m && v)))
		return -EINVAL;
	ygraph_add_vertex(m->g, v);
	return 0;
}

int
ymtpp_run(struct ymtpp *m,
	  void **out,
	  struct yvertex *targetv) {
	struct ymsg *msg;
	void *mdata;
	int r;

	dfpr("%p, %p\n", m, targetv);
	dfpr("!!!!!!!!!!!!!!!!!!!!!!!!!!! start run !!!!!!!!!!!!!!!!!!!!\n");
	if (unlikely(!(targetv && m)))
		return -EINVAL;

	dchkpt();

	/* Cyclic loop is NOT allowed */
	r = check_cyclic_link(m, targetv);
	if (unlikely(1 == r))
		return -EPERM; /* cyclick link */
	else if (unlikely(r))
		return r;

	dchkpt();
	dfpr("+++ prepare");
	if (unlikely(r = prepare_pp_run(m, targetv)))
		return r;

	if (unlikely(r = post_message(m, MCODE_START, NULL, NULL)))
		return r;

	dfpr("+++ start");
	/* Run message loop */
	while (1) {
		int mcode;

		msg = ymsgq_de(m->mq);
		mcode = msg->code;
		mdata = msg->data;
		ymsg_destroy(msg);

		dfpr("code=%s", _mcode_name[mcode]);
		switch (mcode) {
		case MCODE_START:
			r = mhndr_start(m, targetv);
			break;
		case MCODE_ERROR:
			/* TODO : Is this really required? */
			mhndr_error(m);
			goto exit_mloop;
		case MCODE_JOB_END:
			r = mhndr_job_end(m, mdata);
			break;
		case MCODE_JOB_ERROR:
			r = mhndr_job_error(m, mdata);
			break;
		case MCODE_ALL_DONE:
			/* all done successfully */
			r = 0;
			goto exit_mloop;
		default: /* This is NOT expected - unknown code */
			r = -EINVAL;
			yassert(0);
		}
		if (r) {
			/* Critical internal error
			 * Any way to handle this case smoothly???
			 */
			;
		}

		if (unlikely(r
			     && post_message(m,
					     MCODE_ERROR,
					     NULL,
					     NULL)))
			yassert(0); /* unrecoverable error */

		if (unlikely(!m->runningjobs
			     && post_message(m,
					     MCODE_ALL_DONE,
					     mdata,
					     NULL)))
			yassert(0); /* unrecoverable error */
	}

 exit_mloop:

	/* Successful running! */
	if (likely(!m->err)) {
		struct job *j = mdata;
		if (likely(out))
			*out = j->jobout;
		else
			(*j->j.ofree)(j->jobout);
		/* output data is returned to caller or already freed.
		 * This SHOULD NOT be freed by anywhere else in mtpp.
		 */
		j->jobout = NULL;
		return 0;
	}

	/* Oops... running encounters error!
	 * Let's do cleanup process
	 */

	/* clean up jobout data */
	iterate_depending_vertices_dfs(m,
                                       targetv,
                                       NULL,
                                       &cleanup_jobout_cb);
	return -EINVAL;
}
