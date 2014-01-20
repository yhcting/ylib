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
#include "yp.h"
#include "test.h"

static void
test_yp(void) {
	int i;
	int *p, *p2;
	int *o = ypget(ypmalloc(sizeof(int) * 3));
	p2 = ypget(o);
	p = p2;
	for (i = 0; i < 3; i++)
		*p++ = 1;
	/* sp is assigned to another pointer */
	ypput(p2); /* put */
	p = o;
	for (i = 0; i < 3; i++)
		++*p++;
	/*
	 * should be freed at this point
	 */
	ypput(o);
}

TESTFN(test_yp, yp)
