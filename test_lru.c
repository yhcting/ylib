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

#include <string.h>
#include <assert.h>

#include "ycommon.h"
#include "ylru.h"
#include "test.h"


static void
vfree(void *v) {
	yfree(v);
}

static void *
vcreate(u32 *data_size,
	const void *key, u32 keysz) {
	int *i;
	*data_size = sizeof(int);
	i = ymalloc(sizeof(*i));
	*i = 10;
	return i;
}

static void
test_lru(void) {
	int *pi;
	struct ylru_cb cbs = {
		.free = &vfree,
		.create = NULL
	};
	struct ylru *lru = ylru_create(sizeof(int) * 3, &cbs);

	pi = (int *)ylru_get(lru, NULL, "key0", 5);
	yassert(!pi);

	pi = ymalloc(sizeof(*pi));
	*pi = 100;
	ylru_put(lru, "k100", 5, pi, sizeof(*pi));
	yassert(sizeof(int) == ylru_sz(lru));

	pi = (int *)ylru_get(lru, NULL, "k100", 5);
	yassert(100 == *pi);
	yassert(0 == ylru_sz(lru));
	ylru_put(lru, "k100", 5, pi, sizeof(*pi));

	pi = ymalloc(sizeof(*pi));
	*pi = 200;
	ylru_put(lru, "k200", 5, pi, sizeof(*pi));
	yassert(sizeof(int) * 2 == ylru_sz(lru));

	pi = ymalloc(sizeof(*pi));
	*pi = 300;
	ylru_put(lru, "k300", 5, pi, sizeof(*pi));
	yassert(sizeof(int) * 3 == ylru_sz(lru));

	pi = ymalloc(sizeof(*pi));
	*pi = 400;
	ylru_put(lru, "k400", 5, pi, sizeof(*pi));
	yassert(sizeof(int) * 3 == ylru_sz(lru));

	/* Now lru is [ 200 - 300 - 400 (newest) ] */

	pi = (int *)ylru_get(lru, NULL, "k100", 5);
	yassert(!pi);
	pi = (int *)ylru_get(lru, NULL, "k200", 5);
	yassert(200 == *pi);
	yassert(sizeof(int) * 2 == ylru_sz(lru));
	ylru_put(lru, "k200", 5, pi, sizeof(*pi));

	/* Now, [ 300 - 400 - 200 (newest) ] */

	pi = ymalloc(sizeof(*pi));
	*pi = 500;
	ylru_put(lru, "k500", 5, pi, sizeof(*pi));
	yassert(sizeof(int) * 3 == ylru_sz(lru));

	/* Should be [ 400 - 200 - 500 ] */

	pi = (int *)ylru_get(lru, NULL, "k300", 5);
	yassert(!pi);

	ylru_clean(lru);
	ylru_destroy(lru);

	cbs.create = &vcreate;
	lru = ylru_create(sizeof(int) * 3, &cbs);
	pi = (int *)ylru_get(lru, NULL, "k000", 5);
	yassert(pi);
	yassert(10 == *pi);
	ylru_put(lru, "k000", 5, pi, sizeof(*pi));
	ylru_destroy(lru);
}


TESTFN(test_lru, lru)
