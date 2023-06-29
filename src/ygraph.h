/******************************************************************************
 * Copyright (C) 2011, 2012, 2013, 2014, 2015, 2023
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

/**
 * @file ygraph.h
 * @brief Header file to use graph module
 *
 * This file supports very simple graph concepts.
 * In case of graph, relation between vertex and vertex data is very strong.
 * That is, data of vertex isn't useful out of the graph.
 * So, this is reason why data of each vertex and edge is contained in graph
 * structure as `char d[0]` not `void *d`.
 * This is biggest difference from list or tree structure.
 */

#pragma once

#include <string.h>

#include "ydef.h"
#include "ylistl.h"
#include "yhash.h"

/**
 * Vertex object.
 */
struct yvertex {
	/* @cond */
	struct ylistl_link ie; /* head of Incoming Edge list */
	struct ylistl_link oe; /* head of Outgoing Edge list */
	struct ylistl_link lk; /* link for vertex list */
	/* @endcond */
};

/**
 * Edge object.
 * This object should be created only by @ref ygraph_add_edge
 * DO NOT create this object directly.
 */
struct yedge {
	struct yvertex *vt; /**< Vertex To: destination vertex */
	struct yvertex *vf; /**< Vertex From: source vertex */
	/* @cond */
	struct ylistl_link ilk; /* link for incoming edge list */
	struct ylistl_link olk; /* link for outgoing edge list */
	/* @endcond */
};

/**
 * Graph object.
 */
struct ygraph {
	/* @cond */
	struct ylistl_link vl; /**< head of vertex list */
	/* @endcond */
};

/******************************************************************************
 *
 * MACROS
 *
 *****************************************************************************/
/**
 * Iterates outgoing edges of vertex @p v.
 *
 * @param v (struct @ref yvertex *) vertex
 * @param e (struct @ref yedge *) variable used as iteration cursor
 */
#define ygraph_foreach_oedge(v, e) \
	ylistl_foreach_item(e, &(v)->oe, struct yedge, olk)

/**
 * Iterates outgoing edges of vertex @p v, with removal-safe.
 *
 * @param v (struct @ref yvertex *) vertex
 * @param e (struct @ref yedge *) variable used as iteration cursor
 * @param etmp (struct @ref yedge *) used as temporary storage.
 */
#define ygraph_foreach_oedge_safe(v, e, etmp) \
	ylistl_foreach_item_safe(e, etmp, &(v)->oe, struct yedge, olk)

/**
 * Iterates incoming edges of vertex @p v.
 *
 * @param v (struct @ref yvertex *) vertex
 * @param e (struct @ref yedge *) variable used as iteration cursor
 */
#define ygraph_foreach_iedge(v, e) \
	ylistl_foreach_item(e, &(v)->ie, struct yedge, ilk)

/**
 * Iterates incoming edges of vertex @p v, with removal-safe.
 *
 * @param v (struct @ref yvertex *) vertex
 * @param e (struct @ref yedge *) variable used as iteration cursor
 * @param etmp (struct @ref yedge *) used as temporary storage.
 */
#define ygraph_foreach_iedge_safe(v, e, etmp) \
	ylistl_foreach_item_safe(e, etmp, &(v)->ie, struct yedge, ilk)


/**
 * Iterates vertices in graph @p g.
 *
 * @param g (struct @ref ygraph *) graph.
 * @param v (struct @ref yvertex *) variable used as iteration cursor
 */
#define ygraph_foreach_vertex(g, v) \
	ylistl_foreach_item(v, &(g)->vl, struct yvertex, lk)

/**
 * Iterates vertices in graph @p g, with removal-safe.
 *
 * @param g (struct @ref ygraph *) graph.
 * @param v (struct @ref yvertex *) variable used as iteration cursor
 * @param vtmp (struct @ref yvertex *) used as temporary storage.
 */
#define ygraph_foreach_vertex_safe(g, v, vtmp) \
	ylistl_foreach_item_safe(v, vtmp, &(g)->vl, struct yvertex, lk)


/**
 * Iterates edges in graph @p g, with removal-safe.
 *
 * @param g (struct @ref ygraph *) graph.
 * @param v (struct @ref yvertex *) variable used as iteration cursor
 * @param e (struct @ref yedge *) variable used as iteration cursor
  */
#define ygraph_foreach_edge(g, v, e)	\
	ygraph_foreach_vertex(g, v)	\
		ygraph_foreach_oedge(v, e)

/**
 * Iterates edges in graph @p g, with removal-safe.
 *
 * @param g (struct @ref ygraph *) graph.
 * @param v (struct @ref yvertex *) variable used as iteration cursor
 * @param e (struct @ref yedge *) variable used as iteration cursor
 * @param etmp (struct @ref yedge *) used as temporary storage.
 */
