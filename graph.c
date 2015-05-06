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

#include "ygraph.h"

#ifndef NAN
#	error NAN is required at ygraph.
#endif /* NAN */

/******************************************************************************
 *
 * GRAPH
 *
 *****************************************************************************/


/******************************************************************************
 *
 * VERTEX
 *
 *****************************************************************************/
static struct yedge *
vertex_find_edge(const struct yvertex *from, const struct yvertex *to) {
	struct yedge *e;
	ygraph_foreach_oedge(from, e) {
		if (e->vt == to) {
			yassert(from == e->vf);
			return e;
		}
	}
	return NULL;
}

static int
vertex_has_edge(const struct yvertex *from, const struct yvertex *to) {
	return !!vertex_find_edge(from, to);
}

/******************************************************************************
 *
 * EDGE
 *
 *****************************************************************************/
static struct yedge *
edge_create(struct yvertex *from, struct yvertex *to, double weight) {
	struct yedge *e = ymalloc(sizeof(*e));
	if (unlikely(!e))
		return NULL;
	ylistl_init_link(&e->ilk);
	ylistl_init_link(&e->olk);
	e->vf = from;
	e->vt = to;
	e->w = weight;
	return e;
}

static inline void
edge_destroy(struct yedge *e) {
	yfree(e);
}

static void
edge_remove(struct yedge *e) {
	ylistl_del(&e->olk);
	ylistl_del(&e->ilk);
}

/******************************************************************************
 *
 * INTERFACES
 *
 *****************************************************************************/
int
ygraph_vertex_set_name(struct yvertex *v, const char *name) {
	int len;
	if (unlikely(!(v && name)))
		return -1;
	len = strlen(name);
	if (unlikely(len >= sizeof(v->name)))
		return -1;
	memcpy(v->name, name, len + 1); /* +1 for trailing 0 */
	return 0;
}

int
ygraph_vertex_init(struct yvertex *v) {
	if (unlikely(!v))
		return -1;
	memset(v->name, 0, sizeof(v->name));
	ylistl_init_link(&v->ie);
	ylistl_init_link(&v->oe);
	ylistl_init_link(&v->lk);
	return 0;
}

struct yvertex *
ygraph_vertex_create(u32 datasz) {
	struct yvertex *v = ymalloc(sizeof(*v) + datasz);
	if (unlikely(!v))
		return NULL;
	if (unlikely(0 > ygraph_vertex_init(v))) {
		yfree(v);
		return NULL;
	}
	return v;
}

struct ygraph *
ygraph_create(void (*fcb)(void *)) {
	struct ygraph *g = ymalloc(sizeof(*g));
	if (unlikely(!g))
		return NULL;
	if (unlikely(0 > ygraph_init(g, fcb))) {
		yfree(g);
		return NULL;
	}
	return g;
}

int
ygraph_init(struct ygraph *g,
	    void (*fcb)(void *)) {
	if (unlikely(!g))
		return -EINVAL;
	g->fcb = fcb;
	ylistl_init_link(&g->vs);
	if (unlikely(!(g->vh = yhash_create(NULL))))
		return -ENOMEM;
	return 0;
}

void
ygraph_clean(struct ygraph *g) {
	struct yvertex *v, *vtmp;
	struct yedge *e, *etmp;
	if (unlikely(!g))
		return;
	yhash_destroy(g->vh);
	ygraph_foreach_vertex_removal_safe(g, v, vtmp) {
		/* Destroying only outgoing edge is enough to iterate all
		 *   edges in the graph.
		 */
		ygraph_foreach_oedge_removal_safe(v, e, etmp) {
			edge_remove(e);
			edge_destroy(e);
		}
		ygraph_vertex_destroy(v, g->fcb);
	}
	g->vh = NULL;
	ylistl_init_link(&g->vs);
}

void
ygraph_destroy(struct ygraph *g) {
	if (unlikely(!g))
		return;
	ygraph_clean(g);
	yfree(g);
}

int
ygraph_add_vertex(struct ygraph *g, struct yvertex *v) {
	if (unlikely(!(g && v)))
		return -1;
	if (unlikely(1 != yhash_add(g->vh, &v, sizeof(v), (void *)1)))
		return -1;
	ylistl_add_last(&g->vs, &v->lk);
	return 0;
}

int
ygraph_has_vertex(const struct ygraph *g, const struct yvertex *v) {
	if (unlikely(!(g && v)))
		return 0;
	return !!yhash_find(g->vh, &v, sizeof(v));
}

