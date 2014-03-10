/******************************************************************************
 * Copyright (C) 2011, 2012, 2013, 2014
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

#include "ydef.h"
#include "ylistl.h"

struct tstfn {
	void             (*fn)(void);
	const char        *modname;
	struct ylistl_link lk;
};

static YLISTL_DECL_HEAD(_tstfnl);
static int  _mem_count = 0;

void
dregister_tstfn(void (*fn)(void), const char *mod) {
	/* malloc should be used instead of dmalloc */
	struct tstfn* n = malloc(sizeof(*n));
	n->fn = fn;
	n->modname = mod;
	ylistl_add_last(&_tstfnl, &n->lk);
}


void *
dmalloc(unsigned int sz) {
	_mem_count++;
	return malloc(sz);
}

void *
dcalloc(unsigned int n, unsigned int sz) {
	_mem_count++;
	return calloc(n, sz);
}

void
dfree(void *p) {
	_mem_count--;
	free(p);
}

int
dmem_count(void) {
	return _mem_count;
}

int main() {
	int           sv;
	struct tstfn *p;
	ylistl_foreach_item(p, &_tstfnl, struct tstfn, lk) {
		printf("<< Test [%s] >>\n", p->modname);
		sv = dmem_count();
		(*p->fn)();
		if (sv != dmem_count()) {
			printf("Unbalanced memory at [%s]!\n"
			       "    balance : %d\n",
			       p->modname,
			       dmem_count() - sv);
			yassert(0);
		}
		printf(" => PASSED\n");
	}

	printf(">>>>>> TEST PASSED <<<<<<<\n");
	return 0;
}
