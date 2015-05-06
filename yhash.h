/******************************************************************************
 * Copyright (C) 2011, 2012, 2013, 2014
 * Younghyung Cho. <yhcting77@gmail.com>
 * All rights reserved.
 *
 * This file is part of ylib
 *
 * This program is licensed under the FreeBSD license
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * official policies, either expressed or implied, of the FreeBSD Project.
 *****************************************************************************/



#ifndef __YHASh_h__
#define __YHASh_h__

#include "ydef.h"

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
EXPORT u32
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
EXPORT u32
yhash_keys(const struct yhash *h,
	   const void **keysbuf, /* in/out */
	   u32 *keysszbuf, /* in/out */
	   u32 bufsz);

/**
 * For hash internal access.
 * DO NOT use unless you know exactly what you are doing.
 */
EXPORT int
yhash_add2(struct yhash *h,
	   /* pointing internally generated-deep-copied address */
	   const void ** const pkey,
	   const void *key, u32 keysz,
	   void *v);

/**
 * @v	   : user value(item)
 * @key    : hash key
 * @keysz  : size of hash key
 * @return : -1 for error
 *           otherwise # of newly added item. (0 means overwritten).
 */
EXPORT int
yhash_add(struct yhash *h,
	  const void *key, u32 keysz,
	  void *v);

/**
 * If these is no matched item, nothing happened.
 * @keysz  : size of hash key
 *           See, comment of yhash_add for details.
 * @return : -1 for error otherwise # of deleted. (0 means nothing to delete)
 */
EXPORT int
yhash_del(struct yhash *h,
	  const void *key, u32 keysz);

/**
 * @value: if NULL, yhash_del2 is exactly same with yhash_del.
 *         otherwise, data value is NOT freed and stored in it.
 */
EXPORT int
yhash_del2(struct yhash *h,
	   void **value,
	   const void *key, u32 keysz);

/**
 * @keysz  : size of hash key
 *           See, comment of yhash_add for details.
 * @return : NULL if fails.
 */
EXPORT void *
yhash_find(const struct yhash *h,
	   const void *key, u32 keysz);

#endif /* __YHASh_h__ */
