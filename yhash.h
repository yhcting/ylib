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



#ifndef __YHASh_h__
#define __YHASh_h__

#include <ydef.h>

struct yhash;

/**
 * @return : 0 if success, otherwise error number.
 */
EXPORT int
yhash_init(struct yhash *h, void (*fcb)(void *));

/**
 * @fcb : callback to free user value(item)
 *	  (NULL means, item doesn't need to be freed.)
 */
EXPORT struct yhash *
yhash_create(void (*fcb)(void *));

EXPORT void
yhash_clean(struct yhash *h);

EXPORT void
yhash_destroy(struct yhash *h);

/**
 * @return : number of elements in hash.
 */
EXPORT unsigned int
yhash_sz(const struct yhash *h);

/**
 * @keysbuf: Buffer to contain pointer of key.
 *           'shallow copy' of key is stored at buffer.
 *           So, DO NOT free 'key' pointer.
 * @keysszbuf: Buffer to contain each key's length.
 *             So, size should be same with @keysbuf.
 *             if NULL, this is ignored.
 * @bufsz: size of @keysbuf and @keysszbuf
 * @return : number keys assigned to @keysbuf
 *           return value == @bufsz means @keysbuf is not enough large to
 *             contains all keys in the hash.
 */
EXPORT unsigned int
yhash_keys(const struct yhash *h,
	   const void **keysbuf, /* in/out */
	   unsigned int *keysszbuf, /* in/out */
	   unsigned int bufsz);

/**
 * For hash internal access.
 * DO NOT use unless you know exactly what you are doing.
 */
EXPORT int
yhash_add2(struct yhash *h,
	   /* pointing internally generated-deep-copied address */
	   const void ** const pkey,
	   const void *key, unsigned int keysz,
	   void *v);

/**
 * @v	   : user value(item)
 * @key    : hash key
 * @keysz  : size of hash key
 *           NOTE!
 *               0 == keysz hash special meaning.
 *               '0 == keysz' means, value of key itself - not pointed one -
 *               is key value.
 *               This is useful when hashing memory address.
 * @return : -1 for error
 *           otherwise # of newly added item. (0 means overwritten).
 */
EXPORT int
yhash_add(struct yhash *h,
	  const void *key, unsigned int keysz,
	  void *v);

/**
 * If these is no matched item, nothing happened.
 * @return : -1 for error otherwise # of deleted. (0 means nothing to delete)
 */
EXPORT int
yhash_del(struct yhash *h,
	  const void *key, unsigned int keysz);

/**
 * @value: if NULL, yhash_del2 is exactly same with yhash_del.
 *         otherwise, data value is NOT freed and stored in it.
 */
EXPORT int
yhash_del2(struct yhash *h,
	   void **value,
	   const void *key, unsigned int keysz);

/**
 * @return : NULL if fails.
 */
EXPORT void *
yhash_find(const struct yhash *h,
	   const void *key, unsigned int keysz);

#endif /* __YHASh_h__ */
