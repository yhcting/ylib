/******************************************************************************
 * Copyright (C) 2015, 2023
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

#define MIN_INIT_CAPACITY (4096 / sizeof(struct yheap_node *))

struct yheap {
	struct ydynb *b;
	void (*vfree)(struct yheap_node *);
#ifndef CONFIG_YHEAP_STATIC_CMP_MAX_HEAP
	int (*cmp)(const struct yheap_node *, const struct yheap_node *);
#endif
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

unused static INLINE int
has_parent(u32 i) {
	return 0 < parenti(i);
}

static INLINE bool
is_rooti(u32 i) {
	return 1 == i;
}

static INLINE u32
lasti(const struct yheap *h) {
	/* -1 to remove overhead comes from virtual root(element[0]) */
	return ydynb_sz(h->b) - 1;
}

static INLINE int
is_validi(const struct yheap *h, u32 i) {
	return 0 < i && i <= lasti(h);
}

#ifdef CONFIG_DEBUG

/**
 * Initalize debugging information to new node
 */
static INLINE void
dbg_init(struct yheap *h, struct yheap_node *e) {
	e->h = h;
}

/**
 * Is valid node of this heap?
 */
static INLINE bool
dbg_has(struct yheap *h, struct yheap_node *e) {
	return h == e->h;
}

#else /* CONFIG_DEBUG */

static INLINE void
dbg_init(struct yheap *h, struct yheap_node *e) {}

/**
 * Is valid node of this heap?
 */
static INLINE bool
dbg_has(struct yheap *h, struct yheap_node *e) {
	return TRUE;
}

#endif /* CONFIG_DEBUG */

static INLINE struct yheap_node *
gete(const struct yheap *h, u32 i) {
	yassert(i <= lasti(h));
	return ((struct yheap_node **)ydynb_buf(h->b))[i];
}

static INLINE void
sete(struct yheap *h, u32 i, struct yheap_node *e) {
	yassert(i < ydynb_limit(h->b));
	if (0 < i) { e->i = i; }
	((struct yheap_node **)ydynb_buf(h->b))[i] = e;
}

static INLINE void
adde(struct yheap *h, struct yheap_node *e) {
	/* To improve performance, ydynb_append is NOT used to avoid memcpy */
	ydynb_incsz(h->b, 1);
	sete(h, lasti(h), e);
}

static INLINE void
rmlaste(struct yheap *h) {
	ydynb_decsz(h->b, 1);
}

static INLINE void
swap(struct yheap *h, u32 i0, u32 i1) {
	struct yheap_node *tmp = gete(h, i0);
	sete(h, i0, gete(h, i1));
	sete(h, i1, tmp);
}

#ifdef CONFIG_YHEAP_STATIC_CMP_MAX_HEAP

static INLINE int
cmpe(unused struct yheap *h, struct yheap_node *a, struct yheap_node *b) {
	return a->v - b->v;
}

#else /* CONFIG_YHEAP_STATIC_CMP_MAX_HEAP */

static INLINE int
cmpe(struct yheap *h, struct yheap_node *a, struct yheap_node *b) {
	return (*h->cmp)(a, b);
}

#endif /* CONFIG_YHEAP_STATIC_CMP_MAX_HEAP */

static INLINE int
cmp(struct yheap *h, u32 a, u32 b) {
	return cmpe(h, gete(h, a), gete(h, b));
}

/**
 * Down heap
 */
static INLINE void
down(struct yheap *h, u32 i) {
	yassert(is_validi(h, i));
	while (TRUE) {
		u32 p = i;
		u32 l = lefti(i);
		u32 r = righti(i);
		u32 end = lasti(h);

		if (l <= end && cmp(h, i, l) < 0)
			i = l;
		if (r <= end && cmp(h, i, r) < 0)
			i = r;
		if (p == i)
			break; // done
		swap(h, p, i);
	}
}

/**
 * Up heap
 */
static INLINE void
up(struct yheap *h, u32 i) {
	u32 p;
	yassert(is_validi(h, i));
	while (0 < (p = parenti(i)) /* has parent */) {
		if (cmp(h, i, p) <= 0)
			break; /* Heap structuring is done. */
		swap(h, i, p);
		i = p;
	}
}

/******************************************************************************
 *
 *
 *
 *****************************************************************************/