#define ygraph_foreach_edge_safe(g, v, e, etmp)	\
	ygraph_foreach_vertex(g, v)		\
		ygraph_foreach_oedge_safe(v, e, etmp)


/******************************************************************************
 *
 * GRAPH - VERTEX
 *
 *****************************************************************************/
/**
 * Initialize vertex data structure.
 *
 * @param v Vertex
 */
static YYINLINE void
ygraph_init_vertex(struct yvertex *v) {
	ylistl_init_link(&v->ie);
	ylistl_init_link(&v->oe);
	ylistl_init_link(&v->lk);
}

/**
 * Add vertex @p v to the graph @p g.
 * Vertex can't be added to more than on Graph.
 *
 * @param g Graph
 * @param v Vertex
 */
static YYINLINE void
ygraph_add_vertex(struct ygraph *g, struct yvertex *v) {
	ylistl_add_last(&g->vl, &v->lk);
}

/**
 * Deatch vertex from a graph where this vertex is attached.
 *
 * @return 0 if success. Otherwise @c -errno. (ex. vertex isn't in the graph)
 */
YYEXPORT int
ygraph_remove_vertex(struct ygraph *, struct yvertex *);


/**
 * Get number of vertices in the graph.
 *
 * @param g Graph
 */
static YYINLINE int
ygraph_vertex_size(struct ygraph *g) {
	return ylistl_size(&g->vl);
}

/**
 * The vertex is in the graph?
 *
 * @return boolean
 */
YYEXPORT bool
ygraph_has_vertex(const struct ygraph *, const struct yvertex *);

/**
 * Check the vertex is in cycle
 *
 * @param n_visited Number of vertices visited during cyclic check.
 *	This can be NULL.
 * @return
 * 	- 1 : cyclic link exists
 *	- 0 : cyclic link DOESN'T exists
 *	- <0: @c -errno
 */
YYEXPORT int
ygraph_has_cycle(
	const struct ygraph *,
	const struct yvertex *,
	int *n_visited);

/**
 * Get number of incoming edges of the vertex.
 *
 * @param g Graph
 * @param v Vertex
 * @return Number of incoming edges
 */
static YYINLINE int
ygraph_iedge_size(struct ygraph *g, struct yvertex *v) {
	return ylistl_size(&v->ie);
}

/**
 * Get number of outgoing edges of the vertex.
 *
 * @param g Graph
 * @param v Vertex
 * @return Number of outgoing edges
 */
static YYINLINE int
ygraph_oedge_size(struct ygraph *g, struct yvertex *v) {
	return ylistl_size(&v->oe);
}


/******************************************************************************
 *
 * GRAPH - EDGE
 *
 *****************************************************************************/
/**
 * Find edge whose destination is @p to at @p from.
 *
 * @param from Source vertex
 * @param to Destination vertex
 * @return NULL if fail to find.
 */
YYEXPORT struct yedge *
ygraph_find_edge(
	const struct ygraph *,
	const struct yvertex *from,
	const struct yvertex *to);


/**
 * Does graph have given edge?
 *
 * @param g Graph
 * @param from Source vertex
 * @param to Destination vertex
 */
static YYINLINE bool
ygraph_has_edge(
	const struct ygraph *g,
	const struct yvertex *from,
	const struct yvertex *to
) {
	return !!ygraph_find_edge(g, from, to);
}


/**
 * Add edge to the graph @p g.
 * Function may be fails if
 * - one of vertex isn't in the graph @p g.
 * - edge already exists.
 *
 * @param e Newly added edge object. if NULL, this is ignored.
 * @param from Source vertex of edge
 * @param to Destination vertex of edge
 * @return 0 if success. Otherwise @c -errno.
 */
YYEXPORT int
ygraph_add_edge(
	struct ygraph *,
	struct yedge **e,
	struct yvertex *from,
	struct yvertex *to);

/**
 * Remove edge.
 *
 * @param from source vertex of edge
 * @param to destination vertex of edge
 * @return 0 if success. Otherwise @c -errno.
 */
YYEXPORT int
ygraph_remove_edge(
	struct ygraph *,
	struct yvertex *from,
	struct yvertex *to);


/******************************************************************************
 *
 * GRAPH
 *
 *****************************************************************************/
/**
 * Initialize ygraph object
 */
static YYINLINE void
ygraph_init(struct ygraph *g) {
	ylistl_init_link(&g->vl);
}

/**
 * Cleanup ygraph object. Object becomes invalid after clean.
 */
static YYINLINE void
ygraph_clean(struct ygraph *g) {
	struct yvertex *v, *vtmp;
	ygraph_foreach_vertex_safe(g, v, vtmp) {
		ygraph_remove_vertex(g, v);
	}
}
