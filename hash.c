/******************************************************************************
 *    Copyright (C) 2011, 2012, 2013, 2014
 *    Younghyung Cho. <yhcting77@gmail.com>
 *
 *    This file is part of ylib
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as
 *    published by the Free Software Foundation either version 3 of the
 *    License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License
 *    (<http://www.gnu.org/licenses/lgpl.html>) for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.	If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/


#include <memory.h>
#include <string.h>
#include <errno.h>

#include "yhash.h"
#include "ylistl.h"
/* crc is used as hash function */
#include "ycrc.h"

#include "common.h"

#define MAX_HBITS 32
#define MIN_HBITS 4

/* hash node */
struct hn {
	struct ylistl_link lk;
	void              *key;   /* full key */
	uint32_t	   keysz; /* size of key */
	uint32_t	   v32;  /* 32bit hash value */
	void              *v;     /* user value */
};

struct yhash {
	struct ylistl_link *map;
	uint32_t	    sz;	   /* hash size */
	uint8_t	            mapbits;	   /* bits of map table = 1<<mapbits */
	void		  (*fcb)(void *);/* free callback */
};

static inline uint32_t
hmapsz(const struct yhash *h) {
	return (1 << h->mapbits);
}

static inline uint32_t
hv__(uint32_t mapbits, uint32_t v32) {
	return v32 >> (32 - mapbits);
}

static inline uint32_t
hv_(const struct yhash *h, uint32_t v32) {
	return hv__(h->mapbits, v32);
}

static inline uint32_t
hv(const struct yhash *h, const struct hn *n) {
	return hv_(h, n->v32);
}

static inline uint32_t
hv32(const void *key, uint32_t keysz) {
	if (keysz)
		return ycrc32(0, key, keysz);
	else
		/*
		 * special case
		 * To improve portability, 'uint64_t' is used.
		 */
		return (uint32_t)((uintptr_t)key & (uintptr_t)0xffffffff);
}

/* Modify hash space to 'bits'*/
static struct yhash *
hmodify(struct yhash *h, uint32_t bits) {
	int		    i;
	struct hn	   *n, *tmp;
	struct ylistl_link *oldmap;
	uint32_t	    oldmapsz;

	if (bits < MIN_HBITS)
		bits = MIN_HBITS;
	if (bits > MAX_HBITS)
		bits = MAX_HBITS;

	if (h->mapbits == bits)
		return h; /* nothing to do */

	oldmap = h->map;
	oldmapsz = hmapsz(h);

	h->mapbits = bits; /* map size is changed here */
	h->map = ymalloc(sizeof(*h->map) * hmapsz(h));
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
	return h;
}

static struct hn *
hfind(const struct yhash *h, const void *key, uint32_t keysz) {
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
	if (h->fcb)
		(*h->fcb)(v);
}


static struct hn *
ncreate(const void *key, uint32_t keysz, void *v) {
	struct hn *n = ymalloc(sizeof(*n));
	yassert(n);
	if (keysz) {
		n->key = ymalloc(keysz);
		yassert(n->key);
		memcpy(n->key, key, keysz);
	} else
		n->key = (uint8_t *)key;
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

int
yhash_init(struct yhash *h, void (*fcb)(void *)) {
	int i;
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
	int	    i;
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
	yfree(h->map);
}

void
yhash_destroy(struct yhash *h) {
	yhash_clean(h);
	yfree(h);
}

uint32_t
yhash_sz(const struct yhash *h) {
	return h->sz;
}

uint32_t
yhash_keys(const struct yhash *h,
	   const void **keysbuf,
	   uint32_t *keysszbuf,
	   uint32_t bufsz) {
	uint32_t    r, i;
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
yhash_add(struct yhash *h,
	  const void *key, uint32_t keysz,
	  void *v) {
	struct hn *n = hfind(h, key, keysz);

	if (n) {
		/* overwrite value */
		vdestroy(h, n->v);
		n->v = v;
		return 0;
	} else {
		/* we need to expand hash map size if hash seems to be full */
		if (h->sz > hmapsz(h))
			hmodify(h, h->mapbits + 1);
		n = ncreate(key, keysz, v);
		ylistl_add_last(&h->map[hv(h, n)], &n->lk);
		h->sz++;
		return 1;
	}
}

int
yhash_del(struct yhash *h,
	  const void *key, uint32_t keysz) {
	struct hn *n = hfind(h, key, keysz);
	if (n) {
		ylistl_del(&n->lk);
		ndestroy(h, n);
		h->sz--;
		if (h->sz < hmapsz(h) / 4)
			hmodify(h, h->mapbits - 1);
		return 1;
	} else
		return 0;
}

void *
yhash_find(const struct yhash *h,
	   const void *key, uint32_t keysz) {
	struct hn *n = hfind(h, key, keysz);
	return n? n->v: NULL;
}
