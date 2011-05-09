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

/*
 *
 */

/*
 * TODO :
 *    adjust visibility
 *     (Use default visibility in temporal)
 */
#define EXPORT __attribute__ ((visibility ("default")))

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
#       define offset_of(type, member) ((unsigned int) &((type*)0)->member)
#endif

#ifndef container_of
#       define container_of(ptr, type, member)			\
	((type*)(((char*)(ptr)) - offset_of(type, member)))
#endif

#define unroll16( expr, count, cond)	\
	switch( (count) & 0xf ) {	\
        case 0: while (cond){		\
			expr;		\
	case 15: expr;			\
	case 14: expr;			\
	case 13: expr;			\
	case 12: expr;			\
	case 11: expr;			\
	case 10: expr;			\
	case 9: expr;			\
	case 8: expr;			\
	case 7: expr;			\
	case 6: expr;			\
	case 5: expr;			\
	case 4: expr;			\
	case 3: expr;			\
	case 2: expr;			\
	case 1: expr;			\
	}				\
}

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
