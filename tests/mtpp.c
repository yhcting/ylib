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
#include "test.h"
#ifdef CONFIG_DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "common.h"
#include "ymtpp.h"

struct workarg {
	int id;
	char *str;
};

struct workout {
	useconds_t usec;
	char *str;
};

static void
free_workarg(struct workarg *a) {
	yfree(a->str);
	yfree(a);
}

static void
free_workout(struct workout *a) {
	if (unlikely(!a))
		return;
	yfree(a->str);
	yfree(a);
}

static useconds_t
get_rand_usleep(const struct workarg *wa __unused) {
	/* min : 10 ms + random 0 ~ 200 ms */
	return 10000 + (((s64)rand() * 200L) / RAND_MAX) * 1000;
}

static int
worker_func(void **out,
	    const void *arg,
	    const struct ymtppjobarg *jargs) {
	useconds_t usec;
	struct workout *wout;
	const struct workout *jad __unused;
	const struct workarg *wa = arg;
	dpr(">>>>> %s (%d)\n", wa->str, wa->id);
	while (jargs->name && jargs->data) {
		jad = (const struct workout *)jargs->data;
		dpr("	 %s : %s\n", wa->str, jad->str);
		++jargs;
	}
	usec = get_rand_usleep(wa);
	wout = ymalloc(sizeof(*out));
	wout->usec = usec;
	wout->str = ymalloc(512);
	sprintf(wout->str, "%s : %d : %u", wa->str, wa->id, usec);
	*out = wout;
	usleep(usec);
	dpr("<<<<< %s (%d)\n", wa->str, wa->id);
	return 0;
}

static int
gen_error_func(void **out,
	       const void *arg,
	       const struct ymtppjobarg *jargs) {
	return -1;
}

static void
init_job(struct ymtppjob *ccj,
	 int id,
	 const char *name) {
	struct workarg *wa = ymalloc(sizeof(*wa));
	yassert(wa);
	wa->id = id;
	wa->str = ymalloc(strlen(name) + 1);
	yassert(wa->str);
	strcpy(wa->str, name);
	ccj->arg = wa;
	ccj->afree = (void (*)(void *))&free_workarg;
	ccj->ofree = (void (*)(void *))&free_workout;
	ccj->run = &worker_func;
	strcpy(ccj->name, name);
}

/* JOB network
 *		+-> D -----------+
 *		|		 |
 *		+-> F ---+----+	 |   +-> B
 *		|	 |    |	 |   |	 |
 *		|	 v    v	 v   |	 v
 * K -> J -> I -+-> G -> E -> [ A ] -+-> C
 *		|   ^		^
 *		|   |		|
 *		+-> H		L
 */
static void
test_mtpp(void) {
	int r, i;
	struct ymtpp *m;
	struct workout *workout;
	ymtpp_task_t nA, nB, nC, nD, nE, nF, nG, nH, nI, nJ, nK, nL;

	m = ymtpp_create(0);
	yassert(m);
	srand(time(NULL));

	i = 0;

#define add_task(CHAR)							\
	do {								\
		n##CHAR = ymtpp_create_task(m);				\
		init_job(ymtpp_task_job(n##CHAR), i++, #CHAR);		\
		r = ymtpp_add_task(m, n##CHAR);				\
		yassert(!r);						\
	} while (0)

	add_task(A);
	add_task(B);
	add_task(C);
	add_task(D);
	add_task(E);
	add_task(F);
	add_task(G);
	add_task(H);
	add_task(I);
	add_task(J);
	add_task(K);
	add_task(L);

#undef add_task

	ymtpp_add_dependency(m, nA, nD);
	ymtpp_add_dependency(m, nA, nE);
	ymtpp_add_dependency(m, nA, nF);
	ymtpp_add_dependency(m, nA, nL);

	ymtpp_add_dependency(m, nB, nA);

	ymtpp_add_dependency(m, nC, nB);
	ymtpp_add_dependency(m, nC, nA);

	ymtpp_add_dependency(m, nD, nI);

	ymtpp_add_dependency(m, nE, nF);
	ymtpp_add_dependency(m, nE, nG);

	ymtpp_add_dependency(m, nF, nI);

	ymtpp_add_dependency(m, nG, nI);
	ymtpp_add_dependency(m, nG, nH);

	ymtpp_add_dependency(m, nH, nI);

	ymtpp_add_dependency(m, nI, nJ);

	ymtpp_add_dependency(m, nJ, nK);

	workout = NULL;

	/* make simple cyclic link */
	ymtpp_add_dependency(m, nK, nJ);
	yassert(0 > ymtpp_run(m, (void **)&workout, nK));
	yassert(!workout);
	ymtpp_remove_dependency(m, nK, nJ);

	/* make simple cyclic link */
	ymtpp_add_dependency(m, nK, nI);
	yassert(0 > ymtpp_run(m, (void **)&workout, nK));
	yassert(!workout);
	ymtpp_remove_dependency(m, nK, nI);

	/* make cyclic link */
	ymtpp_add_dependency(m, nH, nA);
	yassert(0 > ymtpp_run(m, (void **)&workout, nA));
	yassert(!workout);
	ymtpp_remove_dependency(m, nH, nA);

	/* no cyclic link from now on */
	workout = NULL;
	yassert(!ymtpp_run(m, (void **)&workout, nA));
	yassert(workout);
	free_workout(workout);

	/* set ccjob to test error case */
	ymtpp_task_job(nE)->run = &gen_error_func;
	workout = NULL;
	yassert(0 > ymtpp_run(m, (void **)&workout, nA));
	yassert(!workout);

	/* back to original test function */
	ymtpp_task_job(nE)->run = &worker_func;
	workout = NULL;
	yassert(!ymtpp_run(m, (void **)&workout, nA));
	yassert(workout);
	free_workout(workout);

	workout = NULL;
	yassert(!ymtpp_run(m, (void **)&workout, nA));
	yassert(workout);
	free_workout(workout);

	workout = NULL;
	yassert(!ymtpp_run(m, (void **)&workout, nC));
	yassert(workout);
	free_workout(workout);

	ymtpp_destroy(m);

}


TESTFN(test_mtpp, mtpp)

#endif /* CONFIG_DEBUG */
