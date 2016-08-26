/******************************************************************************
 * Copyright (C) 2015
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
#include <errno.h>

#include "common.h"
#include "ydynb.h"

static INLINE u32
padsz(u16 esz, u8 align) {
	return (align - esz % align) % align;
}

/* Element size including paddings */
static INLINE u32
elemsz(u16 esz, u8 align) {
	return esz + padsz(esz, align);
}

/****************************************************************************
 *
 * Interfaces
 *
 ****************************************************************************/
YYEXPORT void
ydynb_dump(const struct ydynb *b) {
	dpr(
"limit: %u\n"
"sz   : %u\n"
"esz  : %u\n"
"eszex: %u\n",
	b->limit, b->sz, b->esz, b->eszex);
}

struct ydynb *
ydynb_create(u32 init_limit, uint16_t esz, uint8_t align) {
	struct ydynb *b;
	if (unlikely(0 == init_limit))
		return NULL;
	if (unlikely(!(b = (struct ydynb *)ymalloc(sizeof(*b)))))
		return NULL;
	b->limit = init_limit;
	b->sz = 0;
	b->esz = esz;
	b->eszex = elemsz(esz, align);
	if (unlikely(!(b->b = ymalloc(init_limit * b->eszex)))) {
		yfree(b);
		return NULL;
	}
	return b;
}

void *
ydynb_destroy(struct ydynb *b, bool pop_buf) {
	void *r = NULL;
	if (pop_buf)
		r = b->b;
	else
		yfree(b->b);
	yfree(b);
	return r;
}

int
ydynb_expand(struct ydynb *b) {
	void *tmp = yrealloc(b->b, b->limit * b->eszex * 2);
	if (unlikely(!tmp))
		return -ENOMEM;
	b->b = tmp;
	b->limit *= 2;
	return 0;
}

int
ydynb_shrink(struct ydynb *b, u32 sz_to) {
	void *tmp;
	if (unlikely(b->limit <= sz_to
		     || b->sz > sz_to))
		return -EINVAL;
	tmp = yrealloc(b->b, sz_to * b->eszex);
	if (unlikely(!tmp))
		return -ENOMEM;
	b->b = tmp;
	b->limit = sz_to;
	return 0;
}

int
ydynb_appends(struct ydynb *b, const void *ea, u32 easz) {
	int r;
	u32 cpbytes;
	if (unlikely(!easz))
		return 0; /* Nothing to do */
	r = ydynb_expand2(b, easz);
	if (unlikely(0 > r))
		return r;
	cpbytes = b->eszex * (easz - 1);
	memcpy(ydynb_getfree(b), ea, cpbytes);
	b->sz += (easz - 1);
	memcpy(ydynb_getfree(b), (char *)ea + cpbytes, b->esz);
	b->sz++;
	return 0;
}
