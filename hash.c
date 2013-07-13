/*****************************************************************************
 *    Copyright (C) 2011 Younghyung Cho. <yhcting77@gmail.com>
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

#include "yhash.h"
#include "ylistl.h"
/* crc is used as hash function */
#include "ycrc.h"

#include "common.h"

#define _MAX_HBITS 32
#define _MIN_HBITS 4

/* hash node */
struct _hn {
	struct ylistl_link    lk;
	uint8_t*	      key;   /* full key */
	uint32_t	      keysz; /* size of key */
	uint32_t	      hv32;  /* 32bit hash value */
	void*		      v;     /* user value */
};

struct yhash {
	struct ylistl_link*   map;
	uint32_t	      sz;	   /* hash size */
	uint8_t	              mapbits;	   /* bits of map table = 1<<mapbits */
	void		    (*fcb)(void*); /* free callback */
};

static inline uint32_t
_hmapsz(const struct yhash* h) {
	return (1 << h->mapbits);
}

static inline uint32_t
_hv__(uint32_t mapbits, uint32_t hv32) {
	return hv32 >> (32 - mapbits);
}

static inline uint32_t
_hv_(const struct yhash* h, uint32_t hv32) {
	return _hv__(h->mapbits, hv32);
}

static inline uint32_t
_hv(const struct yhash* h, const struct _hn* n) {
	return _hv_(h, n->hv32);
}

static inline uint32_t
_hv32(const uint8_t* key, uint32_t keysz) {
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
static struct yhash*
_hmodify(struct yhash* h, uint32_t bits) {
	int		    i;
	struct _hn	   *n, *tmp;
	struct ylistl_link* oldmap;
	uint32_t	    oldmapsz;

	if (bits < _MIN_HBITS)
		bits = _MIN_HBITS;
	if (bits > _MAX_HBITS)
		bits = _MAX_HBITS;

	if (h->mapbits == bits)
		return h; /* nothing to do */

	oldmap = h->map;
	oldmapsz = _hmapsz(h);

	h->mapbits = bits; /* map size is changed here */
	h->map = ymalloc(sizeof(struct ylistl_link) * _hmapsz(h));
	yassert(h->map);
	for (i=0; i<_hmapsz(h); i++)
		ylistl_init_link(&h->map[i]);
	/* re assign hash nodes */
	for (i=0; i<oldmapsz; i++) {
		ylistl_foreach_item_removal_safe(n,
						 tmp,
						 &oldmap[i],
						 struct _hn,
						 lk) {
			ylistl_del(&n->lk);
			ylistl_add_last(&h->map[_hv(h, n)], &n->lk);
		}
	}
	yfree(oldmap);
	return h;
}

static struct _hn*
_hfind(const struct yhash* h, const uint8_t* key, uint32_t keysz) {
	struct _hn*	     n;
	uint32_t	     hv32 = _hv32(key, keysz);
	struct ylistl_link*  hd = &h->map[_hv_(h, hv32)];
	if (keysz) {
		ylistl_foreach_item(n, hd, struct _hn, lk)
			if (keysz == n->keysz
			    && n->hv32 == hv32
			    && 0 == memcmp(key, n->key, keysz))
				break;
	} else {
		/*
		 * special case : 'n->key' value itself is key.
		 */
		ylistl_foreach_item(n, hd, struct _hn, lk)
			if (keysz == n->keysz
			    && key == n->key)
				break;
	}
	return (&n->lk == hd)? NULL: n;
}

static inline void
_vdestroy(const struct yhash* h, void* v) {
	if (h->fcb)
		(*h->fcb)(v);
}


static struct _hn*
_ncreate(const uint8_t* key, uint32_t keysz, void* v) {
	struct _hn* n = ymalloc(sizeof(*n));
	yassert(n);
	if (keysz) {
		n->key = ymalloc(keysz);
		yassert(n->key);
		memcpy(n->key, key, keysz);
	} else
		n->key = (uint8_t*)key;
	n->keysz = keysz;
	n->hv32 = _hv32(key, keysz);
	n->v = v;
	ylistl_init_link(&n->lk);
	return n;
}

static inline void
_ndestroy(const struct yhash* h, struct _hn* n) {
	if (n->keysz)
		yfree(n->key);
	_vdestroy(h, n->v);
	yfree(n);
}

struct yhash*
yhash_create(void(*fcb)(void*)) {
	int	      i;
	struct yhash* h = ymalloc(sizeof(*h));
	yassert(h);
	h->sz = 0;
	h->mapbits = _MIN_HBITS;
	h->map = (struct ylistl_link*)ymalloc(sizeof(struct ylistl_link)
					      * _hmapsz(h));
	yassert(h->map);
	for (i=0; i<_hmapsz(h); i++)
		ylistl_init_link(&h->map[i]);
	h->fcb = fcb;
	return h;
}

enum yret
yhash_destroy(struct yhash* h) {
	int	     i;
	struct _hn  *n, *tmp;
	for (i=0; i<_hmapsz(h); i++) {
		ylistl_foreach_item_removal_safe(n,
						 tmp,
						 &h->map[i],
						 struct _hn,
						 lk) {
			ylistl_del(&n->lk);
			_ndestroy(h, n);
		}
	}
	yfree(h->map);
	yfree(h);
	return YROk;
}

uint32_t
yhash_sz(const struct yhash* h) {
	return h->sz;
}

struct yhash*
yhash_add(struct yhash* h,
	  const uint8_t* key, uint32_t keysz,
	  void* v) {
	struct _hn* n = _hfind(h, key, keysz);

	if (n) {
		/* overwrite value */
		_vdestroy(h, n->v);
		n->v = v;
		yretset(YRWOverwrite);
	} else {
		/* we need to expand hash map size if hash seems to be full */
		if (h->sz > _hmapsz(h))
			_hmodify(h, h->mapbits+1);
		n = _ncreate(key, keysz, v);
		ylistl_add_last(&h->map[_hv(h, n)], &n->lk);
		h->sz++;
	}

	return h;
}

struct yhash*
yhash_del(struct yhash* h,
	  const uint8_t* key, uint32_t keysz) {
	struct _hn* n = _hfind(h, key, keysz);
	if (n) {
		ylistl_del(&n->lk);
		_ndestroy(h, n);
		h->sz--;
		if (h->sz < _hmapsz(h) / 4)
			_hmodify(h, h->mapbits-1);
	} else {
		yretset(YRWNothing);
	}
	return h;
}

void*
yhash_find(const struct yhash* h,
	   const uint8_t* key, uint32_t keysz) {
	struct _hn* n = _hfind(h, key, keysz);
	return n? n->v: NULL;
}
