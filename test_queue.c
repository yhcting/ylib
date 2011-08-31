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


#include "yqueue.h"
#include "common.h"
#include "test.h"

#include <assert.h>


static void
_test_queue(void) {
	int* pi;
	struct yqueue* q = yqueue_create(NULL);

	pi = (int*)ymalloc(sizeof(int));
	*pi = 0;
	yqueue_en(q, pi);
	yassert(1 == yqueue_size(q));
	pi = (int*)ymalloc(sizeof(int));
	*pi = 1;
	yqueue_en(q, pi);
	yassert(2 == yqueue_size(q));

	pi = yqueue_de(q);
	yassert(0 == *pi);
	yassert(1 == yqueue_size(q));
	yfree(pi);

	yqueue_destroy(q);
}

TESTFN(_test_queue, queue)
