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

/**
 * @file ygraph.h
 * @brief Header file to use graph module
 *
 * This file supports very simple graph concepts.
 * In case of graph, relation between vertex and vertex data is very strong.
 * That is, data of vertex isn't useful out of the graph.
 * So, this is reason why data of each vertex and edge is contained in graph
 * structure as \a char \a d[0] not \a void \a *d.
 * This is biggest difference from list or tree structure.
 */

#ifndef __YGRAPh_h__
#define __YGRAPh_h__

#include <string.h>

#include "ydef.h"
#include "ylistl.h"
#include "yhash.h"

/**
 * Vertex object.
 * This object should be created only by {@link ygraph_create_vertex}.
 * DO NOT create this object directly.
 */
struct yvertex {
	/* \cond */
	struct ylistl_link ie; /* head of Incoming Edge list */
	struct ylistl_link oe; /* head of Outgoing Edge list */
	struct ylistl_link lk; /* link for vertex list */
	/* \endcond */
	char d[0]; /**< custome extra data from here */
};

/**
 * Edge object.
 * This object should be created only by {@link ygraph_add_edge}
 * DO NOT create this object directly.
 */
struct yedge {
	struct yvertex *vt; /**< Vertex To: destination vertex */
	struct yvertex *vf; /**< Vertex From: source vertex */
	/* \cond */
	struct ylistl_link ilk; /* link for incoming edge list */
	struct ylistl_link olk; /* link for outgoing edge list */
	/* \endcond */
	char d[0]; /**< custome extra data from here */
};

/**
 * Graph object.
 * This object should be created only by {@link ygraph_create}
 * DO NOT create this object directly.
 */
struct ygraph {
	/* \cond */
	struct ylistl_link vl; /**< head of vertex list */
	void (*vdfree)(void *); /* call back to free vertex extra data */
	uint32_t vexdsz; /* vertex extra data size */
	void (*edfree)(void *); /* call back to free edge extra data */
	uint32_t eexdsz; /* edge extra data size */
	/* \endcond */
};

/******************************************************************************
 *
 * MACROS
 *
 *****************************************************************************/
/**
 * Iterates outgoing edges of vertex \a v.
 *
 * @param v (struct yvertex *) vertex
 * @param e (struct yedge *) variable used as iteration cursor
 */
#define ygraph_foreach_oedge(v, e)				\
	ylistl_foreach_item(e, &(v)->oe, struct yedge, olk)

/**
 * Iterates outgoing edges of vertex \a v, with removal-safe.
 *
 * @param v (struct yvertex *) vertex
 * @param e (struct yedge *) variable used as iteration cursor
 * @param etmp (struct yedge *) used as temporary storage.
 */
#define ygraph_foreach_oedge_removal_safe(v, e, etmp)			\
	ylistl_foreach_item_removal_safe(e, etmp, &(v)->oe, struct yedge, olk)

/**
 * Iterates incoming edges of vertex \a v.
 *
 * @param v (struct yvertex *) vertex
 * @param e (struct yedge *) variable used as iteration cursor
 */
#define ygraph_foreach_iedge(v, e)				\
	ylistl_foreach_item(e, &(v)->ie, struct yedge, ilk)

/**
 * Iterates incoming edges of vertex \a v, with removal-safe.
 *
 * @param v (struct yvertex *) vertex
 * @param e (struct yedge *) variable used as iteration cursor
 * @param etmp (struct yedge *) used as temporary storage.
 */
#define ygraph_foreach_iedge_removal_safe(v, e, etmp)			\
	ylistl_foreach_item_removal_safe(e, etmp, &(v)->ie, struct yedge, ilk)


/**
 * Iterates vertices in graph \a g.
 *
 * @param g (struct ygraph *) graph.
 * @param v (struct yvertex *) variable used as iteration cursor
 */
#define ygraph_foreach_vertex(g, v)				\
	ylistl_foreach_item(v, &(g)->vl, struct yvertex, lk)

/**
 * Iterates vertices in graph \a g, with removal-safe.
 *
 * @param g (struct ygraph *) graph.
 * @param v (struct yvertex *) variable used as iteration cursor
 * @param vtmp (struct yvertex *) used as temporary storage.
 */
#define ygraph_foreach_vertex_removal_safe(g, v, vtmp)			\
	ylistl_foreach_item_removal_safe(v, vtmp, &(g)->vl, struct yvertex, lk)


