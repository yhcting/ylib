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
 * @file ydynb.h
 * @brief Header file for dynamic buffer
 */

#ifndef __YDYNb_h__
#define __YDYNb_h__

#include <memory.h>
#include <string.h>
#include <errno.h>

#include "ydef.h"

/** DYNmaic Buffer - White box structure. */
struct ydynb {
	uint32_t limit; /**< maximum capacity (# of elements) */
	uint32_t sz; /**< current used (# of elements) */
	uint16_t esz; /**< size of element in bytes */
	/** size of element in bytes including paddings for alignment */
	uint32_t eszex;
	void *b; /**< data buffer pointer */
};

YYEXPORT void
ydynb_dump(const struct ydynb *b);

/**
 * Create new dynamic-buffer.
 * 
 * @param init_limit Initial buffer capacity(# of elements) that should be >0.
 * @param esz Element sz in bytes
 * @param align Alignment(bytes) of each element in the buffer.
 * @return NULL if fails (usually, Out Of Memory, Invalid parameter)
 */
YYEXPORT struct ydynb *
ydynb_create(uint32_t init_limit, uint16_t esz, uint8_t align);

/**
 * Create new dynamic-buffer without alignment.
 * See {@link ydynb_create} for details
 */
static YYINLINE struct ydynb *
ydynb_create2(uint32_t init_limit, uint16_t esz) {
	return ydynb_create(init_limit, esz, 1);
}

/**
 * Reset dynamic-buffer-object to empty.
 *
 * @param b Dynamic buffer object
 */
static YYINLINE void
ydynb_reset(struct ydynb *b) {
	b->sz = 0;
}


/**
 * Destroy dynamic-buffer
 * Memory also be freed.
 * So, pointer \a b is not valid anymore.
 */
YYEXPORT void
ydynb_destroy(struct ydynb *);

/** Get used size(# of elements)  */
static YYINLINE uint32_t
ydynb_sz(const struct ydynb *b) {
	return b->sz;
}

/**
 * Get free size(# of elements).
 *
 * @param b Dynamic buffer object
 * @return Free size in bytes
 */
static YYINLINE uint32_t
ydynb_freesz(const struct ydynb *b) {
	return b->limit - b->sz;
}

/**
 * Get element with array index.
 *
 * @param b Dynamic buffer object
 * @param i Index that should be less then buffer size.
 * @return Pointer of i-th element.
 */
static YYINLINE void *
ydynb_get(const struct ydynb *b, uint32_t i) {
	YYassert(i < b->sz);
	return (void *)((char *)b->b + i * b->eszex);
}

/**
 * Get raw pointer of head of free buffer
 * (Address of next byte of used bytes.)
 *
 * @param b Dynamic buffer object
 * @return Pointer to free space for elements.
 */
static YYINLINE void *
ydynb_getfree(const struct ydynb *b) {
	return (void *)((char *)b->b + b->sz * b->eszex);
}

/**
 * Increase buffer size in double.
 *
 * @return 0 if success. Otherwise -errno.
 */
YYEXPORT int
ydynb_expand(struct ydynb *);

/**
 * Increase buffer size until it can cover \a sz_required.
 *
 * @param b Dynamic buffer object
 * @param sz_required Free size(# of elements) required to the buffer.
 * @return 0 if success. Otherwise -errno.
 */
static YYINLINE int
ydynb_expand2(struct ydynb *b, uint32_t sz_required) {
	while (sz_required > ydynb_freesz(b)
	       && !ydynb_expand(b));
	return sz_required <= ydynb_freesz(b)? 0: -ENOMEM;
}

/**
 * Shrink buffer size to \a sz_to to reduce memory consumption.
 *
 * @param sz_to Size(# of elements) that buffer should be shrinked to.
 * @return 0 if success, otherwise -errno.
 */
YYEXPORT int
ydynb_shrink(struct ydynb *, uint32_t sz_to);

/**
 * Append aligned-element-array to buffer.
 * This function assumes that array is already aligned.
 *
 * @param ea Aligned array of elements
 * @param easz Size of element-array(# of elements in array)
 * @return 0 if success. Otherwise -errno.
 */
YYEXPORT int
ydynb_appends(struct ydynb *, const void *ea, uint32_t easz);

/**
 * Append one element to at the end of list.
 *
 * @param ea Element data.
 * @return 0 if success. Otherwise -errno.
 */
static YYINLINE int
ydynb_append(struct ydynb *b, const void *ea) {
	return ydynb_appends(b, ea, 1);
}


#endif /* __YDYNb_h__ */
