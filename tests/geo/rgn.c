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
#include "../test.h"
#ifdef CONFIG_DEBUG
#include "geo/yrgn.h"
#include "yut.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*****************************************************************************
 *
 * Dynamic random test
 *
 *****************************************************************************/
#define COORD_LIMIT 100
#define PLAIN_SZ (COORD_LIMIT * COORD_LIMIT)
static char plain0[COORD_LIMIT][COORD_LIMIT]; /* [Y][X] */
static char plain1[COORD_LIMIT][COORD_LIMIT]; /* [Y][X] */

static void
fillrect(char plain[COORD_LIMIT][COORD_LIMIT],
	 const struct yrect *r, char v) {
	int x, y;
	for (y = r->t; y < r->b; y++)
		for (x = r->l; x < r->r; x++)
			plain[y][x] = v;
}

static void
fillrect_on(char plain[COORD_LIMIT][COORD_LIMIT],
	    const struct yrect *r,
	    char existv,
	    char newv) {
	int x, y;
	for (y = r->t; y < r->b; y++)
		for (x = r->l; x < r->r; x++)
			if (existv == plain[y][x])
				plain[y][x] = newv;
}

static void
change_val(char plain[COORD_LIMIT][COORD_LIMIT], char from, char to) {
	int x, y;
	for (y = 0; y < COORD_LIMIT; y++)
		for (x = 0; x < COORD_LIMIT; x++)
			if (from == plain[y][x])
				plain[y][x] = to;
}

static int
myrand(void) {
	return (int)((long)rand() * (COORD_LIMIT - 1) / RAND_MAX);
}

static void
clear_plain(char plain[COORD_LIMIT][COORD_LIMIT]) {
	memset(plain, 0, PLAIN_SZ);
}

static struct yrgn *
create_random_rgn(void) {
	struct yrgn *g0, *g1;
	struct yrect r;
	int cnt = myrand();
	g0 = ygeog_create_empty();
	clear_plain(plain0);
	clear_plain(plain1);
	while (cnt--) {
		ygeor_set3(&r, myrand(), myrand(), myrand(), myrand());
		fillrect(plain0, &r, 1);
		g1 = ygeog_union2(g0, &r);
		ygeog_destroy(g0);
		g0 = g1;
	}

	ygeog_foreach_rect_begin(g0, r) {
		fillrect(plain1, &r, 1);
	} ygeog_foreach_rect_end();

	/* two plains should be same */
	yassert(!memcmp(plain0, plain1, PLAIN_SZ));
	return g0;
}

static void
test_rand(void) {
	struct yrect r;
	struct yrgn *g0, *g1, *g;
	g0 = create_random_rgn();
	g1 = create_random_rgn();

	/* union
	 * -------
	 */
	clear_plain(plain0);
	clear_plain(plain1);
	ygeog_foreach_rect_begin(g0, r) {
		fillrect(plain0, &r, 1);
	} ygeog_foreach_rect_end();
	ygeog_foreach_rect_begin(g1, r) {
		fillrect(plain0, &r, 1);
	} ygeog_foreach_rect_end();

	g = ygeog_union(g0, g1);
	ygeog_foreach_rect_begin(g, r) {
		fillrect(plain1, &r, 1);
	} ygeog_foreach_rect_end();
	ygeog_destroy(g);
	yassert(!memcmp(plain0, plain1, PLAIN_SZ));

	/* diff
	 * ----
	 */
	clear_plain(plain0);
	clear_plain(plain1);
	ygeog_foreach_rect_begin(g0, r) {
		fillrect(plain0, &r, 1);
	} ygeog_foreach_rect_end();
	ygeog_foreach_rect_begin(g1, r) {
		fillrect(plain0, &r, 0);
	} ygeog_foreach_rect_end();

	g = ygeog_diff(g0, g1);
	ygeog_foreach_rect_begin(g, r) {
		fillrect(plain1, &r, 1);
	} ygeog_foreach_rect_end();
	ygeog_destroy(g);
	yassert(!memcmp(plain0, plain1, PLAIN_SZ));

	/* intersect
	 * ---------
	 */
	clear_plain(plain0);
	clear_plain(plain1);
	ygeog_foreach_rect_begin(g0, r) {
		fillrect(plain0, &r, 1);
	} ygeog_foreach_rect_end();
	ygeog_foreach_rect_begin(g1, r) {
		/* set to '2' cell already filled above - intersection */
		fillrect_on(plain0, &r, 1, 2);
	} ygeog_foreach_rect_end();
	/* clear non-intersection cells */
	change_val(plain0, 1, 0);
	/* Now, only intersection-cells are filled with '2' */

	g = ygeog_intersect(g0, g1);
	ygeog_foreach_rect_begin(g, r) {
		fillrect(plain1, &r, 2);
	} ygeog_foreach_rect_end();
	ygeog_destroy(g);
	yassert(!memcmp(plain0, plain1, PLAIN_SZ));

	ygeog_destroy(g0);
	ygeog_destroy(g1);
}


