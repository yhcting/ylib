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


#include <memory.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include "common.h"
#include "yhash.h"
#include "ylistl.h"
/* crc is used as hash function */
#include "ycrc.h"


#define MAX_HBITS 32
#define MIN_HBITS 4

/* hash node */
struct hn {
	struct ylistl_link lk;
	void *key; /* full key */
	u32 hv32; /* 32bit hash value */
	void *v; /* data value */
};

/* order of struct member has meaning */
struct yhash {
	/* members that may be different according to hash contents */
	struct ylistl_link *map;
	u32 sz; /* hash size */
	u8 mapbits; /* bits of map table = 1<<mapbits */

	/* contents-independent members (hash attributes)
	 * 'mode' SHOULD be top of 'hash attributes'
	 */
        int mode; /* hash mode - DO NOT move to other position */
	void (*vfree)(void *); /* free for hash value */
        void (*kfree)(void *); /* free for hash key */
        /* return copied object */
        int (*kcp)(void **, const void *); /* copy hash key */
        /* return 0 if same, otherwise, non-zero value. */
        int (*kcmp)(const void *, const void *); /* compare two keys */
	u32 (*hfunc)(const void *); /* hash function NOT NULL */
};


/****************************************************************************
 *
 * Predefined functions for hash
 *
 ****************************************************************************/
/*---------------------------------------------------------------------------
 * Common
 *--------------------------------------------------------------------------*/
static void
hfree_default(void *v) {
        if (likely(v))
                yfree(v);
}

static void (*
hfree_get_func(void (*free_func)(void *))) (void *) {
	if (YHASH_PREDEFINED_FREE == free_func)
		return &hfree_default;
	else
		return free_func;
}

/*---------------------------------------------------------------------------
 *
 *--------------------------------------------------------------------------*/
static int
kcp_i(void **ni, const void *i) {
        /* return address value itself */
	*ni = (void *)i;
        return 0;
}

static u32
hfunc_i(const void *k) {
        return (intptr_t)k & 0xffffffff;
}

/**
 * @return NULL for errors or '0' is returned.
 */
static int
kcp_s(void **news, const void *s) {
        size_t sz = strlen((const char *)s);
        void *ns = ymalloc(sz + 1);
        if (unlikely(!ns))
                return -ENOMEM;
        memcpy(ns, s, sz + 1);
        *news = ns;
	return 0;
}

static void
kfree_s(void *s) {
        if (unlikely(s))
                yfree(s);
}

static int
kcmp_s(const void *s0, const void *s1) {
        return strcmp((const char *)s0, (const char *)s1);
}

static u32
hfunc_s(const void *k) {
        return ycrc32(0, (const u8 *)k, (u32)strlen((const char *)k) + 1);
}


/****************************************************************************
 *
 *
 *
 ****************************************************************************/
static INLINE u32
hmapsz(const struct yhash *h) {
	return (1 << h->mapbits);
}

static INLINE u32
hv__(u32 mapbits, u32 hv32) {
	return hv32 >> (32 - mapbits);
}

static INLINE u32
hv_(const struct yhash *h, u32 hv32) {
	return hv__(h->mapbits, hv32);
}

static INLINE u32
hv(const struct yhash *h, const struct hn *n) {
	return hv_(h, n->hv32);
}


/****************************************************************************
 *
 *
 *
 ****************************************************************************/
/*
 * create copy of given key
 * @newkey : variable where newly created key is set.
 */
static int
nkey_create(const struct yhash *h, void **newkey, const void *key) {
	if (h->kcp)
		return (*h->kcp)(newkey, key);
	else
		*newkey = (void *)key;
	return 0;
}

static void
nkey_destroy(const struct yhash *h, struct hn *n) {
	if (h->kfree)
		return (*h->kfree)(n->key);
}

static int
nkey_cmp(const struct yhash *h, const void *k0, const void *k1) {
	if (h->kcmp)
		return (*h->kcmp)(k0, k1);
	else
		return k0 != k1;
}

/****************************************************************************
 *
 *
 *
 ****************************************************************************/
/*
 * 0 for success othersize error number
 */
