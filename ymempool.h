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


/*
 * Memory pool is tested at ylisp and writer.
 * So, there is no specific test file....
 */

#ifndef __YMEMPOOl_h__
#define __YMEMPOOl_h__

#include "yret.h"

/*
 * mp : Memory Pool
 */
struct ymp;

/*
 * size of pool will be expanded if needed.
 * this is just 'initial' size
 * grpsz  : elem group size size in pool (number of element)
 * elemsz : element size (in bytes)
 */
struct ymp*
ymp_create(int grpsz, int elemsz);

void
ymp_destroy(struct ymp* mp);

/*
 * get one block from pool.
 */
void*
ymp_get(struct ymp* mp);

/*
 * return block to pool.
 */
void
ymp_put(struct ymp* mp, void* block);

/*
 * interrupt shrinking.
 * this is NOT implemented yet!!
 */
enum yret
ymp_stop_shrink(struct ymp*);

/*
 * shrink memory pool.
 */
enum yret
ymp_shrink(struct ymp*);

/*
 * return number of element size
 */
int
ymp_sz(struct ymp* mp);

#endif /* __YMEMPOOl_h__ */
