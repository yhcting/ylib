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
#ifndef __YSEt_h__
#define __YSEt_h__

#include "yhash.h"

/* yset structure is dummy wrapper of struct yash * */
typedef struct yhash * yset_t;

static inline yset_t
yset_create(void) {
	return yhash_create(NULL);
}

static inline void
yset_destroy(yset_t s) {
	return yhash_destroy(s);
}

/**
 * @return : number of elements in the set.
 */
static inline unsigned int
yset_sz(const yset_t s) {
	return yhash_sz(s);
}

/**
 * @elemsbuf: Buffer to contain pointer of element.
 *            'shallow copy' of elements are stored at buffer.
 *            So, DO NOT free 'elements' pointer.
 * @elemsszbuf: Buffer to contain each element's length.
 *              So, size should be same with @elemsbuf.
 *              if NULL, this is ignored.
 * @bufsz: sizeof @elemsbuf and @elemsszbuf.
 * @return: number keys assigned to @elemsbuf
 *          return value == @bufsz means @elemsbuf is not large enough to
 *            contains all elements in the set.
 */
static inline unsigned int
yset_elements(const yset_t s,
	      const void **elemsbuf, /* in/out */
	      unsigned int *elemsszbuf, /* in/out */
	      unsigned int bufsz) {
	return yhash_keys(s, elemsbuf, elemsszbuf, bufsz);
}

/**
 * @v: user value(item)
 * @elem: hash key
 * @elemsz: size of element
 *          NOTE!
 *              0 == elemsz hash special meaning.
 *              '0 == elemsz' means, value of key itself - not pointed one -
 *              is element.
 *              This is useful when hashing memory address.
 * @return: -1 for error
 *          otherwise # of newly added item. (0 means overwritten).
 */
static inline int
yset_add(yset_t s, const void *elem, unsigned int elemsz) {
	return yhash_add(s, elem, elemsz, (void *)1);
}

/**
 * If these is no matched item, nothing happened.
 * @v: user value(item)
 * @return: -1 for error otherwise # of deleted. (0 means nothing to delete)
 */
static inline int
yset_del(yset_t s, const void *elem, unsigned int elemsz) {
	return yhash_del(s, elem, elemsz);
}

/**
 * @return: boolean.
 */
static inline int
yset_contains(const yset_t s, const void *elem, unsigned int elemsz) {
	return !!yhash_find(s, elem, elemsz);
}

#endif /* __YSEt_h__ */
