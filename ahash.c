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

#include "yahash.h"
#include "ylistl.h"

#include "common.h"

#define _MAX_HBITS 32
#define _MIN_HBITS 4

/* hash node */
struct _hn {
	struct ylistl_link    lk;
	uintptr_t	      a;  /* address */
};

struct yahash {
	struct ylistl_link   *map;
	uint32_t	      sz;	   /* hash size */
	uint8_t	              mapbits;	   /* bits of map table = 1<<mapbits */
};

static inline uint32_t
_hmapsz(const struct yahash *h) {
	return (1 << h->mapbits);
}

static inline uint32_t
_hv__(uint32_t mapbits, uintptr_t hv) {
	return hv >> (sizeof(uintptr_t) * 8 - mapbits);
}

static inline uint32_t
_hv_(const struct yahash *h, uintptr_t hv) {
	return _hv__(h->mapbits, hv);
}

static inline uint32_t
_hv(const struct yahash *h, const struct _hn *n) {
	return _hv_(h, n->a);
}

/* Modify hash space to 'bits'*/
static struct yahash *
_hmodify(struct yahash *h, uint32_t bits) {
	int		    i;
	struct _hn	   *n, *tmp;
	struct ylistl_link *oldmap;
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

static struct _hn *
_hfind(const struct yahash *h, uintptr_t a) {
	struct _hn          *n;
	struct ylistl_link  *hd = &h->map[_hv_(h, a)];

	ylistl_foreach_item(n, hd, struct _hn, lk)
		if (n->a == a)
			break;

	return (&n->lk == hd)? NULL: n;
}

static struct _hn *
_ncreate(uintptr_t a) {
	struct _hn *n = ymalloc(sizeof(*n));
	yassert(n);
	n->a = a;
	ylistl_init_link(&n->lk);
	return n;
}

static inline void
_ndestroy(const struct yahash *h, struct _hn *n) {
	yfree(n);
}

struct yahash *
yahash_create(void) {
	int	       i;
	struct yahash *h = ymalloc(sizeof(*h));
	yassert(h);
	h->sz = 0;
	h->mapbits = _MIN_HBITS;
	h->map = (struct ylistl_link*)ymalloc(sizeof(struct ylistl_link)
					      * _hmapsz(h));
	yassert(h->map);
	for (i=0; i<_hmapsz(h); i++)
		ylistl_init_link(&h->map[i]);
	return h;
}

enum yret
yahash_destroy(struct yahash *h) {
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
yahash_sz(const struct yahash *h) {
	return h->sz;
}

struct yahash *
yahash_add(struct yahash *h, void *p) {
	uintptr_t   a = (uintptr_t)p;
	struct _hn *n = _hfind(h, a);

	if (n)
		/* overwrite value */
		yretset(YRWOverwrite);
	else {
		/* we need to expand hash map size if hash seems to be full */
		if (h->sz > _hmapsz(h))
			_hmodify(h, h->mapbits+1);
		n = _ncreate(a);
		ylistl_add_last(&h->map[_hv(h, n)], &n->lk);
		h->sz++;
	}

	return h;
}

struct yahash *
yahash_del(struct yahash *h, void *p) {
	uintptr_t   a = (uintptr_t)p;
	struct _hn *n = _hfind(h, a);
	if (n) {
		ylistl_del(&n->lk);
		_ndestroy(h, n);
		h->sz--;
		if (h->sz < _hmapsz(h) / 4)
			_hmodify(h, h->mapbits-1);
	} else
		yretset(YRWNothing);

	return h;
}

int
yahash_check(const struct yahash *h, void *p) {
	struct _hn *n = _hfind(h, (uintptr_t)p);
	return n? TRUE: FALSE;
}
