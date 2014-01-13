/******************************************************************************
 *    Copyright (C) 2011, 2012, 2013, 2014
 *    Younghyung Cho. <yhcting77@gmail.com>
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


#include <stdio.h>
#include <string.h>

#include "yhash.h"
#include "common.h"
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
	uint32_t r;
	const uint8_t *keys[4096];
	uint32_t keyssz[4096];

	/*
	 * Test normal hash.
	 */
	struct yhash *h = yhash_create(&vfree);

	for (i = 0; i < 1024; i++) {
		snprintf(buf, sizeof(buf), "this is key %d", i);
		v = ymalloc(strlen(buf)+1);
		strcpy(v, buf);
		/* key and value is same */
		yhash_add(h, (uint8_t *)buf, strlen(buf) + 1, v);
		yassert(i+1 == yhash_sz(h));
	}

	r = yhash_keys(h, (const void **)keys, keyssz, 10);
	yassert(10 == r);
	for (i = 0; i < r; i++)
		yassert(keyssz[i] == ((uint32_t)strlen((char *)keys[i]) + 1));
	/*
	for (i = 0; i < r; i++)
		printf("%s\n", keys[i]);
	*/
	r = yhash_keys(h, (const void **)keys, keyssz, 4096);
	yassert(1024 == r);
	for (i = 0; i < r; i++)
		yassert(keyssz[i] == ((uint32_t)strlen((char *)keys[i]) + 1));

	for (i = 256; i < 512; i++) {
		snprintf(buf, sizeof(buf), "this is key %d", i);
		v = yhash_find(h, (uint8_t *)buf, strlen(buf)+1);
		yassert(v && 0 == strcmp(v, buf));
	}

	for (i = 1023; i >= 0; i--) {
		snprintf(buf, sizeof(buf), "this is key %d", i);
		yhash_del(h, (uint8_t *)buf, strlen(buf)+1);
		yassert(i == yhash_sz(h));
	}
	r = yhash_keys(h, (const void **)keys, keyssz, 10);
	yassert(0 == r);

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
		yhash_add(h, (uint8_t *)v, 0, v);
		ptsv[i] = v;
		yassert(i+1 == yhash_sz(h));
	}

	for (i = 256; i < 512; i++) {
		snprintf(buf, sizeof(buf), "this is key %d", i);
		v = yhash_find(h, (uint8_t *)ptsv[i], 0);
		yassert(v && 0 == strcmp(v, buf));
	}

	for (i = 1023; i >= 0; i--) {
		yhash_del(h, (uint8_t *)ptsv[i], 0);
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
