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

#include "common.h"

#include <string.h>
#include <malloc.h>
#include <pthread.h>
#include <stdio.h>

/*
 * fbp memory strip (FbpMS)
 * ------------------------
 *
 *     * U(used) / F(free)
 *
 *                fbp
 *             +-------+
 *             |   F   | <- index [mp_sz()-1]
 *             +-------+
 *             |   F   |
 *             +-------+
 *             |  ...  |
 *             +-------+
 *             |   F   |
 *             +-------+
 *      fbi -> |   F   |
 *             +-------+
 *             |   U   |
 *             +-------+
 *             |  ...  |
 *             +-------+
 *             |   U   |
 *             +-------+
 *             |   U   | <- index [0]
 *             +-------+
 *
 *
 * fbp format
 * ----------
 *
 *         0   1   2   <- group index
 *       +---+---+---+ ...
 *       | F | F | F |
 *       | b | b | b |
 *       | p | p | p | ...
 *       | M | M | M |
 *       | S | S | S |
 *       |...|...|...|
 *       +---+---+---+ ...
 *
 *
 * memory block group structure
 * ----------------------------
 *
 * Similar with 'fbp format'
 *
 *
 * Expanding Algorithm
 * -------------------
 *
 * Add additional strip to fbp and block group.
 *
 *
 * Shrinking
 * ---------
 *
 * Actually, shrinking with enough performance is very difficult.
 * Consider it later!
 *
 */

struct _dummy {
	void* dummy;
};

struct _blk {
	/*
	 * 'i' is required to tracking used block.
	 * If sw don't need to track used block,
	 *   this variable 'i' is not required.
	 * So, we can save memory space.
	 * (Actually, writer don't need to track used block.
	 *  But for reusable module, this is implemented at this time.)
	 *
	 * [!NOTE!]
	 * Why 'struct {} d' is used?
	 * Data format of this structure is
	 *
	 * requrested memory address
	 *         |
	 *         v
	 *     +---+--------------------------+
	 *     | i | requested memory address |
	 *     +---+--------------------------+
	 *
	 * So, returned address should be aligned one.
	 * And usually, this is aligned by sizeof(void*)
	 */
	int           i; /* index of free block pointer */
	struct _dummy d;
};

struct ymp {
	unsigned char**   grp;    /* groups of blocks */
	int               grpsz;  /* size of grp - number of element in group*/
	int               nrgrp;  /* number of group allocated */
	struct _blk***    fbp;    /* Free Block Pointer group */
	int               esz;    /* Element SiZe */
	int               fbi;    /* Free Block Index */
	pthread_mutex_t   m;      /* Mutex for MT safety */
};


/******************************************************************************
 * Intenal functions
 *****************************************************************************/

static inline void
_init_lock(struct ymp* mp) {
	pthread_mutex_init(&mp->m, NULL);
}

static inline void
_lock(struct ymp* mp) {
	if (pthread_mutex_lock(&mp->m))
		yassert(0);
}

static inline void
_unlock(struct ymp* mp) {
	if (pthread_mutex_unlock(&mp->m))
		yassert(0);
}

static inline void
_destroy_lock(struct ymp* mp) {
	pthread_mutex_destroy(&mp->m);
}

static inline int
_align_adjust(struct ymp* mp) {
	return sizeof(void*) - (mp->esz % sizeof(void*));
}

static inline int
_esz(struct ymp* mp) {
	return mp->esz + _align_adjust(mp);
}

static inline int
_blksz(struct ymp* mp) {
	return sizeof(struct _blk) - sizeof(struct _dummy) + _esz(mp);
}

/*
 * @i    : index of fbp (ex. fbi)
 */
static inline struct _blk*
_fbpblk(struct ymp* mp, int i) {
	return *(mp->fbp[i / mp->grpsz] + i % mp->grpsz);
}

/*
 * @i    : index of memory block pool.
 */
static inline struct _blk*
_blk(struct ymp* mp, int i) {
	return (struct _blk*)(mp->grp[i / mp->grpsz]
			      + i % mp->grpsz * _blksz(mp));
}


static inline int
_is_freeblk(struct ymp* mp, struct _blk* b) {
	return (mp->fbi <= b->i);
}

static inline int
_is_usedblk(struct ymp* mp, struct _blk* b) {
	return !_is_freeblk(mp, b);
}

/*
 * @i    : index of fbp (ex. fbi)
 */
static inline void
_setfbp(struct ymp* mp, int i, struct _blk* b) {
	*(mp->fbp[i / mp->grpsz] + i % mp->grpsz) = b;
}

static inline int
_sz(struct ymp* mp) {
	return mp->nrgrp * mp->grpsz;
}

static inline int
_usedsz(struct ymp* mp) {
	return mp->fbi;
}

/*
 * expand memory pool by 1 group
 */
static void
_expand(struct ymp* mp) {
	int             i;
	unsigned char** newgrp;
	struct _blk***  newfbp;
	int             blksz;

	/* pre-calulate frequently used value */
	blksz = _blksz(mp);

	newgrp = ymalloc(sizeof(*newgrp) * (mp->nrgrp + 1));
	newfbp = ymalloc(sizeof(*newfbp) * (mp->nrgrp + 1));
	yassert(newgrp && newfbp);

	/* allocate new fbp group */
	newfbp[mp->nrgrp] = ymalloc(sizeof(**newfbp) * mp->grpsz);
	/* allocate new block group */
	newgrp[mp->nrgrp] = ymalloc(sizeof(**newgrp)
				    * mp->grpsz
				    * blksz);
	yassert(newfbp[mp->nrgrp] && newgrp[mp->nrgrp]);

	/* initialize fbp & block group */
	for (i = 0; i < mp->grpsz; i++) {
		newfbp[mp->nrgrp][i]
			= (struct _blk*)newgrp[mp->nrgrp] + i * blksz;
		newfbp[mp->nrgrp][i]->i = mp->nrgrp * mp->grpsz + i;
	}

	/* keep previous values */
	memcpy(newgrp, mp->grp, mp->nrgrp * sizeof(*newgrp));
	memcpy(newfbp, mp->fbp, mp->nrgrp * sizeof(*newfbp));

	/* update mp structure */
	yfree(mp->grp);
	yfree(mp->fbp);
	mp->grp = newgrp;
	mp->fbp = newfbp;
	mp->nrgrp++;
}

