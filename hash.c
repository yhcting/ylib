/******************************************************************************
 * Copyright (C) 2011, 2012, 2013, 2014
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

#include "ycommon.h"
#include "yhash.h"
#include "ylistl.h"
/* crc is used as hash function */
#include "ycrc.h"


#define MAX_HBITS 32
#define MIN_HBITS 4

/* hash node */
struct hn {
	struct ylistl_link lk;
	void              *key;   /* full key */
	unsigned int	   keysz; /* size of key */
	uint32_t	   v32;   /* 32bit hash value */
	void              *v;     /* user value */
};

struct yhash {
	struct ylistl_link *map;
	unsigned int	    sz;	   /* hash size */
	uint8_t	            mapbits;	   /* bits of map table = 1<<mapbits */
	void		  (*fcb)(void *);/* free callback */
};

static inline unsigned int
hmapsz(const struct yhash *h) {
	return (1 << h->mapbits);
}

static inline unsigned int
hv__(unsigned int mapbits, uint32_t v32) {
	return v32 >> (32 - mapbits);
}

static inline unsigned int
hv_(const struct yhash *h, uint32_t v32) {
	return hv__(h->mapbits, v32);
}

static inline unsigned int
hv(const struct yhash *h, const struct hn *n) {
	return hv_(h, n->v32);
}

static inline uint32_t
hv32(const void *key, unsigned int keysz) {
	if (keysz)
		return ycrc32(0, key, keysz);
	else
		/*
		 * special case
		 * To improve portability, 'uint64_t' is used.
		 */
		return (uint32_t)((uintptr_t)key & (uintptr_t)0xffffffff);
}

/*
 * 0 for success othersize error number
 */
static int
hmodify(struct yhash *h, unsigned int bits) {
	unsigned int	    i;
	struct hn	   *n, *tmp;
	struct ylistl_link *oldmap;
	unsigned int	    oldmapsz;
	unsigned int        oldbits;

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
		ylistl_foreach_item_removal_safe(n,
						 tmp,
						 &oldmap[i],
						 struct hn,
						 lk) {
			ylistl_del(&n->lk);
			ylistl_add_last(&h->map[hv(h, n)], &n->lk);
		}
	}
	yfree(oldmap);
	return 0;
}

static struct hn *
hfind(const struct yhash *h, const void *key, unsigned int keysz) {
	struct hn          *n;
	uint32_t	    v32 = hv32(key, keysz);
	struct ylistl_link *hd = &h->map[hv_(h, v32)];
	if (keysz) {
		ylistl_foreach_item(n, hd, struct hn, lk)
			if (keysz == n->keysz
			    && n->v32 == v32
			    && 0 == memcmp(key, n->key, keysz))
				break;
	} else {
		/*
		 * special case : 'n->key' value itself is key.
		 */
		ylistl_foreach_item(n, hd, struct hn, lk)
			if (keysz == n->keysz
			    && key == n->key)
				break;
	}
	return (&n->lk == hd)? NULL: n;
}

static inline void
vdestroy(const struct yhash *h, void *v) {
	if (h->fcb && v)
		(*h->fcb)(v);
}

/*
 * return NULL for failure (usually, Out Of Memory.
 */
static struct hn *
ncreate(const void *key, unsigned int keysz, void *v) {
	struct hn *n = ymalloc(sizeof(*n));
	if (unlikely(!n))
		return NULL;
	if (likely(keysz)) {
		if (unlikely(!(n->key = ymalloc(keysz)))) {
			yfree(n);
			return NULL;
		}
		memcpy(n->key, key, keysz);
	} else
		/* 'const' qualifier is discarded here intentionally.
		 * 'key' is NOT pointer here. It is value.
		 * So, const qualifier has no meaning
		 */
		n->key = (void *)key;
	n->keysz = keysz;
	n->v32 = hv32(key, keysz);
	n->v = v;
	ylistl_init_link(&n->lk);
	return n;
}

static inline void
ndestroy(const struct yhash *h, struct hn *n) {
	if (n->keysz)
		yfree(n->key);
	vdestroy(h, n->v);
	yfree(n);
}