int
ygraph_remove_vertex2(struct ygraph *g, struct yvertex *v, int destroy) {
	struct yedge *e, *etmp;
	if (unlikely(!(g && v)))
		return -1;

	/* removing connecting edges */
	ygraph_foreach_oedge_removal_safe(v, e, etmp) {
		edge_remove(e);
		edge_destroy(e);
	}

	ygraph_foreach_iedge_removal_safe(v, e, etmp) {
		edge_remove(e);
		edge_destroy(e);
	}

	/* check "vertex is isolated" from other vertices */
	yassert(v->ie.next == v->ie.prev
		&& v->oe.next == v->oe.prev);

	if (unlikely(1 != yhash_del(g->vh, &v, sizeof(v))))
		return -1;
	ylistl_del(&v->lk);

	if (destroy)
		ygraph_vertex_destroy(v, g->fcb);
	return 0;
}

int
ygraph_remove_vertex(struct ygraph *g, struct yvertex *v) {
	return ygraph_remove_vertex2(g, v, TRUE);
}

struct yvertex *
ygraph_find_vertex(const struct ygraph *g,
		   const char *name) {
	struct yvertex *v;
	if (unlikely(!(g && name)))
		return NULL;
	ygraph_foreach_vertex(g, v) {
		if (unlikely(!strcmp(v->name, name)))
			return v;
	}
	return NULL;
}

int
ygraph_add_edge(struct ygraph  *g,
		struct yvertex *from,
		struct yvertex *to,
		double		weight) {
	struct yedge *e;
	if (unlikely(!(g && from && to)
		     || 1 != ygraph_has_vertex(g, from)
		     || 1 != ygraph_has_vertex(g, to)))
		return -1;
	if (vertex_has_edge(from, to))
		return -1;
	if (unlikely(!(e = edge_create(from, to, weight))))
		return -1;
	ylistl_add_last(&from->oe, &e->olk);
	ylistl_add_last(&to->ie, &e->ilk);
	return 0;
}

int
ygraph_remove_edge(struct ygraph  *g,
		   struct yvertex *from,
		   struct yvertex *to) {
	struct	yedge *e;
	if (unlikely(!(g && from && to)
		     || 1 != ygraph_has_vertex(g, from)
		     || 1 != ygraph_has_vertex(g, to)))
		return -1;

	if (unlikely(!(e = vertex_find_edge(from, to))))
		return -1;

	edge_remove(e);
	edge_destroy(e);
	return 0;
}

int
ygraph_update_edge_weight(struct ygraph	 *g,
			  struct yvertex *from,
			  struct yvertex *to,
			  double	  weight) {
	struct yedge *e;
	if (unlikely(!(g && from && to)
		     || isnan(weight)
		     || 1 != ygraph_has_vertex(g, from)
		     || 1 != ygraph_has_vertex(g, to)))
		return -1;
	if (unlikely(!(e = vertex_find_edge(from, to))))
		return -1;
	e->w = weight;
	return 0;
}

int
ygraph_has_edge(const struct ygraph *g,
		const struct yvertex *from,
		const struct yvertex *to) {
	if (unlikely(!(g && from && to)
		     || 1 != ygraph_has_vertex(g, from)
		     || 1 != ygraph_has_vertex(g, to)))
		return 0;
	return vertex_has_edge(from, to);
}

double
ygraph_get_edge_weight(const struct ygraph *g,
		       const struct yvertex *from,
		       const struct yvertex *to) {
	struct yedge *e;
	if (unlikely(!(g && from && to)
		     || 1 != ygraph_has_vertex(g, from)
		     || 1 != ygraph_has_vertex(g, to)))
		return NAN;
	if (unlikely(!(e = vertex_find_edge(from, to))))
		return -1;
	return e->w;
}

int
ygraph_add_vertex_prev(struct ygraph *g,
		       struct yvertex *basev,
		       struct yvertex *v) {
	if (unlikely(!(g && basev && v)
		     || !ygraph_has_vertex(g, basev)))
		return -1;
	if (unlikely(ygraph_add_vertex(g, v)))
		return -1;
	if (unlikely(ygraph_add_edge(g, basev, v, 0))) {
		ygraph_remove_vertex2(g, v, FALSE);
		return -1;
	}
	return 0;
}

int
ygraph_add_vertex_next(struct ygraph *g,
		       struct yvertex *basev,
		       struct yvertex *v) {
	if (unlikely(!(g && basev && v)
		     || !ygraph_has_vertex(g, basev)))
		return -1;
	if (unlikely(ygraph_add_vertex(g, v)))
		return -1;
	if (unlikely(ygraph_add_edge(g, v, basev, 0))) {
		ygraph_remove_vertex2(g, v, FALSE);
		return -1;
	}
	return 0;
}
