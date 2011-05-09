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

#include "yhash.h"
#include "common.h"

#include <assert.h>

extern int dmem_count();

static void
_free(void* v) {
	yfree(v);
}

void
test_hash() {
	int           sv = dmem_count();
	int           i;
	char          buf[4096];
	char*         v;
	struct yhash* h = yhash_create(&_free);

	for (i=0; i<1024; i++) {
		snprintf(buf, sizeof(buf), "this is key %d", i);
		v = ymalloc(strlen(buf)+1);
		strcpy(v, buf);
		/* key and value is same */
		yhash_add(h, (unsigned char*)buf, strlen(buf) + 1, v);
		yassert(i+1 == yhash_sz(h));
	}

	for (i=256; i<512; i++) {
		snprintf(buf, sizeof(buf), "this is key %d", i);
		v = yhash_find(h, (unsigned char*)buf, strlen(buf)+1);
		yassert(v && 0 == strcmp(v, buf));
	}

	for (i=1023; i>=0; i--) {
		snprintf(buf, sizeof(buf), "this is key %d", i);
		yhash_del(h, (unsigned char*)buf, strlen(buf)+1);
		yassert(i == yhash_sz(h));
	}

	yhash_destroy(h);

	yassert(sv == dmem_count());
}

