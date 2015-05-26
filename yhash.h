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

/*
 * This is NOT MT-safe.
 */

#ifndef __YHASh_h__
#define __YHASh_h__

#include "ydef.h"

/**
 * Hash key type.
 */
enum {
        YHASH_KEYTYPE_I,
        YHASH_KEYTYPE_S,
        YHASH_KEYTYPE_O
};


/**
 * predefined function IDs
 */
/* 'free()' function for malloc() */
#define YHASH_PREDEFINED_FREE ((void (*)(void *))1)



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
 * @vfree function to free hash value.
 *        'NULL' not to free.
 *        'HFREE_FREE' to use standard 'free' function.
 */
EXPORT struct yhash *
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
 * @vfree function to free hash value.
 *        'NULL' not to free.
 *        'HFREE_FREE' to use standard 'free' function.
 * @key_deepcopy TRUE for using deepcopy for hash 'key', otherwise FALSE.
 */
EXPORT struct yhash *
yhashs_create(void (*vfree)(void *), int key_deepcopy);

/******************************************************************************
 *
 * Interfaces for general-object-key hash
 * prefix : 'yhasho_'
 *
 *****************************************************************************/
/**
 * Create hash that uses general object value as key(YHASH_KEYTYPE_O).
 * Actions when "argument == NULL":
 * @vfree   memory is NOT freed (can be 'HFREE_FREE')
 * @keyfree memory is NOT freed (can be 'HFREE_FREE')
 * @keycopy shallow copy (pointer address is copied).
 * @keycmp  mem-addresses of key-objects are compared.
 */
EXPORT struct yhash *
yhasho_create(void  (*vfree)(void *),
              void  (*keyfree)(void *),
	      /* return: 0 for success. Otherwise errno. */
              int   (*keycopy)(void **newkey, const void *),
	      /* return: 0 if same. Otherewise non-zero */
              int   (*keycmp)(const void *, const void *),
              u32   (*hfunc)(const void *key));

/******************************************************************************
 *
 * Common Interfaces
 * prefix : 'yhash_'
 *
 *****************************************************************************/
/**
 * Create new empty hash that is same with given hash.
 * (All are same except for it is empty)
 */
EXPORT struct yhash *
yhash_create(const struct yhash *);

EXPORT void
yhash_destroy(struct yhash *);

EXPORT int
yhash_clean(struct yhash *);

EXPORT u32
yhash_sz(const struct yhash *);

/**
 * 'sametype' means all hash attributes (ex. key type, free function etc) are
 * same.
 * @return TRUE/FALSE
 */
EXPORT int
yhash_is_sametype(const struct yhash *, const struct yhash *);

/**
 * @return YHASH_KEYTYPE_X (X can be I, S or O)
 */
EXPORT int
yhash_keytype(const struct yhash *);

/**
 * @return number of keys stored in keysbuf.
 */
EXPORT u32
yhash_keys(const struct yhash *, const void **keysbuf, u32 bufsz);

/**
 * @return -1 for error
 *         otherwise # of newly added item. (0 means overwritten).
 */
EXPORT int
yhash_add(struct yhash *, void *key, void *v);

/**
 * @oldv Existing overwritten hash value matching given key, is stored.
 *       If operation doesn't overwrite anything, this is un-touched.
 *       if NULL, this is exactly same with 'yhash_add'.
 * @return -1 for error
 *         otherwise # of newly added item. (0 means overwritten).
 */
EXPORT int
yhash_add2(struct yhash *, void *key, void **oldv, void *v);

/**
 * This accesses hash-internal-data directly
 * DO NOT use unless you know exactly what you are doing.
 * This is useful to reuse key as read-only value.
 * @phkey address of key used in hash.
 *        if NULL, this exactly same with yhashs_add
 * @return -1 for error
 *         otherwise # of newly added item. (0 means overwritten).
 */
EXPORT int
yhash_add3(struct yhash *, const void ** const phkey, void *key, void *v);

/**
 * @return number of deleted elements. (0 means nothing deleted)
 */
EXPORT int
yhash_del(struct yhash *h, const void *key);

/**
 * @return number of deleted elements. (0 means nothing deleted)
 */
EXPORT int
yhash_del2(struct yhash *, void **value, const void *key);

/**
 * @value value matching given key. NULL is allowed
 * @return 0 if key is in hash. Otherwise non-zero.
 */
EXPORT int
yhash_find(const struct yhash *, void **value, const void *key);


#endif /* __YHASh_h__ */
