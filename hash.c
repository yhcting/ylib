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
	u32	           keysz; /* size of key */
	u32	           hv32;  /* 32bit hash value */
	void              *v;     /* user value */
};

struct yhash {
	struct ylistl_link *map;
	u32	            sz;      /* hash size */
	u8	            mapbits; /* bits of map table = 1<<mapbits */
	void		  (*fcb)(void *); /* free callback */
	u32               (*hfunc)(const void *, u32); /* hash function */
};


/****************************************************************************
 * Predefined hash functions
 ****************************************************************************/
static u32
hfunc_crc32(const void *key, u32 keysz) {
	return ycrc32(0, (const u8 *)key, keysz);
}

static u32
hfunc_int(const void *key, u32 keysz) {
	yassert(keysz >= sizeof(u32));
	return *(u32 *)key & 0xffffffff;
}


/****************************************************************************
 *
 ****************************************************************************/
static inline u32
hmapsz(const struct yhash *h) {
	return (1 << h->mapbits);
}

static inline u32
hv__(u32 mapbits, u32 hv32) {
	return hv32 >> (32 - mapbits);
}

static inline u32
hv_(const struct yhash *h, u32 hv32) {
	return hv__(h->mapbits, hv32);
}

static inline u32
hv(const struct yhash *h, const struct hn *n) {
	return hv_(h, n->hv32);
}


/****************************************************************************
 *
 ****************************************************************************/
static inline const void *
nkey(const struct hn *n) {
	return n->keysz > sizeof(void *)? n->key: (void *)&n->key;
}

/*
 * create clone(deep-copy) of given key
 * @newkey : variable where newly created key is set.
 */
static int
nkey_create(void **newkey, const void *key, u32 keysz) {
	void *nkey;
	if (keysz > sizeof(void *)) {
		if (unlikely(!(nkey = ymalloc(keysz))))
			return ENOMEM;
		memcpy(nkey, key, keysz);
	} else
		/* keysz is smaller than size of pointer.
		 * Then, whole key value can be stored at 'key' pointer.
		 * (memory allocation is not required.)
		 */
		memcpy((void *)&nkey, key, keysz);
	*newkey = nkey;
	return 0;
}

static void
nkey_destroy(struct hn *n) {
	/* if keysz is smaller than size of pointer,
	 * memory is NOT allocated to store key.
	 * See 'nkey_create' function for details
	 */
	if (n->keysz > sizeof(void *))
		yfree(n->key);
}


/****************************************************************************
 *
 ****************************************************************************/
/*
 * 0 for success othersize error number
 */
static int
hmodify(struct yhash *h, u32 bits) {
	u32	            i;
	struct hn	   *n, *tmp;
	struct ylistl_link *oldmap;
	u32	            oldmapsz;
	u32                 oldbits;

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
hfind(const struct yhash *h, const void *key, u32 keysz) {
	struct hn          *n;
	u32	            hv32 = (*h->hfunc)(key, keysz);
	struct ylistl_link *hd = &h->map[hv_(h, hv32)];
	ylistl_foreach_item(n, hd, struct hn, lk) {
		if (keysz == n->keysz
		    && n->hv32 == hv32
		    && !memcmp(key, nkey(n), keysz))
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
ncreate(struct yhash *h,
	const void *key,
	u32 keysz,
	void *v) {
	struct hn *n = ymalloc(sizeof(*n));
	if (unlikely(!n))
		return NULL;
	if (unlikely(nkey_create(&n->key, key, keysz))) {
		yfree(n);
		return NULL;
	}
	n->keysz = keysz;
	n->hv32 = (*h->hfunc)(key, keysz);
	n->v = v;
	ylistl_init_link(&n->lk);
	return n;
}

static inline void
ndestroy(const struct yhash *h, struct hn *n) {
	nkey_destroy(n);
	vdestroy(h, n->v);
	yfree(n);
}

static inline void
hclean_nodes(struct yhash *h) {
	u32 i;
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

/****************************************************************************
 *
 ****************************************************************************/
static int
hash_init(struct yhash *h,
	  void (*fcb)(void *),
	  u32 (*hfunc)(const void *, u32)) {
	u32 i;
	h->sz = 0;
	h->mapbits = MIN_HBITS;
	h->map = (struct ylistl_link *)ymalloc(sizeof(*h->map) * hmapsz(h));
	if (unlikely(!h->map))
		return ENOMEM;
	for (i = 0; i < hmapsz(h); i++)
		ylistl_init_link(&h->map[i]);
	h->hfunc = hfunc;
	h->fcb = fcb;
	return 0;
}

static struct yhash *
hash_create_internal(void (*fcb)(void *), u32 (*hfunc)(const void *, u32)) {
	struct yhash *h = ymalloc(sizeof(*h));
	if (unlikely(!h))
		return NULL;

	if (unlikely(hash_init(h, fcb, hfunc))) {
		yfree(h);
		return NULL;
	}
	return h;
}


/****************************************************************************
 *
 ****************************************************************************/
struct yhash *
yhash_create(void (*fcb)(void *)) {
	return hash_create_internal(fcb, &hfunc_crc32);
}

struct yhash *
yhash_create2(void (*fcb)(void *), int hfunc_type) {
	switch (hfunc_type) {
	case HASH_FUNC_CRC:
		return hash_create_internal(fcb, &hfunc_crc32);
	case HASH_FUNC_INT:
		return hash_create_internal(fcb, &hfunc_int);
	default:
		return NULL;
	}
}

struct yhash *
yhash_create3(void (*fcb)(void *), u32 (*hfunc)(const void *, u32)) {
	return hash_create_internal(fcb, hfunc);
}

void
yhash_clean(struct yhash *h) {
	hclean_nodes(h);
	yfree(h->map);
	hash_init(h, h->fcb, h->hfunc);
}

void
yhash_destroy(struct yhash *h) {
	hclean_nodes(h);
	yfree(h->map);
	yfree(h);
}

u32
yhash_sz(const struct yhash *h) {
	return h->sz;
}

u32
yhash_keys(const struct yhash *h,
	   const void **keysbuf,
	   u32 *keysszbuf,
	   u32 bufsz) {
	u32 r, i;
	struct hn *n;
	r = 0;
	for (i = 0; i < hmapsz(h); i++) {
		ylistl_foreach_item(n,
				    &h->map[i],
				    struct hn,
				    lk) {
			if (r < bufsz) {
				keysbuf[r] = nkey(n);
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
	   const void *key, u32 keysz,
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
	n = ncreate(h, key, keysz, v);
	if (unlikely(!n))
		return -1;
	if (pkey)
		*pkey = nkey(n);
	ylistl_add_last(&h->map[hv(h, n)], &n->lk);
	h->sz++;
	return 1;
}

int
yhash_add(struct yhash *h,
	  const void *key, u32 keysz,
	  void *v) {
	return yhash_add2(h, NULL, key, keysz, v);
}

int
yhash_del2(struct yhash *h,
	   void **value,
	   const void *key, u32 keysz) {
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
	  const void *key, u32 keysz) {
	return yhash_del2(h, NULL, key, keysz);
}

void *
yhash_find(const struct yhash *h,
	   const void *key, u32 keysz) {
	struct hn *n = hfind(h, key, keysz);
	return n? n->v: NULL;
}
