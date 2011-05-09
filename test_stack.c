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


#include "ystack.h"
#include "common.h"

#include <assert.h>

extern int dmem_count();

void test_stack() {
	int* pi;
	int sv = dmem_count();
	struct ystack*  s = ystack_create(NULL);

	pi = (int*)ymalloc(sizeof(int));
	*pi = 0;
	ystack_push(s, pi);
	yassert(1 == ystack_size(s));
	pi = (int*)ymalloc(sizeof(int));
	*pi = 1;
	ystack_push(s, pi);
	yassert(2 == ystack_size(s));

	pi = ystack_pop(s);
	yassert(1 == *pi);
	yassert(1 == ystack_size(s));
	yfree(pi);

	ystack_destroy(s);
	yassert(sv == dmem_count());
}
