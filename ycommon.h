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


#ifndef __YCOMMOn_h__
#define __YCOMMOn_h__

#include "ydef.h"

/*****************************************************************************
 *
 * General Macros
 *
 *****************************************************************************/
#ifndef abs
#	define abs(x) (((x)>0)?(x):-(x))
#endif

#ifndef swap
#	define swap(x,y,tmp) do { (tmp)=(x);(x)=(y);(y)=(tmp); } while (0)
#endif

#ifndef min
#	define min(x,y) (((x)<(y))?x:y)
#endif

#ifndef max
#	define max(x,y) (((x)<(y))?y:x)
#endif

/*****************************************************************************
 *
 * Bit operations
 *
 *****************************************************************************/
#define bits(x, offset, bitsz) \
	(((x) >> (offset)) & ((1LL << (bitsz)) - 1))

#define bit(x, offset) bits(x, offset, 1)

#define clear_bits(x, offset, bitsz) \
	((x) & ~(((1LL << (bitsz)) - 1) << (offset)))
/*
 * replace bits from 'offset' to 'offset' + 'bitsz' of 'x' with bits
 *   from 0 to 'bitsz' of 'v'.
 */
#define set_bits(x, offset, bitsz, v) \
	(clear_bits(x, offset, bitsz)			\
	 | ((((1LL << (bitsz)) - 1) & (v)) << (offset)))

#define test_bit(x, bit) \
	(((x) >> (bit)) & 1LL)

#define set_bit(x, bit) \
	((x) | (1LL << (bit)))

#define clear_bit(x, bit) \
	((x) & ~(1LL << (bit)))


/*****************************************************************************
 *
 * Others
 *
 *****************************************************************************/
#define unroll16(count, stmt)					\
	/* Just in case, minimize referencing 'count' argument. \
	 * Reason: 'count' argument may update some values.	\
	 */							\
	do {							\
		int ___n___;					\
		int ___c___ = (count);				\
		if (unlikely(___c___ < 0)) {			\
			___c___ = 0;				\
		}						\
		___n___ = ___c___ / 16;				\
		switch (___c___ & 0xf) {			\
		case 0: while ( ___n___--) {			\
				stmt;				\
			case 15: stmt;				\
			case 14: stmt;				\
			case 13: stmt;				\
			case 12: stmt;				\
			case 11: stmt;				\
			case 10: stmt;				\
			case 9:	 stmt;				\
			case 8:	 stmt;				\
			case 7:	 stmt;				\
			case 6:	 stmt;				\
			case 5:	 stmt;				\
			case 4:	 stmt;				\
			case 3:	 stmt;				\
			case 2:	 stmt;				\
			case 1:	 stmt;				\
			}					\
		}						\
	} while (0)


#define arrsz(a) ((int)(sizeof(a)/sizeof((a)[0])))

#define sec2ns(x) ((x) * 1000 * 1000 * 1000)

/*****************************************************************************
 *
 * Debugging
 *
 *****************************************************************************/
/* For debugging */
#ifdef CONFIG_DEBUG

#	include <malloc.h>
#	include <assert.h>

#	define ymalloc   dmalloc
#	define ycalloc   dcalloc
#	define yfree     dfree
#	define yassert(x)   assert(x)

extern void * dmalloc(u32);
extern void * dcalloc(u32, u32);
extern void   dfree(void *);

#else /* CONFIG_DEBUG */

#	include <malloc.h>

#	define ymalloc   malloc
#	define ycalloc   calloc
#	define yfree     free
#	define yassert(x)   do { } while (0)

#endif /* CONFIG_DEBUG */


#endif /* __YCOMMOn_h__ */
