/******************************************************************************
 * Copyright (C) 2016, 2021, 2023
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>


#include "ylog.h"
#include "yerrno.h"
#include "ymsghandler.h"
#include "ymsglooper.h"
#include "ythreadex.h"


struct targ {
	int id;
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


static void
on_started(struct ythreadex *threadex) {
	struct targ *ta = ythreadex_get_arg(threadex);
	ta->st[ta->sti++] = YTHREADEX_STARTED;
	ylogv("%ld: %d: started\n", ythreadex_get_id(threadex), ta->id);
}

static void
on_done(struct ythreadex *threadex, void *result, int errcode) {
	struct targ *ta = ythreadex_get_arg(threadex);
	ta->st[ta->sti++] = YTHREADEX_DONE;
	ylogv("%ld: %d: done\n", ythreadex_get_id(threadex), ta->id);
}

static void
on_cancelling(struct ythreadex *threadex, bool started) {
	struct targ *ta = ythreadex_get_arg(threadex);
	ta->st[ta->sti++] = YTHREADEX_CANCELLING;
	ylogv("%ld: %d: canceling\n", ythreadex_get_id(threadex), ta->id);
	ta->cancel = TRUE;
}

static void
on_cancelled(struct ythreadex *threadex, int errcode) {
	struct targ *ta = ythreadex_get_arg(threadex);
	ta->st[ta->sti++] = YTHREADEX_CANCELLED;
	ylogv("%ld: %d: cancelled\n", ythreadex_get_id(threadex), ta->id);
}

static void
on_progress_init(struct ythreadex *threadex, long max_prog) {
	struct targ *ta = ythreadex_get_arg(threadex);
	ta->progcnt += 1000;
	ylogv("%ld: %d: prog init\n", ythreadex_get_id(threadex), ta->id);
}

static void
on_progress(struct ythreadex *threadex, long prog) {
	struct targ *ta = ythreadex_get_arg(threadex);
	ta->progcnt++;
	ylogv("%ld: %d: prog\n", ythreadex_get_id(threadex), ta->id);
}

static int
thread_run(struct ythreadex *threadex, void **result) {
	int i;
	struct tres *tr;
	struct targ *ta = ythreadex_get_arg(threadex);
	ylogv("%d: >>> thread run\n", ta->id);
	*result = ymalloc(sizeof(*tr));
	tr = (struct tres *)*result;
	memset(tr, 0, sizeof(*tr));
	ythreadex_publish_progress_init(threadex, 100);
	for (i = 0; i < ta->sleep_cnt; i++) {
		if (YTHREADEX_CANCELLING == ythreadex_get_state(threadex))
			break;
		usleep(ta->sleep_interval * 1000);
	}
	if (ta->retval)
		return ta->retval;
	memset(tr, 0, sizeof(*tr));
	tr->s = ymalloc(32);
	sprintf(tr->s, "res:%d", tr->r);
	*result = tr;
	ylogv("%d: <<< thread done\n", ta->id);
	return 0;
}

static struct ythreadex_listener _lis = {
	.on_started = &on_started,
	.on_done = &on_done,
	.on_cancelling = &on_cancelling,
	.on_cancelled = &on_cancelled,
	.on_progress_init = &on_progress_init,
	.on_progress = &on_progress
};

unused static void
tc1(struct ymsghandler *mh) {
	struct ythreadex *yt;
	struct targ *ta;
	struct tres *tr;
	int retry;
	/* Normal successful thread */
	ta = ymalloc(sizeof(*ta));
	memset(ta, 0, sizeof(*ta));
	ta->s = ymalloc(16);
	strcpy(ta->s, "ok:arg");
	yt = ythreadex_create(
		"ok",
		mh,
		YTHREADEX_NORMAL,
		&_lis,
		ta,
		&free_arg,
		&free_result,
		&thread_run);
	yassert(ythreadex_destroy(yt));
	yassert(!ythreadex_start_sync(yt));

	/* Even if thread is ended, event handler may not be called yet.
	 * So, wait until event handler is called.
	 */
	retry = 1000;
	while (!ythreadex_is_terminated(ta->st[1])
		&& retry-- > 0
	) { usleep(1000); }

	tr = ythreadex_get_result(yt);
	/* STARTED, DONE. And PROGRESS_INIT */
	ylogv("%d, %d, %d, %d, %d, %c\n",
		ta->sti,
		ta->st[0],
		ta->st[1],
		ta->progcnt,
		tr->r,
		tr->s[0]);
	yassert(2 == ta->sti
		&& YTHREADEX_STARTED == ta->st[0]
		&& YTHREADEX_DONE == ta->st[1]
		&& 1000 == ta->progcnt
		&& 0 == tr->r
		&& 'r' == tr->s[0]);
	yassert(!ythreadex_destroy(yt));
}

