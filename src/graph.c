/******************************************************************************
 * Copyright (C) 2011, 2012, 2013, 2014
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

#include <math.h>
#include <errno.h>
#include <string.h>

#include "common.h"
#include "ygraph.h"
#include "yset.h"
#include "ylist.h"

#ifndef NAN
#error NAN is required at ygraph.
#endif /* NAN */

/******************************************************************************
 *
 * EDGE
 *
 *****************************************************************************/
static struct yedge *
edge_create(struct yvertex *from, struct yvertex *to) {
	struct yedge *e = ymalloc(sizeof(*e));
	if (unlikely(!e))
		return NULL;
	ylistl_init_link(&e->ilk);
	ylistl_init_link(&e->olk);
	e->vf = from;
	e->vt = to;
	return e;
}

static INLINE void
edge_destroy(struct yedge *e) {
	yfree(e);
}

static void
edge_remove(struct yedge *e) {
	ylistl_remove(&e->olk);
	ylistl_remove(&e->ilk);
}

/******************************************************************************
 *
 * Interfaces
 *
 *****************************************************************************/
int
ygraph_remove_vertex(struct ygraph *g, struct yvertex *v) {
	struct yedge *e, *etmp;
	yassert(v);

	/* removing connecting edges */
	ygraph_foreach_oedge_safe(v, e, etmp) {
		edge_remove(e);
		edge_destroy(e);
	}

	ygraph_foreach_iedge_safe(v, e, etmp) {
		edge_remove(e);
		edge_destroy(e);
	}

	/* check "vertex is isolated" from other vertices */
	yassert(v->ie.next == v->ie.prev && v->oe.next == v->oe.prev);

	ylistl_remove(&v->lk);
	return 0;
}

bool
ygraph_has_vertex(const struct ygraph *g, const struct yvertex *v) {
	const struct yvertex *vn;
	ygraph_foreach_vertex(g, vn) {
		if (vn == v)
			return TRUE;
	}
	return FALSE;
}

int
ygraph_has_cycle(
	unused const struct ygraph *g,
	const struct yvertex *basev,
	int *n_visited
) {
	struct yvertex *v;
	struct yedge* e;
	struct ylist *vs = NULL; /* Vertex Stack */
	struct ylist *es = NULL; /* Edge Stack */
	int nv = 0;
	yset_t visited = NULL;
	int r = 0;

#define mark_visited(v)						\
	if (unlikely(0 > (r = yset_add(visited, (void *)v)))) {	\
		goto done;					\
	}
#define is_visited(v) yset_has(visited, (void *)v)

	visited = yseti_create();
	/* DFS */
	vs = ylist_create(0, NULL);
	es = ylist_create(0, NULL);

	if (unlikely(!vs || !es || !visited)) {
		r = -ENOMEM;
		goto done;
	}

	if (unlikely(r = ylist_push(vs, (struct yvertex *)basev)))
		goto done;
	ygraph_foreach_iedge(basev, e) {
		if (unlikely(r = ylist_push(es, e)))
			goto done;
	}
	nv++; /* count base vertex */
	r = 0;
	while (ylist_size(es)) {
		e = ylist_pop(es);
		yassert(e);
		v = e->vf;

		/* Adjust DFS vertex history.
		 * Top of vertex stack SHOULD be same with target vertex
		 *   of current visiting edge.
		 * That is, vertex stack has right DFS visit-history.
		 */
		while (e->vt != ylist_peek_last(vs)
		       && ylist_size(vs) > 0)
			ylist_pop(vs);

		if (is_visited(v)) {
			if (ylist_has(vs, v)) {
				r = 1;
				goto done;
			}
			/* It's already discovered. Skip next steps! */
			continue;
		}

		/* This is first visit */

		/* save to history stack */
		nv++;
		mark_visited(v);
		if (unlikely(r = ylist_push(vs, v)))
			goto done;

		ygraph_foreach_iedge(v, e) {
			if (unlikely(r = ylist_push(es, e)))
				goto done;
		}
	}

	if (n_visited)
		*n_visited = nv;

 done:
	if (likely(vs))
		ylist_destroy(vs);
	if (likely(es))
		ylist_destroy(es);
	if (likely(visited))
		yset_destroy(visited);
	return r;

#undef is_visited
#undef mark_visited

}

struct yedge *
ygraph_find_edge(
	unused const struct ygraph *g,
	const struct yvertex *from,
	const struct yvertex *to
) {
	yassert(g && from && to);
	struct yedge *e;
	ygraph_foreach_oedge(from, e) {
		if (unlikely(e->vt == to)) {
			yassert(from == e->vf);
			return e;
		}
	}
	return NULL;
}

int
ygraph_add_edge(
	struct ygraph *g,
	struct yedge **oe,
	struct yvertex *from,
	struct yvertex *to
) {
	struct yedge *e;
	yassert(g && from && to);
	if (ygraph_has_edge(g, from, to))
		return -EEXIST;
	if (unlikely(!(e = edge_create(from, to))))
		return -ENOMEM;
	ylistl_add_last(&from->oe, &e->olk);
	ylistl_add_last(&to->ie, &e->ilk);
	if (oe)
		*oe = e;
	return 0;
}

int
ygraph_remove_edge(
	struct ygraph *g,
	struct yvertex *from,
	struct yvertex *to
) {
	struct	yedge *e;
	yassert(g && from && to);
	if (unlikely(!(e = ygraph_find_edge(g, from, to))))
		return -ENOENT;
	edge_remove(e);
	edge_destroy(e);
	return 0;
}
