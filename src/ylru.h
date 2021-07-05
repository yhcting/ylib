/******************************************************************************
 * Copyright (C) 2011, 2012, 2013, 2014, 2015
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

/**
 * @file ylru.h
 * @brief Simple LRU cache
 */

#ifndef __YLRu_h__
#define __YLRu_h__

#include "ydef.h"

/** Predefined function ID. 'free()' function for 'malloc()' */
#define YLRU_PREDEFINED_FREE ((void (*)(void *))1)

/** lru object */
struct ylru;

/**
 * Create lru cache that uses integer value as key.
 *
 * @param maxsz Maximum size that cache can keep (NOT bytes).
 *	'0' means infinite (as many as possible)
 * @param datafree Function to free user data evicted from cache
 *	'NULL' means 'DO NOT free'.
 * @param datacreate Function to create data if cache misses.
 *	'NULL' means 'DO NOT create'.
 * @param datasize Function to calculate data size.
 *	This is to compare with 'maxsz' of cache.
 *	'NULL' means fixed value (1).
 * @return NULL for fails (ex. there is NULL in arguments)
 */
YYEXPORT struct ylru *
ylrui_create(
	uint32_t maxsz,
	void (*datafree)(void *),
	void *(*datacreate)(const void *key),
	uint32_t (*datasize)(const void *));


/**
 * Create lru cache that uses string value as key.
 * @see ylrui_create
 */
YYEXPORT struct ylru *
ylrus_create(
	uint32_t maxsz,
	void (*datafree)(void *),
	void *(*datacreate)(const void *key),
	uint32_t (*datasize)(const void *));

/**
 * Create lru cache that uses string value as key.
 * See @ref ylrui_create and @ref yhasho_create in {@link yhash.h}
 */
YYEXPORT struct ylru *
ylruo_create(
	uint32_t maxsz,
	/* functions to handle cache data */
	void (*datafree)(void *),
	void *(*datacreate)(const void *key),
	uint32_t  (*datasize)(const void *),
	/* functions to handle key object */
	void (*keyfree)(void *),
	int (*keycopy)(void **newkey, const void *),
	int (*keycmp)(const void *, const void *),
	uint32_t (*hfunc)(const void *key));

/**
 * Create empty lru cache that has same attributes with given one.
 * @see yhash_create
 *
 * @return NULL if fails
 */
YYEXPORT struct ylru *
ylru_create(const struct ylru *);

/**
 * Make cache empty.
 * Cache itself is NOT destroied.
 */
YYEXPORT void
ylru_reset(struct ylru *);

/**
 * Destroy lru object.
 * Object pointer becomes invalid.
 */
YYEXPORT void
ylru_destroy(struct ylru *);

/**
 * Put data to lru cache.
 *
 * @param key Key
 * @param data Data
 * @return 0 for success. Otherwise @c -errno.
 */
YYEXPORT int
ylru_put(struct ylru *, const void *key, void *data);

/**
 * Get data from LRU cache.
 * It is important to note that value is remove from cache.
 * So, after using it, user SHOULD put it again to caching it again.
 *
 * @param data 'NULL' is NOT allowed.
 * @param key Key value of data to get.
 * @return
 *	- 0 for getting data(existing one, or newly created one)
 *	- 1 cache missed and not newly created.
 *	- <0 error (@c -errno)
 */
YYEXPORT int
ylru_get(struct ylru *, void **data, const void *key);

/**
 * Get size of cached data.
 * This is based on @c datasize function passed when cache object is created.
 * See @c datasize at @ref ylrui_create, @ref ylrus_create and
 * @ref ylruo_create
 *
 * @return data size of the cache.
 */
YYEXPORT uint32_t
ylru_sz(struct ylru *);


#endif /* __YLRu_h__ */