struct yheap *
yheap_create(
	u32 capacity,
	void (*vfree)(struct yheap_node *)
#ifndef CONFIG_YHEAP_STATIC_CMP_MAX_HEAP
	, int (*cmp)(const struct yheap_node *, const struct yheap_node *)
#endif
) {
	struct yheap *h;
#ifndef CONFIG_YHEAP_STATIC_CMP_MAX_HEAP
	if (unlikely(!cmp))
		return NULL; /* invalid argument. */
#endif
	if (unlikely(!(h = ymalloc(sizeof(*h)))))
		return NULL;
	if (capacity < MIN_INIT_CAPACITY)
		capacity = MIN_INIT_CAPACITY;
	/* '+1' for dummy virtual root. */
	if (unlikely(0 > (h->b = ydynb_create2(capacity + 1, sizeof(void *)))))
		goto fail;
	h->vfree = vfree;
#ifndef CONFIG_YHEAP_STATIC_CMP_MAX_HEAP
	h->cmp = cmp;
#endif
	/* element 0 is fixed virtual root */
	adde(h, NULL);

	return h;

 fail:
	yfree(h);
	return NULL;
}

YYEXPORT struct yheap *
yheap_create2(
	struct yheap_node **arr,
	uint32_t arrsz,
	void (*vfree)(struct yheap_node *)
#ifndef CONFIG_YHEAP_STATIC_CMP_MAX_HEAP
	, int (*cmp)(const struct yheap_node *, const struct yheap_node *)
#endif
) {
	struct yheap *h = yheap_create(arrsz, vfree
#ifndef CONFIG_YHEAP_STATIC_CMP_MAX_HEAP
		,cmp
#endif
	);

	if (unlikely(!h))
		return NULL;

	for (u32 i = 0; i < arrsz; i++) {
		dbg_init(h, arr[i]);
		adde(h, arr[i]);
	}

	/* build heap - bottom-up: O(n) */
	for (u32 i = ydynb_sz(h->b) / 2; i > 0; i--)
		down(h, i);

	return h;
}

void
yheap_reset(struct yheap *h) {
	yassert(ydynb_sz(h->b) >= 1);
	if (h->vfree) {
		void **p = (void **)ydynb_buf(h->b);
		void **pend = (void **)ydynb_getfree(h->b);
		p++; /* skip first virtual root(NULL) */
		while (p < pend)
			(*h->vfree)(*p++);
	}
	sete(h, 0, NULL);
	ydynb_setsz(h->b, 1);
}

void
yheap_destroy(struct yheap *h) {
	yheap_reset(h);
	ydynb_destroy(h->b, FALSE);
	yfree(h);
}

u32
yheap_sz(const struct yheap *h) {
	/* Empty heap only has 1 virtual root(element[0]) */
	yassert(ydynb_sz(h->b) >= 1);
	return lasti(h);
}

int
yheap_push(struct yheap *h, struct yheap_node *e) {
	int r;
	yassert(!dbg_has(h, e));
	if (unlikely(0 > (r = ydynb_expand2(h->b, 1))))
		return r;
	dbg_init(h, e);
	adde(h, e);
	up(h, lasti(h)); /* heapify-up */
	return 0;
}

struct yheap_node *
yheap_peek(const struct yheap *h) {
	if (likely(yheap_sz(h) > 0))
		return gete(h, 1);
	else
		return NULL;
}

struct yheap_node *
yheap_pop(struct yheap *h) {
	struct yheap_node *e;
	if (unlikely(!yheap_sz(h)))
		return NULL;
	e = gete(h, 1); /* root element */
	swap(h, 1, lasti(h));
	rmlaste(h);
	if (likely(0 < yheap_sz(h)))
		down(h, 1); /* heapify-down new root element*/
	return e;
}

struct yheap_node *
yheap_pushpop(struct yheap *h, struct yheap_node *e) {
	yassert(!dbg_has(h, e));
	if (yheap_sz(h) > 0 && (*h->cmp)(ydynb_get(h->b, 1), e) > 0) {
		struct yheap_node *r = ydynb_get(h->b, 1);
		dbg_init(h, e);
		sete(h, 1, e);
		down(h, 1);
		return r;
	} else {
		return e;
	}
}

struct yheap_node *
yheap_poppush(struct yheap *h, struct yheap_node *e) {
	struct yheap_node *r;
	yassert(!dbg_has(h, e));
	if (yheap_sz(h) > 0) {
		r = ydynb_get(h->b, 1);
		dbg_init(h, e);
		sete(h, 1, e);
		down(h, 1);
	} else {
		r = NULL;
		yheap_push(h, e);
	}
	return r;
}

void
yheap_heapify(struct yheap *h, struct yheap_node *e) {
	u32 i = e->i;
	u32 p = parenti(i);
	yassert(is_validi(h, i) && dbg_has(h, e));
	if (!is_rooti(i) && cmp(h, i, p) > 0)
		up(h, i);
	else
		down(h, i);
}

int
yheap_iterates(
	struct yheap *h,
	void *ctx,
	int (*cb)(struct yheap_node *e, void *ctx)
) {
	u32 i;
	u32 heapsz = yheap_sz(h);
	for (i = 1; i <= heapsz; i++) {
		if (!(*cb)(gete(h, i), ctx))
			return 1;
	}
	return 0;
}