#define _find_next_blk(func)			\
	int          i;				\
	struct _blk* b;				\
	for (i = *idx; i < _sz(mp); i++) {	\
		b = _blk(mp, i);		\
		if (func(mp, b)) {		\
			*idx = i + 1;		\
			return b;		\
		}				\
	}					\
	return NULL;

static struct _blk*
_next_used(struct ymp* mp, int* idx /* inout */) {
	_find_next_blk(_is_usedblk);
}

static struct _blk*
_next_free(struct ymp* mp, int* idx /* inout */) {
	_find_next_blk(_is_freeblk);
}

#undef _find_next_blk

/*
 * Policy is not checked here.
 * This is pure implementation.
 * (Performance is not considered yet.)
 *
 * Algorithm
 * ---------
 *
 * block pool
 *
 * index:0            index:_usedsz()
 *     |                  |
 *     v                  v
 *    +--+-----+---+-----+--+-----+---+------
 *    |  | ... | F | ... |  | ... | U |
 *    +--+-----+---+-----+--+-----+---+-----
 *               ^       |          ^
 *               |       |          |
 *               +--------- switch -+
 *                       |
 *                       |
 *                       |-> start to search used block from here
 *                       |   (fragmented block)
 *
 */
static void
_defragment(struct ymp* mp) {
	int          bi1, bi2;
	struct _blk *b1, *b2;

	bi1 = 0;
	/* find first fragmented used block */
	bi2 = _usedsz(mp);
	while ( NULL != (b2 = _next_used(mp, &bi2))) {
		b1 = _next_free(mp, &bi1);
		yassert(bi1 <= _usedsz(mp));
		/* switch block */
		memcpy(b1, b2, _blksz(mp));
		_setfbp(mp, b1->i, b2);
		_setfbp(mp, b2->i, b1);
	}
}

static void
_shrink(struct ymp* mp) {
	int     from, /* shrink from - index */
		i;

	_defragment(mp);

	/*
	 * Try to free all free block strip.
	 * 'mp->fbi' itself indicates 'Free block'.
	 * So, '-1' is applied.
	 * '+1' for free-block-strip.
	 */
	from = (mp->fbi - 1) / mp->grpsz + 1;
	for (i = from; i < mp->nrgrp; i++) {
		/*
		 * we don't need to shrink mp->fbp/mp->grp buffer itself.
		 * this is not big deal in terms of memory usage.
		 * so, free only allocated fbp strip.
		 */
		yfree(mp->fbp[i]);
		yfree(mp->grp[i]);
	}
	/* number of group is shrinked */
	mp->nrgrp = from;
}

/******************************************************************************
 * Interface Functions
 *****************************************************************************/

struct ymp*
ymp_create(int grpsz, int elemsz) {
	struct ymp* mp;

	yassert(grpsz > 0 && elemsz > 0);
	mp = ymalloc(sizeof(*mp));
	mp->grp = ymalloc(sizeof(*mp->grp));
	mp->grpsz = grpsz;
	mp->nrgrp = 0;
	mp->fbp = ymalloc(sizeof(*mp->fbp));
	mp->esz = elemsz;
	mp->fbi = 0;
	_init_lock(mp);

	/* allocate 1-block-group for initial state */
	_expand(mp);

	return mp;
}

void
ymp_destroy(struct ymp* mp) {
	int i;
	_destroy_lock(mp);
	yfree(mp->fbp);
	for (i = 0; i < mp->nrgrp; i++)
		yfree(mp->grp[i]);
	yfree(mp->grp);
	yfree(mp);
}

/*
 * get one block from pool.
 */
void*
ymp_get(struct ymp* mp) {
	struct _blk* b;

	_lock(mp);
	if (mp->fbi >= _sz(mp))
		_expand(mp);

	b = _fbpblk(mp, mp->fbi);
	mp->fbi++;
	_unlock(mp);

	return (void*)(&b->d);
}

/*
 * return block to pool.
 */
void
ymp_put(struct ymp* mp, void* block) {
	struct _blk* b;
	struct _blk* ub; /* used block */
	int    ti;

	b = container_of(block, struct _blk, d);
	_lock(mp);
	yassert(mp->fbi > 0);
	mp->fbi--;
	ub = _fbpblk(mp, mp->fbi);

	/* swap free block pointer */
	ti = b->i; b->i = ub->i; ub->i = ti;

	_setfbp(mp, ub->i, ub);
	_setfbp(mp, b->i, b);
	_unlock(mp);
}

enum yret
ymp_shrink(struct ymp* mp) {
	_lock(mp);
	_shrink(mp);
	_unlock(mp);
	return YROk;
}


enum yret
ymp_stop_shrink(struct ymp* mp) {
	return YREVALNot_implemented;
}

/*
 * return number of element size
 */
int
ymp_sz(struct ymp* mp) {
	int sz;
	_lock(mp);
	sz = _sz(mp);
	_unlock(mp);
	return sz;
}
