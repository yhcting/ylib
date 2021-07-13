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

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "common.h"
#include "yhash.h"
#include "ycrc.h"

#include <assert.h>


static void
vfree(void *v) {
	yfree(v);
}

static int
strkey_copy(const void **newkey, const void *key) {
	char *nk = ymalloc(strlen((char *)key) + 1);
	if (!nk)
		return ENOMEM;
	strcpy(nk, (char *)key);
	*newkey = (void *)nk;
	return 0;
}

static int
strkey_cmp(const void *k0, const void *k1) {
	return strcmp((char *)k0, (char *)k1);
}

static u32
strkey_hash(const void *key) {
	return ycrc32(0, (u8 *)key, strlen((char *)key) + 1);
}

static void
test_hasho(void) {
	int i;
	char buf[4096];
	void *v;
	int r;
	const char *keys[4096];

	/*
	 * Test normal hash.
	 */
	struct yhash *h = yhasho_create(
		YHASH_MEM_FREE,
		YHASH_MEM_FREE,
		&strkey_copy,
		&strkey_cmp,
		&strkey_hash);

	for (i = 0; i < 1024; i++) {
		snprintf(buf, sizeof(buf), "this is key %d", i);
		v = ymalloc(strlen(buf) + 1);
		strcpy(v, buf);
		/* key and value is same */
		yhash_set(h, (void *)buf, v);
		yassert(i + 1 == yhash_sz(h));
	}

	r = yhash_keys(h, (const void **)keys, 10);
	yassert(10 == r);
	for (i = 0; i < r; i++)
		yassert(strlen((char *)keys[i]) == strlen((char *)keys[i]));
	/*
	for (i = 0; i < r; i++)
		printf("%s\n", keys[i]);
	*/
	r = yhash_keys(h, (const void **)keys, 4096);
	yassert(1024 == r);
	for (i = 0; i < r; i++)
		yassert(strlen((char *)keys[i]) == strlen((char *)keys[i]));

	for (i = 256; i < 512; i++) {
		snprintf(buf, sizeof(buf), "this is key %d", i);
	        r = yhash_get(h, (void *)buf, &v);
		yassert(!r && 0 == strcmp(v, buf));
	}

	for (i = 1023; i >= 0; i--) {
		snprintf(buf, sizeof(buf), "this is key %d", i);
		yhash_remove(h, (void *)buf);
		yassert(i == yhash_sz(h));
	}
	r = yhash_keys(h, (const void **)keys, 10);
	yassert(0 == r);

	yhash_reset(h);
	yhash_destroy(h);
}

static void
test_hashs(void) {
	int i;
	char buf[4096];
	void *v;
	int r;
	const char *keys[4096];
	/*
	 * Test normal hash.
	 */
	struct yhash *h = yhashs_create(YHASH_MEM_FREE, TRUE);

	for (i = 0; i < 1024; i++) {
		snprintf(buf, sizeof(buf), "this is key %d", i);
		v = ymalloc(strlen(buf) + 1);
		strcpy(v, buf);
		/* key and value is same */
		yhash_set(h, (void *)buf, v);
		yassert(i + 1 == yhash_sz(h));
	}

	r = yhash_keys(h, (const void **)keys, 10);
	yassert(10 == r);
	for (i = 0; i < r; i++)
		yassert(strlen((char *)keys[i]) == strlen((char *)keys[i]));
	/*
	for (i = 0; i < r; i++)
		printf("%s\n", keys[i]);
	*/
	r = yhash_keys(h, (const void **)keys, 4096);
	yassert(1024 == r);
	for (i = 0; i < r; i++)
		yassert(strlen((char*)keys[i]) == strlen((char *)keys[i]));

	for (i = 256; i < 512; i++) {
		snprintf(buf, sizeof(buf), "this is key %d", i);
	        r = yhash_get(h, (void *)buf, &v);
		yassert(!r && 0 == strcmp(v, buf));
	}

	for (i = 1023; i >= 0; i--) {
		snprintf(buf, sizeof(buf), "this is key %d", i);
		yhash_remove(h, (void *)buf);
		yassert(i == yhash_sz(h));
	}
	r = yhash_keys(h, (const void **)keys, 10);
	yassert(0 == r);

	yhash_reset(h);
	yhash_destroy(h);
}

static void
test_hashi(void) __unused;
static void
test_hashi(void) {
	int i, r;
	char buf[4096];
	char *ptsv[1024];
	void *v;
	/*
	 * Test address hash.
	 */
	struct yhash *h = yhashi_create(&vfree);

	for (i = 0; i < 1024; i++) {
		snprintf(buf, sizeof(buf), "this is key %d", i);
		v = ymalloc(strlen(buf)+1);
		strcpy(v, buf);
		/* key and value is same */
		yhash_set(h, (void *)v, v);
		ptsv[i] = v;
		yassert(i+1 == yhash_sz(h));
	}

	for (i = 256; i < 512; i++) {
		snprintf(buf, sizeof(buf), "this is key %d", i);
		r = yhash_get(h, (void *)ptsv[i], &v);
		yassert(!r && 0 == strcmp(v, buf));
	}

	for (i = 1023; i >= 0; i--) {
		yhash_remove(h, (void *)ptsv[i]);
		yassert(i == yhash_sz(h));
	}

	yhash_destroy(h);
}


static void
test_hash(void) {
	test_hasho();
	test_hashs();
	test_hashi();
}

TESTFN(hash)

#endif /* CONFIG_DEBUG */