unused static void
tc2(struct ymsghandler *mh) {
	struct ythreadex *yt;
	struct targ *ta;
	struct tres *tr;
	unused void *retval;
	/* Normal successful thread */
	ta = ymalloc(sizeof(*ta));
	memset(ta, 0, sizeof(*ta));
	ta->s = ymalloc(16);
	strcpy(ta->s, "ok:arg");
	yt = ythreadex_create(
		"ok",
		mh,
		YTHREADEX_NORMAL,
		&_lis,
		ta,
		&free_arg,
		&free_result,
		&thread_run);
	yassert(ythreadex_destroy(yt));
	yassert(!ythreadex_start(yt));
	yassert(!ythreadex_join(yt, &retval));

	/* Even if thread is ended, event handler may not be called yet.
	 * So, wait until event handler is called.
	 */
	int retry = 100;
	while (!ythreadex_is_terminated(ythreadex_get_state(yt))
		&& retry-- > 0
	) { usleep(1000); }

	retry = 1000;
	while (!ythreadex_is_terminated(ta->st[1])
		&& retry-- > 0
	) { usleep(1000); }

	tr = ythreadex_get_result(yt);
	/* STARTED, DONE. And PROGRESS_INIT */
	ylogv("%d, %s, %s, %d, %d, %c\n",
		ta->sti,
		ythreadex_get_state_str(ta->st[0]),
		ythreadex_get_state_str(ta->st[1]),
		ta->progcnt,
		tr->r,
		tr->s[0]);
	yassert(2 == ta->sti
		&& YTHREADEX_STARTED == ta->st[0]
		&& YTHREADEX_DONE == ta->st[1]
		&& 1000 == ta->progcnt
		&& 0 == tr->r
		&& 'r' == tr->s[0]);
	yassert(!ythreadex_destroy(yt));
}

unused static void
tc3(struct ymsghandler *mh) {
	struct ythreadex *yt;
	struct targ *ta;
	void *retval;
	int retry;
	enum ythreadex_state st;
	/* Normal successful thread */
	ta = ymalloc(sizeof(*ta));
	memset(ta, 0, sizeof(*ta));
	ta->s = ymalloc(16);
	ta->sleep_cnt = 10 + rand() % 20;
	ta->sleep_interval = 10;
	strcpy(ta->s, "ok:arg");
	yt = ythreadex_create(
		"ok",
		mh,
		YTHREADEX_NORMAL,
		&_lis,
		ta,
		&free_arg,
		&free_result,
		&thread_run);
	yassert(!ythreadex_start(yt));
	yassert(!ythreadex_cancel(yt, FALSE));
	ythreadex_join(yt, &retval);
	st = ythreadex_get_state(yt);
	ylogv("tc3: thread stat: %d\n", st);
	yassert(YTHREADEX_TERMINATED_CANCELLED == st
		|| YTHREADEX_CANCELLED == st);
	retry = 100;
	while (!ythreadex_is_terminated(ythreadex_get_state(yt))
		&& retry-- > 0
	) { usleep(1000); }
	yassert(!ythreadex_destroy(yt));

}

