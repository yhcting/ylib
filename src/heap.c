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

/*
 * Heap structure
 *
 *           6
 *        /     \
 *       5       4
 *     /   \    /
 *    2     1  3
 *
 * [ Array representation ]
 *   0   1   2   3   4   5   6  ...
 * +---+---+---+---+---+---+---+---
 * |   | 6 | 5 | 4 | 2 | 1 | 3 | ...
 * +---+---+---+---+---+---+---+---
 *
 */


#include "common.h"
#include "yheap.h"
#include "ydynb.h"

#define MIN_INIT_CAPACITY (4096 / sizeof(void *))

struct yheap {
	struct ydynb *b;
	void (*vfree)(void *);
	int (*cmp)(const void *, const void *);
};


/******************************************************************************
 *
 *
 *
 *****************************************************************************/

/******************************************************************************
 *
 * Heap utils
 *
 *****************************************************************************/
static INLINE u32
lefti(u32 i) {
	return i * 2;
}

static INLINE u32
righti(u32 i) {
	return lefti(i) + 1;
}

static INLINE u32
parenti(u32 i) {
	return i / 2;
}

static INLINE int
has_parent(u32 i) {
	return 0 < parenti(i);
}

static INLINE u32
lasti(const struct yheap *h) {
	/* -1 to remove overhead comes from virtual root(element[0]) */
	return h->b->sz - 1;
}

static INLINE int
is_validi(const struct yheap *h, u32 i) {
	return 0 < i && i <= lasti(h);
}

static INLINE void *
gete(const struct yheap *h, u32 i) {
	yassert(i <= lasti(h));
	return ((void **)h->b->b)[i];
}

static INLINE void
sete(struct yheap *h, u32 i, void *e) {
	yassert(i < h->b->limit);
	((void **)h->b->b)[i] = e;
}

static INLINE void
adde(struct yheap *h, void *e) {
	/* To improve performance, ydynb_append is NOT used to avoid memcpy */
	h->b->sz++;
	sete(h, lasti(h), e);
}

static INLINE void
rmlaste(struct yheap *h) {
	h->b->sz--;
}

static INLINE void
swape(struct yheap *h, u32 i0, u32 i1) {
	void *tmp = gete(h, i0);
	sete(h, i0, gete(h, i1));
	sete(h, i1, tmp);
}

/******************************************************************************
 *
 *
 *
 *****************************************************************************/
struct yheap *
yheap_create(u32 capacity,
	     void (*vfree)(void *),
	     int (*cmp)(const void *, const void *)) {
	struct yheap *h;
	if (unlikely(!cmp))
		return NULL; /* invalid argument. */
	if (unlikely(!(h = ymalloc(sizeof(*h)))))
		return NULL;
	if (capacity < MIN_INIT_CAPACITY)
		capacity = MIN_INIT_CAPACITY;
	/* 'void *' type is not needed to be align. */
	if (unlikely(0 > (h->b = ydynb_create2(capacity, sizeof(void *)))))
		goto fail;
	h->vfree = vfree;
	h->cmp = cmp;

	/* element 0 is fixed virtual root */
	sete(h, 0, NULL);
	h->b->sz = 1;

	return h;

 fail:
	yfree(h);
	return NULL;
}

void
yheap_reset(struct yheap *h) {
	yassert(h->b->sz >= 1);
	if (h->vfree) {
		void **p = (void **)h->b->b;
		void **pend = (void **)ydynb_getfree(h->b);
		p++; /* skip first virtual root(NULL) */
		while (p < pend)
			(*h->vfree)(*p++);
	}
	sete(h, 0, NULL);
	h->b->sz = 1;
}

void
yheap_destroy(struct yheap *h) {
	yheap_reset(h);
	ydynb_destroy(h->b);
	yfree(h);
}

u32
yheap_sz(const struct yheap *h) {
	/* Empty heap only has 1 virtual root(element[0]) */
	yassert(h->b->sz >= 1);
	return lasti(h);
}

int
yheap_add(struct yheap *h, void *e) {
	u32 i, r;
	if (unlikely(0 > (r = ydynb_expand2(h->b, 1))))
		return r;
	adde(h, e);

	i = lasti(h);
	/* re-structuring heap */
	while (has_parent(i)) {
		u32 p = parenti(i);
		if (0 >= (*h->cmp)(gete(h, i), gete(h, p)))
			break; /* Heap structuring is done. */
		swape(h, i, p);
		i = p;
	}
	return 0;
}

void *
yheap_peek(const struct yheap *h) {
	if (likely(yheap_sz(h) > 0))
		return gete(h, 1);
	else
		return NULL;
}

void *
yheap_pop(struct yheap *h) {
	void *e;
	if (unlikely(!yheap_sz(h)))
		return NULL;
	e = gete(h, 1); /* top element */
	swape(h, 1, lasti(h));
	rmlaste(h);

	/* re-structuring heap */
	u32 i = 1;
	do {
		u32 lc, rc; /* Left Child, Right Child */
		u32 maxc = 0;
		lc = lefti(i);
		rc = righti(i);

		/* get bigger child */
		if (!is_validi(h, lc))
			/* There is no child.
			 * Heap makes sure that right node MUST be attached
			 * after left node is attached.
			 */
			break;
		else if (!is_validi(h, rc))
			/* Only left child is available */
			maxc = lc;
		else
			/* Both left and right are available. */
			maxc = 0 < (*h->cmp)(gete(h, lc), gete(h, rc))? lc: rc;

		if (0 < (*h->cmp)(gete(h, maxc), gete(h, i))) {
			swape(h, maxc, i);
			i = maxc;
		} else
			break;
	} while (is_validi(h, i));

	return e;
}

int
yheap_iterates(struct yheap *h,
	       void *tag,
	       int (*cb)(void *e, void *tag)) {
	u32 i;
	u32 heapsz = yheap_sz(h);
	for (i = 0; i <= heapsz; i++) {
		if (!(*cb)(gete(h, i), tag))
			return 1;
	}
	return 0;
}
