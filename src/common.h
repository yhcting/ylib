/******************************************************************************
 * Copyright (C) 2015, 2023
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

#pragma once

#include <stdlib.h>

#include "ylog.h"
#include "def.h"


/*****************************************************************************
 *
 * Debugging
 *
 *****************************************************************************/
/* For debugging */
#ifdef CONFIG_DEBUG

#include <malloc.h>
#include <assert.h>
#include <stdio.h>

#define ymalloc(sz) dmalloc(sz, __FILE__, __LINE__)
#define yrealloc(p, sz) drealloc(p, sz, __FILE__, __LINE__)
#define ycalloc(n, sz) dcalloc(n, sz, __FILE__, __LINE__)
#define yfree dfree
#define yassert(x) assert(x)
#define ystrdup(p) dstrdup(p, __FILE__, __LINE__)

EXPORT void *dmalloc(size_t, const char *, int);
EXPORT void *drealloc(void *, size_t, const char *, int);
EXPORT void *dcalloc(size_t, size_t, const char *, int);
EXPORT void dfree(void *);
EXPORT char *dstrdup(const char *, const char *, int);

#else /* CONFIG_DEBUG */

#include <malloc.h>

#define ymalloc malloc
#define yrealloc realloc
#define ycalloc calloc
#define yfree free
#define yassert(x) do { } while (0)
#define ystrdup strdup

#endif /* CONFIG_DEBUG */


/*****************************************************************************
 *
 *
 *
 *****************************************************************************/
#ifdef YDPRINT

/* Debug PRint */
#define dpr(a, b...) printf(a, ##b);
/* Debug Function PRint */
#define dfpr(a, b...) printf("%s: " a "\n", __func__, ##b);
/* Debug POSition PRint */
#define dpospr(a, b...) \
	printf("%s: %d: " a "\n", __FILE__, __LINE__, ##b);
/* Debug CHecK PoinT */
#define dchkpt() printf("%s: %d\n", __FILE__, __LINE__);

#else /* YDPRINT */

#define dpr(a, b...) do { } while (0)
#define dfpr(a, b...) do { } while (0)
#define dpospr(a, b...) do { } while (0)
#define dchkpt() do { } while (0)

#endif /* YDPRINT */




/*****************************************************************************
 *
 *
 *
 *****************************************************************************/
#define declare_lock(lOCKtYPE, cONTAINERtYPE, nAME, iNIToPT)		\
	static inline void						\
	init_##nAME##_lock(cONTAINERtYPE *o) {				\
		fatali0(pthread_##lOCKtYPE##_init(			\
			&o->nAME##_lock, iNIToPT));			\
	}								\
	static inline void						\
	lock_##nAME(cONTAINERtYPE *o) {					\
		fatali0(pthread_##lOCKtYPE##_lock(&o->nAME##_lock));	\
	}								\
	static inline void						\
	unlock_##nAME(cONTAINERtYPE *o) {				\
		fatali0(pthread_##lOCKtYPE##_unlock(&o->nAME##_lock));	\
	}								\
	static inline void						\
	destroy_##nAME##_lock(cONTAINERtYPE *o) {			\
		fatali0(pthread_##lOCKtYPE##_destroy(&o->nAME##_lock)); \
	}



/*****************************************************************************
 *
 *
 *
 *****************************************************************************/
/**
 * Die(exit process with exit code 1).
 */
#define die()			\
	do {			\
		yassert(FALSE);	\
		exit(1);	\
	} while (FALSE)

/**
 * Die(exit process with exit code 1 with fatal log message.)
 */
#define die2(fmt, args...)		\
	do {				\
		ylogf(fmt"\n", ##args);	\
		die();			\
	} while (FALSE)

/**
 * Die if cond is NOT TRUE.
 */
#define fatal(cond)			\
	if (unlikely(!(cond))) {	\
		die();			\
	}

/**
 * Die if cond is NOT TRUE with given message.
 */
#define fatal2(cond, fmt, args...)	\
	if (unlikely(!(cond))) {	\
		die2(fmt, ##args);	\
	}

/**
 * Call fatal function that returns 'int 0' if success.
 * If function fails, 'assert' is triggered.
 */
#define fatali0(int0_return_func_call_stmt)			\
	do {							\
		int rr___  = int0_return_func_call_stmt;	\
		fatal2(0 == rr___,				\
			"Failure at MUST-SUCCESS-Function!: %d",\
			rr___);					\
	} while (FALSE)
