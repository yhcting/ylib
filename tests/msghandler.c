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

#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "common.h"
#include "ymsg.h"
#include "ymsglooper.h"
#include "ymsghandler.h"


extern void msg_clear(void);


struct msgA {
	int a, b;
};

struct msgB {
	char *s;
};

static void
free_msgA(void *v) {
	yfree(v);
}

static void
free_msgB(void *v) {
	struct msgB *m = (struct msgB *)v;
	yfree(m->s);
	yfree(m);
}

static void
run(void *a) {
	struct msgB *m = (struct msgB *)a;
	//printf("Hello Run\n");
	yassert('A' == m->s[0]
		&& 'B' == m->s[1]
		&& 'C' == m->s[2]);
}



static void
handle0(const struct ymsg *m) {
	if (YMSG_TYP_DATA == m->type) {
		struct msgA *ma = (struct msgA *)m->data;
		yassert(m->code == ma->a && ma->a == ma->b);
		//printf("Hello data...\n");
	}
}


static void
test_msghandler(void) {
	int i;
	struct ymsghandler *mh0, *mh1;
	struct ymsglooper *ml0 = ymsglooper_start_looper_thread(FALSE);
	struct ymsglooper *ml1 = ymsglooper_start_looper_thread(FALSE);
	mh0 = ymsghandler_create(ml0, NULL); /* use default handle */
	mh1 = ymsghandler_create(ml1, &handle0);

	for (i = 0; i < 10; i++) {
		struct msgA *ma = ymalloc(sizeof(*ma));
		ma->a = ma->b = i;
		ymsghandler_post_data(mh1, i, ma, &free_msgA);
	}

	for (i = 0; i < 10; i++) {
		struct msgB *m = ymalloc(sizeof(*m));
		m->s = ymalloc(100);
	        sprintf(m->s, "ABC %d: message", i);
		ymsghandler_post_exec(mh0, m, &free_msgB, &run);
	}
	ymsglooper_stop(ymsghandler_get_looper(mh0));
	ymsglooper_stop(ymsghandler_get_looper(mh1));

	ymsghandler_destroy(mh0);
	ymsghandler_destroy(mh1);

	while (!(ymsglooper_get_state(ml0) == YMSGLOOPER_TERMINATED
		 && ymsglooper_get_state(ml1) == YMSGLOOPER_TERMINATED))
		usleep(1000 * 50);
	ymsglooper_destroy(ml0);
	ymsglooper_destroy(ml1);
}

static void
clear_msghandler(void) {
	msg_clear();
}


TESTFN(msghandler)
CLEARFN(msghandler)

#endif /* CONFIG_DEBUG */
