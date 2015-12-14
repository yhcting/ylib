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

#ifndef NAN
#  error NAN is required at ygraph.
#endif /* NAN */

/******************************************************************************
 *
 * EDGE
 *
 *****************************************************************************/
static struct yedge *
edge_create(struct yvertex *from, struct yvertex *to, u32 exdsz) {
	struct yedge *e = ymalloc(sizeof(*e) + exdsz);
	if (unlikely(!e))
		return NULL;
	ylistl_init_link(&e->ilk);
	ylistl_init_link(&e->olk);
	e->vf = from;
	e->vt = to;
	memset(&e->d, 0, exdsz);
	return e;
}

static INLINE void
edge_destroy(struct ygraph *g, struct yedge *e) {
	if (g->edfree)
		(*g->edfree)((void *)&e->d);
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
struct ygraph *
ygraph_create(u32 vexdatasz,
	      void (*vdfree)(void *),
	      u32 eexdatasz,
	      void (*edfree)(void *)) {
	struct ygraph *g = ymalloc(sizeof(*g));
	if (unlikely(!g))
		return NULL;
	g->vdfree = vdfree;
	g->vexdsz = vexdatasz;
	g->edfree = edfree;
	g->eexdsz = eexdatasz;
	ylistl_init_link(&g->vl);
	return g;
}

void
ygraph_destroy(struct ygraph *g) {
	struct yvertex *v, *vtmp;
	ygraph_foreach_vertex_removal_safe(g, v, vtmp) {
		ygraph_remove_vertex(g, v);
		ygraph_destroy_vertex(g, v);
	}
	yfree(g);
}

struct yvertex *
ygraph_create_vertex(struct ygraph *g) {
	struct yvertex *v;
	if (unlikely(!(v = ymalloc(sizeof(*v) + g->vexdsz))))
		return NULL;
	ylistl_init_link(&v->ie);
	ylistl_init_link(&v->oe);
	ylistl_init_link(&v->lk);
	return v;
}

void
ygraph_destroy_vertex(struct ygraph *g, struct yvertex *v) {
	if (g->vdfree)
		(*g->vdfree)((void *)&v->d);
	yfree(v);
}

int
ygraph_remove_vertex(struct ygraph *g __unused, struct yvertex *v) {
	struct yedge *e, *etmp;
	yassert(v);

	/* removing connecting edges */
	ygraph_foreach_oedge_removal_safe(v, e, etmp) {
		edge_remove(e);
		edge_destroy(g, e);
	}

	ygraph_foreach_iedge_removal_safe(v, e, etmp) {
		edge_remove(e);
		edge_destroy(g, e);
	}

	/* check "vertex is isolated" from other vertices */
	yassert(v->ie.next == v->ie.prev
		&& v->oe.next == v->oe.prev);

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

struct yedge *
ygraph_find_edge(const struct ygraph *g __unused,
		 const struct yvertex *from,
		 const struct yvertex *to) {
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
ygraph_add_edge(struct ygraph *g,
		struct yedge **oe,
		struct yvertex *from,
		struct yvertex *to) {
	struct yedge *e;
	yassert(g && from && to);
	if (ygraph_has_edge(g, from, to))
		return -EEXIST;
	if (unlikely(!(e = edge_create(from, to, g->eexdsz))))
		return -ENOMEM;
	ylistl_add_last(&from->oe, &e->olk);
	ylistl_add_last(&to->ie, &e->ilk);
	if (oe)
		*oe = e;
	return 0;
}

int
ygraph_remove_edge(struct ygraph *g,
		   struct yvertex *from,
		   struct yvertex *to) {
	struct	yedge *e;
	yassert(g && from && to);
	if (unlikely(!(e = ygraph_find_edge(g, from, to))))
		return -ENOENT;
	edge_remove(e);
	edge_destroy(g, e);
	return 0;
}