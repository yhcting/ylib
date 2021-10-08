/******************************************************************************
 * Copyright (C) 2017, 2021
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

#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>

#include "ylog.h"
#include "common.h"
#include "yerrno.h"
#include "ytask.h"
#include "ymsglooper.h"
#include "ymsghandler.h"
#include "ytaskdepman.h"


static void
on_jobs_done(struct ytaskdepman *tdm,
	     struct ytask *tsk) {
	if (tsk)
		ylogv("Task Done: %s\n", ytask_get_name(tsk));
}

static int
task_run(struct ytask *tsk, void **result) {
	ylogv(">>> Task %s: run\n", ytask_get_name(tsk));
	usleep((rand() % 10) * 10000);
	ylogv("<<< Task %s: run\n", ytask_get_name(tsk));
	return 0;
}

static struct ytask *
mk_task(struct ymsghandler *mh, const char *name) {
	return ytask_create(name, mh, NULL, NULL, NULL, &task_run);
}

static void
tc3(struct ymsghandler *mh) {
	struct ytaskdepman *tdm;
	struct ytask *t0, *t1, *t2, *t3, *t4;
	/* 0<-+--1
	 * ^  |
	 * |  +--2<--3
	 * +---4
	 */
	tdm = ytaskdepman_create(mh, &on_jobs_done, 3);
	t0 = mk_task(mh, "T-00");
	yassert(!ytaskdepman_add_task(tdm, t0));
	t1 = mk_task(mh, "T-01");
	yassert(!ytaskdepman_add_task(tdm, t1));
	t2 = mk_task(mh, "T-02");
	yassert(!ytaskdepman_add_task(tdm, t2));
	t3 = mk_task(mh, "T-03");
	yassert(!ytaskdepman_add_task(tdm, t3));
	t4 = mk_task(mh, "T-04");
	yassert(!ytaskdepman_add_task(tdm, t4));

	yassert(!ytaskdepman_add_dependency(tdm, t0, t1));
	yassert(!ytaskdepman_add_dependency(tdm, t0, t2));
	yassert(!ytaskdepman_add_dependency(tdm, t0, t4));
	yassert(!ytaskdepman_add_dependency(tdm, t2, t3));

	yassert(!ytaskdepman_start(tdm));
	while(ytaskdepman_destroy(tdm));
	usleep(100 * 1000); /* wait enough time */
}

static void
tc2(struct ymsghandler *mh) {
	struct ytaskdepman *tdm;
	struct ytask *t0, *t1;
	tdm = ytaskdepman_create(mh, &on_jobs_done, 3);
	t0 = mk_task(mh, "T-00");
	yassert(!ytaskdepman_add_task(tdm, t0));
	t1 = mk_task(mh, "T-01");
	yassert(!ytaskdepman_add_task(tdm, t1));
	yassert(!ytaskdepman_add_dependency(tdm, t0, t1));
	yassert(!ytaskdepman_remove_dependency(tdm, t0, t1));
	yassert(!ytaskdepman_add_dependency(tdm, t0, t1));
	yassert(!ytaskdepman_start(tdm));
	while(ytaskdepman_destroy(tdm))
		usleep(10 * 1000);
	usleep(100 * 1000); /* wait enough time */
}

static void
tc1(struct ymsghandler *mh) {
	struct ytaskdepman *tdm;
	struct ytask *t;
	tdm = ytaskdepman_create(mh, &on_jobs_done, 3);
	t = mk_task(mh, "T-00");
	yassert(!ytaskdepman_add_task(tdm, t));
	yassert(!ytaskdepman_start(tdm));
	while(ytaskdepman_destroy(tdm))
		usleep(10 * 1000);
}

static void
tc0(struct ymsghandler *mh) {
	struct ytaskdepman *tdm;
	struct ytask *t;
	tdm = ytaskdepman_create(mh, &on_jobs_done, 3);
	yassert(tdm);
	ytaskdepman_cancel(tdm);
	while(ytaskdepman_destroy(tdm));

	/* simple verification */
	tdm = ytaskdepman_create(mh, &on_jobs_done, 3);
	yassert(YTASKDEPMAN_EMPTY == ytaskdepman_verify(tdm, NULL));
	ytaskdepman_cancel(tdm);
	while(ytaskdepman_destroy(tdm));

	tdm = ytaskdepman_create(mh, &on_jobs_done, 3);
	t = mk_task(mh, "T-00");
	yassert(!ytaskdepman_add_task(tdm, t));
	yassert(!ytaskdepman_remove_task(tdm, t));
	ytask_cancel(t);
	yassert(YTASKDEPMAN_EMPTY == ytaskdepman_verify(tdm, NULL));
	ytaskdepman_cancel(tdm);
	while(ytaskdepman_destroy(tdm));
	while(ytask_destroy(t));

	tdm = ytaskdepman_create(mh, &on_jobs_done, 3);
	t = mk_task(mh, "T-00");
	yassert(!ytaskdepman_add_task(tdm, t));
	ytaskdepman_cancel(tdm);
	while(ytaskdepman_destroy(tdm));
}


static void
test_taskdepman(void) {
	struct ymsglooper *ml;
	struct ymsghandler *mh;
	srand(time(NULL));

	ml = ymsglooper_start_looper_thread();
	mh = ymsghandler_create(ml, NULL, NULL, NULL); /* use default handle */

	ylogv("----- TC0 --------\n");
	tc0(mh);
	ylogv("----- TC1 --------\n");
	tc1(mh);
	ylogv("----- TC2 --------\n");
	tc2(mh);
	ylogv("----- TC3 --------\n");
	tc3(mh);

	ymsglooper_stop(ml);
	while (YMSGLOOPER_TERMINATED != ymsglooper_get_state(ml))
		usleep(10 * 1000);
	/* clean up all others */
	ymsghandler_destroy(mh);
	ymsglooper_destroy(ml);
}


extern void taskdepman_clear(void);
extern void msglooper_clear(void);
extern void msghandler_clear(void);
extern void log_clear(void);
static void
clear_taskdepman(void) {
	log_clear();
	msglooper_clear();
	msghandler_clear();
	taskdepman_clear();
}


TESTFN(taskdepman)
CLEARFN(taskdepman)

#endif /* CONFIG_DEBUG */
