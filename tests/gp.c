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

#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "ylog.h"
#include "common.h"
#include "yerrno.h"
#include "ygp.h"

struct myvalue {
	struct ygp gp;
	int a;
};


static void *
gpaction(void *arg) {
	struct ygp *gp = arg;
	usleep(((rand() % 100) + 10) * 1000);
	ygpget(gp); //1
	ygpget(gp); //2
	ygpput(gp); //1
	ygpget(gp); //2
	ygpget(gp); //3
	ygpput(gp); //2
	ygpput(gp); //1
	ygpget(gp); //2
	ygpput(gp); //1
	ygpget(gp); //2
	ygpget(gp); //3
	ygpput(gp); //2
	ygpput(gp); //1
	ygpput(gp); //0
	return NULL;
}


static void
test_gp(void) {
	pthread_t pthd[16];
	void *retval;
	int i;
	struct ygp *gp;
	struct myvalue *myv;
	srand(time(NULL));

	myv = ymalloc(sizeof(*myv));
	gp = &myv->gp;
	ygpinit(gp, myv, &yfree);
	ygpget(gp);
	ygpget(gp);
	ygpget(gp);
	ygpput(gp);
	ygpdestroy(gp);

	myv = ymalloc(sizeof(*myv));
	gp = &myv->gp;
	ygpinit(gp, myv, &yfree);
	ygpget(gp);
	i = 16;
	while (i--)
		pthread_create(&pthd[i], NULL, gpaction, gp);
	i = 16;
	while (i--)
		pthread_join(pthd[i], &retval);
	ygpput(gp);

	return;
}

extern void gp_clear(void);
static void
clear_gp(void) {
	gp_clear();
}


TESTFN(gp)
CLEARFN(gp)

#endif /* CONFIG_DEBUG */
