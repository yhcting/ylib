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


/*
 * Memory pool is tested at ylisp and writer.
 * So, there is no specific test file....
 */

#ifndef __YMEMPOOl_h__
#define __YMEMPOOl_h__

#include "ydef.h"

/*
 * Options
 */
enum {
	YMP_mt_safe = 0x1,
};

/*
 * mp : Memory Pool
 */
struct ymp;

/**
 * size of pool will be expanded if needed.
 * this is just 'initial' size
 * @grpsz  : elem group size size in pool (number of element)
 * @elemsz : element size (in bytes)
 * @opt    : option.
 */
EXPORT struct ymp *
ymp_create(int grpsz, int elemsz, int opt);

EXPORT void
ymp_destroy(struct ymp *);

/**
 * get one block from pool.
 * NULL if fails. (Ex. Out Of Memory).
 */
EXPORT void *
ymp_get(struct ymp *);

/**
 * return block to pool.
 */
EXPORT void
ymp_put(struct ymp *, void *block);


#ifdef CONFIG_MEMPOOL_DYNAMIC
/**
 * interrupt shrinking.
 * this is NOT implemented yet!!
 * return  : -1 for error.
 */
EXPORT int
ymp_stop_shrink(struct ymp *);

/**
 * shrink memory pool.
 * 
 */
EXPORT int
ymp_shrink(struct ymp *, int margin);

#else /* CONFIG_MEMPOOL_DYNAMIC */
/*
 * Not supported at Non-dynamic-mempool.
 */

static inline int
ymp_stop_shrink(struct ymp *mp) {
	return -1;
}

/*
 * shrink memory pool.
 */
static inline int
ymp_shrink(struct ymp *mp, int margin) {
	return -1;
}

#endif /* CONFIG_MEMPOOL_DYNAMIC */

/*
 * return number of element
 */
EXPORT int
ymp_sz(struct ymp *);

/*
 * return number of used element
 */
EXPORT int
ymp_usedsz(struct ymp *);

#endif /* __YMEMPOOl_h__ */
