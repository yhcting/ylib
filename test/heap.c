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
#ifdef CONFIG_TEST

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "yheap.h"

#define TESTARRSZ 10000


struct node {
	int v;
#ifdef CONFIG_YHEAP_STATIC_CMP_MAX_HEAP
	struct yheap_node hn;
#endif
};

static int
cmp_node(const void *v0, const void *v1) {
	return ((const struct node *)v0)->v - ((const struct node *)v1)->v;
}

#ifdef CONFIG_YHEAP_STATIC_CMP_MAX_HEAP
static inline yheap_node_t *
heap_node(struct node *n) {
	return &n->hn;
}

static void
node_free(yheap_node_t *hn) {
	yfree(YYcontainerof(hn, struct node, hn));
}

static struct yheap *
create_heap(void (*vfree)(yheap_node_t *)) {
	return yheap_create(0, vfree);
}

static inline void
node_setv(struct node *n, int v) {
	n->v = v;
	n->hn.v = v;
}

#else /* CONFIG_YHEAP_STATIC_CMP_MAX_HEAP */

static inline yheap_node_t *
heap_node(struct node *n) {
	return (void *)n;
}

static void
node_free(yheap_node_t *hn) {
	yfree(hn);
}

static struct yheap *
create_heap(void (*vfree)(yheap_node_t *)) {
	return yheap_create(0, vfree, &cmp_node);
}

static inline void
node_setv(struct node *n, int v) {
	n->v = v;
}

#endif /* CONFIG_YHEAP_STATIC_CMP_MAX_HEAP */

static void
chkeq(struct node *a, struct node *b) {
	if (a->v != b->v) {
		printf("%d, %d\n", a->v, b->v);
		yassert(0);
	}
}

static void
test_heap(void) {
	int i, j;
	struct node a[TESTARRSZ];
	struct node *pa[TESTARRSZ];
	struct node b[TESTARRSZ];
	struct node *pi;
	struct yheap *h = create_heap(NULL);
	srand((int)time(NULL));
	for (i = 0; i < TESTARRSZ; i++) {
		node_setv(&a[i], rand());
		b[i] = a[i];
		yheap_add(h, heap_node(&a[i]));
	}
	yassert(TESTARRSZ == yheap_sz(h));
	qsort(b, TESTARRSZ, sizeof(b[0]), &cmp_node);
	pi = yheap_peek(h);
	chkeq(pi, &b[TESTARRSZ - 1]);
	j = TESTARRSZ;
	for (i = 0; i < TESTARRSZ; i++) {
		pi = yheap_pop(h);
		j--;
		chkeq(pi, &b[j]);
	}
	yheap_destroy(h);
	h = create_heap(NULL);

	/* fill again to test heap after clean */
	for (i = 0; i < TESTARRSZ; i++) {
		node_setv(&a[i], rand());
		b[i] = a[i];
		yheap_add(h, heap_node(&a[i]));
	}
	qsort(b, TESTARRSZ, sizeof(b[0]), &cmp_node);
	pi = yheap_peek(h);
	chkeq(pi, &b[TESTARRSZ - 1]);
	j = TESTARRSZ;
	for (i = 0; i < TESTARRSZ; i++) {
		pi = yheap_pop(h);
		j--;
		chkeq(pi, &b[j]);
	}
	yheap_destroy(h);

	/********************************
	 * Test for allocated memory
	 ********************************/
	h = create_heap(&node_free);
	for (i = 0; i < TESTARRSZ; i++) {
		pa[i] = ymalloc(sizeof(*pa[i]));
		node_setv(pa[i], rand());
		yheap_add(h, heap_node(pa[i]));
	}
	yheap_destroy(h);

	h = create_heap(&node_free);
	for (i = 0; i < TESTARRSZ; i++) {
		pa[i] = ymalloc(sizeof(*pa[i]));
		node_setv(pa[i], rand());
		yheap_add(h, heap_node(pa[i]));
	}
	yheap_destroy(h);
}

TESTFN(heap)

#endif /* CONFIG_TEST */
