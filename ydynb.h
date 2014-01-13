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



#ifndef __YDYNb_h__
#define __YDYNb_h__

#include <stdint.h>
#include <memory.h>
#include <string.h>
#include <errno.h>

#include "ydef.h"

/* DYNmaic Buffer - White box structure. */
struct ydynb {
	uint32_t  limit;
	uint32_t  sz;
	void     *b;
};

/**
 * @return: 0 if success otherwise error number.
 */
static inline int
ydynb_init(struct ydynb *b, uint32_t init_limit) {
	b->sz = 0;
	b->b = ymalloc(init_limit);
	if (unlikely(!b->b))
		return ENOMEM;
	b->limit = init_limit;
	return 0;
}

/**
 * @return: NULL if fails (usually, Out Of Memory)
 */
static inline struct ydynb *
ydynb_create(uint32_t init_limit) {
	struct ydynb *b = (struct ydynb *)ymalloc(sizeof(*b));
	if (likely(!ydynb_init(b, init_limit)))
		return b;
	yfree(b);
	return NULL;
}

static inline void
ydynb_clean(struct ydynb *b) {
	if (likely(b->b))
		yfree(b->b);
	b->b = NULL;
	b->limit = b->sz = 0;
}

/**
 * @return: 0 if success, otherwise error number.
 */
static inline int
ydynb_reset(struct ydynb *b, uint32_t init_limit) {
	if (unlikely(b->limit == init_limit)) {
		b->sz = 0;
		return 0;
	}
	ydynb_clean(b);
	return ydynb_init(b, init_limit);
}

static inline void
ydynb_destroy(struct ydynb *b) {
	ydynb_clean(b);
	yfree(b);
}

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

static inline void *
ydynb_buf(const struct ydynb *b) {
	return b->b;
}

static inline void *
ydynb_cur(const struct ydynb *b) {
	return (void *)(((char *)b->b) + b->sz);
}

/**
 * increase buffer size by two times.
 * due to using memcpy, it cannot be static inline
 * @return: 0 if success, otherwise error number.
 */
static inline int
ydynb_expand(struct ydynb *b) {
	void *tmp = ymalloc(b->limit * 2);
	if (unlikely(!tmp))
		return ENOMEM;
	memcpy(tmp, b->b, b->sz);
	yfree(b->b);
	b->b = tmp;
	b->limit *= 2;
	return 0;
}

/**
 * @return: TRUE if free size is larger than required, otherwise FALSE
 */
static inline int
ydynb_secure(struct ydynb *b, uint32_t sz_required) {
	while (sz_required > ydynb_freesz(b)
	       && (!ydynb_expand(b))) {}
	return sz_required <= ydynb_freesz(b);
}


/**
 * @return: 0 if success, otherwise error number
 */
static inline int
ydynb_shrink(struct ydynb *b, uint32_t sz_to) {
	void *tmp;
	if (unlikely(b->limit <= sz_to
		     || b->sz > sz_to))
		return EINVAL;

	tmp = ymalloc(sz_to);
	if (unlikely(!tmp))
		return ENOMEM;

	yassert(b->b);
	memcpy(tmp, b->b, b->sz);
	yfree(b->b);
	b->b = tmp;
	b->limit = sz_to;
	return 0;
}

static inline int
ydynb_append(struct ydynb *b, const void *d, uint32_t dsz) {
	int r = ydynb_secure(b, dsz);
	if (unlikely(!r))
		return ENOMEM;
	memcpy(ydynb_cur(b), d, dsz);
	b->sz += dsz;
	return 0;
}

#endif /* __YDYNb_h__ */
