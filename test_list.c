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
#include "test.h"

#include <assert.h>

struct dummy {
	int  id;
	int *mem;
};

static void
free_dummycb(void *arg) {
	if (arg) {
		struct dummy *dummy = (struct dummy *)arg;
		if (dummy->mem)
			yfree(dummy->mem);
		yfree(dummy);
	}
}


/**
 * Linked list test.
 */
static void
test_list(void) {
	int  i;
	int *p;
	struct ylist *lst;
	struct ylist_walker *w;

	lst = ylist_create(NULL);
	ylist_destroy(lst);

	lst = ylist_create(NULL);
	p = (int *)ymalloc(sizeof(*p));
	*p = 0;
	ylist_add_last(lst, ylist_create_node(p));

	w = ylist_walker_create(lst, YLIST_WALKER_FORWARD);
	yassert(ylist_walker_has_next(w));
	p = (int *)ylist_walker_next_forward(w);
	yassert(0 == *p);
	yassert(!ylist_walker_has_next(w));
	ylist_walker_destroy(w);

	w = ylist_walker_create(lst, YLIST_WALKER_FORWARD);
	yfree(ylist_walker_next_forward(w));
	ylist_free_node(ylist_walker_del(w));
	yassert(0 == ylist_size(lst));
	ylist_walker_destroy(w);

	for (i = 0; i < 10; i++) {
		p = (int *)ymalloc(*p);
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
		if (*p % 2) {
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
		i += 2;
	}
	ylist_walker_destroy(w);
	ylist_destroy(lst);
	lst = ylist_create(NULL);

	/* remove even numbers - head is removed. */
	for (i=0; i<10; i++) {
		p = (int *)ymalloc(sizeof(*p));
		*p = i;
		ylist_add_last(lst, ylist_create_node(p));
	}

	w = ylist_walker_create(lst, YLIST_WALKER_FORWARD);
	while (ylist_walker_has_next(w)) {
		p = ylist_walker_next_forward(w);
		if (!(*p % 2)) {
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
		i += 2;
	}
	ylist_walker_destroy(w);
	ylist_destroy(lst);

	{ /* Just Scope */
		struct dummy *dum;
		lst = ylist_create(free_dummycb);
		for (i = 0; i < 10; i++) {
			dum = (struct dummy *)ymalloc(sizeof(*dum));
			dum->id = i;
			dum->mem = (int *)ymalloc(sizeof(*dum->mem));
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
}

TESTFN(test_list, list)
