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



#ifndef __YHASh_h__
#define __YHASh_h__

#include "yret.h"

struct yhash;

/**
 * @fcb : callback to free user value(item)
 *	  (NULL means, item doesn't need to be freed.)
 */
EXPORT struct yhash*
yhash_create(void(*fcb)(void*));

/**
 * @return : reserved for future use.
 */
EXPORT enum yret
yhash_destroy(struct yhash* h);

/**
 * @return : number of elements in hash.
 */
EXPORT unsigned int
yhash_sz(struct yhash* h);

/**
 * @v	   : user value(item)
 * @return : return hash @h - self.
 */
EXPORT struct yhash*
yhash_add(struct yhash* h,
	  const unsigned char* key, unsigned int keysz,
	  void* v);

/**
 * If these is no matched item, nothing happened.
 * @v	   : user value(item)
 * @return : return hash @h - self.
 */
EXPORT struct yhash*
yhash_del(struct yhash* h,
	  const unsigned char* key, unsigned int keysz);

/**
 * @return : NULL if fails.
 */
EXPORT void*
yhash_find(struct yhash* h,
	   const unsigned char* key, unsigned int keysz);

#endif /* __YHASh_h__ */
