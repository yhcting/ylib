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
 * @file yhash.h
 * @brief Header file to use hash module
 *
 * This is NOT MT(Multithread)-safe.
 */

#ifndef __YHASh_h__
#define __YHASh_h__

#include "ydef.h"

/** Predefined function ID. 'free()' function for malloc() */
#define YHASH_MEM_FREE ((void (*)(void *))1)

/** yhash object. */
struct yhash;

/******************************************************************************
 *
 * Interfaces for integer-key hash
 * prefix : 'yhashi_'
 *
 *****************************************************************************/
/**
 * Create hash that uses integer value as key(YHASH_KEYTYPE_I).
 * In this hash, mem. address of key is interpreted as integer value.
 * ex. (void *)0x01 => integer '1'
 *
 * @param vfree
 *	function to free hash value.
 *	set as NULL not to free.
 *	set as @ref YHASH_MEM_FREE to use standard 'free' function.
 * @return NULL if fails(ex. ENOMEM). Otherwise new hash object.
 */
YYEXPORT struct yhash *
yhashi_create(void (*vfree)(void *));

/******************************************************************************
 *
 * Interfaces for string-key hash
 * prefix : 'yhashs_'
 *
 *****************************************************************************/
/**
 * Create hash that uses string value as key(YHASH_KEYTYPE_S).
 * In this hash, mem. address of key is interpreted as string that has null
 * terminator.
 *
 * @param vfree See {@code vfree} at @ref yhashi_create
 * @param key_deepcopy TRUE to use deepcopy for hash 'key', otherwise FALSE.
 * @return NULL if fails(ex. ENOMEM). Otherwise new hash object.
 */
YYEXPORT struct yhash *
yhashs_create(void (*vfree)(void *), bool key_deepcopy);

/******************************************************************************
 *
 * Interfaces for general-object-key hash
 * prefix : 'yhasho_'
 *
 *****************************************************************************/
/**
 * Create hash that uses general object value as key(YHASH_KEYTYPE_O).
 *
 * @param vfree See @c vfree at @ref yhashi_create
 * @param keyfree Same with @c vfree
 * @param keycopy
 *	Callback function to copy key. Set NULL to use shallow copy.
 *	This should return 0 if success. Otherwise non-zero.
 * @param keyeq
 *	Callback function to compare key objects.
 *	Set as NULL to compare mem-addresses of key-objects.
 *	This should return 0 if same. Otherewise non-zero.
 * @param hfunc Hash function creating 32bit hash value from @c key.
 * @return NULL if fails(ex. ENOMEM). Otherwise new hash object.
 */
YYEXPORT struct yhash *
yhasho_create(
	void (*vfree)(void *),
	void (*keyfree)(void *),
	int (*keycopy)(const void **newkey, const void *),
	int (*keyeq)(const void *, const void *),
	uint32_t (*hfunc)(const void *key));

/******************************************************************************
 *
 * Common Interfaces
 * prefix : 'yhash_'
 *
 *****************************************************************************/
/**
 * Create new empty hash that is same with given hash.
 * (All hash attributes are same, but hash is empty.)
 *
 * @return NULL if fails(ex. ENOMEM). Otherwise new hash object.
 */
YYEXPORT struct yhash *
yhash_create(const struct yhash *);

/**
 * Destroy hash object. Hash becomes invalid.
 */
YYEXPORT void
yhash_destroy(struct yhash *);

/**
 * Reset hash object.
 * All hash values are destroied and hash becomes empty.
 *
 * @return 0 if success. Otherwise @c -errno.
 */
YYEXPORT int
yhash_reset(struct yhash *);

/**
 * Get hash size(number of keys in the hash).
 */
YYEXPORT uint32_t
yhash_sz(const struct yhash *);

/**
 * 'sametype' means all hash attributes (ex. key type, free function etc) are
 * same.
 *
 * @return boolean
 */
YYEXPORT bool
yhash_is_sametype(const struct yhash *, const struct yhash *);

/**
 * Get keys in the hash.
 * If size of @p keysbuf is not large enough, this fills @p keysbuf fully and
 * returns @p bufsz.
 * So, caller may need to compare returned value and hash size.
 *
 * @param keysbuf Buffer to store key object.
 * @param bufsz Size of @p keysbuf.
 * @return Number of keys stored in keysbuf.
 */
YYEXPORT uint32_t
yhash_keys(const struct yhash *, const void **keysbuf, uint32_t bufsz);

/**
 * Add new value to hash.
 *
 * @param key Key
 * @param v Value
 * @return # of newly added item (0 means overwritten). @c -errno if fails.
 */
YYEXPORT int
yhash_set(struct yhash *, void *key, void *v);

/**
 * Set new item to hash and get existing value.
 *
 * @param key Key
 * @param oldv
 *	Existing old value object.
 *	If function doesn't overwrite anything, this is un-touched.
 *	If this is NULL, function is exactly same with @ref yhash_set
 * @param v new value object
 * @return # of newly added item (0 means overwritten). @c -errno if fails.
 */
YYEXPORT int
yhash_set2(struct yhash *, void *key, void **oldv, void *v);

/**
 * This function accesses hash-internal-data directly
 * Therefore, DO NOT use unless you know exactly what you are doing.
 * This is useful to reuse key as read-only value.
 *
 * @param phkey Address of key used in hash.
 *	If this is NULL, function is same with @ref yhash_set
 * @param key Key
 * @param v New value
 * @return # of newly added item (0 means overwritten). @c -errno if fails.
 */
YYEXPORT int
yhash_set3(struct yhash *, const void ** const phkey, void *key, void *v);

/**
 * Delete key from hash. Value in the hash is destroied.
 *
 * @param key Key
 * @return Number of deleted values (0 means nothing deleted).
 * 	@c -errno if fails.
 */
YYEXPORT int
yhash_remove(struct yhash *, const void *key);

/**
 * Delete key from hash and get value in the hash.
 *
 * @param key Key
 * @param value Value in the hash.
 *	If this is NULL, function is same with @ref yhash_remove
 * @return Number of deleted values. (0 means nothing deleted).
 *	@c -errno if fails.
 */
YYEXPORT int
yhash_remove2(struct yhash *, const void *key, void **value);

/**
 * Find key and get value.
 *
 * @param value Value in the hash. If it is NULL, it is ignored.
 * @param key Key
 * @return 0 if success. @c -errno if fails.
 */
YYEXPORT int
yhash_get(const struct yhash *, const void *key, void **value);

/**
 * Is the @p key is in the hash?
 *
 * @param h Hash object
 * @param key Hash key
 */
static YYINLINE bool
yhash_has(const struct yhash *h, const void *key) {
	return !yhash_get(h, key, NULL);
}


#endif /* __YHASh_h__ */
