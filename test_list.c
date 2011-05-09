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


#include "common.h"
#include "ylist.h"

#include <assert.h>

extern int dmem_count();

struct _dummy{
	int     id;
	int*    mem;
} ;

void _free_dummycb(void* arg) {
	if (NULL != arg) {
		struct _dummy* dummy = (struct _dummy*)arg;
		if (NULL != dummy->mem)
			yfree(dummy->mem);
		yfree(dummy);
	}
}


/**
 * Linked list test.
 */
void test_list() {
	int             mem_cnt_sv;
	int             i;
	int*            p;
	struct ylist*   lst;
	struct ylist_walker*   w;

	mem_cnt_sv = dmem_count();

	lst = ylist_create(NULL);
	ylist_destroy(lst);

	lst = ylist_create(NULL);
	p = (int*)ymalloc(sizeof(int));
	*p = 0;
	ylist_add_last(lst, ylist_create_node(p));

	w = ylist_walker_create(lst, YLIST_WALKER_FORWARD);
	yassert(ylist_walker_has_next(w));
	p = (int*)ylist_walker_next_forward(w);
	yassert(0 == *p);
	yassert(!ylist_walker_has_next(w));
	ylist_walker_destroy(w);

	w = ylist_walker_create(lst, YLIST_WALKER_FORWARD);
	yfree(ylist_walker_next_forward(w));
	ylist_free_node(ylist_walker_del(w));
	yassert(0 == ylist_size(lst));
	ylist_walker_destroy(w);

	for (i=0; i<10; i++) {
		p = (int*)ymalloc(sizeof(int));
		*p = i;
		ylist_add_last(lst, ylist_create_node(p));
	}
	yassert(10 == ylist_size(lst));

	/* simple iteration */
	i = 0;
	w = ylist_walker_create(lst, YLIST_WALKER_FORWARD);
	while (ylist_walker_has_next(w)) {
		p = ylist_walker_next_forward(w);
		yassert(i == *p);
		i++;
	}
	ylist_walker_destroy(w);

	i = 9;
	w = ylist_walker_create(lst, YLIST_WALKER_BACKWARD);
	while (ylist_walker_has_next(w)) {
		p = ylist_walker_next_backward(w);
		yassert(i == *p);
		i--;
	}
	ylist_walker_destroy(w);

	/* remove odd numbers - tail is removed. */
	w = ylist_walker_create(lst, YLIST_WALKER_FORWARD);
	while (ylist_walker_has_next(w)) {
		p = ylist_walker_next_forward(w);
		if (*p%2) {
			ylist_free_item(ylist_walker_list(w), p);
			ylist_free_node(ylist_walker_del(w));
		}
	}
	ylist_walker_destroy(w);

	i = 0;
	w = ylist_walker_create(lst, YLIST_WALKER_FORWARD);
	while (ylist_walker_has_next(w)) {
		p = ylist_walker_next_forward(w);
		yassert(i == *p);
		i+=2;
	}
	ylist_walker_destroy(w);
	ylist_destroy(lst);
	lst = ylist_create(NULL);

	/* remove even numbers - head is removed. */
	for (i=0; i<10; i++) {
		p = (int*)ymalloc(sizeof(int));
		*p = i;
		ylist_add_last(lst, ylist_create_node(p));
	}

	w = ylist_walker_create(lst, YLIST_WALKER_FORWARD);
	while (ylist_walker_has_next(w)) {
		p = ylist_walker_next_forward(w);
		if (!(*p%2)) {
			ylist_free_item(ylist_walker_list(w), p);
			ylist_free_node(ylist_walker_del(w));
		}
	}
	ylist_walker_destroy(w);

	i = 1;
	w = ylist_walker_create(lst, YLIST_WALKER_FORWARD);
	while (ylist_walker_has_next(w)) {
		p = ylist_walker_next_forward(w);
		yassert(i == *p);
		i+=2;
	}
	ylist_walker_destroy(w);
	ylist_destroy(lst);

	{ /* Just Scope */
		struct _dummy*      dum;
		lst = ylist_create(_free_dummycb);
		for (i=0; i<10; i++) {
			dum = (struct _dummy*)ymalloc(sizeof(struct _dummy));
			dum->id = i;
			dum->mem = (int*)ymalloc(sizeof(int));
			*(dum->mem) = i;
			ylist_add_last(lst, ylist_create_node(dum));
		}
		yassert(10 == ylist_size(lst));


		w = ylist_walker_create(lst, YLIST_WALKER_FORWARD);
		while (ylist_walker_has_next(w)) {
			dum = ylist_walker_next_forward(w);
			if (5 == dum->id) {
				ylist_free_node(ylist_walker_del(w));
				ylist_free_item(lst, dum);
			}
		}
		ylist_walker_destroy(w);
		yassert(9 == ylist_size(lst));


		w = ylist_walker_create(lst, YLIST_WALKER_FORWARD);
		while (ylist_walker_has_next(w)) {
			dum = ylist_walker_next_forward(w);
			yassert(5 != dum->id);
		}
		ylist_walker_destroy(w);


		ylist_destroy(lst);
	}

	yassert(mem_cnt_sv == dmem_count());
}
