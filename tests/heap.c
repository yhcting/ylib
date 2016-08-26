/******************************************************************************
 * Copyright (C) 2015
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
#include <time.h>
#include <assert.h>

#include "yheap.h"

#define TESTARRSZ 10000

static void
chkeq(int a, int b) {
	if (a != b) {
		printf("%d, %d\n", a, b);
		yassert(0);
	}
}

static int
cmp_int(const void *v0, const void *v1) {
	const int *i0 = (const int *)v0;
	const int *i1 = (const int *)v1;
	if (*i0 > *i1)
		return 1;
	else if (*i0 < *i1)
		return -1;
	else
		return 0;
}

static void
test_heap(void) {
	int i, j;
	int a[TESTARRSZ];
	int *pa[TESTARRSZ];
	int b[TESTARRSZ];
	int *pi;
	struct yheap *h = yheap_create(0, NULL, &cmp_int);
	srand((int)time(NULL));
	for (i = 0; i < TESTARRSZ; i++) {
		a[i] = rand();
		b[i] = a[i];
		yheap_add(h, (void *)&a[i]);
	}
	yassert(TESTARRSZ == yheap_sz(h));
	qsort(b, TESTARRSZ, sizeof(b[0]), &cmp_int);
	pi = yheap_peek(h);
	chkeq(*pi, b[TESTARRSZ - 1]);
	j = TESTARRSZ;
	for (i = 0; i < TESTARRSZ; i++) {
		pi = yheap_pop(h);
		j--;
		chkeq(*pi, b[j]);
	}
	yheap_destroy(h);
	h = yheap_create(0, NULL, &cmp_int);

	/* fill again to test heap after clean */
	for (i = 0; i < TESTARRSZ; i++) {
		a[i] = rand();
		b[i] = a[i];
		yheap_add(h, (void *)&a[i]);
	}
	qsort(b, TESTARRSZ, sizeof(b[0]), &cmp_int);
	pi = yheap_peek(h);
	yassert(*pi == b[TESTARRSZ - 1]);
	j = TESTARRSZ;
	for (i = 0; i < TESTARRSZ; i++) {
		pi = yheap_pop(h);
		j--;
		chkeq(*pi, b[j]);
	}
	yheap_destroy(h);

	/********************************
	 * Test for allocated memory
	 ********************************/
	h = yheap_create(0, &yfree, &cmp_int);
	for (i = 0; i < TESTARRSZ; i++) {
		pa[i] = ymalloc(sizeof(*pa[i]));
		*pa[i] = rand();
		yheap_add(h, (void *)pa[i]);
	}
	yheap_destroy(h);

	h = yheap_create(0, &yfree, &cmp_int);
	for (i = 0; i < TESTARRSZ; i++) {
		pa[i] = ymalloc(sizeof(*pa[i]));
		*pa[i] = rand();
		yheap_add(h, (void *)pa[i]);
	}
	yheap_destroy(h);
}

TESTFN(heap)

#endif /* CONFIG_DEBUG */
