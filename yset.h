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
static inline u32
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
static inline u32
yset_elements(const yset_t s,
	      const void **elemsbuf, /* in/out */
	      u32         *elemsszbuf, /* in/out */
	      u32          bufsz) {
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
yset_add(yset_t s, const void *elem, u32 elemsz) {
	return yhash_add(s, elem, elemsz, (void *)1);
}

/**
 * If these is no matched item, nothing happened.
 * @v: user value(item)
 * @return: -1 for error otherwise # of deleted. (0 means nothing to delete)
 */
static inline int
yset_del(yset_t s, const void *elem, u32 elemsz) {
	return yhash_del(s, elem, elemsz);
}

/**
 * @return: boolean.
 */
static inline int
yset_contains(const yset_t s, const void *elem, u32 elemsz) {
	return !!yhash_find(s, elem, elemsz);
}

#endif /* __YSEt_h__ */
