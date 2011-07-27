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


#include "ytree.h"
#include "common.h"
#include "test.h"

#include <assert.h>

#define _assign(ptr, type, val)				\
	do {						\
		(ptr) = (type*)ymalloc(sizeof(type));	\
		*(ptr) = (val);				\
	} while (0)

/*
 * Test Tree
 *                F
 *              /   \
 *            B       G
 *          /   \       \
 *         A     D       I
 *              / \     /
 *             C   E   H
 */
static void
_build_test_tree(struct ytree* t) {
	char*           c;
	struct ytree_node *n, *p;

	/* Build test tree */
	_assign(c, char, 'F');
	p = ytree_create_node(c);
	ytree_add_to_empty(t, p);

	_assign(c, char, 'B');
	n = ytree_create_node(c);
	ytree_add_last_child(p, n);

	p = n;
	_assign(c, char, 'A');
	n = ytree_create_node(c);
	ytree_add_last_child(p, n);

	_assign(c, char, 'D');
	n = ytree_create_node(c);
	ytree_add_last_child(p, n);

	p = n;
	_assign(c, char, 'C');
	n = ytree_create_node(c);
	ytree_add_last_child(p, n);

	_assign(c, char, 'E');
	n = ytree_create_node(c);
	ytree_add_last_child(p, n);


	p = ytree_root(t);
	_assign(c, char, 'G');
	n = ytree_create_node(c);
	ytree_add_last_child(p, n);

	p = n;
	_assign(c, char, 'I');
	n = ytree_create_node(c);
	ytree_add_last_child(p, n);


	p = n;
	_assign(c, char, 'H');
	n = ytree_create_node(c);
	ytree_add_last_child(p, n);
}

#undef _assign


#define _item(n) ytree_item(char, n)


static void
_test_walker(const struct ytree* t, int type, const char* referseq) {
	struct ytree_walker* w;
	const struct ytree_node* n;
	int i = 0;
	w = ytree_walker_create(ytree_root(t), type);
	while (ytree_walker_has_next(w)) {
		n = w->next(w);
		yassert(referseq[i] == _item(n));
		i++;
	}
	ytree_walker_destroy(w);
}

/**
 * Test tree operation
 */
static void
_test_operation(struct ytree* t) {
	struct ytree_node* n;
	struct ytree_node* s; /* sub */

	/* Find 'D' */
	n = ytree_root(t); /* F */
	n = ytree_first_child(n); /* B */
	n = ytree_last_child(n); /* D */
	s = ytree_del(n); /* n is top of subtree */

	/* Find 'G' */
	n = ytree_root(t); /* F */
	n = ytree_last_child(n); /* G */
	ytree_add_first_child(n, s);

	/*
	 * Now tree becomes
	 *                F
	 *              /   \
	 *            B       G
	 *          /       /   \
	 *         A       D     I
	 *                / \    /
	 *               C   E  H
	 */
	{ /* Just scope */
		static const char __vseq[]
			= { 'F', 'B', 'G', 'A', 'D', 'I', 'C', 'E', 'H' };
		_test_walker(t, YTREE_WALKER_LEVEL_OT, __vseq);
	}

	s = ytree_last_child(n); /* I */
	s = ytree_del(s);
	ytree_add_next(n, s);
	/*
	 * Now tree becomes
	 *                  F
	 *                / \  \
	 *               B   G  I
	 *              /   /  /
	 *             A   D  H
	 *                / \
	 *               C   E
	 */
	{ /* Just scope */
		static const char __vseq[]
			= { 'F', 'B', 'G', 'I', 'A', 'D', 'H', 'C', 'E' };
		_test_walker(t, YTREE_WALKER_LEVEL_OT, __vseq);
	}

	/* Try to remove root */
	s = ytree_del(ytree_root(t));
	yassert(ytree_is_empty(t));

	/* restore */
	ytree_add_to_empty(t, s);
	{ /* Just scope */
		static const char __vseq[]
			= { 'F', 'B', 'G', 'I', 'A', 'D', 'H', 'C', 'E' };
		_test_walker(t, YTREE_WALKER_LEVEL_OT, __vseq);
	}

}


static void
_verify_test_tree(const struct ytree* t) {
	struct ytree_node* n;
	struct ytree_node* nsv;

	n = ytree_root(t);
	yassert(n);
	yassert('F' == _item(n));
	/* root should not have sibling! */
	nsv = ytree_next(n);
	yassert(1 == ytree_sibling_size(n));

	nsv = n = ytree_first_child(n);
	yassert('B' == _item(n));
	n = ytree_next(n);
	yassert('G' == _item(n));
	n = nsv;

	n = ytree_first_child(n);
	yassert('A' == _item(n));
	n = ytree_next(n);
	yassert('D' == _item(n));

	n = ytree_last_child(n);
	yassert('E' == _item(n));
	n = ytree_prev(n);
	yassert('C' == _item(n));

	n = nsv;
	n = ytree_next(n);
	yassert('G' == _item(n));
	n = ytree_last_child(n);
	yassert('I' == _item(n));
	n = ytree_last_child(n);
	yassert('H' == _item(n));

}

#undef _item

static void
_test_tree(void) {
	/* pre-order traversal*/
	static const char __preot[]
		= {'F', 'B', 'A', 'D', 'C', 'E', 'G', 'I', 'H'};
	/* level-order traversal */
	static const char __levelot[]
		= {'F', 'B', 'G', 'A', 'D', 'I', 'C', 'E', 'H'};
	/* post-order traversal */
	static const char __postot[]
		= {'A', 'C', 'E', 'D', 'B', 'H', 'I', 'G', 'F'};
	/* reverse pre-order traversal - traverse right to left */
	static const char __r2lpreot[]
		= {'F', 'G', 'I', 'H', 'B', 'D', 'E', 'C', 'A'};
	/* reverse post-order traversal - traverse right to left */
	static const char __r2lpostot[]
		= {'H', 'I', 'G', 'E', 'C', 'D', 'A', 'B', 'F'};

	int sv = dmem_count();
	struct ytree*      t;

	printf("== Testing tree ==\n");

	t = ytree_create(NULL);
	_build_test_tree(t);

	_verify_test_tree(t);

	_test_walker(t, YTREE_WALKER_PRE_OT, __preot);
	_test_walker(t, YTREE_WALKER_LEVEL_OT, __levelot);
	_test_walker(t, YTREE_WALKER_POST_OT, __postot);
	_test_walker(t, YTREE_WALKER_R2L_PRE_OT, __r2lpreot);
	_test_walker(t, YTREE_WALKER_R2L_POST_OT, __r2lpostot);

	_test_operation(t);


	yassert(!ytree_is_empty(t));

	ytree_destroy(t);
	yassert(sv == dmem_count());
}

TESTFN(_test_tree)
