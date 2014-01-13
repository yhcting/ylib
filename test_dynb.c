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

#include <string.h>
#include <assert.h>

#include "common.h"
#include "ydynb.h"
#include "test.h"


static void
test_dynb(void) {
	struct ydynb *b = ydynb_create(2);
	yassert(2 == ydynb_freesz(b));
	ydynb_append(b, "ab ", 3);
	yassert(1 == ydynb_freesz(b));
	ydynb_append(b, "cde fgh ", 8);
	/* include trailing 0 */
	ydynb_append(b, "1234567890123456789012345678901234567890", 41);
	yassert(52 == ydynb_sz(b));
	yassert(!strcmp("ab cde fgh 1234567890123456789012345678901234567890",
			(const char *)ydynb_buf(b)));
	yassert(EINVAL == ydynb_shrink(b, 0));
	yassert(EINVAL == ydynb_shrink(b, 0xffff));
	yassert(EINVAL == ydynb_shrink(b, ydynb_sz(b) - 1));
	ydynb_shrink(b, ydynb_sz(b));
	yassert(0 == ydynb_freesz(b));
	ydynb_destroy(b);
}


TESTFN(test_dynb, dynb)
