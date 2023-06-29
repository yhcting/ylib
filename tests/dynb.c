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
#include "test.h"
#ifdef CONFIG_DEBUG

#include <string.h>
#include <assert.h>
#include <inttypes.h>

#include "ydynb.h"
#include "common.h"


struct elem {
	short v;
	char c;
};

static void
test_dynb(void) {
	int i;
	char c;
	char *pc;
	struct ydynb *b;
	struct elem es[10];
	struct elem *pe;
	/* Simple byte(character) array.
	 * =============================
	 */
	b = ydynb_create2(2, 1);
	yassert(2 == ydynb_freesz(b));
	ydynb_appends(b, "ab ", 3);
	yassert(1 == ydynb_freesz(b));
	ydynb_appends(b, "cde fgh ", 8);
	/* include trailing 0 */
	ydynb_appends(b, "1234567890123456789012345678901234567890", 41);
	yassert(52 == ydynb_sz(b));
	yassert(!strcmp("ab cde fgh 1234567890123456789012345678901234567890",
			(const char *)ydynb_buf(b)));
	yassert(-EINVAL == ydynb_shrink(b, 0));
	yassert(-EINVAL == ydynb_shrink(b, 0xffff));
	yassert(-EINVAL == ydynb_shrink(b, ydynb_sz(b) - 1));
	ydynb_shrink(b, ydynb_sz(b));
	yassert(0 == ydynb_freesz(b));
	ydynb_destroy(b, FALSE);

	/* Struct array.
	 * =============
	 */
	b = ydynb_create2(2, sizeof(struct elem));
	/* ydynb_dump(b); */
	yassert(2 == ydynb_freesz(b));
	es[0].v = 0;
	es[0].c = 'A';
	ydynb_append(b, &es[0]);
	yassert(1 == ydynb_sz(b));
	pe = ydynb_get(b, 0);
	yassert(0 == pe->v && 'A' == pe->c);
	for (i = 1; i < 10; i++) {
		es[i].v = i;
		es[i].c = 'A' + i;
	}
	ydynb_appends(b, &es[1], 9);
	yassert(10 == ydynb_sz(b));
	pe = ydynb_get(b, 2);
	yassert(2 == pe->v && 'C' == pe->c);

	ydynb_destroy(b, FALSE);

	/* 4-bytes aligned char
	 * ====================
	 */
	b = ydynb_create(2, 1, 4);
	c = 'A';
	ydynb_append(b, &c);
	c = 'B';
	ydynb_append(b, &c);
	c = 'C';
	ydynb_append(b, &c);
	pc = ydynb_get(b, 1);
	yassert(*pc == 'B');
	yassert((uintptr_t)pc % 4 == 0);
	ydynb_destroy(b, FALSE);

}


TESTFN(dynb)

#endif /* CONFIG_DEBUG */
