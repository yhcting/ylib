/******************************************************************************
 * Copyright (C) 2014
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

/* #define LIB_DEBUG */

#include <limits.h>
#include <pthread.h>
#include <stdio.h>

#include "libdbg.h"
#include "ygraph.h"
#include "ymtconcur.h"
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
enum {
	MCODE_START,	  /* start running */
	MCODE_ERROR,	  /* internal error */
	/* one job is done.
	 * Message data is 'return value from job'.
	 * data : struct job_routine_ret.
	 */
	MCODE_JOB_END,
	MCODE_JOB_ERROR,
};

struct ymtconcur {
	struct ygraph	    g;
	struct yvertex	   *targetv;	 /* target vertex */
	struct ymq	   *mq;
	struct ylistl_link  readyQ;
	unsigned int	    maxjobs;	 /* # jobs can be run in parallel */
	unsigned int	    runningjobs; /* # jobs now running */
	int		    err;	 /* error value. 0 means "OK" */
};

/* ccjob : ConCurrent JOB */
struct ccjob {
	struct ymtconcurjob j;
	struct ylistl_link lk; /* for Queue */
	pthread_t thread; /* Thread dedicated to this job */
	/* Waiting Job CouNT. -1 if NOT initialized. */
	int wjcnt;
	void *jobout; /* 'out' value of this job */
	/* job Output REFerence CouNT.
	 * Output value of the job is used as input value of next jobs.
	 * And Output value should be freed after all next jobs are finished.
	 * Like above, output of ccjob is used several places simultaneously.
	 * So, reference count is requred to know when this object can be
	 *   freed safely.
	 */
	int jorefcnt;
	int flag;  /* used as flag */
};

struct job_thread_arg {
	struct ymtconcur *m;
	struct ccjob	 *j;
};

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
 *    <0 : fail
 */
static int
post_message(struct ymtconcur *m,
	     int code,
	     void *data,
	     void (*freecb)(void *)) {
	struct ymsg *msg;
	uint8_t pri = YMSG_PRI_NORMAL;

	dfpr("code=%d", code);
	/* critical messages */
	switch (code) {
	case MCODE_START:
	case MCODE_ERROR:
	case MCODE_JOB_ERROR:
		pri = YMSG_PRI_VERY_HIGH;
	}

	if (unlikely(!(msg = ymsg_create())))
		return -1;
	ymsg_init_data(msg, pri, code, data);
	msg->free = freecb;
	if (unlikely(ymq_en(m->mq, msg))) {
		ymsg_destroy(msg);
		return -1;
	}
	return 0;
}


/******************************************************************************
 *
 *
 *
 *****************************************************************************/
static void
ccjob_init(struct ccjob *j, const struct ymtconcurjob *job) {
	memmove(&j->j, job, sizeof(j->j));
	ylistl_init_link(&j->lk);
	j->thread = 0;
	j->wjcnt = -1;
	j->jobout = NULL;
	j->jorefcnt = 0;
	j->flag = 0;
}

static inline void
ccjob_get_jobout(struct ccjob *j) {
	++j->jorefcnt;
	dfpr("%s: %d", j->j.name, j->jorefcnt);
}

static void
ccjob_put_jobout(struct ccjob *j) {
	if (unlikely(0 >= j->jorefcnt))
		return;
	--j->jorefcnt;
	dfpr("%s: %d", j->j.name, j->jorefcnt);
	if (unlikely(!j->jorefcnt
		     && j->jobout && j->j.free_out)) {
		dfpr("[%s] out freed", j->j.name);
		(*j->j.free_out)(j->jobout);
		j->jobout = NULL;
	}
}

static void
ccjob_clean(struct ccjob *j) {
	/* free additional argument */
	if (j->j.arg && j->j.free_arg)
		(*j->j.free_arg)(j->j.arg);
	if (j->jobout && j->j.free_out)
		(*j->j.free_out)(j->jobout);
}

static inline struct ccjob *
v2job(struct yvertex *v) {
	return (struct ccjob *)ygraph_vertex_get_data(v);
}

