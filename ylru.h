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

struct ylru;

struct ylru_cb {
	/* free user data evicted from cache. */
	void  (*free)(void *);
	/* create data if cache miss */
	void *(*create)(u32 *data_size, /* size of created data */
			const void *key, u32 keysz);
};

EXPORT struct ylru *
ylru_create(u32 maxsz,
	    const struct ylru_cb *cbs);

/**
 * Make cache empty.
 * Cache itself is NOT destroied.
 */
EXPORT void
ylru_clean(struct ylru *);

EXPORT void
ylru_destroy(struct ylru *);

/**
 * @data_size: size of user defined unit. NOT memory size of data.
 *     This value is used to calculate lru cache size.
 * @return: 0 for success, otherwise error number
 */
EXPORT int
ylru_put(struct ylru *,
	 const void *key, u32 keysz,
	 void *data, u32 data_size);

/**
 * NOTE: value is remove from cache!
 * So, after using it, user SHOULD put it again for future use.
 * @data_size: NULL is ignored.
 * @return: NULL if cache miss and fail to create new data by 'ylru_cb.create'
 */
EXPORT void *
ylru_get(struct ylru *,
	 u32 *data_size, /* returned data size */
	 const void *key, u32 keysz);

/**
 * Get size of cached data.
 * This is based on 'size' of 'ylru_put'.
 */
EXPORT u32
ylru_sz(struct ylru *);


#endif /* __YLRu_h__ */
