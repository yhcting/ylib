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
 * @file ymempool.h
 * @brief Header file to use memory pool.
 */

#pragma once

/*
 * Memory pool is tested at ylisp and writer.
 * So, there is no specific test file....
 */

#include "ydef.h"

/*
 * Options
 */
enum {
	YMEMPOOL_mt_safe = 0x1, /**< MultiThread-safe */
};

/** Memory pool object */
struct ymempool;

/**
 * Create memory pool.
 * Size of pool will be expanded if needed.
 *
 * @param grpsz Elem group size size in pool (number of element)
 * @param elemsz Element size (in bytes)
 * @param opt Option. See @ref YMEMPOOL_mt_safe
 * @return NULL if fails.
 */
YYEXPORT struct ymempool *
ymempool_create(int grpsz, int elemsz, int opt);

/**
 * Destroy memory pool.
 */
YYEXPORT void
ymempool_destroy(struct ymempool *);

/**
 * Get one block from pool.
 *
 * @return NULL if fails. (Ex. Out Of Memory).
 */
YYEXPORT void *
ymempool_get(struct ymempool *);

/**
 * Return memory block got from @ref ymempool_get, to pool.
 */
YYEXPORT void
ymempool_put(struct ymempool *, void *block);

/**
 * Get size of memory pool.
 *
 * @return Number of element
 */
YYEXPORT int
ymempool_sz(struct ymempool *);

/**
 * Get used size of memory pool.
 *
 * @return Number of used element
 */
YYEXPORT int
ymempool_usedsz(struct ymempool *);
