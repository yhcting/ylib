/******************************************************************************
 * Copyright (C) 2023
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

#include <string.h>
#include <stdint.h>

#include "yrx.h"

struct ctx {
	int v;
	int e;
};

static void sbnext(void *ctx, const void *v) {
	struct ctx *c = ctx;
	c->v = (int)(intptr_t)v;
}

static void sberror(void *ctx, const void *v) {
	struct ctx *c = ctx;
	c->e = (int)(intptr_t)v;
}

static void sbcomplete(void *ctx) {
	struct ctx *c = ctx;
	c->v = -1;
	c->e = -1;
}

static void
test_rx(void) {
	struct ctx c0;
	struct ctx c1;
	c0.v = 0;
	c0.e = 0;
	c1.v = 0;
	c1.e = 0;
	struct yrx *rx = yrx_create();
	yassert(rx);
	(void)yrx_subscribe(rx,
		&c0, NULL,
		&sbnext, &sberror, &sbcomplete);
	(void)yrx_subscribe(rx,
		&c1, NULL,
		&sbnext, &sberror, &sbcomplete);
	yrx_next(rx, (void *)1);
	yassert(1 == c0.v && 1 == c1.v && 0 == c0.e);
	yrx_next(rx, (void *)2);
	yassert(2 == c0.v && 2 == c1.v);
	yrx_error(rx, (void *)1);
	yassert(2 == c0.v && 2 == c1.v && 1 == c0.e && 1 == c1.e);
	yrx_complete(rx);
	yassert(-1 == c0.v && -1 == c0.e && -1 == c1.v && -1 == c1.e);
}

TESTFN(rx)

#endif /* CONFIG_TEST */
