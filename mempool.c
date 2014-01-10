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

#include "ymempool.h"

/*
 * fbp memory strip (FbpMS)
 * ------------------------
 *
 *     * U(used) / F(free)
 *
 *                fbp
 *             +-------+
 *             |   F   | <- index [sz()-1]
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

struct dummy {
	void *dummy;
};

struct blk {
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
	struct dummy  d;
};

struct ymp {
#ifndef CONFIG_MEMPOOL_DYNAMIC
	unsigned char **grp;    /* groups of blocks */
#endif
	int             grpsz;  /* size of grp - number of element in group*/
	int             nrgrp;  /* number of group allocated */
	struct blk   ***fbp;    /* Free Block Pointer group */
	int             esz;    /* Element SiZe */
	int             fbi;    /* Free Block Index */
	int             opt;    /* option */
	pthread_mutex_t m;      /* Mutex for MT safety */
};

/******************************************************************************
 * Intenal functions
 *****************************************************************************/
static inline int
opt_is_mt_safe(struct ymp *mp) {
	return mp->opt & YMP_mt_safe;
}

static inline void
init_lock(struct ymp *mp) {
	if (opt_is_mt_safe(mp))
		pthread_mutex_init(&mp->m, NULL);
}

static inline void
lock(struct ymp *mp) {
	if (opt_is_mt_safe(mp))
		if (pthread_mutex_lock(&mp->m))
			yassert(0);
}

static inline void
unlock(struct ymp *mp) {
	if (opt_is_mt_safe(mp))
		if (pthread_mutex_unlock(&mp->m))
			yassert(0);
}

static inline void
destroy_lock(struct ymp *mp) {
	if (opt_is_mt_safe(mp))
		pthread_mutex_destroy(&mp->m);
}

static inline int
align_adjust(struct ymp *mp) {
	return sizeof(void *) - (mp->esz % sizeof(void *));
}

static inline int
esz(struct ymp *mp) {
	return mp->esz + align_adjust(mp);
}

static inline int
blksz(struct ymp *mp) {
	return sizeof(struct blk) - sizeof(struct dummy) + esz(mp);
}

/*
 * @i    : index of fbp (ex. fbi)
 */
static inline struct blk *
fbpblk(struct ymp *mp, int i) {
	struct blk *b = *(mp->fbp[i / mp->grpsz] + i % mp->grpsz);
	yassert(b->i == i);
	return b;
}

static inline int
is_freeblk(struct ymp *mp, struct blk *b) {
	return (mp->fbi <= b->i);
}

static inline int
is_usedblk(struct ymp *mp, struct blk *b) {
	return !is_freeblk(mp, b);
}

/*
 * @i    : index of fbp (ex. fbi)
 */
static inline void
setfbp(struct ymp *mp, int i, struct blk *b) {
	*(mp->fbp[i / mp->grpsz] + i % mp->grpsz) = b;
	b->i = i;
}

static inline int
sz(struct ymp *mp) {
	return mp->nrgrp * mp->grpsz;
}

static inline int
usedsz(struct ymp *mp) {
	return mp->fbi;
}

/*
 * Policy
 */
static inline int
need_shrink(struct ymp *mp) {
	return (mp->fbi * 2 / mp->grpsz < mp->nrgrp);
}

#ifdef CONFIG_MEMPOOL_DYNAMIC

/*
 * expand memory pool by 1 group
 */
static void
expand(struct ymp *mp) {
	int            i;
	struct blk  ***newfbp;

	newfbp = ymalloc(sizeof(*newfbp) * (mp->nrgrp + 1));
	yassert(newfbp);

	/* allocate new fbp group */
	newfbp[mp->nrgrp] = ymalloc(sizeof(**newfbp) * mp->grpsz);
	yassert(newfbp[mp->nrgrp]);

	/* initialize fbp & block group */
	for (i = 0; i < mp->grpsz; i++) {
		newfbp[mp->nrgrp][i]
			= (struct blk *)ymalloc(blksz(mp));
		newfbp[mp->nrgrp][i]->i = sz(mp) + i;
	}

	/* keep previous values */
	memcpy(newfbp, mp->fbp, mp->nrgrp * sizeof(*newfbp));

	/* update mp structure */
	yfree(mp->fbp);
	mp->fbp = newfbp;
	mp->nrgrp++;
}

static void
shrink(struct ymp *mp, int margin) {
	int from, i, j;
	/* start index of empty group */
	from = (mp->fbi - 1) / mp->grpsz + 1 + margin;
	if (from > mp->nrgrp)
		from = mp->nrgrp;
	for (i = from; i < mp->nrgrp; i++) {
		for (j = 0; j < mp->grpsz; j++)
			yfree(mp->fbp[i][j]);
		yfree(mp->fbp[i]);
	}
	mp->nrgrp = from;
}

#else /* CONFIG_MEMPOOL_DYNAMIC */

static inline void
shrink(struct ymp *mp, int margin) { }

/*
 * @i    : index of memory block pool.
 */
static inline struct blk *
blk(struct ymp *mp, int i) {
	return (struct blk *)(mp->grp[i / mp->grpsz]
			      + i % mp->grpsz * blksz(mp));
}

