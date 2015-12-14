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
#include <string.h>
#include <stdint.h>
#include "common.h"
#include "yutils.h"
#include "yp.h"

/*
 * [ Assumption ]
 * sizeof(s32) <= sizeof(void *) && sizeof(s64) >= sizeof(void *).
 * (This is true for all-known popular platform)
 *
 * should be align with pointer size.
 *
 * Memory layout is
 * +-----------------+
 * | sizeof(s32)     |
 * | reference count |
 * +-----------------+
 * | sizeof(s64)     |
 * | size allocated  |
 * +-----------------+
 * | sizeof(s32)     |
 * | magic number    |
 * +-----------------+
 * | user memory     |
 * | area            |
 * +-----------------+
 * | sizeof(s32)     |
 * | magic number    |
 * +-----------------+
 *
 * magic number : array of one-byte-magic-value
 *
 * [ NOTE ]
 * 'Magic Number' and 'size allocated' slot exists only with CONFIG_DEBUG
 */

struct ypmblk {
	s32 refcnt; /* reference count */
#ifdef CONFIG_DEBUG
	u64 sz; /* Size of user memory allocated
                 * Excluding overheads
                 */
	u32 magic;
#endif /* CONFIG_DEBUG */
	/* to make 8-byte-alignment for user memory block */
	uintptr_t blk[0]; /* User memory block position.
			   * In case of DEBUG, 4-byte-magic-number is
			   *   added at the end
			   */
};


#ifdef CONFIG_DEBUG

#define YP_MAGIC 0xde

#define tail_magic_guard(p)				\
	((void *)(((char *)&(p)->blk) + (p)->sz))

static const char _magicn[sizeof(((struct ypmblk *)0)->magic)] =
	{[0 ... (yut_arrsz(_magicn) - 1)] = YP_MAGIC };

void *
ypmalloc(u32 sz) {
	struct ypmblk *p;
	p = ymalloc(sizeof(*p) + sz + sizeof(p->magic));
	if (unlikely(!p))
		return NULL;

	p->refcnt = 0;
	p->sz = sz;
        /* set head-magic-guard */
        memset(&p->magic, YP_MAGIC, sizeof(p->magic));
        /* set tail-magic-guard */
        memset(tail_magic_guard(p), YP_MAGIC, sizeof(p->magic));
	return (void *)&p->blk;
}

void
ypfree(void *v) {
	struct ypmblk *p = containerof(v, struct ypmblk, blk);
	/* check magic-guard */
	yassert(!memcmp(&_magicn, &p->magic, sizeof(p->magic))
		&& !memcmp(&_magicn, tail_magic_guard(p), sizeof(p->magic)));
	yfree(p);
}

void
ypput(void *v) {
	struct ypmblk *p = containerof(v, struct ypmblk, blk);
	/* check magic-guard */
	yassert(!memcmp(&_magicn, &p->magic, sizeof(p->magic))
		&& !memcmp(&_magicn, tail_magic_guard(p), sizeof(p->magic)));
	--p->refcnt;
	yassert(0 <= p->refcnt);
	if (unlikely(p->refcnt <= 0))
		yfree(p);
}

void *
ypget(void *v) {
	struct ypmblk *p = containerof(v, struct ypmblk, blk);
	/* check magic-guard */
	yassert(!memcmp(&_magicn, &p->magic, sizeof(p->magic))
		&& !memcmp(&_magicn, tail_magic_guard(p), sizeof(p->magic)));
	++p->refcnt;
	return v;
}

#else /* CONFIG_DEBUG */

void *
ypmalloc(u32 sz) {
	struct ypmblk *p;
	p = ymalloc(sizeof(*p) + sz);
	if (unlikely(!p))
		return NULL;
	p->refcnt = 0;
	return (void *)&p->blk;
}

void
ypfree(void *v) {
	yfree(containerof(v, struct ypmblk, blk));
}

void
ypput(void *v) {
	struct ypmblk *p = containerof(v, struct ypmblk, blk);
	--p->refcnt;
	yassert(0 <= p->refcnt);
	if (unlikely(p->refcnt <= 0))
		yfree(p);
}

void *
ypget(void *v) {
	struct ypmblk *p = containerof(v, struct ypmblk, blk);
	++p->refcnt;
	return v;
}

#endif /* CONFIG_DEBUG */

