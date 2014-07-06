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


/******************************************************************************
 *
 * Naming convention
 *      "__xxxx" : internal use only!
 *      "_xxxx"  : do not use directly if possible
 *                 (means "BE CAREFUL when use this type interface")
 *      "xxxx"   : interface
 *      "xxxx_"  : core sub-function of function "xxxx".
 *                 This can be extended to "xxxx__", "xxxx___" and so on.
 *
 *****************************************************************************/

#ifndef __YDEf_h__
#define __YDEf_h__

#ifndef CONFIG_IGNORE_CONFIG
#include "config.h"
#endif

/*
 *
 */

/*
 * TODO :
 *    adjust visibility
 *     (Use default visibility in temporal)
 */
#define EXPORT __attribute__ ((visibility ("default")))


/*****************************************************************************
 *
 * Basic macros
 *
 *****************************************************************************/

/* if compiler doesn't support 'inline' directive, we should enable below */
/* #define inline */

#ifndef NULL
#	define NULL ((void *)0)
#endif

#ifndef TRUE
#	define TRUE 1
#endif

#ifndef FALSE
#	define FALSE 0
#endif

#ifndef offset_of
#	define offset_of(type, member) ((unsigned long) &((type *)0)->member)
#endif

#ifndef container_of
#	define container_of(ptr, type, member)			\
	((type *)(((char *)(ptr)) - offset_of(type, member)))
#endif

/**************
 * GCC Specific
 **************/
#ifndef likely
#	define likely(x)	__builtin_expect(!!(x), 1)
#endif

#ifndef unlikely
#	define unlikely(x)	__builtin_expect(!!(x), 0)
#endif

#ifndef barrier
#	define barrier()       __asm__ __volatile__("": : :"memory")
#endif


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

extern void * dmalloc(unsigned int);
extern void * dcalloc(unsigned int, unsigned int);
extern void   dfree(void *);

#else /* CONFIG_DEBUG */

#	include <malloc.h>

#	define ymalloc   malloc
#	define ycalloc   calloc
#	define yfree     free
#	define yassert(x)   do { } while (0)

#endif /* CONFIG_DEBUG */

#endif /* __YDEf_h__ */
