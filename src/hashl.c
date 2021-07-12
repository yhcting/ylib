/******************************************************************************
 * Copyright (C) 2021
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


#include <memory.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include "common.h"
#include "yhashl.h"
#include "ylistl.h"
/* crc is used as hash function */
#include "ycrc.h"


#define MAX_HBITS 32
#define MIN_HBITS 4
#define DEFAULT_HBITS 8 /* 256 */

/****************************************************************************
 *
 * Predefined functions for hash
 *
 ****************************************************************************/
static u32
hf_default_ptr(const void *k) {
	return (u32)(0xffffffff & (intptr_t)k);
}

static u32
hf_default_str(const void *k) {
        return ycrc32(0, (const u8 *)k, (u32)strlen((const char *)k));
}

static int
keq_default_ptr(const void *k0, const void *k1) {
	return k0 == k1 ? 0 : 1;
}

static int
keq_default_str(const void *k0, const void *k1) {
	return strcmp((const char *)k0, (const char *)k1);
}

/****************************************************************************
 *
 *
 *
 ****************************************************************************/
static INLINE u32
hv__(u32 mapbits, u32 hv32) {
	return hv32 >> (32 - mapbits);
}

static INLINE u32
hv_(const struct yhashl *h, u32 hv32) {
	return hv__(h->mapbits, hv32);
}

static INLINE u32
hv(const struct yhashl *h, const struct yhashl_node *n) {
	return hv_(h, n->hv32);
}

/****************************************************************************
 *
 *
 *
 ****************************************************************************/
int
yhash_hremap(struct yhashl *h, u32 bits) {
	u32 i;
	struct yhashl_node *n, *tmp;
	struct ylistl_link *oldmap;
	u32 oldmapsz;
	u32 oldbits;

	if (bits < MIN_HBITS)
		bits = MIN_HBITS;
	if (bits > MAX_HBITS)
		bits = MAX_HBITS;

	if (h->mapbits == bits)
		return 0;

	oldmap = h->map;
	oldmapsz = yhashl_hmapsz(h);
	oldbits = h->mapbits;

	h->mapbits = bits; /* map size is changed here */
	if (unlikely(!(h->map = ymalloc(sizeof(*h->map) * yhashl_hmapsz(h))))) {
		/* restore to original value */
		h->map = oldmap;
		h->mapbits = oldbits;
		return -ENOMEM;
	}
	for (i = 0; i < yhashl_hmapsz(h); i++)
		ylistl_init_link(&h->map[i]);
	/* re assign hash nodes */
	for (i = 0; i < oldmapsz; i++) {
		ylistl_foreach_item_safe(
			n, tmp, &oldmap[i], struct yhashl_node, lk
		) {
			ylistl_remove(&n->lk);
			ylistl_add_last(&h->map[hv(h, n)], &n->lk);
		}
	}
	yfree(oldmap);
	return 0;
}

/****************************************************************************
 *
 *
 *
 ****************************************************************************/
struct yhashl *
yhashl_create2(
	yhashl_hfunc_t hfunc,
	yhashl_keyeq_t keqfunc,
	uint32_t initbits
) {
	u32 i;
	struct yhashl *h;
	if (initbits < MIN_HBITS || initbits > MAX_HBITS)
		return NULL;

	h = ymalloc(sizeof(*h));
	if (unlikely(!h)) return NULL;
	h->sz = 0;
	h->mapbits = initbits;
	if (YHASHL_HFUNC_PTR == hfunc)
		hfunc = &hf_default_ptr;
	else if (YHASHL_HFUNC_STR == hfunc)
		hfunc = &hf_default_str;
	h->h = hfunc;
	if (YHASHL_KEYEQ_PTR == keqfunc)
		keqfunc = &keq_default_ptr;
	else if (YHASHL_KEYEQ_STR == keqfunc)
		keqfunc = &keq_default_str;
	h->keq = keqfunc;

	/* allocate and initialize slot list heads */
	h->map = (struct ylistl_link *)ymalloc(
		sizeof(*h->map) * yhashl_hmapsz(h));
	if (unlikely(!h->map)) {
		yfree(h);
		return NULL;
	}
	for (i = 0; i < yhashl_hmapsz(h); i++)
		ylistl_init_link(&h->map[i]);
	return h;
}

struct yhashl *
yhashl_create(yhashl_hfunc_t hfunc, yhashl_keyeq_t keqfunc) {
	return yhashl_create2(hfunc, keqfunc, DEFAULT_HBITS);
}

void
yhashl_destroy(struct yhashl *h) {
	yfree(h->map);
	yfree(h);
}


struct yhashl_node *
yhashl_set(struct yhashl *h, const void *key, struct yhashl_node *nnew) {
	struct yhashl_node *n = yhashl_get(h, key);
	if (n) {
		nnew->hv32 = n->hv32;
		nnew->key = n->key;
		ylistl_replace(&n->lk, &nnew->lk);
		ylistl_init_link(&n->lk);
		return n;
	}
	nnew->hv32 = (*h->h)(key);
	nnew->key = key; /* Always shallow copy */
	/* LRU concept. To find recently added node quickly */
	ylistl_add_first(&h->map[hv(h, nnew)], &nnew->lk);
	h->sz++;
	return NULL;
}

void
yhashl_remove_node(struct yhashl *h, struct yhashl_node *n) {
	ylistl_remove(&n->lk);
	yhashl_init_node(n);
	h->sz--;
}

struct yhashl_node *
yhashl_remove(struct yhashl *h, const void *key) {
	struct yhashl_node *n = yhashl_get(h, key);
	if (unlikely(!n)) return NULL;
	yhashl_remove_node(h, n);
	return n;
}

struct yhashl_node *
yhashl_get(const struct yhashl *h, const void *key) {
	struct yhashl_node *n;
	u32 hv32 = (*h->h)(key);
	struct ylistl_link *hd = &h->map[hv_(h, hv32)];
	ylistl_foreach_item(n, hd, struct yhashl_node, lk) {
		if (n->hv32 == hv32 && !(*h->keq)(key, n->key))
			break;
	}
	return (&n->lk == hd) ? NULL : n;
}
