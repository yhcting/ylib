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

#ifndef __YLRu_h__
#define __YLRu_h__

#include "ydef.h"

#define YLRU_PREDEFINED_FREE ((void (*)(void *))1)

struct ylru;

/**
 * @maxsz Maximum size that cache can keep (NOT bytes).
 *        '0' means infinite (as many as possible)
 * @datafree Function to free user data evicted from cache
 *           'NULL' means 'DO NOT free'
 * @datacreate Function to create data if cache miss
 *             'NULL' means 'DO NOT create'
 * @datasize Function to calculate data size.
 *           This is to compare with 'maxsz' of cache
 *           'NULL' means fixed value (1).
 * @return NULL for fails (ex. there is NULL in arguments)
 */
EXPORT struct ylru *
ylrui_create(u32 maxsz,
	     void  (*datafree)(void *),
	     void *(*datacreate)(const void *key),
	     u32   (*datasize)(const void *));


EXPORT struct ylru *
ylrus_create(u32 maxsz,
	     void  (*datafree)(void *),
	     void *(*datacreate)(const void *key),
	     u32   (*datasize)(const void *));

/**
 * See comment of 'ylrui_create' and 'yhasho_create' for details
 */
EXPORT struct ylru *
ylruo_create(u32 maxsz,
	     /* functions to handle cache data */
	     void  (*datafree)(void *),
	     void *(*datacreate)(const void *key),
	     u32   (*datasize)(const void *),
	     /* functions to handle key object */
	     void  (*keyfree)(void *),
	     int   (*keycopy)(void **newkey, const void *),
	     int   (*keycmp)(const void *, const void *),
	     u32   (*hfunc)(const void *key));

/**
 * Create empty lru cache that has same attributes with given one.
 * See 'yhash_create' for details.
 */
EXPORT struct ylru *
ylru_create(const struct ylru *);

/**
 * Make cache empty.
 * Cache itself is NOT destroied.
 */
EXPORT void
ylru_clean(struct ylru *);

EXPORT void
ylru_destroy(struct ylru *);

/**
 * @return: 0 for success, otherwise error number
 */
EXPORT int
ylru_put(struct ylru *, const void *key, void *data);

/**
 * NOTE: value is remove from cache!
 * So, after using it, user SHOULD put it again for future use.
 * @data 'NULL' is NOT allowed.
 * @return  0 for getting data(existing one, or newly created one)
 *          1 cache missed and not newly created.
 *         <0 error (-errno)
 */
EXPORT int
ylru_get(struct ylru *, void **data, const void *key);

/**
 * Get size of cached data.
 * This is based on 'size' of 'ylru_put'.
 */
EXPORT u32
ylru_sz(struct ylru *);


#endif /* __YLRu_h__ */
