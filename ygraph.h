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

#ifndef __YGRAPh_h__
#define __YGRAPh_h__

#include "ylistl.h"
#include "yhash.h"

#define YGRAPH_MAX_VERTEX_NAME_LEN 63

struct yvertex {
	/* vertex name - usage of this field is totally up to client.
	 * But, originally this is designed to identify vertex.
	 * (Unique in the graph is recommended(but not required))
	 */
	char		   name[YGRAPH_MAX_VERTEX_NAME_LEN + 1];
	void		  *d;  /* vertex data */
	struct ylistl_link ie; /* head of Incoming Edge list */
	struct ylistl_link oe; /* head of Outgoing Edge list */
	struct ylistl_link lk; /* link for vertex list */
};

struct yedge {
	double		   w;	/* edge weight */
	struct yvertex	  *vt;	/* Vertex To: destination vertex */
	struct yvertex	  *vf;	/* Vertex From: source vertex */
	struct ylistl_link ilk; /* link for incoming edge list */
	struct ylistl_link olk; /* link for outgoing edge list */
};

struct yhash;
struct ygraph {
	/* call back to free vertex data */
	void		 (*fcb)(void *);
	struct ylistl_link vs;	/* head of vertex list */
	struct yhash	  *vh;	/* vertex hash */
};

/******************************************************************************
 *
 * Macros
 *
 *****************************************************************************/
/**
 * @v : struct yvertex *.
 * @e : struct yedge *. variable used as iteration cursor
 */
#define ygraph_foreach_oedge(v, e)				\
	ylistl_foreach_item(e, &v->oe, struct yedge, olk)

/**
 * @v : struct yvertex *.
 * @e : struct yedge *. variable used as iteration cursor
 * @tmp: another 'struct yedge *' used as temporary storage.
 */
#define ygraph_foreach_oedge_removal_safe(v, e, etmp)			\
	ylistl_foreach_item_removal_safe(e, etmp, &v->oe, struct yedge, olk)

/**
 * @v : struct yvertex *.
 * @e : struct yedge *. variable used as iteration cursor
 */
#define ygraph_foreach_iedge(v, e)				\
	ylistl_foreach_item(e, &v->ie, struct yedge, ilk)

/**
 * @v : struct yvertex *.
 * @e : struct yedge *. variable used as iteration cursor
 * @tmp: another 'struct yedge *' used as temporary storage.
 */
#define ygraph_foreach_iedge_removal_safe(v, e, etmp)			\
	ylistl_foreach_item_removal_safe(e, etmp, &v->ie, struct yedge, ilk)


/**
 * @g : struct ygraph *.
 * @v : struct yvertex. variable used as iteration cursor
 */
#define ygraph_foreach_vertex(g, v)				\
	ylistl_foreach_item(v, &g->vs, struct yvertex, lk)

/**
 * @g : struct ygraph *.
 * @v : struct yvertex. variable used as iteration cursor
 * @tmp: another 'struct yvertex *' used as temporary storage.
 */
#define ygraph_foreach_vertex_removal_safe(g, v, vtmp)			\
	ylistl_foreach_item_removal_safe(v, vtmp, &g->vs, struct yvertex, lk)

/******************************************************************************
 *
 * GRAPH
 *
 *****************************************************************************/
/**
 * ygraph is created and initialized.
 * @fcb : call back to free vertex data.
 */
struct ygraph *
ygraph_create(void (*fcb)(void *));

/**
 * return
 *     0: success
 *    <0: fails (ex. vertex is already in the graph)
 */
int
ygraph_init(struct ygraph *,
	    void (*fcb)(void *));

/**
 * ygraph itself is NOT destroied.
 */
void
ygraph_clean(struct ygraph *);

void
ygraph_destroy(struct ygraph *);


/******************************************************************************
 *
 * VERTEX
 *
 *****************************************************************************/

/**
 * return
 *     0: success
 *    <0: error
 */
int
ygraph_init_vertex(struct yvertex *);

/**
 * return
 *     0: success
 *    <0: fails (ex. too long name)
 */
int
ygraph_set_vertex_name(struct yvertex *v, const char *name);

const char *
ygraph_get_vertex_name(const struct yvertex *v) {
	return v->name;
}

static inline void
ygraph_set_vertex_data(struct yvertex *v, void *d) {
	v->d = d;
}

static inline void *
ygraph_get_vertex_data(const struct yvertex *v) {
	return v->d;
}

/**
 * return
 *     0: success
 *    <0: fails (ex. vertex is already in the graph)
 */
int
ygraph_add_vertex(struct ygraph *, struct yvertex *);


/**
 * return
 *     true
 *     false
 */
int
ygraph_has_vertex(const struct ygraph *, const struct yvertex *);


/**
 * Remove vertex from the graph and destroy it.
 * Pointer to the vertex data is NOT valid anymore.
 *
 * return
 *	0: success
 *     <0: fails (ex. vertex doesn't belongs to ygraph)
 */
int
ygraph_remove_vertex(struct ygraph *, struct yvertex *);

/**
 * Remove vertex from the graph and destroy it.
 * @destroy 'true' for destroying vertex too
 *	    (exactly same with ygraph_remove_vertex)
 *	    'false' for just removing vertex (NOT destroied)
 * return
 *	0: success
 *     <0: fails (ex. vertex doesn't belongs to ygraph)
 */
int
ygraph_remove_vertex2(struct ygraph *, struct yvertex *, int destroy);

/**
 * This uses linear search. That is, expensive operation.
 * return
 *     NULL - fail to find.
 */
struct yvertex *
ygraph_find_vertex(const struct ygraph *, const char *name);

/******************************************************************************
 *
 * EDGE
 *
 *****************************************************************************/

/**
 * return
 *     0: success
 *    <0: fails
 *	  * one of vertex doesn't belongs to ygraph.
 *	  * edge already exists.
 */
int
ygraph_add_edge(struct ygraph *,
		struct yvertex *from,
		struct yvertex *to,
		double		weight);

/**
 * return
 *     0: success
 *    <0: error
 */
int
ygraph_remove_edge(struct ygraph *,
		   struct yvertex *from,
		   struct yvertex *to);


/**
 * return
 *     0: true
 *    <0: error (ex. invalid edge, weight is NAN)
 */
int
ygraph_update_edge_weight(struct ygraph *,
			  struct yvertex *from,
			  struct yvertex *to,
			  double	  weight);

/**
 * return
 *     true (edge is in the graph)
 *     false
 */
int
ygraph_has_edge(const struct ygraph *,
		const struct yvertex *from,
		const struct yvertex *to);

/**
 * return
 *     weight: success
 *     NAN: error
 */
double
ygraph_get_edge_weight(const struct ygraph *,
		       const struct yvertex *from,
		       const struct yvertex *to);


#endif /* __YGRAPh_h__ */