static inline void
hclean_nodes(struct yhash *h) {
	unsigned int i;
	struct hn  *n, *tmp;
	for (i = 0; i < hmapsz(h); i++) {
		ylistl_foreach_item_removal_safe(n,
						 tmp,
						 &h->map[i],
						 struct hn,
						 lk) {
			ylistl_del(&n->lk);
			ndestroy(h, n);
		}
	}
}

int
yhash_init(struct yhash *h, void (*fcb)(void *)) {
	unsigned int i;
	h->sz = 0;
	h->mapbits = MIN_HBITS;
	h->map = (struct ylistl_link *)ymalloc(sizeof(*h->map) * hmapsz(h));
	if (unlikely(!h->map))
		return ENOMEM;
	for (i = 0; i < hmapsz(h); i++)
		ylistl_init_link(&h->map[i]);
	h->fcb = fcb;
	return 0;
}

struct yhash *
yhash_create(void(*fcb)(void *)) {
	struct yhash *h = ymalloc(sizeof(*h));
	if (unlikely(!h))
		return NULL;
	if (unlikely(yhash_init(h, fcb))) {
		yfree(h);
		return NULL;
	}
	return h;
}

void
yhash_clean(struct yhash *h) {
	hclean_nodes(h);
	yfree(h->map);
	yhash_init(h, h->fcb);
}

void
yhash_destroy(struct yhash *h) {
	hclean_nodes(h);
	yfree(h->map);
	yfree(h);
}

unsigned int
yhash_sz(const struct yhash *h) {
	return h->sz;
}

unsigned int
yhash_keys(const struct yhash *h,
	   const void **keysbuf,
	   unsigned int *keysszbuf,
	   unsigned int bufsz) {
	unsigned int r, i;
	struct hn *n;
	r = 0;
	for (i = 0; i < hmapsz(h); i++) {
		ylistl_foreach_item(n,
				    &h->map[i],
				    struct hn,
				    lk) {
			if (r < bufsz) {
				keysbuf[r] = n->key;
				if (keysszbuf)
					keysszbuf[r] = n->keysz;
				++r;
			} else
				return r;
		}
	}
	return r;
}

int
yhash_add2(struct yhash *h,
	   const void ** const pkey,
	   const void *key, unsigned int keysz,
	  void *v) {
	struct hn *n = hfind(h, key, keysz);
	if (n) {
		/* overwrite value */
		vdestroy(h, n->v);
		n->v = v;
		return 0;
	}

	/* we need to expand hash map size if hash seems to be full */
	if (h->sz > hmapsz(h))
		/* return value is ignored intentionally.
		 * Actually, even if hmodify fails, hash can still
		 *   continue to add new value.
		 */
		hmodify(h, h->mapbits + 1);
	n = ncreate(key, keysz, v);
	if (unlikely(!n))
		return -1;
	if (pkey)
		*pkey = n->key;
	ylistl_add_last(&h->map[hv(h, n)], &n->lk);
	h->sz++;
	return 1;
}

int
yhash_add(struct yhash *h,
	  const void *key, unsigned int keysz,
	  void *v) {
	return yhash_add2(h, NULL, key, keysz, v);
}

int
yhash_del2(struct yhash *h,
	   void **value,
	   const void *key, unsigned int keysz) {
	struct hn *n = hfind(h, key, keysz);
	if (!n)
		return 0;

	ylistl_del(&n->lk);
	if (value) {
		*value = n->v;
		n->v = NULL; /* to avoid freeing data in 'ndestroy' */
	}
	ndestroy(h, n);
	h->sz--;
	if (h->sz < hmapsz(h) / 4)
		/* return value is ignored intentionally.
		 * even if hmodify fails, nothing harmful.
		 */
		hmodify(h, h->mapbits - 1);
	return 1;
}


int
yhash_del(struct yhash *h,
	  const void *key, unsigned int keysz) {
	return yhash_del2(h, NULL, key, keysz);
}

void *
yhash_find(const struct yhash *h,
	   const void *key, unsigned int keysz) {
	struct hn *n = hfind(h, key, keysz);
	return n? n->v: NULL;
}