static inline struct yvertex *
job2v(struct ccjob *j) {
	return container_of(j, struct yvertex, d);
}

static struct yvertex *
create_job_vertex(const struct ymtconcurjob *job) {
	struct yvertex *v;

	if (unlikely(!(v = ygraph_vertex_create(sizeof(struct ccjob)))))
		return NULL;
	if (unlikely(ygraph_vertex_init(v)))
		goto alloc_vertex;
	ccjob_init(v2job(v), job);
	return v;

 alloc_vertex:
	ygraph_vertex_destroy(v, NULL);
	return NULL;
}

static inline void
destroy_job_vertex(struct yvertex *v) {
	ygraph_vertex_destroy(v, NULL);
}

/*
 * return
 *     1 : success and walking is done.
 *     0 : success but walking is interrupted by callback's return
 *    -1 : error
 */
static int
walk_depending_vertices_dfs(struct ymtconcur *m,
			    struct yvertex *basev,
			    void *user,
			    /* callback called per each vertex.
			     * return
			     *	   1 : keep walking
			     *	   0 : stop walking
			     *	  -1 : error
			     */
			    int (*cb)(struct yvertex *, void *)) {
	int r;
	struct ylist *vs;
	struct yvertex *v;
	struct yedge* e;

#define mark_discovered__(v) do { v2job(v)->flag = 1; } while (0)
#define mark_undiscovered__(v) do { v2job(v)->flag = 0; } while (0)
#define is_discovered__(v) (!!v2job(v)->flag)

	dfpr(">>>>>>>>>>>>>>>>>>>>>>>");
	/* DFS */
	if (unlikely(!(vs = ylist_create(0, NULL))))
		return -1;

	/* clear all flag of vertex */
	ygraph_foreach_vertex(&m->g, v) {
		mark_undiscovered__(v);
	}

	if (unlikely(0 > ylist_push(vs, basev)))
		goto fail;

	r = 1;
	while (ylist_size(vs)) {
		int cbr;
		v = ylist_pop(vs);
		if (is_discovered__(v))
			/* It's already discovered. Skip next steps! */
			continue;

		dfpr("callback: %s", v2job(v)->j.name);
		/* This is first visit */
		cbr = (*cb)(v, user);
		/* This would better to be called after callback, because
		 *   at some callbacks, flag may be reset in it
		 */
		mark_discovered__(v);

		if (unlikely(0 > cbr))
			goto fail;

		if (unlikely(!cbr)) {
			r = 0;
			break;
		}

		ygraph_foreach_iedge(v, e) {
			if (unlikely(0 > ylist_push(vs, e->vf)))
				goto fail;
		}
	}

	ylist_destroy(vs);
	return r;

 fail:
	ylist_destroy(vs);
	return -1;


#undef is_discovered__
#undef mark_undiscovered__
#undef mark_discovered__
}

static int
prepare_concur_run_cb(struct yvertex *v,
		      void *user __attribute__((unused))) {
	struct ccjob *j = v2job(v);
	ccjob_init(j, &j->j); /* initialize with existing job info */
	v2job(v)->wjcnt = ylistl_size(&v->ie);
	return 1;
}

/*
 * return
 *     0 : success
 *    <0 : error
 */
static int
prepare_concur_run(struct ymtconcur *m, struct yvertex *targetv) {
	struct yvertex *v;
	/* set wjcnt to -1 which means "NOT participants" */
	ygraph_foreach_vertex(&m->g, v) {
		struct ccjob *j = v2job(v);
		j->wjcnt = -1;
	}

	/* initialize readyQ */
	ylistl_init_link(&m->readyQ);

	/* initialize participant ccjobs */
	if (likely(1 == walk_depending_vertices_dfs(m,
						    targetv,
						    NULL,
						    &prepare_concur_run_cb)))
		return 0;
	return -1;
}

/*
 * Check cyclic link
 *
 * return
 *     1 : cyclic link exists
 *     0 : cyclic link DOESN'T exists
 *    <0 : error
 */