/**
 * Iterates vertices in graph \a g, with removal-safe.
 * \a ygraph_foreach_edge_end should be followed at the end of iteration body.
 *
 * @param g (struct ygraph *) graph.
 * @param v (struct yvertex *) variable used as iteration cursor
 * @param e (struct yedge *) variable used as iteration cursor
  */
#define ygraph_foreach_edge_begin(g, v, e)	\
	ygraph_foreach_vertex(g, v) {		\
		ygraph_foreach_oedge(v, e)

/**
 * Closing \a ygraph_foreach_edge_begin iteration body.
 */
#define ygraph_foreach_edge_end() }

/**
 * Iterates vertices in graph \a g, with removal-safe.
 * See \a ygraph_foreach_edge_begin.
 *
 * @param g (struct ygraph *) graph.
 * @param v (struct yvertex *) variable used as iteration cursor
 * @param e (struct yedge *) variable used as iteration cursor
 * @param etmp (struct yedge *) used as temporary storage.
 */
#define ygraph_foreach_edge_removal_safe_begin(g, v, e, etmp)	\
	ygraph_foreach_vertex(g, v) {				\
		ygraph_foreach_oedge_removal_safe(v, e, etmp)

/**
 * See \a ygraph_foreach_edge_end.
 */
#define ygraph_foreach_edge_removal_safe_end() }


/******************************************************************************
 *
 * GRAPH
 *
 *****************************************************************************/
/**
 * Create ygraph object.
 *
 * @param vexdatasz Size of extra data of vertices in the graph.
 * @param vdfree Call back to free extra data of vertex.
 * @param eexdatasz Size of extra data of edges in the graph.
 * @param edfree Call back to free extra data of edge.
 * @return NULL if fails (usually, ENOMEM).
 */
YYEXPORT struct ygraph *
ygraph_create(uint32_t vexdatasz,
	      void (*vdfree)(void *),
	      uint32_t eexdatasz,
	      void (*edfree)(void *));

/**
 * Destroy graph object.
 * ygraph object \a g becomes invalid.
 */
YYEXPORT void
ygraph_destroy(struct ygraph *);

/******************************************************************************
 *
 * GRAPH - VERTEX
 *
 *****************************************************************************/
/**
 * Create vertex object whose data storage is \a datasz.
 *
 * @return NULL for error. Otherwise Success
 */
YYEXPORT struct yvertex *
ygraph_create_vertex(struct ygraph *);

/**
 * Destroy vertex and free resources.
 * Vertex pointer \a v becomes invalid.
 *
 * @param g Graph object in where vertex \a v is created.
 * @param v Vertex that is created in the graph \a g.
 */
YYEXPORT void
ygraph_destroy_vertex(struct ygraph *g, struct yvertex *v);

/**
 * Add vertex \a v to the graph \a g.
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
 * To destroy vertex, See {@link ygraph_destroy_vertex}
 *
 * @return 0 if success. Otherwise -errno. (ex. vertex isn't in the graph)
 */
YYEXPORT int
ygraph_remove_vertex(struct ygraph *, struct yvertex *);


/**
 * The vertex \a v is in the graph \a g?
 *
 * @param g Graph
 * @param v Vertex
 * @return boolean
 */
YYEXPORT bool
ygraph_has_vertex(const struct ygraph *g, const struct yvertex *v);

/******************************************************************************
 *
 * GRAPH - EDGE
 *
 *****************************************************************************/
/**
 * Find edge whose destination is \a to at \a from.
 *
 * @param from Source vertex
 * @param to Destination vertex
 * @return NULL if fail to find.
 */
YYEXPORT struct yedge *
ygraph_find_edge(const struct ygraph *,
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
ygraph_has_edge(const struct ygraph *g,
		const struct yvertex *from,
		const struct yvertex *to) {
	return !!ygraph_find_edge(g, from, to);
}


/**
 * Add edge to the graph \a g.
 * Function may be fails if
 * - one of vertex isn't in the graph \a g.
 * - edge already exists.
 *
 * @param e Newly added edge object. if NULL, this is ignored.
 * @param from Source vertex of edge
 * @param to Destination vertex of edge
 * @return 0 if success. Otherwise -errno.\n
 */
YYEXPORT int
ygraph_add_edge(struct ygraph *,
		struct yedge **e,
		struct yvertex *from,
		struct yvertex *to);

/**
 * Remove edge.
 *
 * @param from source vertex of edge
 * @param to destination vertex of edge
 * @return 0 if success. Otherwise -errno.
 */
YYEXPORT int
ygraph_remove_edge(struct ygraph *,
		   struct yvertex *from,
		   struct yvertex *to);

#endif /* __YGRAPh_h__ */
