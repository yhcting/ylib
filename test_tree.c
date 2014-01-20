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


#include "ycommon.h"
#include "ytree.h"
#include "test.h"

#include <assert.h>

#define assign(ptr, type, val)				\
	do {						\
		(ptr) = (type *)ymalloc(sizeof(type));	\
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
build_test_tree(struct ytree *t) {
	char *c;
	struct ytree_node *n, *p;

	/* Build test tree */
	assign(c, char, 'F');
	p = ytree_create_node(c);
	ytree_add_to_empty(t, p);

	assign(c, char, 'B');
	n = ytree_create_node(c);
	ytree_add_last_child(p, n);

	p = n;
	assign(c, char, 'A');
	n = ytree_create_node(c);
	ytree_add_last_child(p, n);

	assign(c, char, 'D');
	n = ytree_create_node(c);
	ytree_add_last_child(p, n);

	p = n;
	assign(c, char, 'C');
	n = ytree_create_node(c);
	ytree_add_last_child(p, n);

	assign(c, char, 'E');
	n = ytree_create_node(c);
	ytree_add_last_child(p, n);


	p = ytree_root(t);
	assign(c, char, 'G');
	n = ytree_create_node(c);
	ytree_add_last_child(p, n);

	p = n;
	assign(c, char, 'I');
	n = ytree_create_node(c);
	ytree_add_last_child(p, n);


	p = n;
	assign(c, char, 'H');
	n = ytree_create_node(c);
	ytree_add_last_child(p, n);
}

#undef assign


#define item(n) ytree_item(char, n)


static void
test_iterator(const struct ytree *t, int type, const char *referseq) {
	struct ytreei *itr;
	const struct ytree_node *n __attribute__((unused));
	int i = 0;
	itr = ytreei_create(ytree_root(t), type);
	while (ytreei_has_next(itr)) {
		n = itr->next(itr);
		yassert(referseq[i] == item(n));
		i++;
	}
	ytreei_destroy(itr);
}

/**
 * Test tree operation
 */
static void
test_operation(struct ytree *t) {
	struct ytree_node *n;
	struct ytree_node *s; /* sub */

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
		test_iterator(t, YTREEI_LEVEL_OT, __vseq);
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
		test_iterator(t, YTREEI_LEVEL_OT, __vseq);
	}

	/* Try to remove root */
	s = ytree_del(ytree_root(t));
	yassert(ytree_is_empty(t));

	/* restore */
	ytree_add_to_empty(t, s);
	{ /* Just scope */
		static const char __vseq[]
			= { 'F', 'B', 'G', 'I', 'A', 'D', 'H', 'C', 'E' };
		test_iterator(t, YTREEI_LEVEL_OT, __vseq);
	}

}


static void
verify_test_tree(const struct ytree *t) {
	struct ytree_node *n;
	struct ytree_node *nsv;

	n = ytree_root(t);
	yassert(n);
	yassert('F' == item(n));
	/* root should not have sibling! */
	nsv = ytree_next(n);
	yassert(1 == ytree_sibling_size(n));

	nsv = n = ytree_first_child(n);
	yassert('B' == item(n));
	n = ytree_next(n);
	yassert('G' == item(n));
	n = nsv;

	n = ytree_first_child(n);
	yassert('A' == item(n));
	n = ytree_next(n);
	yassert('D' == item(n));

	n = ytree_last_child(n);
	yassert('E' == item(n));
	n = ytree_prev(n);
	yassert('C' == item(n));

	n = nsv;
	n = ytree_next(n);
	yassert('G' == item(n));
	n = ytree_last_child(n);
	yassert('I' == item(n));
	n = ytree_last_child(n);
	yassert('H' == item(n));

}

#undef item

static void
test_tree(void) {
	/* pre-order traversal*/
	static const char preot[]
		= {'F', 'B', 'A', 'D', 'C', 'E', 'G', 'I', 'H'};
	/* level-order traversal */
	static const char levelot[]
		= {'F', 'B', 'G', 'A', 'D', 'I', 'C', 'E', 'H'};
	/* post-order traversal */
	static const char postot[]
		= {'A', 'C', 'E', 'D', 'B', 'H', 'I', 'G', 'F'};
	/* reverse pre-order traversal - traverse right to left */
	static const char r2lpreot[]
		= {'F', 'G', 'I', 'H', 'B', 'D', 'E', 'C', 'A'};
	/* reverse post-order traversal - traverse right to left */
	static const char r2lpostot[]
		= {'H', 'I', 'G', 'E', 'C', 'D', 'A', 'B', 'F'};

	struct ytree *t;

	t = ytree_create(NULL);

	build_test_tree(t);
	verify_test_tree(t);

	test_iterator(t, YTREEI_PRE_OT, preot);
	test_iterator(t, YTREEI_LEVEL_OT, levelot);
	test_iterator(t, YTREEI_POST_OT, postot);
	test_iterator(t, YTREEI_R2L_PRE_OT, r2lpreot);
	test_iterator(t, YTREEI_R2L_POST_OT, r2lpostot);
	test_operation(t);

	yassert(!ytree_is_empty(t));
	ytree_destroy(t);
}

TESTFN(test_tree, tree)

