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


#include <stdio.h>
#include <string.h>

#include "ycommon.h"
#include "yhash.h"
#include "test.h"

#include <assert.h>


static void
vfree(void *v) {
	yfree(v);
}

static void
test_hash_normal(void) {
	int    i;
	char   buf[4096];
	char  *v;
	unsigned int r;
	const char *keys[4096];
	unsigned int keyssz[4096];

	/*
	 * Test normal hash.
	 */
	struct yhash *h = yhash_create(&vfree);

	for (i = 0; i < 1024; i++) {
		snprintf(buf, sizeof(buf), "this is key %d", i);
		v = ymalloc(strlen(buf) + 1);
		strcpy(v, buf);
		/* key and value is same */
		yhash_add(h, (void *)buf, strlen(buf) + 1, v);
		yassert(i + 1 == yhash_sz(h));
	}

	r = yhash_keys(h, (const void **)keys, keyssz, 10);
	yassert(10 == r);
	for (i = 0; i < r; i++)
		yassert(keyssz[i] == ((unsigned int)strlen((char *)keys[i]) + 1));
	/*
	for (i = 0; i < r; i++)
		printf("%s\n", keys[i]);
	*/
	r = yhash_keys(h, (const void **)keys, keyssz, 4096);
	yassert(1024 == r);
	for (i = 0; i < r; i++)
		yassert(keyssz[i] == ((unsigned int)strlen((char *)keys[i]) + 1));

	for (i = 256; i < 512; i++) {
		snprintf(buf, sizeof(buf), "this is key %d", i);
		v = yhash_find(h, (void *)buf, strlen(buf)+1);
		yassert(v && 0 == strcmp(v, buf));
	}

	for (i = 1023; i >= 0; i--) {
		snprintf(buf, sizeof(buf), "this is key %d", i);
		yhash_del(h, (void *)buf, strlen(buf)+1);
		yassert(i == yhash_sz(h));
	}
	r = yhash_keys(h, (const void **)keys, keyssz, 10);
	yassert(0 == r);

	yhash_clean(h);
	yhash_destroy(h);
}

static void
test_hash_address(void) {
	int   i;
	char  buf[4096];
	char *ptsv[1024];
	char *v;
	/*
	 * Test address hash.
	 */
	struct yhash *h = yhash_create(&vfree);

	for (i = 0; i < 1024; i++) {
		snprintf(buf, sizeof(buf), "this is key %d", i);
		v = ymalloc(strlen(buf)+1);
		strcpy(v, buf);
		/* key and value is same */
		yhash_add(h, (void *)v, 0, v);
		ptsv[i] = v;
		yassert(i+1 == yhash_sz(h));
	}

	for (i = 256; i < 512; i++) {
		snprintf(buf, sizeof(buf), "this is key %d", i);
		v = yhash_find(h, (void *)ptsv[i], 0);
		yassert(v && 0 == strcmp(v, buf));
	}

	for (i = 1023; i >= 0; i--) {
		yhash_del(h, (void *)ptsv[i], 0);
		yassert(i == yhash_sz(h));
	}

	yhash_destroy(h);
}

static void
test_hash(void) {
	test_hash_normal();
	test_hash_address();
}

TESTFN(test_hash, hash)
