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
#include "test.h"
#ifdef CONFIG_DEBUG

#include <string.h>
#include <assert.h>

#include "common.h"
#include "ylru.h"


static void
data_free(void *v) __unused;

static void
data_free(void *v) {
	yfree(v);
}

static void *
data_create(const void *key) {
	int *i;
	i = ymalloc(sizeof(*i));
	*i = 10;
	return i;
}

static u32
data_size(const void *d) {
	return sizeof(int);
}

static void
test_lru(void) {
	int *pi;
	struct ylru *lru = ylrus_create(sizeof(int) * 3,
					YLRU_PREDEFINED_FREE,
					NULL,
					&data_size);

	yassert(1 == ylru_get(lru, (void **)&pi, "key0"));
	pi = ymalloc(sizeof(*pi));
	*pi = 100;
	ylru_put(lru, "k100", pi);
	yassert(sizeof(int) == ylru_sz(lru));

	yassert(0 == ylru_get(lru, (void **)&pi, "k100"));
	yassert(100 == *pi);
	yassert(0 == ylru_sz(lru));
	ylru_put(lru, "k100", pi);

	pi = ymalloc(sizeof(*pi));
	*pi = 200;
	ylru_put(lru, "k200", pi);
	yassert(sizeof(int) * 2 == ylru_sz(lru));

	pi = ymalloc(sizeof(*pi));
	*pi = 300;
	ylru_put(lru, "k300", pi);
	yassert(sizeof(int) * 3 == ylru_sz(lru));

	pi = ymalloc(sizeof(*pi));
	*pi = 400;
	ylru_put(lru, "k400", pi);
	yassert(sizeof(int) * 3 == ylru_sz(lru));

	/* Now lru is [ 200 - 300 - 400 (newest) ] */

	yassert(1 == ylru_get(lru, (void **)&pi, "k100"));
	yassert(0 == ylru_get(lru, (void **)&pi, "k200"));
	yassert(200 == *pi);
	yassert(sizeof(int) * 2 == ylru_sz(lru));
	ylru_put(lru, "k200", pi);

	/* Now, [ 300 - 400 - 200 (newest) ] */

	pi = ymalloc(sizeof(*pi));
	*pi = 500;
	ylru_put(lru, "k500", pi);
	yassert(sizeof(int) * 3 == ylru_sz(lru));

	/* Should be [ 400 - 200 - 500 ] */

	yassert(1 == ylru_get(lru, (void **)&pi, "k300"));

	ylru_reset(lru);
	ylru_destroy(lru);

	lru = ylrus_create(sizeof(int) * 3,
			   YLRU_PREDEFINED_FREE,
			   &data_create,
			   &data_size);
	yassert(0 == ylru_get(lru, (void **)&pi, "k000"));
	yassert(10 == *pi);
	ylru_put(lru, "k000", pi);
	ylru_destroy(lru);
}


TESTFN(lru)

#endif /* CONFIG_DEBUG */
