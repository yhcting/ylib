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

#ifndef __Yp_h__
#define __Yp_h__

#include "ydef.h"

/*****************************************************************************
 *
 * Simple Smart Pointer.
 *
 *****************************************************************************/

/*
 * should be align with pointer size.
 * And, sizeof(long) == sizeof(void *) in general.
 *
 * Memory layout is
 * +-----------------+--------------+--------------
 * |   sizeof(long)  | sizeof(long) | user memory
 * | reference count | magic number | area
 * +-----------------+--------------+--------------
 *
 * NOTE : 'Magic Number' slot exists only with CONFIG_DEBUG
 */

#ifdef CONFIG_DEBUG

#define YP_MAGIC 0xfaebcddcL

static inline void *
ypmalloc(unsigned int sz) {
	long *p;
	p = (long *)ymalloc(sz + sizeof(long) * 2);
	*p++ = 0;
	*p++ = YP_MAGIC;
	return (void *)p;
}

#else /* CONFIG_DEBUG */

static inline void *
ypmalloc(unsigned int sz) {
	long *p;
	p = (long *)ymalloc(sz + sizeof(long));
	*p++ = 0;
	return (void *)p;
}


#endif /* CONFIG_DEBUG */

/* SP Put */
static inline void
ypput(void *v) {
	long *p = (long *)v;
	yassert(YP_MAGIC == *--p);
	--*--p;
	yassert(0 <= *p);
	if (unlikely(*p <= 0))
		yfree(p);
}

/* SP Get */
static inline void *
ypget(void *v) {
	long *p = (long *)v;
	yassert(YP_MAGIC == *--p);
	++*--p;
	return v;
}


#endif /* __Yp_h__ */
