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

#include "test.h"
#ifdef CONFIG_TEST

#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "ytask.h"
#include "ytaskmanager.h"
#include "ymsglooper.h"
#include "ymsghandler.h"

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

static void
free_arg(void *v) {
	struct targ *ta = (struct targ *)v;
	if (!ta)
		return;
	if (ta->s)
		yfree(ta->s);
	yfree(ta);
}

static int
task_runnable(struct ytask *tsk, void **result) {
	int i;
	struct targ *ta = ytask_get_arg(tsk);
	ylogv("%s(%ld)[%d] run\n",
		ytask_get_name(tsk),
		ytask_get_id(tsk),
		ytask_get_priority(tsk));
	for (i = 0; i < ta->sleep_cnt; i++) {
		usleep(ta->sleep_interval * 1000);
	}
	*result = NULL;
	return 0;
}

/******************************************************************************
 *
 *
 *
 *****************************************************************************/
static void
tm_on_event(struct ytaskmanager_qevent_listener *tel,
	    struct ytaskmanager *tm,
	    enum ytaskmanager_qevent ev,
	    int readyq_sz,
	    int runq_sz,
	    struct ytask *tsk) {
}

struct ytaskmanager_qevent_listener _tmql = {
	.on_event = &tm_on_event
};

/******************************************************************************
 *
 *
 *
 *****************************************************************************/
#define NR_TEST_TASKS 10

static void
tc0(struct ymsghandler *mh0,
    struct ymsghandler *mh1) {
	struct ytaskmanager *tm;
	struct targ *ta;
	struct ytask *tsk;
	int nrtasks;
	struct ytask *ptsks[2 * NR_TEST_TASKS];
	int ptski = 0;

	tm = ytaskmanager_create(mh0, 3);
	yassert(tm);
	nrtasks = NR_TEST_TASKS;

	while (nrtasks--) {
		usleep((rand() % 50) * 100);
		/* Normal successful thread */
		ta = ycalloc(1, sizeof(*ta));
		ta->s = ymalloc(16);
		strcpy(ta->s, "TC0");
		ta->sleep_cnt = rand() % 10;
		ta->sleep_interval = rand() % 10;
		tsk = ytask_create3(
			"TC0-0",
			mh0,
			rand() % YTHREADEX_NUM_PRIORITY,
			NULL,
			ta,
			&free_arg,
			NULL,
			&task_runnable,
			TRUE);
		ytaskmanager_add_task(tm, tsk);
		ptsks[ptski++] = tsk;
	}
	nrtasks = NR_TEST_TASKS;
	while (nrtasks--) {
		usleep((rand() % 50) * 100);
		/* Normal successful thread */
		ta = ycalloc(1, sizeof(*ta));
		ta->s = ymalloc(16);
		strcpy(ta->s, "TC0");
		ta->sleep_cnt = rand() % 10;
		ta->sleep_interval = rand() % 10;
		tsk = ytask_create3(
			"TC0-1",
			mh1,
			rand() % YTHREADEX_NUM_PRIORITY,
			NULL,
			ta,
			&free_arg,
			NULL,
			&task_runnable,
			TRUE);
		ytaskmanager_add_task(tm, tsk);
		ptsks[ptski++] = tsk;
	}

	/* Wait a little time to avoid EBUSY error that rarely happens. */
	usleep(10 * 1000);
	while (ytaskmanager_destroy(tm))
		usleep(10 * 1000);

	while (ptski--)
		yassert(!ytask_destroy(ptsks[ptski]));

}

static void
test_taskmanager(void) {
	struct ymsglooper *ml0, *ml1;
	struct ymsghandler *mh0, *mh1;
	srand(time(NULL));

	ml0 = ymsglooper_start_looper_thread();
	mh0 = ymsghandler_create(ml0, NULL, NULL, NULL); /* use default handle */
	ml1 = ymsglooper_start_looper_thread();
	mh1 = ymsghandler_create(ml1, NULL, NULL, NULL); /* use default handle */

	tc0(mh0, mh1);

	ymsglooper_stop(ml0);
	ymsglooper_stop(ml1);
	while (YMSGLOOPER_TERMINATED != ymsglooper_get_state(ml0)
		|| YMSGLOOPER_TERMINATED != ymsglooper_get_state(ml1)
	) { usleep(10 * 1000); }

	/* clean up all others */
	ymsghandler_destroy(mh0);
	ymsghandler_destroy(mh1);

	ymsglooper_destroy(ml0);
	ymsglooper_destroy(ml1);

}

extern void log_clear(void);
extern void taskmanager_clear(void);
static void
clear_taskmanager(void) {
	taskmanager_clear();
	log_clear();
}


TESTFN(taskmanager)
CLEARFN(taskmanager)

#endif /* CONFIG_TEST */
