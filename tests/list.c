/******************************************************************************
 * Copyright (C) 2011, 2012, 2013, 2014, 2015
 * Younghyung Cho. <yhcting77@gmail.com>
 * All rights reserved.
 *
 * This file is part of ylib
 *
 * This program is licensed under the FreeBSD license
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * official policies, either expressed or implied, of the FreeBSD Project.
 *****************************************************************************/

#include <assert.h>

#include "common.h"
#include "ylist.h"
#include "test.h"

struct dummy {
	int id;
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
	int i;
	int *p;
	struct ylist *lst;
	struct ylisti *itr;

	lst = ylist_create(0, &yfree);
	ylist_destroy(lst);

	lst = ylist_create(0, &yfree);
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
	yassert(ylist_has(lst, p));
	yassert(!ylist_has(lst, lst));

	/* simple iteration */
	i = 0;
	itr = ylisti_create(lst, YLISTI_FORWARD);
	while (ylisti_has_next(itr)) {
		p = ylisti_next(itr);
		yassert(ylist_has(lst, p));
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


	lst = ylist_create(0, &yfree);
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

	lst = ylist_create(1, &yfree);
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

	ylist_destroy(lst);
}

TESTFN(test_list, list)
