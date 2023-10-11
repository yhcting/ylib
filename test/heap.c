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
#include <string.h>
#include <time.h>

#include "yheap.h"

#define ARRSZ 1000


struct node {
	int v;
	struct yheap_node hn;
};

static inline struct node *
node(const struct yheap_node *hn) {
	return YYcontainerof(hn, struct node, hn);
}

static inline struct yheap_node *
heap_node(struct node *n) {
	return &n->hn;
}

static int
cmp_node(const void *v0, const void *v1) {
	return ((struct node *)v0)->v - ((struct node *)v1)->v;
}

static int
cmp_node_reverse(const void *v0, const void *v1) {
	return -cmp_node(v0, v1);
}

static int
cmp_hnode(const struct yheap_node *v0, const struct yheap_node *v1) {
	return cmp_node(node(v0), node(v1));
}

static void
node_free(struct yheap_node *hn) {
	yfree(node(hn));
}

static struct yheap *
create_heap(void (*vfree)(struct yheap_node *)) {
	return yheap_create(0, vfree, &cmp_hnode);
}

static inline void
node_setv(struct node *n, int v) {
	n->v = v;
#ifdef CONFIG_YHEAP_STATIC_CMP_MAX_HEAP
	n->hn.v = v;
#endif
}

static void
chkeq(struct node *a, struct node *b) {
	if (a->v != b->v) {
		printf("%d, %d\n", a->v, b->v);
		yassert(0);
	}
}

unused static int
printhn(struct yheap_node *hn, void *tag) {
	printf("%d, ", node(hn)->v);
	return TRUE;
}

static void
test_heap(void) {
	int i;
	struct node a[ARRSZ];
	struct yheap_node *phn[ARRSZ];
	struct node *pa[ARRSZ];
	struct node b[ARRSZ];
	struct yheap *h;

	int seed = (int)time(NULL);
	/* printf("Seed: %d\n", seed); */
	srand(seed);

	/*
	 * Push and pop
	 */
 	h = create_heap(NULL);
	for (i = 0; i < ARRSZ; i++) {
		node_setv(&a[i], rand());
		yheap_push(h, heap_node(&a[i]));
	}
	memcpy(b, a, sizeof(b));

	yassert(ARRSZ == yheap_sz(h));
	qsort(b, ARRSZ, sizeof(b[0]), &cmp_node_reverse);
	for (i = 0; i < ARRSZ; i++) {
		chkeq(node(yheap_pop(h)), &b[i]);
	}
	yheap_destroy(h);


	/*
	 * Heap build
	 */
	for (i = 0; i < ARRSZ; i++) {
		node_setv(&a[i], rand());
		phn[i] = &a[i].hn;
	}
	memcpy(b, a, sizeof(b));
	h = yheap_create2(phn, ARRSZ, NULL, &cmp_hnode);
	qsort(b, ARRSZ, sizeof(b[0]), &cmp_node_reverse);
	for (i = 0; i < ARRSZ; i++)
		chkeq(node(yheap_pop(h)), &b[i]);
	yheap_destroy(h);


	/*
	 * Heap after clean
	 */
	h = create_heap(NULL);
	for (i = 0; i < ARRSZ; i++) {
		node_setv(&a[i], rand());
		yheap_push(h, heap_node(&a[i]));
	}
	memcpy(b, a, sizeof(b));

	qsort(b, ARRSZ, sizeof(b[0]), &cmp_node_reverse);
	for (i = 0; i < ARRSZ; i++)
		chkeq(node(yheap_pop(h)), &b[i]);
	yheap_destroy(h);


	/*
	 * Heapify
	 */
	h = create_heap(NULL);
	for (i = 0; i < ARRSZ; i++) {
		node_setv(&a[i], rand());
		yheap_push(h, heap_node(&a[i]));
	}

	for (i = 0; i < ARRSZ / 2; i++) {
		node_setv(&a[i], rand());
		yheap_heapify(h, heap_node(&a[i]));
	}
	memcpy(b, a, sizeof(b));

	qsort(b, ARRSZ, sizeof(b[0]), &cmp_node_reverse);
	for (i = 0; i < ARRSZ; i++)
		chkeq(node(yheap_pop(h)), &b[i]);
	yheap_destroy(h);


	/**
	 * Memory free
	 */
	h = create_heap(&node_free);
	for (i = 0; i < ARRSZ; i++) {
		pa[i] = ymalloc(sizeof(*pa[i]));
		node_setv(pa[i], rand());
		yheap_push(h, heap_node(pa[i]));
	}
	yheap_destroy(h);

	h = create_heap(&node_free);
	for (i = 0; i < ARRSZ; i++) {
		pa[i] = ymalloc(sizeof(*pa[i]));
		node_setv(pa[i], rand());
		yheap_push(h, heap_node(pa[i]));
	}
	yheap_destroy(h);
}

TESTFN(heap)

#endif /* CONFIG_TEST */
