/*****************************************************************************
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



#ifndef __YDYNb_h__
#define __YDYNb_h__

#include <stdint.h>
#include <memory.h>
#include <string.h>

#include "ydef.h"
#include "yret.h"

/* DYNmaic Buffer */
typedef struct ydynb {
	uint32_t  limit;
	uint32_t  sz;
	uint8_t  *b;
};

static inline uint32_t
ydynb_limit(const struct ydynb *b) {
	return b->limit;
}

static inline uint32_t
ydynb_freesz(const struct ydynb *b) {
	return b->limit - b->sz;
}

static inline uint32_t
ydynb_sz(const struct ydynb *b) {
	return b->sz;
}

static inline uint8_t *
ydynb_buf(const struct ydynb *b) {
	return b->b;
}

static inline uint8_t *
ydynb_ptr(const struct ydynb *b) {
	return b->b + b->sz;
}

/*
 * @return: 0 if success.
 */
static inline enum yret
ydynb_init(struct ydynb *b, uint32_t init_limit) {
	b->sz = 0;
	b->b = (uint8_t *)ymalloc(init_limit);
	if (b->b) {
		b->limit = init_limit;
		return YROk;
	} else {
		b->limit = 0;
		return YREOOM;
	}
}

static inline void
ydynb_reset(struct ydynb *b) {
	b->sz = 0;
}

static inline void
ydynb_clean(struct ydynb *b) {
	if (b->b)
		yfree(b->b);
	b->b = NULL;
	b->limit = b->sz = 0;
}

/*
 * increase buffer size by two times.
 * due to using memcpy, it cannot be static inline
 * @return: <0 if fails.
 */
static inline enum yret
ydynb_expand(struct ydynb *b) {
	uint8_t *tmp = (uint8_t *)ymalloc(b->limit*2);
	if (tmp) {
		memcpy(tmp, b->b, b->sz);
		yfree(b->b);
		b->b = tmp;
		b->limit *= 2;
		return YROk;
	} else
		return YREOOM;
}

static inline int
ydynb_secure(struct ydynb *b, uint32_t sz_required) {
	while ( sz_required > ydynb_freesz(b)
		&& (YROk == ydynb_expand(b)) ) {}
	return sz_required <= ydynb_freesz(b);
}


static inline enum yret
ydynb_shrink(struct ydynb *b, uint32_t sz_to) {
	if ( b->limit > sz_to  && b->sz < sz_to ) {
		uint8_t *tmp = (uint8_t *)ymalloc(sz_to);
		if (tmp) {
			yassert(b->b);
			memcpy(tmp, b->b, b->sz);
			yfree(b->b);
			b->b = tmp;
			b->limit = sz_to;
			return YROk;
		}
	}
	return YREInvalid_param;
}

static inline enum yret
ydynb_append(struct ydynb *b, const uint8_t *d, uint32_t dsz) {
	enum yret r = ydynb_secure(b, dsz);
	if (0 > r)
		return r;
	memcpy(ydynb_ptr(b), d, dsz);
	b->sz += dsz;
	return YROk;
}

#endif /* __YDYNb_h__ */
