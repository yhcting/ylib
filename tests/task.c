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

#include "test.h"
#ifdef CONFIG_DEBUG

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#include "ylog.h"
#include "common.h"
#include "yerrno.h"
#include "ymsghandler.h"
#include "ymsglooper.h"
#include "ytask.h"


struct targ {
	int a;
	int sti; /* next empty slot of st */
	int st[8]; /* enough space to save each state. */
	int progcnt;
	int retval;
	int sleep_cnt;
	int sleep_interval; /* ms */
	int cancel;
	char *s;
};

struct tres {
	int r;
	char *s;
};

static void
free_arg(void *v) {
	struct targ *ta = (struct targ *)v;
	if (!ta)
		return;
	if (ta->s)
		yfree(ta->s);
	yfree(ta);
}

static void
free_result(void *v) {
	struct tres *tr = (struct tres *)v;
	if (!tr)
		return;
	if (tr->s)
		yfree(tr->s);
	yfree(tr);
}

/******************************************************************************
 *
 *
 *
 *****************************************************************************/

static void
on_early_started(struct ytask *tsk) {
	ylogv("[%p]early_started\n", pthread_self());
}

static void
on_late_started(struct ytask *tsk) {
	ylogv("[%p]late_started\n", pthread_self());
}

static void
on_early_done(struct ytask *tsk, void *result, int errcode) {
	ylogv("[%p]early_done\n", pthread_self());
}

static void
on_late_done(struct ytask *tsk, void *result, int errcode) {
	ylogv("[%p]late_done\n", pthread_self());
}

static void
on_early_cancelling(struct ytask *tsk, bool started) {
	ylogv("[%p]early_cancelling: %d\n", pthread_self(), started);
}

static void
on_late_cancelling(struct ytask *tsk, bool started) {
	ylogv("[%p]late_cancelling: %d\n", pthread_self(), started);
}

static void
on_early_cancelled(struct ytask *tsk, int errcode) {
	ylogv("[%p]early_cancelled\n", pthread_self());
}

/*
static void
on_late_cancelled(struct ytask *tsk, int errcode) {
}
*/

static void
on_early_progress_init(struct ytask *tsk, long max_prog) {
	ylogv("[%p]early_progress_init: %ld\n", pthread_self(), max_prog);
}

static void
on_late_progress_init(struct ytask *tsk, long max_prog) {
	ylogv("[%p]late_progress_init: %ld\n", pthread_self(), max_prog);
}

static void
on_early_progress(struct ytask *tsk, long prog) {
	ylogv("[%p]early_progress: %ld\n", pthread_self(), prog);
}

static void
on_late_progress(struct ytask *tsk, long prog) {
	ylogv("[%p]late_progress: %ld\n", pthread_self(), prog);
}

/******************************************************************************
 *
 *
 *
 *****************************************************************************/

static void
on_started(struct ytask *tsk) {
	ylogv("[%p]started\n", pthread_self());
}

static void
on_done(struct ytask *tsk, void *result, int errcode) {
	ylogv("[%p]done\n", pthread_self());
}

static void
on_cancelling(struct ytask *tsk, bool started) {
	ylogv("[%p]cancelling: %d\n", pthread_self(), started);
}

static void
on_cancelled(struct ytask *tsk, int errcode) {
	ylogv("[%p]cancelled\n", pthread_self());
}

static void
on_progress_init(struct ytask *tsk, long max_prog) {
	ylogv("[%p]progress_init: %ld\n", pthread_self(), max_prog);
}

static void
on_progress(struct ytask *tsk, long prog) {
	ylogv("[%p]progress: %ld\n", pthread_self(), prog);
}

/******************************************************************************
 *
 *
 *
 *****************************************************************************/

static struct ytask_listener _tlis = {
	.on_early_started = &on_early_started,
	.on_late_started = &on_late_started,
	.on_early_done = &on_early_done,
	.on_late_done = &on_late_done,
	.on_early_cancelling = &on_early_cancelling,
	.on_late_cancelling = &on_late_cancelling,
	.on_early_cancelled = &on_early_cancelled,
	.on_late_cancelled = NULL,
	.on_early_progress_init = &on_early_progress_init,
	.on_late_progress_init = &on_late_progress_init,
	.on_early_progress = &on_early_progress,
	.on_late_progress = &on_late_progress,
};

static struct ytask_event_listener _telis = {
	.on_started = &on_started,
	.on_done = &on_done,
	.on_cancelling = &on_cancelling,
	.on_cancelled = &on_cancelled,
	.on_progress_init = &on_progress_init,
	.on_progress = &on_progress,
};

static int
runnable0(struct ytask *tsk, void **result) {
	int i;
	struct tres *tr;
	struct targ *ta = ytask_get_arg(tsk);
	ylogv("task run\n");
	*result = ymalloc(sizeof(*tr));
	tr = (struct tres *)*result;
	memset(tr, 0, sizeof(*tr));
	ytask_publish_progress_init(tsk, 100);
	for (i = 0; i < ta->sleep_cnt; i++) {
		ytask_publish_progress(tsk, 10 * i);
		usleep(ta->sleep_interval * 1000);
	}
	if (ta->retval)
		return ta->retval;
	memset(tr, 0, sizeof(*tr));
	tr->s = ymalloc(32);
	sprintf(tr->s, "res:%d", tr->r);
	*result = tr;
	return 0;
}

