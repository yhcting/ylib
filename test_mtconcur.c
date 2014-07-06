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
#define LIB_DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "ycommon.h"
#include "ymtconcur.h"
#include "test.h"
#include "libdbg.h"

struct workarg {
	int    id;
	char  *str;
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
get_rand_usleep(const struct workarg *wa __attribute__((unused))) {
	/* min : 100 ms + random 0 ~ 1000 ms */
	return 100000 + (((long)rand() * 1000L) / RAND_MAX) * 1000;
}

static int
worker_func(void **out,
	    const void *arg,
	    const struct ymtconcurjobarg *jargs) {
	useconds_t usec;
	struct workout *wout;
	const struct workout *jad __attribute__((unused));
	const struct workarg *wa = arg;
	dpr(">>>>> %s (%d)\n", wa->str, wa->id);
	while (!ymtconcur_is_empty_jobarg(jargs)) {
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
	       const struct ymtconcurjobarg *jargs) {
	return -1;
}

static void
init_testjob(struct ymtconcurjob *ccj) {
	ccj->free_arg = (void (*)(void *))&free_workarg;
	ccj->free_out = (void (*)(void *))&free_workout;
	ccj->run_func = &worker_func;
}

static void
alloc_job(struct ymtconcurjob *ccj,
	  int id,
	  const char *name) {
	struct workarg *wa = ymalloc(sizeof(*wa));
	wa->id = id;
	wa->str = ymalloc(strlen(name) + 1);
	strcpy(wa->str, name);
	ccj->arg = wa;
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
test_mtconcur(void) {
	struct ymtconcur *m;
	struct workout *workout;
	struct ymtconcurjob ccj;
	ymtconcurjob_node_t nA, nB, nC, nD, nE, nF, nG, nH, nI, nJ, nK, nL;

	m = ymtconcur_create(0);

	init_testjob(&ccj);
	srand(time(NULL));

	alloc_job(&ccj, 1, "A");
	nA = ymtconcur_add_job(m, &ccj);
	alloc_job(&ccj, 2, "B");
	nB = ymtconcur_add_job(m, &ccj);
	alloc_job(&ccj, 3, "C");
	nC = ymtconcur_add_job(m, &ccj);
	alloc_job(&ccj, 4, "D");
	nD = ymtconcur_add_job(m, &ccj);
	alloc_job(&ccj, 5, "E");
	nE = ymtconcur_add_job(m, &ccj);
	alloc_job(&ccj, 6, "F");
	nF = ymtconcur_add_job(m, &ccj);
	alloc_job(&ccj, 7, "G");
	nG = ymtconcur_add_job(m, &ccj);
	alloc_job(&ccj, 8, "H");
	nH = ymtconcur_add_job(m, &ccj);
	alloc_job(&ccj, 9, "I");
	nI = ymtconcur_add_job(m, &ccj);
	alloc_job(&ccj, 10, "J");
	nJ = ymtconcur_add_job(m, &ccj);
	alloc_job(&ccj, 11, "K");
	nK = ymtconcur_add_job(m, &ccj);
	alloc_job(&ccj, 12, "L");
	nL = ymtconcur_add_job(m, &ccj);

	ymtconcur_add_dependency(m, nA, nD);
	ymtconcur_add_dependency(m, nA, nE);
	ymtconcur_add_dependency(m, nA, nF);
	ymtconcur_add_dependency(m, nA, nL);

	ymtconcur_add_dependency(m, nB, nA);

	ymtconcur_add_dependency(m, nC, nB);
	ymtconcur_add_dependency(m, nC, nA);

	ymtconcur_add_dependency(m, nD, nI);

	ymtconcur_add_dependency(m, nE, nF);
	ymtconcur_add_dependency(m, nE, nG);

	ymtconcur_add_dependency(m, nF, nI);

	ymtconcur_add_dependency(m, nG, nI);
	ymtconcur_add_dependency(m, nG, nH);

	ymtconcur_add_dependency(m, nH, nI);

	ymtconcur_add_dependency(m, nI, nJ);

	ymtconcur_add_dependency(m, nJ, nK);

	workout = NULL;
	/* make cyclic link */
	ymtconcur_add_dependency(m, nH, nA);
	yassert(-1 == ymtconcur_run(m, (void **)&workout, nA));

	ymtconcur_remove_dependency(m, nH, nA);

	workout = NULL;
	ymtconcur_run(m, (void **)&workout, nA);
	free_workout(workout);

	/* set ccjob to test error case */
	ccj.run_func = &gen_error_func;
	ymtconcur_jobnode_set_job(nE, &ccj);

	workout = NULL;
	ymtconcur_run(m, (void **)&workout, nA);

	/* back to original test function */
	init_testjob(&ccj);
	ymtconcur_jobnode_set_job(nE, &ccj);
	workout = NULL;
	ymtconcur_run(m, (void **)&workout, nA);
	free_workout(workout);
	workout = NULL;
	ymtconcur_run(m, (void **)&workout, nA);
	free_workout(workout);

	workout = NULL;
	ymtconcur_run(m, (void **)&workout, nC);
	free_workout(workout);

	ymtconcur_destroy(m);

}


TESTFN(test_mtconcur, mtconcur)
