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
	struct ylist  *lst;
	struct ylisti *itr;

	lst = ylist_create(0, NULL);
	ylist_destroy(lst);

	lst = ylist_create(0, NULL);
	p = (int *)ymalloc(sizeof(*p));
	*p = 3;
	ylist_add_last(lst, p);

	itr = ylisti_create(lst, YLISTI_FORWARD);
	yassert(ylisti_has_next(itr));
	p = (int *)ylisti_next(itr);
	yassert(3 == *p);
	yassert(!ylisti_has_next(itr));
	ylisti_destroy(itr);

	itr = ylisti_create(lst, YLISTI_FORWARD);
	yassert(ylisti_has_next(itr));
	ylisti_next(itr);
	ylist_remove_current(lst, itr, 1);
	yassert(0 == ylist_size(lst));
	ylisti_destroy(itr);

	for (i = 0; i < 10; i++) {
		p = (int *)ymalloc(sizeof(*p));
		*p = i;
		ylist_add_last(lst, p);
	}
	yassert(10 == ylist_size(lst));

	/* simple iteration */
	i = 0;
	itr = ylisti_create(lst, YLISTI_FORWARD);
	while (ylisti_has_next(itr)) {
		p = ylisti_next(itr);
		yassert(i == *p);
		i++;
	}
	ylisti_destroy(itr);

	i = 9;
	itr = ylisti_create(lst, YLISTI_BACKWARD);
	while (ylisti_has_next(itr)) {
		p = ylisti_next(itr);
		yassert(i == *p);
		i--;
	}
	ylisti_destroy(itr);

	/* remove odd numbers - tail is removed. */
	itr = ylisti_create(lst, YLISTI_FORWARD);
	while (ylisti_has_next(itr)) {
		p = ylisti_next(itr);
		if (*p % 2)
			ylist_remove_current(lst, itr, 1);
	}
	ylisti_destroy(itr);

	i = 0;
	itr = ylisti_create(lst, YLISTI_FORWARD);
	while (ylisti_has_next(itr)) {
		p = ylisti_next(itr);
		yassert(i == *p);
		i += 2;
	}
	ylisti_destroy(itr);
	ylist_destroy(lst);


	lst = ylist_create(0, NULL);
	/* remove even numbers - head is removed. */
	for (i = 0; i < 10; i++) {
		p = (int *)ymalloc(sizeof(*p));
		*p = i;
		ylist_add_last(lst, p);
	}

	itr = ylisti_create(lst, YLISTI_FORWARD);
	while (ylisti_has_next(itr)) {
		p = ylisti_next(itr);
		if (!(*p % 2))
			ylist_remove_current(lst, itr, 1);
	}
	ylisti_destroy(itr);

	i = 1;
	itr = ylisti_create(lst, YLISTI_FORWARD);
	while (ylisti_has_next(itr)) {
		p = ylisti_next(itr);
		yassert(i == *p);
		i += 2;
	}
	ylisti_destroy(itr);
	ylist_destroy(lst);

	{ /* Just Scope */
		struct dummy *dum;
		lst = ylist_create(0, free_dummycb);
		for (i = 0; i < 10; i++) {
			dum = (struct dummy *)ymalloc(sizeof(*dum));
			dum->id = i;
			dum->mem = (int *)ymalloc(sizeof(*dum->mem));
			*(dum->mem) = i;
			ylist_add_last(lst, dum);
		}
		yassert(10 == ylist_size(lst));

		itr = ylisti_create(lst, YLISTI_FORWARD);
		while (ylisti_has_next(itr)) {
			dum = ylisti_next(itr);
			if (5 == dum->id)
				ylist_remove_current(lst, itr, 1);
		}
		ylisti_destroy(itr);
		yassert(9 == ylist_size(lst));

		itr = ylisti_create(lst, YLISTI_FORWARD);
		while (ylisti_has_next(itr)) {
			dum = ylisti_next(itr);
			yassert(5 != dum->id);
		}
		ylisti_destroy(itr);
		ylist_destroy(lst);
	}

	lst = ylist_create(1, NULL);
	p = (int *)ymalloc(sizeof(*p));
	*p = 0;
	ylist_add_last(lst, p);
	yassert(1 == ylist_size(lst));

	p = (int *)ymalloc(sizeof(*p));
	*p = 1;
	yassert(ylist_add_last(lst, p));
	yassert(1 == ylist_size(lst));
	yfree(p); /* p is fail to insert to the list */

	p = (int *)ylist_peek_last(lst);
	yassert(0 == *p && 1 == ylist_size(lst));
	p = ylist_remove_last(lst, FALSE);
	yassert(ylist_is_empty(lst));
	yfree(p);

	ylist_clean(lst);
	ylist_destroy(lst);
}

TESTFN(test_list, list)