static int
check_cyclic_link(struct ymtconcur *m, struct yvertex *basev) {
	int r;
	struct ylist *vs; /* Vertex Stack */
	struct ylist *es; /* Edge Stack */
	struct yvertex *v;
	struct yedge* e;

#define mark_discovered__(v) do { v2job(v)->flag = 1; } while (0)
#define mark_undiscovered__(v) do { v2job(v)->flag = 0; } while (0)
#define is_discovered__(v) (!!v2job(v)->flag)


	/* clear all flag of vertex */
	ygraph_foreach_vertex(&m->g, v) {
		mark_undiscovered__(v);
	}

	/* DFS */
	vs = ylist_create(0, NULL);
	es = ylist_create(0, NULL);

	if (unlikely(!vs || !es))
		goto fail;

	if (unlikely(0 > ylist_push(vs, basev)))
		goto fail;
	ygraph_foreach_iedge(basev, e) {
		if (unlikely(0 > ylist_push(es, e)))
			goto fail;
	}

	r = 0;
	while (ylist_size(es)) {
		e = ylist_pop(es);
		v = e->vf;

		/* Adjust DFS vertex history.
		 * Top of vertex stack SHOULD be same with target vertex
		 *   of current visiting edge.
		 * That is, vertex stack has right DFS visit-history.
		 */
		while (e->vt != ylist_peek_last(vs)
		       && ylist_size(vs) > 0)
			ylist_pop(vs);

		if (is_discovered__(v)) {
			if (ylist_have(vs, v)) {
				r = 1;
				goto done;
			}
			/* It's already discovered. Skip next steps! */
			continue;
		}

		/* This is first visit */

		/* save to history stack */
		mark_discovered__(v);
		if (unlikely(0 > ylist_push(vs, v)))
			goto fail;

		ygraph_foreach_iedge(v, e) {
			if (unlikely(0 > ylist_push(es, e)))
				goto fail;
		}
	}
	goto done;

 fail:
	r = -1;

 done:
	if (likely(vs))
		ylist_destroy(vs);
	if (likely(es))
		ylist_destroy(es);
	return r;

#undef is_discovered__
#undef mark_undiscovered__
#undef mark_discovered__

}


static void *
job_thread_routine(void *arg) {
	int r __attribute__((unused));
	int sz;
	struct ccjob *j;
	struct ymtconcur *m;
	struct yvertex *v;
	struct yedge *e;
	struct ymtconcurjobarg *jargs = NULL;
	struct job_thread_arg *jta = arg;

	dfpr(">>>>>");
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
		struct ccjob *ij = v2job(e->vf);
		--sz;
		jargs[sz].name = ij->j.name;
		jargs[sz].data = ij->jobout;
	}

	if (unlikely(0 > (*j->j.run_func)(&j->jobout, j->j.arg, jargs)))
		goto err;

	if (unlikely(0 > post_message(m, MCODE_JOB_END, j, NULL)))
		goto err;

	yfree(jargs);
	return NULL;

 err:
	if (jargs)
		yfree(jargs);
	if (unlikely(0 > post_message(m, MCODE_JOB_ERROR, j, NULL)))
		yassert(0);

	return NULL;
}

/*
 * return
 *    <0 : error
 *     Otherwise # of jobs started.
 */