/******************************************************************************
 *
 *
 *
 *****************************************************************************/
/*
 * Simple normal case.
 */
static void
tc1(struct ymsghandler *mh0,
    struct ymsghandler *mh1 __unused) {
	struct ytask *t;
	struct targ *ta;
	struct tres *tr;
	/* Normal successful thread */
	ta = ycalloc(1, sizeof(*ta));
	ta->s = ymalloc(16);
	strcpy(ta->s, "T0:arg");
	t = ytask_create("T0",
			 mh0,
			 YTHREADEX_NORMAL,
			 &_tlis,
			 ta,
			 &free_arg,
			 &free_result,
			 &runnable0,
			 TRUE);
	yassert(ytask_destroy(t));
	ytask_add_event_listener(t, mh0, &_telis, TRUE);
	yassert(!ytask_start_sync(t));
	usleep(300 * 1000); /* wait 300 ms until callbacks are handled */
	tr = ytask_get_result(t);
	/* STARTED, DONE. And PROGRESS_INIT */
	ylogv("%d, %d, %d, %d, %d, %c\n",
	      ta->sti,
	      ta->st[0],
	      ta->st[1],
	      ta->progcnt,
	      tr->r,
	      tr->s[0]);
	yassert(!ytask_destroy(t));

}

/*
 * ytask_destroy is called before all callback is processed.
 */
static void
tc2(struct ymsghandler *mh0,
    struct ymsghandler *mh1 __unused) {
	struct ytask *t;
	struct targ *ta;
	struct tres *tr __unused;
	/* Normal successful thread */
	ta = ycalloc(1, sizeof(*ta));
	ta->s = ymalloc(16);
	ta->sleep_cnt = 2;
	ta->sleep_interval = 200; /* 200 ms */
	strcpy(ta->s, "T1:arg");
	t = ytask_create("T0",
			 mh0,
			 YTHREADEX_NORMAL,
			 &_tlis,
			 ta,
			 &free_arg,
			 &free_result,
			 &runnable0,
			 TRUE);
	yassert(ytask_destroy(t));
	ytask_add_event_listener(t, mh0, &_telis, TRUE);
	yassert(!ytask_start_sync(t));
	tr = ytask_get_result(t);
	while (ytask_destroy(t)); /* try destroy until success */
	usleep(200 * 1000); /* wailt enough */
}


/*
 * ytask_destroy is called before all callback is processed.
 */
static void
tc3(struct ymsghandler *mh0,
    struct ymsghandler *mh1) {
	struct ytask *t;
	struct targ *ta;
	struct tres *tr __unused;
	ylogv("***************** tc3 S *********************\n");
	/* Normal successful thread */
	ta = ycalloc(1, sizeof(*ta));
	ta->s = ymalloc(16);
	ta->sleep_cnt = 2;
	ta->sleep_interval = 200; /* 200 ms */
	strcpy(ta->s, "T1:arg");
	t = ytask_create("T0",
			 mh0,
			 YTHREADEX_NORMAL,
			 &_tlis,
			 ta,
			 &free_arg,
			 &free_result,
			 &runnable0,
			 TRUE);
	yassert(ytask_destroy(t));
	ytask_add_event_listener(t, mh1, &_telis, TRUE);
	yassert(!ytask_start(t));
	usleep(10 * 1000);
	ytask_add_event_listener(t, mh1, &_telis, TRUE);
	while (!ytask_is_terminated(t))
		usleep(10 * 1000);
	tr = ytask_get_result(t);
	yassert(!ytask_destroy(t));
	ylogv("***************** tc3 E *********************\n");
}


static void
test_task(void) {
	struct ymsglooper *ml0, *ml1;
	struct ymsghandler *mh0, *mh1;
	srand(time(NULL));

	ml0 = ymsglooper_start_looper_thread(FALSE);
	mh0 = ymsghandler_create(ml0, NULL); /* use default handle */
	ml1 = ymsglooper_start_looper_thread(FALSE);
	mh1 = ymsghandler_create(ml1, NULL); /* use default handle */

	tc1(mh0, mh1);
	tc2(mh0, mh1);
	tc3(mh0, mh1);

	/* clean up all others */
	ymsghandler_destroy(mh0);
	ymsghandler_destroy(mh1);
	ymsglooper_stop(ml0);
	ymsglooper_stop(ml1);
	while (YMSGLOOPER_TERMINATED != ymsglooper_get_state(ml0)
	       || YMSGLOOPER_TERMINATED != ymsglooper_get_state(ml1))
		usleep(10*1000);
	ymsglooper_destroy(ml0);
	ymsglooper_destroy(ml1);
	return;
}

extern void task_clear(void);
extern void log_clear(void);
static void
clear_task(void) {
	task_clear();
	log_clear();
}


TESTFN(task)
CLEARFN(task)

#endif /* CONFIG_DEBUG */
