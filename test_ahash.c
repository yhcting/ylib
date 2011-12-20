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


#include <stdio.h>
#include <string.h>

#include "yahash.h"
#include "common.h"
#include "test.h"

#include <assert.h>

static void
_test_ahash(void) {
	int           i;
	char          buf[4096];
	char*         ptsv[1024];
	char*         v;
	/*
	 * Test address hash.
	 */
	struct yahash* h = yahash_create();

	for (i = 0; i < 1024; i++) {
		snprintf(buf, sizeof(buf), "this is key %d", i);
		v = ymalloc(strlen(buf)+1);
		strcpy(v, buf);
		/* key and value is same */
		yahash_add(h, v);
		ptsv[i] = v;
		yassert(i+1 == yahash_sz(h));
	}

	for (i = 256; i < 512; i++) {
		yassert(yahash_check(h, ptsv[i]));
	}

	for (i = 1023; i >= 0; i--) {
		yahash_del(h, ptsv[i]);
		yassert(i == yahash_sz(h));
	}

	for (i = 0; i < 1024; i++)
		yfree(ptsv[i]);

	yahash_destroy(h);
}

TESTFN(_test_ahash, ahash)
