/******************************************************************************
 * Copyright (C) 2011, 2012, 2013, 2014, 2015, 2023
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
 * @file yset.h
 * @brief Header for 'set' module
 */

#pragma once

#include "yhash.h"

/* yset structure is dummy wrapper of struct yash * */
/**
 * Set object
 * Object SHOULD NOT be handled as `struct yhash` directly.
 */
typedef struct yhash * yset_t;

/**
 * Create set whose key-type is integer.
 *
 * @return NULL if fails.
 */
static YYINLINE yset_t
yseti_create(void) {
	return yhashi_create(NULL);
}

/**
 * Create set whose key-type is string.
 * deep-copied key-string is used.
 *
 * @return NULL if fails.
 */
static YYINLINE yset_t
ysets_create(void) {
	return yhashs_create(NULL, TRUE);
}

/**
 * Create set whose key-type is general-object.
 * @see yhasho_create
 *
 * @return NULL if fails.
 */
static YYINLINE yset_t
yseto_create(
	void (*elemfree)(void *),
	int (*elemcopy)(const void **newelem, const void *),
	int (*elemcmp)(const void *, const void *),
	uint32_t (*hfunc)(const void *elem)
) {
	return yhasho_create(NULL, elemfree, elemcopy, elemcmp, hfunc);
}

/**
 * Create empty set that is same type with given set s.
 *
 * @param s Set
 * @return NULL if fails.
 */
static YYINLINE yset_t
yset_create(yset_t s) {
	return yhash_create(s);
}

/**
 * Destroy module object.
 *
 * @param s Set
 */
static YYINLINE void
yset_destroy(yset_t s) {
	return yhash_destroy(s);
}

/**
 * Get size(number of elements) of the set.
 *
 * @param s Set
 * @return Size
 */
static YYINLINE uint32_t
yset_sz(const yset_t s) {
	return yhash_sz(s);
}

/**
 * Get elements in the set.
 *
 * @param s Set
 * @param[in,out] elemsbuf
 * Buffer to contain pointer of element.
 * 'shallow copy' of elements are stored at buffer.
 * So, DO NOT free/modify element's value pointed.
 * @param bufsz sizeof @p elemsbuf
 * @return number keys assigned to @p elemsbuf
 * `return value == bufsz` may mean that @p elemsbuf is
 * not large enough to contains all elements in the set.
 */
static YYINLINE uint32_t
yset_elements(
	const yset_t s,
	const void **elemsbuf, /* in/out */
	uint32_t bufsz
) {
	return yhash_keys(s, elemsbuf, bufsz);
}

/**
 * Add to the set.
 * @p elem is used as key by shallow or deep copy. It is decided when set
 * is created @c ysetX_create.
 *
 * @param s Set
 * @param elem data value(item)
 * @return @c -errno if fails.
 * Otherwise # of newly added item (0 means overwritten).
 */
static YYINLINE int
yset_add(yset_t s, void *elem) {
	return yhash_set(s, elem, (void *)1);
}

/**
 * Delete element from the set.
 * If these is no matched item, nothing happened.
 *
 * @param s Set
 * @param elem data value(item)
 * @return @c -errno if fails.
 * Otherwise # of deleted (0 means nothing to delete).
 */
static YYINLINE int
yset_remove(yset_t s, const void *elem) {
	return yhash_remove(s, elem);
}

/**
 * Is set contains @p elem?
 *
 * @param s Set
 * @param elem Element
 * @return TRUE or FALSE
 */
static YYINLINE bool
yset_has(const yset_t s, const void *elem) {
	return yhash_has(s, elem);
}

/**
 * Clone set. NOT IMPLEMENTED YET.
 *
 * @param s Set
 * @return Cloned set object
 */
static YYINLINE yset_t
yset_clone(const yset_t s) {
	return NULL; /* NOT Implemented yet */
}

/**
 * Create new set which is insection of two sets.
 * Elements are copied from source set.
 *
 * @return newly created @c yset for success, otherwise NULL.
 */
YYEXPORT yset_t
yset_intersect(const yset_t, const yset_t);

/**
 * Create new set which is union of two sets.
 * (NOT IMPLEMENTED YET)
 * @see yset_intersect
 */
YYEXPORT yset_t
yset_union(const yset_t, const yset_t);

/**
 * Create new set which is diff of two sets.
 * (NOT IMPLEMENTED YET)
 * @see yset_intersect
 */
YYEXPORT yset_t
yset_diff(const yset_t, const yset_t);