unused static void
tc4(struct ymsghandler *mh) {
#define NR_THREADS 25
	int i;
	struct ythreadex *yt[NR_THREADS];
	struct targ *ta;
	struct tres *tr;
	void *retval[NR_THREADS];
	for (i = 0; i < NR_THREADS; i++) {
		/* Normal successful thread */
		ta = ymalloc(sizeof(*ta));
		memset(ta, 0, sizeof(*ta));
		/* sleep enough to get 'cancel' message after start */
		ta->id = i;
		ta->sleep_cnt = 100 + rand() % 20;
		ta->sleep_interval = 10;
		ta->s = ymalloc(16);
		strcpy(ta->s, "ok:arg");
		yt[i] = ythreadex_create(
			"ok",
			mh,
			YTHREADEX_NORMAL,
			&_lis,
			ta,
			&free_arg,
			&free_result,
			&thread_run);
	}

	for (i = 0; i < NR_THREADS; i++)
		yassert(!ythreadex_start(yt[i]));

	for (i = 0; i < NR_THREADS; i++) {
		if (i % 3)
			ythreadex_cancel(yt[i], i % 2);
	}
	for (i = 0; i < NR_THREADS; i++)
		ythreadex_join(yt[i], &retval[i]);

	for (i = 0; i < 30; i++)
		usleep(1000 * 100);

	for (i = 0; i < NR_THREADS; i++) {
		enum ythreadex_state st = ythreadex_get_state(yt[i]);
		if (i % 3) {
			/*
			 * This is thread of cancelled.
			 * But, following cases are still possible.
			 * - DONE: It is already finished before cancel is
			 *       requested, and there is no time to move to
			 *       TERMINATED.
			 * - TERMINATED: See above.
			 * However, it shoud be rare because thread_run takes
			 * at least several seconds!
			 */
			switch (st) {
			case YTHREADEX_DONE:
			case YTHREADEX_TERMINATED:
			case YTHREADEX_CANCELLED:
				ylogw("(%ld: %d: %d) 00: Possible but rarely happen!\n",
					ythreadex_get_id(yt[i]), i, st);
				usleep(1000 * 500); i--; /* retry */
				continue;

			break;
			default:
				if (YTHREADEX_TERMINATED_CANCELLED != st) {
					ylogw("(%d: %d: %d) 01: Possible but rarely happen!\n",
						ythreadex_get_id(yt[i]), i, st);
					usleep(1000 * 500); i--; /* retry */
					continue;
					/* yassert(YTHREADEX_TERMINATED_CANCELLED == st); */
				}
			}
		} else {
			/*
			 * DONE is possible but it should very rarely happen!
			 */
			if (YTHREADEX_DONE == st) {
				ylogw("(%ld: %d: %d) 10: Possible but rarely happen!\n",
					ythreadex_get_id(yt[i]), i, st);
				usleep(1000 * 500); i--; /* retry */
				continue;
			} else {
				if (YTHREADEX_TERMINATED != st) {
					ylogw("(%ld: %d: %d) 11: Possible but rarely happen!\n",
						ythreadex_get_id(yt[i]), i, st);
					usleep(1000 * 500); i--; /* retry */
					continue;
					/* yassert(YTHREADEX_TERMINATED == st); */
				}

				tr = ythreadex_get_result(yt[i]);
				ta = ythreadex_get_arg(yt[i]);
				/* STARTED, DONE. And PROGRESS_INIT */
				ylogv("%d, %d, %d, %d, %d, %c\n",
					ta->sti,
					ta->st[0],
					ta->st[1],
					ta->progcnt,
					tr->r,
					tr->s[0]);
				yassert(2 == ta->sti
					&& YTHREADEX_STARTED == ta->st[0]
					&& YTHREADEX_DONE == ta->st[1]
					&& 1000 == ta->progcnt
					&& 0 == tr->r
					&& 'r' == tr->s[0]);
			}
		}
		yassert(!ythreadex_destroy(yt[i]));
	}
#undef NR_THREADS
}


static void
test_threadex(void) {
	struct ymsglooper *ml;
	struct ymsghandler *mh;
	srand(time(NULL));

	ml = ymsglooper_start_looper_thread();
	mh = ymsghandler_create(ml, NULL, NULL, NULL); /* use default handle */

	tc1(mh);
	tc2(mh);
	tc3(mh);
	ylogv("\n\n\n======================================================\n\n\n");
	tc4(mh);

	ymsglooper_stop(ml);
	while (YMSGLOOPER_TERMINATED != ymsglooper_get_state(ml))
		usleep(10*1000);
	/* clean up all others */
	ymsghandler_destroy(mh);
	ymsglooper_destroy(ml);
}

extern void threadex_clear(void);
extern void log_clear(void);
static void
clear_threadex(void) {
	log_clear();
	threadex_clear();
}

TESTFN(threadex)
CLEARFN(threadex)

#endif /* CONFIG_TEST */
