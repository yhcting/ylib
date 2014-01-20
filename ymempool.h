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


/*
 * Memory pool is tested at ylisp and writer.
 * So, there is no specific test file....
 */

#ifndef __YMEMPOOl_h__
#define __YMEMPOOl_h__

#include "ydef.h"

/*
 * Options
 */
enum {
	YMP_mt_safe = 0x1,
};

/*
 * mp : Memory Pool
 */
struct ymp;

/**
 * size of pool will be expanded if needed.
 * this is just 'initial' size
 * @grpsz  : elem group size size in pool (number of element)
 * @elemsz : element size (in bytes)
 * @opt    : option.
 */
EXPORT struct ymp *
ymp_create(int grpsz, int elemsz, int opt);

EXPORT void
ymp_destroy(struct ymp *);

/**
 * get one block from pool.
 * NULL if fails. (Ex. Out Of Memory).
 */
EXPORT void *
ymp_get(struct ymp *);

/**
 * return block to pool.
 */
EXPORT void
ymp_put(struct ymp *, void *block);


#ifdef CONFIG_MEMPOOL_DYNAMIC
/**
 * interrupt shrinking.
 * this is NOT implemented yet!!
 * return  : -1 for error.
 */
EXPORT int
ymp_stop_shrink(struct ymp *);

/**
 * shrink memory pool.
 * 
 */
EXPORT int
ymp_shrink(struct ymp *, int margin);

#else /* CONFIG_MEMPOOL_DYNAMIC */
/*
 * Not supported at Non-dynamic-mempool.
 */

static inline int
ymp_stop_shrink(struct ymp *mp) {
	return -1;
}

/*
 * shrink memory pool.
 */
static inline int
ymp_shrink(struct ymp *mp, int margin) {
	return -1;
}

#endif /* CONFIG_MEMPOOL_DYNAMIC */

/*
 * return number of element
 */
EXPORT int
ymp_sz(struct ymp *);

/*
 * return number of used element
 */
EXPORT int
ymp_usedsz(struct ymp *);

#endif /* __YMEMPOOl_h__ */