static int
hmodify(struct yhash *h, u32 bits) {
	u32 i;
	struct hn *n, *tmp;
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
	oldmapsz = hmapsz(h);
	oldbits = h->mapbits;

	h->mapbits = bits; /* map size is changed here */
	if (unlikely(!(h->map = ymalloc(sizeof(*h->map) * hmapsz(h))))) {
		/* restore to original value */
		h->map = oldmap;
		h->mapbits = oldbits;
		return ENOMEM;
	}

	yassert(h->map);
	for (i = 0; i < hmapsz(h); i++)
		ylistl_init_link(&h->map[i]);
	/* re assign hash nodes */
	for (i = 0; i < oldmapsz; i++) {
		ylistl_foreach_item_removal_safe(
			n, tmp, &oldmap[i], struct hn, lk
		) {
			ylistl_remove(&n->lk);
			ylistl_add_last(&h->map[hv(h, n)], &n->lk);
		}
	}
	yfree(oldmap);
	return 0;
}

static struct hn *
hfind(const struct yhash *h, const void *key) {
	struct hn *n;
	u32 hv32 = (*h->hfunc)(key);
	struct ylistl_link *hd = &h->map[hv_(h, hv32)];
	ylistl_foreach_item(n, hd, struct hn, lk) {
		if (n->hv32 == hv32 && !nkey_cmp(h, key, n->key))
			break;
	}
	return (&n->lk == hd)? NULL: n;
}

static INLINE void
vdestroy(const struct yhash *h, void *v) {
	if (h->vfree)
		(*h->vfree)(v);
}

/*
 * return NULL for failure (usually, Out Of Memory.
 */
static struct hn *
ncreate(struct yhash *h, const void *key, void *v) {
	struct hn *n = ymalloc(sizeof(*n));
	if (unlikely(!n))
		return NULL;
	if (unlikely(nkey_create(h, &n->key, key))) {
		yfree(n);
		return NULL;
	}
	n->hv32 = (*h->hfunc)(key);
	n->v = v;
	ylistl_init_link(&n->lk);
	return n;
}

static INLINE void
ndestroy(const struct yhash *h, struct hn *n, int destroyv) {
	nkey_destroy(h, n);
	if (destroyv)
		vdestroy(h, n->v);
	yfree(n);
}

static void
hdestroy_nodes(struct yhash *h) {
	u32 i;
	struct hn *n, *tmp;
	for (i = 0; i < hmapsz(h); i++) {
		ylistl_foreach_item_removal_safe(
			n, tmp, &h->map[i], struct hn, lk
		) {
			ylistl_remove(&n->lk);
			ndestroy(h, n, TRUE);
		}
	}
}

/****************************************************************************
 *
 *
 *
 ****************************************************************************/
static int
hash_init(
	struct yhash *h,
	int mode,
	void (*vfree)(void *),
	void (*kfree)(void *),
	int (*kcp)(void **, const void *),
	int (*kcmp)(const void *, const void *),
	u32 (*hfunc)(const void *)
) {
	u32 i;
        h->mode = mode;
	h->sz = 0;
	h->mapbits = MIN_HBITS;
	h->map = (struct ylistl_link *)ymalloc(sizeof(*h->map) * hmapsz(h));
	if (unlikely(!h->map))
		return -ENOMEM;
	for (i = 0; i < hmapsz(h); i++)
		ylistl_init_link(&h->map[i]);
        h->vfree = hfree_get_func(vfree);
        h->kfree = hfree_get_func(kfree);
        h->kcp = kcp;
        h->kcmp = kcmp;
	h->hfunc = hfunc;
	return 0;
}

static struct yhash *
hash_create_internal(
	int mode,
	void (*vfree)(void *),
	void (*kfree)(void *),
	int (*kcp)(void **, const void *),
	int (*kcmp)(const void *, const void *),
	u32 (*hfunc)(const void *)
) {
	if (unlikely(!hfunc))
		return NULL;

	struct yhash *h = ymalloc(sizeof(*h));
	if (unlikely(!h))
		return NULL;

	if (unlikely(0 > hash_init(
		h,
		mode,
		vfree,
		kfree,
		kcp,
		kcmp,
		hfunc))
	) {
		yfree(h);
		return NULL;
	}
	return h;
}

int
hash_add(
	struct yhash *h,
	const void ** const phkey,
	const void *key,
	void **oldv,
	void *v,
	bool overwrite
) {
	struct hn *n = hfind(h, key);
	if (n) {
		if (!overwrite)
			return -EEXIST;
		/* overwrite value */
		if (oldv)
			*oldv = n->v;
		else
			vdestroy(h, n->v);
		n->v = v;
		return 0;
	}

	/* We need to expand hash map size if hash seems to be full */
	if (h->sz > hmapsz(h))
		/* return value is ignored intentionally.
		 * Actually, even if hmodify fails, hash can still
		 *   continue to add new value.
		 */
		hmodify(h, h->mapbits + 1);
	if (unlikely(!(n = ncreate(h, key, v))))
		return -ENOMEM;
	if (phkey)
		*phkey = n->key;
	ylistl_add_last(&h->map[hv(h, n)], &n->lk);
	h->sz++;
	return 1;
}

