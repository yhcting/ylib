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
#include <malloc.h>

#include "ydef.h"
#include "ylistl.h"

struct tstfn {
	void             (*fn)(void);
	const char        *modname;
	struct ylistl_link lk;
};

static YLISTL_DECL_HEAD(_tstfnl);
static int  _mem_count = 0;

void
dregister_tstfn(void (*fn)(void), const char *mod) {
	/* malloc should be used instead of dmalloc */
	struct tstfn* n = malloc(sizeof(*n));
	n->fn = fn;
	n->modname = mod;
	ylistl_add_last(&_tstfnl, &n->lk);
}


void *
dmalloc(unsigned int sz) {
	_mem_count++;
	return malloc(sz);
}

void
dfree(void *p) {
	_mem_count--;
	free(p);
}

int
dmem_count(void) {
	return _mem_count;
}

int main() {
	int           sv;
	struct tstfn *p;
	ylistl_foreach_item(p, &_tstfnl, struct tstfn, lk) {
		printf("<< Test [%s] >>\n", p->modname);
		sv = dmem_count();
		(*p->fn)();
		if (sv != dmem_count()) {
			printf("Unbalanced memory at [%s]!\n"
			       "    balance : %d\n",
			       p->modname,
			       dmem_count() - sv);
			yassert(0);
		}
		printf(" => PASSED\n");
	}

	printf(">>>>>> TEST PASSED <<<<<<<\n");
	return 0;
}
