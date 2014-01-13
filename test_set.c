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

#include "common.h"
#include "yset.h"
#include "test.h"

#include <assert.h>

static void
test_set(void) {
	int i, r;
	int *elems[100];
	yset_t s = yset_create();

	for (i = 0; i < 100; i++)
		yassert(1 == yset_add(s, &i, sizeof(i)));
	for (i = 0; i < 50; i++)
		yassert(0 == yset_add(s, &i, sizeof(i)));
	for (i = 0; i < 25; i++)
		yassert(1 == yset_del(s, &i, sizeof(i)));
	for (i = 0; i < 25; i++)
		yassert(0 == yset_del(s, &i, sizeof(i)));
	for (i = 0; i < 25; i++)
		yassert(!yset_contains(s, &i, sizeof(i)));
	for (; i < 50; i++)
		yassert(yset_contains(s, &i, sizeof(i)));
	r = yset_elements(s, (const void **)elems, NULL, arrsz(elems));
	yassert(75 == r);
	for (i = 0; i < r; i++)
		yassert(25 <= *elems[i] && *elems[i] < 100);
	yset_destroy(s);
}

TESTFN(test_set, set)
