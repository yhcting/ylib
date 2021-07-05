/******************************************************************************
 * Copyright (C) 2015
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
#ifdef CONFIG_DEBUG

#include "common.h"
#include "ytreel.h"
#include "test.h"

#include <assert.h>

#define TESTN_SZ 9

struct tn {
	struct ytreel_link lk;
	char c;
};

struct tn np[TESTN_SZ]; /* Node Pool */

#define N(a) (&np[(*(#a)) - 'A'])
#define LK(a) (&N(a)->lk)
#define CH(a) (&N(a)->c)
#define TN(lk) (containerof(lk, struct tn, lk))

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
static struct ytreel_link *
build_test_treel(void) {
	int i;

	for (i = 0; i < TESTN_SZ; i++) {
		ytreel_init_link(&np[i].lk);
		np[i].c = 'A' + i;
	}
	/* 'F' is root */
	ytreel_add_last_child(LK(F), LK(B));
	ytreel_add_last_child(LK(B), LK(A));
	ytreel_add_last_child(LK(B), LK(D));
	ytreel_add_last_child(LK(D), LK(C));
	ytreel_add_last_child(LK(D), LK(E));
	ytreel_add_last_child(LK(F), LK(G));
	ytreel_add_last_child(LK(G), LK(I));
	ytreel_add_last_child(LK(I), LK(H));
	return LK(F);
}

static void
test_iterator(const struct ytreel_link *rlk, int type, const char *referseq) {
	struct ytreeli *itr;
	const struct ytreel_link *lk;
	int i = 0;
	itr = ytreeli_create(rlk, type);
	while (ytreeli_has_next(itr)) {
		yassert(!ytreeli_next(itr));
		lk = ytreeli_get(itr);
		yassert(referseq[i] == TN(lk)->c);
		i++;
	}
	ytreeli_destroy(itr);
}

/**
 * Test tree operation
 */
static void
test_operation(void) {
	ytreel_remove(LK(D));
	ytreel_add_first_child(LK(G), LK(D));

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
		static const char __vseq[] = {
			'F', 'B', 'G', 'A', 'D', 'I', 'C', 'E', 'H' };
		test_iterator(LK(F), YTREELI_LEVEL_OT, __vseq);
	}

	ytreel_remove(LK(I));
	ytreel_add_last_child(LK(F), LK(I));
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
		static const char __vseq[] = {
			'F', 'B', 'G', 'I', 'A', 'D', 'H', 'C', 'E' };
		test_iterator(LK(F), YTREELI_LEVEL_OT, __vseq);
	}
}


static void
verify_test_treel(void) {
	struct tn *n;
	n = N(F); /* root */
	yassert('F' == n->c);
	/* root should not have sibling! */
	yassert(1 == ytreel_sibling_size(LK(F)));
	yassert(!ytreel_has_next(LK(F)));
	yassert(LK(B) == ytreel_first_child(LK(F)));
	yassert(LK(G) == ytreel_next(LK(B)));
	yassert(LK(A) == ytreel_first_child(LK(B)));
	yassert(LK(D) == ytreel_next(LK(A)));
	yassert(LK(E) == ytreel_last_child(LK(D)));
	yassert(LK(C) == ytreel_prev(LK(E)));
	yassert(LK(G) == ytreel_next(LK(B)));
	yassert(LK(I) == ytreel_last_child(LK(G)));
	yassert(LK(H) == ytreel_first_child(LK(I)));
}

static void
test_treel(void) {
	struct ytreel_link *lk;
	/* pre-order traversal*/
	static const char preot[] = {
		'F', 'B', 'A', 'D', 'C', 'E', 'G', 'I', 'H'};
	/* level-order traversal */
	static const char levelot[] = {
		'F', 'B', 'G', 'A', 'D', 'I', 'C', 'E', 'H'};
	/* post-order traversal */
	static const char postot[] = {
		'A', 'C', 'E', 'D', 'B', 'H', 'I', 'G', 'F'};
	/* reverse pre-order traversal - traverse right to left */
	static const char r2lpreot[] = {
		'F', 'G', 'I', 'H', 'B', 'D', 'E', 'C', 'A'};
	/* reverse post-order traversal - traverse right to left */
	static const char r2lpostot[] = {
		'H', 'I', 'G', 'E', 'C', 'D', 'A', 'B', 'F'};

	lk = build_test_treel();
	verify_test_treel();

	test_iterator(lk, YTREELI_PRE_OT, preot);
	test_iterator(lk, YTREELI_LEVEL_OT, levelot);
	test_iterator(lk, YTREELI_POST_OT, postot);
	test_iterator(lk, YTREELI_R2L_PRE_OT, r2lpreot);
	test_iterator(lk, YTREELI_R2L_POST_OT, r2lpostot);
	test_operation();

}

TESTFN(treel)

#endif /* CONFIG_DEBUG */
