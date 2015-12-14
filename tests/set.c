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

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "common.h"
#include "yutils.h"
#include "yset.h"

static void
test_set(void) {
	int i, r;
	int *elems[100];
	yset_t s = yseti_create();

	for (i = 0; i < 100; i++)
		yassert(1 == yset_add(s, (void *)(intptr_t)i));
	for (i = 0; i < 50; i++)
		yassert(0 == yset_add(s, (void *)(intptr_t)i));
	for (i = 0; i < 25; i++)
		yassert(1 == yset_remove(s, (void *)(intptr_t)i));
	for (i = 0; i < 25; i++)
		yassert(0 == yset_remove(s, (void *)(intptr_t)i));
	for (i = 0; i < 25; i++)
		yassert(!yset_has(s, (void *)(intptr_t)i));
	for (; i < 50; i++)
		yassert(yset_has(s, (void *)(intptr_t)i));
	r = yset_elements(s, (const void **)elems, yut_arrsz(elems));
	yassert(75 == r);
	for (i = 0; i < r; i++)
		yassert(25 <= (intptr_t)elems[i] && (intptr_t)elems[i] < 100);
	yset_destroy(s);
}

TESTFN(test_set, set)

#endif /* CONFIG_DEBUG */