static int
run_ready_jobs(struct ymtconcur *m) {
	static pthread_attr_t __thdattr;
	static pthread_attr_t *__pthdattr;
	int count;

	dfpr(">>>>>");
	if (unlikely(!__pthdattr)) {
		if (unlikely(pthread_attr_init(&__thdattr)))
			return -1;
		/* pthread_join() is NOT used!.
		 * So, thread should be detached by default.
		 */
		if (unlikely(pthread_attr_setdetachstate
			     (&__thdattr, PTHREAD_CREATE_DETACHED))) {
			pthread_attr_destroy(&__thdattr);
			return -1;
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
		struct ccjob *ccj;
		struct ylistl_link *plk = m->readyQ.next;
		/* remove first - dequeue from the readyQ */
		ylistl_del(plk);
		ccj = container_of(plk, struct ccjob, lk);
		yassert(!ccj->wjcnt);
		if (unlikely(!(jta = ymalloc(sizeof(*jta)))))
			return -1;
		jta->m = m;
		jta->j = ccj;
		if (unlikely(pthread_create(&ccj->thread,
					    __pthdattr,
					    &job_thread_routine,
					    jta)))
			return -1;
		dfpr("Thread Created");
		++m->runningjobs;
		++count;
	}
	return count;
}

/******************************************************************************
 *
 * Message Handlers
 *
 *****************************************************************************/
static int
mhndr_start_enq_leaf_cb(struct yvertex *v, void *user) {
	struct ymtconcur *m = user;
	/* Is this leaf vertex? */
	if (unlikely(!ylistl_size(&v->ie))) {
		/* This is leaf vertex. Add to the ready Queue */
		dfpr("*** Leaf : %s", v2job(v)->j.name);
		ylistl_add_last(&m->readyQ, &v2job(v)->lk);
	}
	return 1;
}

static int
mhndr_start(struct ymtconcur *m, struct yvertex *targetv) {
	dfpr(">>>>>");
	if (unlikely(0 > walk_depending_vertices_dfs
			 (m, targetv, (void *)m, &mhndr_start_enq_leaf_cb)))
		return -1;
	if (unlikely(0 > run_ready_jobs(m)))
		return -1;
	dfpr("<<<<<");
	return 0;
}

static int
mhndr_error(struct ymtconcur *m) {
	dfpr(">>>>>");
	m->err = 1;
	return 0;
}

/* put data that are used as arguments for this job */
static void
put_job_arguments(struct ccjob *j) {
	struct yedge *e;
	struct yvertex *v = job2v(j);
	ygraph_foreach_iedge(v, e) {
		ccjob_put_jobout(v2job(e->vf));
	}
}

static int
mhndr_job_end(struct ymtconcur *m,
	      struct ccjob *j) {
	struct yedge *e;
	struct yvertex *v = job2v(j);

	dfpr(">>>>>");
	--m->runningjobs; /* one job is endded */

	ygraph_foreach_oedge(v, e) {
		struct ccjob *oj = v2job(e->vt);
		if (0 > oj->wjcnt)
			/* This is NOT participatns */
			continue;
		yassert(oj->wjcnt > 0);
		/* Job is done.
		 * And, output of this job will be used by next jobs.
		 * So, reference count for out data of this job should be
		 *   increased by number of next jobs waiting this job.
		 */
		ccjob_get_jobout(j);
		--oj->wjcnt;
		if (unlikely(!oj->wjcnt))
			/* add to ready Q */
			ylistl_add_last(&m->readyQ, &oj->lk);
	}
	put_job_arguments(j);
	run_ready_jobs(m);

	return 0;
}

static int
mhndr_job_error(struct ymtconcur *m, struct ccjob *j) {
	dfpr(">>>>>");
	--m->runningjobs; /* one job is endded with error*/
	m->err = 1;
	put_job_arguments(j);
	return 0;
}

/******************************************************************************
 *
 * Interfaces
 *
 *****************************************************************************/
struct ymtconcur *
ymtconcur_create(unsigned int maxjobs) {
	struct ymtconcur *m = NULL;
	if (unlikely(!(m = ymalloc(sizeof(*m)))))
		return NULL;

	if (unlikely(ymtconcur_init(m, maxjobs))) {
		yfree(m);
		return NULL;
	}

	return m;
}

int
ymtconcur_init(struct ymtconcur *m, unsigned int maxjobs) {
	if (unlikely(!m))
		return -1;

	m->targetv = NULL;
	if (unlikely(0 > ygraph_init(&m->g, (void(*)(void *))&ccjob_clean)))
		return -1;

	if (unlikely(!(m->mq = ymq_create())))
		goto init_graph;

	ylistl_init_link(&m->readyQ);
	m->maxjobs = !maxjobs? UINT_MAX: maxjobs;
	m->runningjobs = 0;
	m->err = 0;

	return 0;

 init_graph:
	ygraph_clean(&m->g);
	return -1;
}

void
ymtconcur_clean(struct ymtconcur *m) {
	if (unlikely(!m))
		return;

	ymq_destroy(m->mq);
	ygraph_clean(&m->g);
}

void
ymtconcur_destroy(struct ymtconcur *m) {
	if (unlikely(!m))
		return;

	ymtconcur_clean(m);
	yfree(m);
}

int
ymtconcur_add_dependency(struct ymtconcur *m,
			 struct yvertex	  *target,
			 struct yvertex	  *dependent) {
	if (unlikely(!(m && target && dependent)))
		return -1;
	return ygraph_add_edge(&m->g, dependent, target, 0);
}

int
ymtconcur_remove_dependency(struct ymtconcur *m,
			    struct yvertex   *target,
			    struct yvertex   *dependent) {
	if (unlikely(!(m && target && dependent)))
		return -1;
	return ygraph_remove_edge(&m->g, dependent, target);
}


struct yvertex *
ymtconcur_add_job(struct ymtconcur *m,
		  const struct ymtconcurjob *j) {
	struct yvertex *newv;
	if (unlikely(!(m && j)))
		return NULL;
	if (unlikely(!(newv = create_job_vertex(j))))
		return NULL;
	if (unlikely(ygraph_add_vertex(&m->g, newv))) {
		destroy_job_vertex(newv);
		return NULL;
	}
	return newv;
}

int
ymtconcur_jobnode_set_job(struct yvertex *v,
			  const struct ymtconcurjob *job) {
	if (unlikely(!(v && job)))
		return -1;
	memmove(v2job(v), job, sizeof(*job));
	return 0;
}


static int
ymtconcur_run_cleanup_jobout_cb(struct yvertex *v,
				void *user __attribute__((unused))) {
	struct ccjob *j = v2job(v);
	if (j->jobout && j->j.free_out) {
		(*j->j.free_out)(j->jobout);
		j->jobout = NULL;
	}
	return 1;
}

int
ymtconcur_run(struct ymtconcur *m,
	      void **out,
	      struct yvertex *targetv) {
	struct ymsg *msg;
	void *msgdata;
	int r;

	if (unlikely(!(targetv && m)))
		return -1;

	/* Cyclic loop is NOT allowed */
	if (unlikely(check_cyclic_link(m, targetv)))
		return -1;

	if (unlikely(0 > post_message(m, MCODE_START, NULL, NULL)))
		return -1;

	m->targetv = targetv;

	dfpr("+++ prepare");
	if (unlikely(0 > prepare_concur_run(m, targetv)))
		return -1;

	dfpr("+++ start");
	/* Run message loop */
	while (1) {
		msg = ymq_de(m->mq);
		msgdata = msg->data;
		dfpr("code=%d", msg->u.code);
		switch (msg->u.code) {
		case MCODE_START:
			r = mhndr_start(m, targetv);
			break;
		case MCODE_ERROR:
			r = mhndr_error(m);
			break;
		case MCODE_JOB_END:
			r = mhndr_job_end(m, msgdata);
			break;
		case MCODE_JOB_ERROR:
			r = mhndr_job_error(m, msgdata);
			break;
		default: /* This is NOT expected - unknown code */
			r = -1;
			yassert(0);
		}
		ymsg_destroy(msg);
		if (r < 0) {
			if (unlikely(0 > post_message(m,
						      MCODE_ERROR,
						      NULL,
						      NULL)))
				yassert(0); /* unrecoverable error */
		}
		if (unlikely(!m->runningjobs))
			break; /* exit from loop */

	}

	/* Successful running! */
	if (!m->err) {
		struct ccjob *j = msgdata;
		yassert(MCODE_JOB_END == msg->u.code);
		if (out && *out) {
			*out = j->jobout;
			/* output data is returned to caller.
			 * This SHOULD NOT be freed by internal operation.
			 */
			j->jobout = NULL;
		} else {
			(*j->j.free_out)(j->jobout);
			j->jobout = NULL;
		}
		return 0;
	}

	/* Oops... running encounters error!
	 * Let's do cleanup process
	 */

	/* clean up jobout data */
	walk_depending_vertices_dfs(m,
				    targetv,
				    NULL,
				    &ymtconcur_run_cleanup_jobout_cb);
	return -1;
}
