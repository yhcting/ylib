/******************************************************************************
 * Copyright (C) 2011, 2012, 2013, 2014, 2015
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

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <pthread.h>

#include "ylib.h"
#include "common.h"
#include "ylistl.h"

struct tstfn {
	void (*fn)(void);
	void (*clear)(void);
	const char *modname;
	struct ylistl_link lk;
};

static YLISTL_DEFINE_HEAD(_tstfnl);
static int _mem_count = 0;
static pthread_mutex_t _mem_count_lock = PTHREAD_MUTEX_INITIALIZER;

void
dregister_tstfn(void (*fn)(void), const char *mod) {
	/* malloc should be used instead of dmalloc */
	struct tstfn* n = malloc(sizeof(*n));
	n->fn = fn;
	n->clear = NULL;
	n->modname = mod;
	ylistl_add_last(&_tstfnl, &n->lk);
}

void
dunregister_tstfn(void (*fn)(void), const char *mod) {
	struct tstfn *p, *n;
	ylistl_foreach_item_removal_safe(p, n, &_tstfnl, struct tstfn, lk) {
		if (p->fn == fn
		    && !strcmp(p->modname, mod)) {
			ylistl_remove(&p->lk);
			free(p);
		}
	}
}

void
dregister_clearfn(void (*fn)(void), const char *mod) {
	struct tstfn *p;
	ylistl_foreach_item(p, &_tstfnl, struct tstfn, lk) {
		if (!strcmp(p->modname, mod)) {
			p->clear = fn;
			return;
		}
	}
	fprintf(stderr, "Unknown module name for clear function: %s\n", mod);
}

void *
dmalloc(size_t sz) {
	pthread_mutex_lock(&_mem_count_lock);
	_mem_count++;
	pthread_mutex_unlock(&_mem_count_lock);
	return malloc(sz);
}

void *
drealloc(void *p, size_t sz) {
	return realloc(p, sz);
}

void *
dcalloc(size_t n, size_t sz) {
	pthread_mutex_lock(&_mem_count_lock);
	_mem_count++;
	pthread_mutex_unlock(&_mem_count_lock);
	return calloc(n, sz);
}

void
dfree(void *p) {
	pthread_mutex_lock(&_mem_count_lock);
	_mem_count--;
	pthread_mutex_unlock(&_mem_count_lock);
	free(p);
}

int
dmem_count(void) {
	return _mem_count;
}


#ifdef CONFIG_DEBUG
int
main(int argc, const char *argv[]) {
	int sv;
	struct tstfn *p;
	const char *modnm = NULL; /* module name to test */
	if (2 == argc)
		modnm = argv[1];
	else if (2 < argc) {
		fprintf(stderr, "Usage: y [module name]\n");
		return -1;
	}

	pthread_mutex_init(&_mem_count_lock, NULL);

	ylib_init(NULL);
	ylistl_foreach_item(p, &_tstfnl, struct tstfn, lk) {
		int repeat = 5;
		if (modnm
		    && strcmp(modnm, p->modname))
			continue; /* skip not-interesting test */
		printf("<< Test [%s] >>\n", p->modname);
		sv = dmem_count();
		/* Repeat same test several times to detect memory corruption
		 * in each test.
		 * This is useful to avoid following case.
		 *
		 * [ Example ]
		 * - Test order : modA -> modB
		 * - Memory is corrupted at test-modA, especially at the end of
		 * test.
		 * - Test may fail at modB because of memory corruption caused
		 * at test-modA
		 * - This result is NOT good to understand.
		 */
		while (repeat--)
			(*p->fn)();
		if (p->clear)
			(*p->clear)();
		if (sv != dmem_count()) {
			printf("Unbalanced memory at [%s]!\n"
			       "    balance : %d\n",
			       p->modname,
			       dmem_count() - sv);
			return -1;
			/* yassert(0); */
		}
		printf(" => PASSED\n");
	}
	pthread_mutex_destroy(&_mem_count_lock);
	printf(">>>>>> TEST PASSED <<<<<<<\n");
	return 0;
}

#else /* CONFIG_DEBUG */
int
main(int argc, const char *argv[]) {
	printf("TEST SKIPPED. THIS IS NOT DEBUG BUILD!\n");
	return 0;
}
#endif /* CONFIG_DEBUG */
