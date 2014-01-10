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
 * Special version of hash to support memory address hashing efficiently.
 * Key = memory address. Value = Don't care.
 */

#ifndef __YAHASh_h__
#define __YAHASh_h__

#include <stdint.h>
#include <ydef.h>

struct yahash;

/**
 */
EXPORT struct yahash *
yahash_create(void);

/**
 * @return : reserved for future use.
 */
EXPORT int
yahash_destroy(struct yahash *h);

/**
 * @return : number of elements in hash.
 */
EXPORT uint32_t
yahash_sz(const struct yahash *h);

/**
 * @return : return hash @h - self.
 */
EXPORT int
yahash_add(struct yahash *h, void *addr);

/**
 * If these is no matched item, nothing happened.
 * @return : return hash @h - self.
 */
EXPORT int
yahash_del(struct yahash *h, void *addr);

/**
 * @return : 0 (not in hash) 1 (already in hash)
 */
EXPORT int
yahash_check(const struct yahash *h, void *addr);

#endif /* __YAHASh_h__ */
