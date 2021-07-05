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
#include <stdlib.h>

#include "common.h"
#include "yset.h"

yset_t
yset_intersect(const yset_t s0, const yset_t s1) {
        /* NOT TESTED YET */
	yset_t ss; /* source set - smaller set */
	yset_t ts; /* target set - larger set */
	u32 sz0, sz1, sssz, i;
	const void **ebuf = NULL;
	yset_t s = NULL;
	if (unlikely(!s0
		|| !s1
		|| !yhash_is_sametype(s0, s1))
	) { return NULL; }
	if (unlikely(!(s = yset_create(s0))))
		return NULL; /* ENOMEM */
	sz0 = yset_sz(s0);
	sz1 = yset_sz(s1);
	/* Trivial case : There is empty set. */
	if (unlikely(!sz0 || !sz1))
		return s; /* return empty set */

	if (sz0 < sz1) {
		ss = s0;
		ts = s1;
		sssz = sz0;
	} else {
		ss = s1;
		ts = s0;
		sssz = sz1;
	}
	if (unlikely(!(ebuf = (const void **)ymalloc(sizeof(*ebuf) * sssz))))
		goto fail; /* ENOMEM */
	if (unlikely(sssz != yset_elements(ss, ebuf, sssz)))
		/* this is totally unexpected! */
		goto fail;

	i = sssz;
	while (i--) {
		if (yset_has(ts, ebuf[i])) {
			if (unlikely(0 > yset_add(s, (void *)ebuf[i])))
				goto fail; /* ENOMEM */
		}
	}

	goto done;

 fail:
	if (likely(s))
		yset_destroy(s);
	s = NULL;
 done:
	if (likely(ebuf))
		free(ebuf);
	return s;

}

yset_t
yset_union(const yset_t s0, const yset_t s1) {
        /* NOT IMPLEMENTED YET */
        return NULL;
}

yset_t
yset_diff(const yset_t s0, const yset_t s1) {
        /* NOT IMPLEMENTED YET */
        return NULL;
}