/*****************************************************************************
 *
 * Fixed(predefined) test
 *
 *****************************************************************************/
#define NULL_R yrect_struct_init(0, 0, 0, 0)

struct tdst { /* Test Data STructure */
	struct yrect r0[3];
	struct yrect r1[3];
	/* expected result is at most 16 rects
	 * + 1 for ending sentinel
	 */
	struct yrect runion[16];
	struct yrect rintersect[16];
	struct yrect r0diff[16]; /* r0 - r1 */
	struct yrect r1diff[16]; /* r1 - r0 */
};

static const struct tdst tdat[] = { /* One-Rect TEST */
	{ /* DATA: 0 */
		/* op with empty region  */
		{ NULL_R },
		{ NULL_R },
		{ NULL_R },
		{ NULL_R },
		{ NULL_R },
		{ NULL_R },
	},
	{
		/* op with empty rect and rgn */
		{ NULL_R },
		{ yrect_struct_init(10, 20, 200, 200),
		  NULL_R },
		{ NULL_R },
		{ NULL_R },
		{ NULL_R },
		{ NULL_R },
	},
	{
		/* op with empty rect and rgn*/
		{ yrect_struct_init(10, 10, 100, 200),
		  NULL_R },
		{ NULL_R },
		{ NULL_R },
		{ NULL_R },
		{ NULL_R },
		{ NULL_R },
	},
	{
		/* op with empty rect - 0 */
		{ yrect_struct_init(10, 10, 100, 200),
		  NULL_R },
		{ yrect_struct_init(10, 20, 200, 200),
		  NULL_R },
		{ NULL_R },
		{ NULL_R },
		{ NULL_R },
		{ NULL_R },
	},
	{
		/* op with empty rect - 1 */
		{ yrect_struct_init(10, 10, 100, 200),
		  NULL_R },
		{ yrect_struct_init(10, 20, 100, 200),
		  NULL_R },
		{ yrect_struct_init(10, 20, 100, 200),
		  NULL_R },
		{ NULL_R },
		{ NULL_R },
		{ yrect_struct_init(10, 20, 100, 200),
		  NULL_R },
	},
	{ /* DATA: 05 */
		/* op with empty rect - 2 */
		{ yrect_struct_init(10, 20, 100, 200),
		  NULL_R },
		{ yrect_struct_init(10, 20, 200, 200),
		  NULL_R },
		{ yrect_struct_init(10, 20, 100, 200),
		  NULL_R },
		{ NULL_R },
		{ yrect_struct_init(10, 20, 100, 200),
		  NULL_R },
		{ NULL_R },
	},
	{
		/* op subset
		 * +-----+
		 * |+--+ |
		 * ||  | |
		 * |+--+ |
		 * +-----+
		 */
		{ yrect_struct_init(10, 40, 100, 400),
		  NULL_R },
		{ yrect_struct_init(20, 30, 200, 300),
		  NULL_R },
		{ yrect_struct_init(10, 40, 100, 400),
		  NULL_R },
		{ yrect_struct_init(20, 30, 200, 300),
		  NULL_R },
		{ yrect_struct_init(10, 20, 100, 400),
		  yrect_struct_init(20, 30, 100, 200),
		  yrect_struct_init(20, 30, 300, 400),
		  yrect_struct_init(30, 40, 100, 400),
		  NULL_R },
		{ NULL_R },
	},
	{
		/* separate - same y-band
		 * +--+ +--+
		 * |  | |  |
		 * +--+ +--+
		 */
		{ yrect_struct_init(10, 20, 100, 200),
		  NULL_R },
		{ yrect_struct_init(10, 20, 300, 400),
		  NULL_R },
		{ yrect_struct_init(10, 20, 100, 200),
		  yrect_struct_init(10, 20, 300, 400),
		  NULL_R },
		{ NULL_R },
		{ yrect_struct_init(10, 20, 100, 200),
		  NULL_R },
		{ yrect_struct_init(10, 20, 300, 400),
		  NULL_R },
	},
	{
		/* separate - same x-band
		 * +--+
		 * |  |
		 * +--+
		 * +--+
		 * |  |
		 * +--+
		 */
		{ yrect_struct_init(10, 20, 100, 200),
		  NULL_R },
		{ yrect_struct_init(30, 40, 100, 200),
		  NULL_R },
		{ yrect_struct_init(10, 20, 100, 200),
		  yrect_struct_init(30, 40, 100, 200),
		  NULL_R },
		{ NULL_R },
		{ yrect_struct_init(10, 20, 100, 200),
		  NULL_R },
		{ yrect_struct_init(30, 40, 100, 200),
		  NULL_R },
	},
	{
		/* separate - x-band overlap
		 * +----+
		 * |    |
		 * +----+
		 *    +----+
		 *    |    |
		 *    +----+
		 */
		{ yrect_struct_init(10, 20, 100, 300),
		  NULL_R },
		{ yrect_struct_init(30, 40, 200, 400),
		  NULL_R },
		{ yrect_struct_init(10, 20, 100, 300),
		  yrect_struct_init(30, 40, 200, 400),
		  NULL_R },
		{ NULL_R },
		{ yrect_struct_init(10, 20, 100, 300),
		  NULL_R },
		{ yrect_struct_init(30, 40, 200, 400),
		  NULL_R },
	},
	{ /* DATA: 10 */
		/* separate - x-band overlap and adjacent
		 * +----+
		 * |    |
		 * +--+-+--+
		 *    |    |
		 *    +----+
		 */
		{ yrect_struct_init(10, 20, 100, 300),
		  NULL_R },
		{ yrect_struct_init(20, 30, 200, 400),
		  NULL_R },
		{ yrect_struct_init(10, 20, 100, 300),
		  yrect_struct_init(20, 30, 200, 400),
		  NULL_R },
		{ NULL_R },
		{ yrect_struct_init(10, 20, 100, 300),
		  NULL_R },
		{ yrect_struct_init(20, 30, 200, 400),
		  NULL_R },
	},
	{
		/* separate - no-overlappped-band
		 * +--+
		 * |  |
		 * +--+
		 *      +--+
		 *      |  |
		 *      +--+
		 */
		{ yrect_struct_init(10, 20, 100, 200),
		  NULL_R },
		{ yrect_struct_init(30, 40, 300, 400),
		  NULL_R },
		{ yrect_struct_init(10, 20, 100, 200),
		  yrect_struct_init(30, 40, 300, 400),
		  NULL_R },
		{ NULL_R },
		{ yrect_struct_init(10, 20, 100, 200),
		  NULL_R },
		{ yrect_struct_init(30, 40, 300, 400),
		  NULL_R },
	},
	{
		/* overlap - same y-band
		 * +--+-+--+
		 * |//|X|\\|
		 * +--+-+--+
		 */
		{ yrect_struct_init(10, 20, 100, 300),
		  NULL_R },
		{ yrect_struct_init(10, 20, 200, 400),
		  NULL_R },
		{ yrect_struct_init(10, 20, 100, 400),
		  NULL_R },
		{ yrect_struct_init(10, 20, 200, 300),
		  NULL_R },
		{ yrect_struct_init(10, 20, 100, 200),
		  NULL_R },
		{ yrect_struct_init(10, 20, 300, 400),
		  NULL_R },
	},
	{
		/* overlap - same x-band
		 * +--+
		 * |//|
		 * +--+
		 * |XX|
		 * +--+
		 * |\\|
		 * +--+
		 */
		{ yrect_struct_init(10, 30, 100, 200),
		  NULL_R },
		{ yrect_struct_init(20, 40, 100, 200),
		  NULL_R },
		{ yrect_struct_init(10, 40, 100, 200),
		  NULL_R },
		{ yrect_struct_init(20, 30, 100, 200),
		  NULL_R },
		{ yrect_struct_init(10, 20, 100, 200),
		  NULL_R },
		{ yrect_struct_init(30, 40, 100, 200),
		  NULL_R },
	},
	{
		/* overlap - in the middle of y - 0
		 * +-----+
		 * |/////|
		 * |//+-----+
		 * |//|XX\\\|
		 * |//+-----+
		 * |/////|
		 * +-----+
		 */
		{ yrect_struct_init(10, 40, 100, 300),
		  NULL_R },
		{ yrect_struct_init(20, 30, 200, 400),
		  NULL_R },
		{ yrect_struct_init(10, 20, 100, 300),
		  yrect_struct_init(20, 30, 100, 400),
		  yrect_struct_init(30, 40, 100, 300),
		  NULL_R },
		{ yrect_struct_init(20, 30, 200, 300),
		  NULL_R },
		{ yrect_struct_init(10, 20, 100, 300),
		  yrect_struct_init(20, 30, 100, 200),
		  yrect_struct_init(30, 40, 100, 300),
		  NULL_R },
		{ yrect_struct_init(20, 30, 300, 400),
		  NULL_R },
	},
	{ /* DATA: 15 */
		/* overlap - in the middle of y - 1
		 * +-----+
		 * |/////|
		 * |/////|  +-----+
		 * |/////|  |\\\\\|
		 * |/////|  +-----+
		 * |/////|
		 * +-----+
		 */
		{ yrect_struct_init(10, 40, 100, 200),
		  NULL_R },
		{ yrect_struct_init(20, 30, 300, 400),
		  NULL_R },
		{ yrect_struct_init(10, 20, 100, 200),
		  yrect_struct_init(20, 30, 100, 200),
		  yrect_struct_init(20, 30, 300, 400),
		  yrect_struct_init(30, 40, 100, 200),
		  NULL_R },
		{ NULL_R },
		{ yrect_struct_init(10, 40, 100, 200),
		  NULL_R },
		{ yrect_struct_init(20, 30, 300, 400),
		  NULL_R },
	},
	{
		/* overlap - in the middle of x
		 *      +-----+
		 *      |/////|
		 *  +---|XXXXX|--+
		 *  |\\\|XXXXX|\\|
		 *  |\\\+-----+\\|
		 *  |\\\\\\\\\\\\|
		 *  +------------+
		 */
		{ yrect_struct_init(10, 30, 200, 300),
		  NULL_R },
		{ yrect_struct_init(20, 40, 100, 400),
		  NULL_R },
		{ yrect_struct_init(10, 20, 200, 300),
		  yrect_struct_init(20, 40, 100, 400),
		  NULL_R },
		{ yrect_struct_init(20, 30, 200, 300),
		  NULL_R },
		{ yrect_struct_init(10, 20, 200, 300),
		  NULL_R },
		{ yrect_struct_init(20, 30, 100, 200),
		  yrect_struct_init(20, 30, 300, 400),
		  yrect_struct_init(30, 40, 100, 400),
		  NULL_R },
	},
	{
		/* overlap - y - 0
		 * +--+
		 * |  |  +--+
		 * +--+  |  |
		 *       +--+
		 */
		{ yrect_struct_init(10, 30, 100, 200),
		  NULL_R },
		{ yrect_struct_init(20, 40, 300, 400),
		  NULL_R },
		{ yrect_struct_init(10, 20, 100, 200),
		  yrect_struct_init(20, 30, 100, 200),
		  yrect_struct_init(20, 30, 300, 400),
		  yrect_struct_init(30, 40, 300, 400),
		  NULL_R },
		{ NULL_R },
		{ yrect_struct_init(10, 30, 100, 200),
		  NULL_R },
		{ yrect_struct_init(20, 40, 300, 400),
		  NULL_R },
	},
	{
		/* overlap - y - 1
		 * +----+
		 * |    |
		 * |  +----+
		 * |  |    |
		 * +--|    |
		 *    +----+
		 */
		{ yrect_struct_init(10, 30, 100, 300),
		  NULL_R },
		{ yrect_struct_init(20, 40, 200, 400),
		  NULL_R },
		{ yrect_struct_init(10, 20, 100, 300),
		  yrect_struct_init(20, 30, 100, 400),
		  yrect_struct_init(30, 40, 200, 400),
		  NULL_R },
		{ yrect_struct_init(20, 30, 200, 300),
		  NULL_R },
		{ yrect_struct_init(10, 20, 100, 300),
		  yrect_struct_init(20, 30, 100, 200),
		  NULL_R },
		{ yrect_struct_init(20, 30, 300, 400),
		  yrect_struct_init(30, 40, 200, 400),
		  NULL_R },
	},
	{
		/* overlap - cross
		 *      +-----+
		 *      |/////|
		 *  +------------+
		 *  |\\\ XXXXX \\|
		 *  +------------+
		 *      |/////|
		 *      +-----+
		 */
		{ yrect_struct_init(10, 40, 200, 300),
		  NULL_R },
		{ yrect_struct_init(20, 30, 100, 400),
		  NULL_R },
		{ yrect_struct_init(10, 20, 200, 300),
		  yrect_struct_init(20, 30, 100, 400),
		  yrect_struct_init(30, 40, 200, 300),
		  NULL_R },
		{ yrect_struct_init(20, 30, 200, 300),
		  NULL_R },
		{ yrect_struct_init(10, 20, 200, 300),
		  yrect_struct_init(30, 40, 200, 300),
		  NULL_R },
		{ yrect_struct_init(20, 30, 100, 200),
		  yrect_struct_init(20, 30, 300, 400),
		  NULL_R },
	},
	{ /* DATA: 20 */
		/* +--+              <1
		 * |//|      +--+    <2
		 * |//|      |//|
		 * |//| +--+ |//|    <3
		 * +--+ |  | |//|    <4
		 *      |  | +--+    <5
		 *      +--+         <6
		 */
		{ yrect_struct_init(10, 40, 100, 200),
		  yrect_struct_init(20, 50, 500, 600),
		  NULL_R },
		{ yrect_struct_init(30, 60, 300, 400),
		  NULL_R },
		{ yrect_struct_init(10, 20, 100, 200),
		  yrect_struct_init(20, 30, 100, 200),
		  yrect_struct_init(20, 30, 500, 600),
		  yrect_struct_init(30, 40, 100, 200),
		  yrect_struct_init(30, 40, 300, 400),
		  yrect_struct_init(30, 40, 500, 600),
		  yrect_struct_init(40, 50, 300, 400),
		  yrect_struct_init(40, 50, 500, 600),
		  yrect_struct_init(50, 60, 300, 400),
		  NULL_R },
		{ NULL_R },
		{ yrect_struct_init(10, 20, 100, 200),
		  yrect_struct_init(20, 40, 100, 200),
		  yrect_struct_init(20, 40, 500, 600),
		  yrect_struct_init(40, 50, 500, 600),
		  NULL_R },
		{ yrect_struct_init(30, 60, 300, 400),
		  NULL_R },
	},
	{
		/* +---+  +---+
		 * |///|  |///|
		 * +---+  +---+
		 * +---+  +---+
		 * |\\\|  |\\\|
		 * +---+  +---+
		 */
		{ yrect_struct_init(10, 20, 100, 200),
		  yrect_struct_init(10, 20, 300, 400),
		  NULL_R },
		{ yrect_struct_init(30, 40, 100, 200),
		  yrect_struct_init(30, 40, 300, 400),
		  NULL_R },
		{ yrect_struct_init(10, 20, 100, 200),
		  yrect_struct_init(10, 20, 300, 400),
		  yrect_struct_init(30, 40, 100, 200),
		  yrect_struct_init(30, 40, 300, 400),
		  NULL_R },
		{ NULL_R },
		{ yrect_struct_init(10, 20, 100, 200),
		  yrect_struct_init(10, 20, 300, 400),
		  NULL_R },
		{ yrect_struct_init(30, 40, 100, 200),
		  yrect_struct_init(30, 40, 300, 400),
		  NULL_R },
	},
	{
		/* +------+                <1
		 * |      |
		 * |  +=======+            <2
		 * |  |   | +-|---+        <3
		 * |  |   | | |   |
		 * +--|---+ | |   |        <4
		 *    |  +============+    <5
		 *    |  |  | |   |   |
		 *    +==|==|=+   |   |    <6
		 *       |  |     |   |
		 *       +============+    <7
		 *          |     |
		 *          +-----+        <8
		 *
		 * ^  ^  ^^ ^ ^   ^   ^
		 * 1  2  34 5 6   7   8
		 */
		{ yrect_struct_init(10, 40, 100, 400),
		  yrect_struct_init(30, 80, 500, 700),
		  NULL_R },
		{ yrect_struct_init(20, 60, 200, 600),
		  yrect_struct_init(50, 70, 300, 800),
		  NULL_R },
		{ yrect_struct_init(10, 20, 100, 400),
		  yrect_struct_init(20, 30, 100, 600),
		  yrect_struct_init(30, 40, 100, 700),
		  yrect_struct_init(40, 50, 200, 700),
		  yrect_struct_init(50, 60, 200, 800),
		  yrect_struct_init(60, 70, 300, 800),
		  yrect_struct_init(70, 80, 500, 700),
		  NULL_R },
		{ yrect_struct_init(20, 30, 200, 400),
		  yrect_struct_init(30, 40, 200, 400),
		  yrect_struct_init(30, 40, 500, 600),
		  yrect_struct_init(40, 50, 500, 600),
		  yrect_struct_init(50, 70, 500, 700),
		  NULL_R },
		{ yrect_struct_init(10, 20, 100, 400),
		  yrect_struct_init(20, 30, 100, 200),
		  yrect_struct_init(30, 40, 100, 200),
		  yrect_struct_init(30, 40, 600, 700),
		  yrect_struct_init(40, 50, 600, 700),
		  yrect_struct_init(70, 80, 500, 700),
		  NULL_R },
		{ yrect_struct_init(20, 30, 400, 600),
		  yrect_struct_init(30, 40, 400, 500),
		  yrect_struct_init(40, 50, 200, 500),
		  yrect_struct_init(50, 60, 200, 500),
		  yrect_struct_init(50, 60, 700, 800),
		  yrect_struct_init(60, 70, 300, 500),
		  yrect_struct_init(60, 70, 700, 800),
		  NULL_R },
	},
};

