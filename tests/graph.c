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
#include "test.h"
#ifdef CONFIG_DEBUG

#include <string.h>
#include <assert.h>

#include "common.h"
#include "ygraph.h"


/* Test Graph
 *
 *		-------
 *		|     |
 * v000 -----> v001 <--
 *  |--------> v002 ------> v012 <------ v112
 *  |		 |<--------    ---------
 *  |		 |	       |       |
 *  |		 |	       |<-------
 *  |		 |--------> v022 -------
 *  |		 |<--------  ^	     ^ |
 *  |	     ----|	     |	     | |
 *  |	     |	 |<-- v032   |	     | |
 *  |	     v		     |	     | |
 *  |-----> v003 -------------	     | |
 *  |--------------------------------- |
 *  |<----------------------------------
 */

struct mynode {
	struct yvertex v;
	const char *name;
};

static struct mynode *
find_vertex(struct ygraph *g, const char *name) {
	struct yvertex *v;
	struct mynode *mn;
	ygraph_foreach_vertex(g, v) {
		mn = YYcontainerof(v, struct mynode, v);
		if (!strcmp(mn->name, name))
			return mn;
	}
	return NULL;
}

static void
make_test_graph(struct ygraph *g) {
	struct mynode *v000, *v001, *v002, *v003, *v012, *v022, *v032;
	struct mynode *v112;

#define __add_v(g, N, NAMe)			\
	do {					\
		N = ymalloc(sizeof(*N));	\
		N->name = NAMe;			\
		ygraph_init_vertex(&N->v);	\
		ygraph_add_vertex(g, &N->v);	\
	} while (0)

#define __add_edge(g, oe, mn0, mn1) \
	ygraph_add_edge(g, oe, &mn0->v, &mn1->v);

	__add_v(g, v000, "v000");
	__add_v(g, v001, "v001");
	__add_v(g, v002, "v002");
	__add_v(g, v003, "v003");
	__add_v(g, v012, "v012");
	__add_v(g, v022, "v022");
	__add_v(g, v032, "v032");
	__add_v(g, v112, "v112");

	__add_edge(g, NULL, v000, v001);
	__add_edge(g, NULL, v000, v002);
	__add_edge(g, NULL, v000, v003);
	__add_edge(g, NULL, v000, v022);
	__add_edge(g, NULL, v001, v001);
	__add_edge(g, NULL, v002, v012);
	__add_edge(g, NULL, v002, v022);
	__add_edge(g, NULL, v002, v003);
	__add_edge(g, NULL, v003, v022);
	__add_edge(g, NULL, v012, v002);
	__add_edge(g, NULL, v022, v002);
	__add_edge(g, NULL, v022, v000);
	__add_edge(g, NULL, v022, v022);
	__add_edge(g, NULL, v032, v002);
	__add_edge(g, NULL, v112, v012);

#undef __add_edge
#undef __add_v
}


static void
test_graph(void) {
	struct ygraph g_;
	struct ygraph *g;
	struct yedge *e;
	struct yvertex *v, *vtmp;
	struct mynode *v000, *v002, *v012, *v112, *v022;

	g = &g_;
	/*
	 * Graph construction & destruction
	 */
	ygraph_init(g);
	ygraph_clean(g);

	/*
	 * Verify graph construction.
	 */
	ygraph_init(g);
	make_test_graph(g);

	v000 = find_vertex(g, "v000");
	v002 = find_vertex(g, "v002");
	v012 = find_vertex(g, "v012");
	v112 = find_vertex(g, "v112");
	v022 = find_vertex(g, "v022");

	yassert(!ygraph_has_vertex(g, NULL));

	yassert(ygraph_has_edge(g, &v002->v, &v012->v));
	yassert(ygraph_has_edge(g, &v012->v, &v002->v));
	yassert(ygraph_has_edge(g, &v112->v, &v012->v));
	yassert(ygraph_has_edge(g, &v022->v, &v022->v));
	yassert(!ygraph_has_edge(g, &v002->v, &v112->v));

	/*
	 * Test graph update operations
	 */
	yassert(!ygraph_remove_edge(g, &v002->v, &v022->v));
	yassert(!ygraph_has_edge(g, &v002->v, &v022->v));
	yassert(!ygraph_remove_edge(g, &v022->v, &v022->v));
	yassert(!ygraph_has_edge(g, &v022->v, &v022->v));
	yassert(!ygraph_add_edge(g, NULL, &v022->v, &v022->v));
	yassert(ygraph_has_edge(g, &v022->v, &v022->v));
	yassert(!ygraph_remove_vertex(g, &v022->v));

	yfree(v022);
	/*
	 * Graph sanity test
	 */
	ygraph_foreach_oedge(&v002->v, e) {
		yassert(e->vt != &v022->v
			&& e->vf != &v022->v);
	}
	ygraph_foreach_iedge(&v002->v, e) {
		yassert(e->vt != &v022->v
			&& e->vf != &v022->v);
	}
	ygraph_foreach_oedge(&v000->v, e) {
		yassert(e->vt != &v022->v
			&& e->vf != &v022->v);
	}
	ygraph_foreach_iedge(&v000->v, e) {
		yassert(e->vt != &v022->v
			&& e->vf != &v022->v);
	}

	ygraph_foreach_vertex_removal_safe(g, v, vtmp) {
		ygraph_remove_vertex(g, v);
		yfree(YYcontainerof(v, struct mynode, v));
	}

	ygraph_clean(g);
}

TESTFN(graph)

#endif /* CONFIG_DEBUG */