static inline void
fbpdump(struct ymp *mp) {
	int i;
	printf("sz : %d, fbi : %d\n", sz(mp), mp->fbi);
	for (i = 0; i < sz(mp); i++) {
		printf("fbp[%d] -> %p (i:%d)\n",
		       i, fbpblk(mp, i), fbpblk(mp, i)->i);
	}
}

static inline void
blkdump(struct ymp *mp) {
	int i;
	printf("sz : %d, fbi : %d\n", sz(mp), mp->fbi);
	for (i = 0; i < sz(mp); i++) {
		printf("blk(%d:%p) : i(%d)\n",
		       i, blk(mp, i), blk(mp, i)->i);
	}
}

/*
 * expand memory pool by 1 group
 */
static void
expand(struct ymp *mp) {
	int             i;
	unsigned char **newgrp;
	struct blk   ***newfbp;
	int             bsz;

	/* pre-calulate frequently used value */
	bsz = blksz(mp);

	newgrp = ymalloc(sizeof(*newgrp) * (mp->nrgrp + 1));
	newfbp = ymalloc(sizeof(*newfbp) * (mp->nrgrp + 1));
	yassert(newgrp && newfbp);

	/* allocate new fbp group */
	newfbp[mp->nrgrp] = ymalloc(sizeof(**newfbp) * mp->grpsz);
	/* allocate new block group */
	newgrp[mp->nrgrp] = ymalloc(sizeof(**newgrp)
				    * mp->grpsz
				    * bsz);
	yassert(newfbp[mp->nrgrp] && newgrp[mp->nrgrp]);

	/* initialize fbp & block group */
	for (i = 0; i < mp->grpsz; i++) {
		newfbp[mp->nrgrp][i]
			= (struct blk *)(newgrp[mp->nrgrp] + i * bsz);
		newfbp[mp->nrgrp][i]->i = sz(mp) + i;
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

#endif /* CONFIG_MEMPOOL_DYNAMIC */

/******************************************************************************
 * Interface Functions
 *****************************************************************************/

struct ymp*
ymp_create(int grpsz, int elemsz, int opt) {
	struct ymp *mp;

	yassert(grpsz > 0 && elemsz > 0);
	mp = ymalloc(sizeof(*mp));
#ifndef CONFIG_MEMPOOL_DYNAMIC
	mp->grp = ymalloc(sizeof(*mp->grp));
#endif
	mp->grpsz = grpsz;
	mp->nrgrp = 0;
	mp->fbp = ymalloc(sizeof(*mp->fbp));
	mp->esz = elemsz;
	mp->fbi = 0;
	mp->opt = opt;
	init_lock(mp);

	/* allocate 1-block-group for initial state */
	expand(mp);

	return mp;
}

void
ymp_destroy(struct ymp *mp) {
	int i;
	destroy_lock(mp);
#ifdef CONFIG_MEMPOOL_DYNAMIC
	for (i = 0; i < mp->nrgrp; i++) {
		int j;
		for (j = 0; j < mp->grpsz; j++)
			yfree(mp->fbp[i][j]);
		yfree(mp->fbp[i]);
	}
	yfree(mp->fbp);
#else
	for (i = 0; i < mp->nrgrp; i++) {
		yfree(mp->fbp[i]);
		yfree(mp->grp[i]);
	}
	yfree(mp->fbp);
	yfree(mp->grp);
#endif
	yfree(mp);
}

/*
 * get one block from pool.
 */
void*
ymp_get(struct ymp *mp) {
	struct blk *b;

	lock(mp);
	if (mp->fbi >= sz(mp))
		expand(mp);

	b = fbpblk(mp, mp->fbi);
	yassert(b->i == mp->fbi);

	mp->fbi++;
	unlock(mp);

	return (void*)(&b->d);
}

/*
 * return block to pool.
 */
void
ymp_put(struct ymp *mp, void *block) {
	struct blk *b;
	struct blk *ub; /* used block */
	int          ti;

	b = container_of(block, struct blk, d);
	lock(mp);
	yassert(mp->fbi > 0
		&& b->i < mp->fbi);

	mp->fbi--;
	ub = fbpblk(mp, mp->fbi);

	/* swap free block pointer */
	ti = b->i;
	setfbp(mp, ub->i, b);
	setfbp(mp, ti, ub);

	if (need_shrink(mp))
		shrink(mp, 1);

	unlock(mp);
}

#ifdef CONFIG_MEMPOOL_DYNAMIC
int
ymp_shrink(struct ymp *mp, int margin) {
	lock(mp);
	shrink(mp, margin);
	unlock(mp);
	return 0;
}

int
ymp_stop_shrink(struct ymp *mp) {
	return -1;
}
#endif /* CONFIG_MEMPOOL_DYNAMIC */

int
ymp_sz(struct ymp *mp) {
	int s;
	lock(mp);
	s = sz(mp);
	unlock(mp);
	return s;
}

int
ymp_usedsz(struct ymp *mp) {
	int s;
	lock(mp);
	s = usedsz(mp);
	unlock(mp);
	return s;
}