static inline bool
is_null_r(const struct yrect *r) {
	static struct yrect null_r = NULL_R;
	return ygeob_eq(&r->yb, &null_r.yb)
		&& ygeob_eq(&r->xb, &null_r.xb);
}


static void
test_fixed(const struct tdst *tp, int tdsz) { /* Input Rect SZ */
	struct yrgn *g0, *g1, *g;
	const struct yrect *pr;
	struct yrect r;
	int cnt = 0;
	while (cnt < tdsz) {
		dpr("\n=============================================\n"
		    "            TEST: Fixed. TC %2d\n"
		    "=============================================\n",
		    cnt);
		g0 = g = ygeog_create_empty();
		pr = tp->r0;
		while (!is_null_r(pr)) {
			g0 = ygeog_union2(g, pr);
			ygeog_destroy(g);
			g = g0;
			pr++;
		}

		g1 = g = ygeog_create_empty();
		pr = tp->r1;
		while (!is_null_r(pr)) {
			g1 = ygeog_union2(g, pr);
			ygeog_destroy(g);
			g = g1;
			pr++;
		}

		/**********************************/
		pr = tp->runion;
		g = ygeog_union(g0, g1);
		ygeog_foreach_rect_begin(g, r) {
			dpr("V: union0:(%d, %d)(%d, %d) / (%d, %d)(%d, %d)\n",
			    r.t, r.b, r.l, r.r, pr->t, pr->b, pr->l, pr->r);
			yassert(!ygeor_is_empty(pr)
				&& ygeor_eq(&r, pr++));
		} ygeog_foreach_rect_end();
		ygeog_destroy(g);

		pr = tp->runion;
		g = ygeog_union(g1, g0); /* check commutative */
		ygeog_foreach_rect_begin(g, r) {
			dpr("V: union1:(%d, %d)(%d, %d) / (%d, %d)(%d, %d)\n",
			    r.t, r.b, r.l, r.r, pr->t, pr->b, pr->l, pr->r);
			yassert(!ygeor_is_empty(pr)
				&& ygeor_eq(&r, pr++));
		} ygeog_foreach_rect_end();
		ygeog_destroy(g);

		/**********************************/
		pr = tp->rintersect;
		g = ygeog_intersect(g0, g1);
		ygeog_foreach_rect_begin(g, r) {
			dpr("V: inter0:(%d, %d)(%d, %d) / (%d, %d)(%d, %d)\n",
			    r.t, r.b, r.l, r.r, pr->t, pr->b, pr->l, pr->r);
			yassert(!ygeor_is_empty(pr)
				&& ygeor_eq(&r, pr++));
		} ygeog_foreach_rect_end();
		ygeog_destroy(g);

		pr = tp->rintersect;
		g = ygeog_intersect(g0, g1); /* check commutative */
		ygeog_foreach_rect_begin(g, r) {
			dpr("V: inter1:(%d, %d)(%d, %d) / (%d, %d)(%d, %d)\n",
			    r.t, r.b, r.l, r.r, pr->t, pr->b, pr->l, pr->r);
			yassert(!ygeor_is_empty(pr)
				&& ygeor_eq(&r, pr++));
		} ygeog_foreach_rect_end();
		ygeog_destroy(g);

		/**********************************/
		pr = tp->r0diff;
		g = ygeog_diff(g0, g1);
		ygeog_foreach_rect_begin(g, r) {
			dpr("V: diff0:(%d, %d)(%d, %d) / (%d, %d)(%d, %d)\n",
			    r.t, r.b, r.l, r.r, pr->t, pr->b, pr->l, pr->r);
			yassert(!ygeor_is_empty(pr)
				&& ygeor_eq(&r, pr++));
		} ygeog_foreach_rect_end();
		ygeog_destroy(g);

		pr = tp->r1diff;
		g = ygeog_diff(g1, g0);
		ygeog_foreach_rect_begin(g, r) {
			dpr("V: diff1:(%d, %d)(%d, %d) / (%d, %d)(%d, %d)\n",
			    r.t, r.b, r.l, r.r, pr->t, pr->b, pr->l, pr->r);
			yassert(!ygeor_is_empty(pr)
				&& ygeor_eq(&r, pr++));
		} ygeog_foreach_rect_end();
		ygeog_destroy(g);

		/**********************************/
		ygeog_destroy(g0);
		ygeog_destroy(g1);
		tp++;
		cnt++;
	}
}


static void
test_geo_rgn(void)  {
	int cnt = 1000;
	srand(time(NULL));
        /* NOT IMPLEMENTED YET */
	dpr(
"**************************************************************************\n"
"\n"
"                        TEST: Fixed\n"
"\n"
"**************************************************************************\n"
	    );
	test_fixed(tdat, yut_arrsz(tdat));
	dpr(
"**************************************************************************\n"
"\n"
"                        TEST: Random\n"
"\n"
"**************************************************************************\n"
	    );
	while (cnt--)
		test_rand();
}



TESTFN(test_geo_rgn, geo_rgn)

#endif /* CONFIG_DEBUG */
