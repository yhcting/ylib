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
#include <malloc.h>

#include "ydef.h"
#include "ylistl.h"

struct _tstfn {
	void                (*fn)(void);
	struct ylistl_link    lk;
};

static YLISTL_DECL_HEAD(_tstfnl);
static int  _mem_count = 0;

void
dregister_tstfn(void (*fn)(void)) {
	/* malloc should be used instead of dmalloc */
	struct _tstfn* n = malloc(sizeof(*n));
	n->fn = fn;
	ylistl_add_last(&_tstfnl, &n->lk);
}


void* dmalloc(unsigned int sz) {
	_mem_count++;
	return malloc(sz);
}

void dfree(void* p) {
	_mem_count--;
	free(p);
}

int dmem_count(void) { return _mem_count; }


int main() {
	struct _tstfn*    p;
	ylistl_foreach_item(p, &_tstfnl, struct _tstfn, lk)
		(*p->fn)();

	printf(">>>>>> TEST SUCCESS <<<<<<<\n");
	return 0;
}