/****************************************************************************
 *
 *
 *
 ****************************************************************************/
struct yhash *
yhashi_create(void (*vfree)(void *)) {
        return hash_create_internal(
		YHASH_KEYTYPE_I,
		vfree, /* vfree */
		NULL, /* kfree */
		&kcp_i, /* kcp */
		NULL, /* kcmp */
		&hfunc_i);
}

struct yhash *
yhashs_create(void (*vfree)(void *), bool key_deepcopy) {
        return hash_create_internal(
		YHASH_KEYTYPE_S,
		vfree, /* vfree */
		&kfree_s, /* kfree */
		key_deepcopy ? &kcp_s : NULL, /* kcp */
		&kcmp_s, /* kcmp */
		&hfunc_s);
}

struct yhash *
yhasho_create(
	void (*vfree)(void *),
	void (*keyfree)(void *),
	int (*keycopy)(void **, const void *),
	int (*keycmp)(const void *, const void *),
	u32 (*hfunc)(const void *key)
) {
        return hash_create_internal(
		YHASH_KEYTYPE_O,
		vfree, /* vfree */
		keyfree, /* kfree */
		keycopy, /* kcp */
		keycmp, /* kcmp */
		hfunc);
}

/*---------------------------------------------------------------------------
 *
 *--------------------------------------------------------------------------*/
struct yhash *
yhash_create(const struct yhash *h) {
	return hash_create_internal(
		h->mode,
		h->vfree,
		h->kfree,
		h->kcp,
		h->kcmp,
		h->hfunc);
}

int
yhash_reset(struct yhash *h) {
	hdestroy_nodes(h);
	yfree(h->map);
	return hash_init(
		h,
		h->mode,
		h->vfree,
		h->kfree,
		h->kcp,
		h->kcmp,
		h->hfunc);
}

void
yhash_destroy(struct yhash *h) {
	hdestroy_nodes(h);
	yfree(h->map);
	yfree(h);
}

u32
yhash_sz(const struct yhash *h) {
	return h->sz;
}

bool
yhash_is_sametype(const struct yhash *h0, const struct yhash *h1) {
	size_t modeoff = offsetof(struct yhash, mode);
	return !!memcmp(
		(const char *)h0 + modeoff,
		(const char *)h1 + modeoff,
		sizeof(struct yhash) - modeoff);
}

u32
yhash_keys(const struct yhash *h, const void **keysbuf, u32 bufsz) {
	u32 r, i;
	struct hn *n;
	r = 0;
	for (i = 0; i < hmapsz(h); i++) {
		ylistl_foreach_item(n, &h->map[i], struct hn, lk) {
			if (r < bufsz) {
				keysbuf[r] = n->key;
				++r;
			} else
				return r;
		}
	}
	return r;
}

int
yhash_add(struct yhash *h, const void *key, void *v, int overwrite) {
	return hash_add(h, NULL, key, NULL, v, overwrite);
}

int
yhash_add2(struct yhash *h, const void *key, void **oldv, void *v) {
	return hash_add(h, NULL, key, oldv, v, TRUE);
}

int
yhash_add3(
	struct yhash *h,
	const void ** const phkey,
	void *key,
	void *v,
	bool overwrite
) {
	return hash_add(h, phkey, key, NULL, v, overwrite);
}

int
yhash_remove2(struct yhash *h, void **value, const void *key) {
	int destroyv = TRUE;
	struct hn *n = hfind(h, key);
	if (unlikely(!n))
		return 0;

	ylistl_remove(&n->lk);
	if (value) {
		*value = n->v;
		destroyv = FALSE;
	}
	ndestroy(h, n, destroyv);
	h->sz--;
	if (h->sz < hmapsz(h) / 4)
		/* return value is ignored intentionally.
		 * even if hmodify fails, nothing harmful.
		 */
		hmodify(h, h->mapbits - 1);
	return 1;
}


int
yhash_remove(struct yhash *h, const void *key) {
	return yhash_remove2(h, NULL, key);
}

int
yhash_find(const struct yhash *h, void **value, const void *key) {
	struct hn *n = hfind(h, key);
	if (likely(n)) {
		if (value)
			*value = n->v;
		return 0;
	}
	return -ENOENT;
}
