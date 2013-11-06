/*****************************************************************************
 *    Copyright (C) 2011 Younghyung Cho. <yhcting77@gmail.com>
 *
 *    This file is part of ylib
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as
 *    published by the Free Software Foundation either version 3 of the
 *    License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License
 *    (<http://www.gnu.org/licenses/lgpl.html>) for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.	If not, see <http://www.gnu.org/licenses/>.
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

#include "config.h"

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
#       define NULL ((void*)0)
#endif

#ifndef TRUE
#       define TRUE 1
#endif

#ifndef FALSE
#       define FALSE 0
#endif

#ifndef offset_of
#       define offset_of(type, member) ((unsigned long) &((type*)0)->member)
#endif

#ifndef container_of
#       define container_of(ptr, type, member)			\
	((type*)(((char*)(ptr)) - offset_of(type, member)))
#endif

/**************
 * GCC Specific
 **************/
#ifndef likely
#       define likely(x)	__builtin_expect(!!(x), 1)
#endif

#ifndef unlikely
#       define unlikely(x)	__builtin_expect(!!(x), 0)
#endif

#define barrier()       __asm__ __volatile__("": : :"memory")


/*****************************************************************************
 *
 * Debugging
 *
 *****************************************************************************/

#define YDBG

/* For debugging */
#ifdef YDBG

#       include <malloc.h>
#       include <assert.h>

#       define ymalloc(x)   dmalloc(x)
#       define yfree(x)     dfree(x)
#       define yassert(x)   assert(x)

    extern void* dmalloc(unsigned int);
    extern void  dfree(void*);

#else /* YDBG */

#       include <malloc.h>

#       define ymalloc(x)   malloc(x)
#       define yfree(x)     free(x)
#       define yassert(x)   do {} while (0)

#endif /* YDBG */

#endif /* __YDEf_h__ */
