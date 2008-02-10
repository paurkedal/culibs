/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2006--2007  Petter Urkedal <urkedal@nbi.dk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CUGRA_GRAPH_H
#define CUGRA_GRAPH_H

#include <cugra/fwd.h>
#include <cu/clos.h>
#include <cu/dlink.h>
#include <cu/ptr.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cugra_graph_h cugra/graph.h: Graph Struct and Primitives
 * @{\ingroup cugra_mod
 * This defines a directed graph with access to both input and output arcs
 * for each vertex, using doubly linked lists.  Thus single arc updates is
 * O(1) and single vertex updates is linear in the number of incident arcs.
 *
 * The structure can quite easily be used for undirected graphs, as well,
 * using the io/out generic methods.  This is done by iterating over the \ref
 * cugra_direction_t argument before iterating over the incident arcs
 * \code
 * cugra_direction_t dir;
 * cugra_arc_t e;
 * for (dir = 0; dir < 2; ++dir)
 * for (e = cugra_vertex_arcs_begin(dir, v);
 *      e != cugra_vertex_arcs_end(dir, v);
 *      e = cugra_vertex_arcs_next(dir, e))
 *     process(e);
 * \endcode
 */

#define cugra_opt_undirected 1

typedef enum {
    cugra_direction_BEGIN,
    cugra_direction_out = 0, /*!< Index for accessing out-arcs of a vertex. */
    cugra_direction_in  = 1, /*!< Index for accessing in-arcs of a vertex. */
    cugra_direction_END
} cugra_direction_t;

struct cugra_graph_s
{
    unsigned int options;
    struct cu_dlink_s vertices;
};

struct cugra_vertex_s
{
    struct cu_dlink_s in_graph;
    struct cu_dlink_s adj_link[2];
};

struct cugra_adjlink_s
{
    struct cu_dlink_s link;
    cugra_vertex_t vertex;
};

struct cugra_arc_s
{
    struct cugra_adjlink_s adj[2];
};

/*!Construct \a G as an empty graph. */
void cugra_graph_cct(cugra_graph_t G, unsigned int options);

/*!Return an empty graph. */
cugra_graph_t cugra_graph_new(unsigned int options);

/*!Moves all vertices and arcs from \a G_src into \a G_dst. */
void cugra_graph_merge(cugra_graph_t G_dst, cugra_graph_t G_src);

/*!True iff \a G is a directed graph. */
CU_SINLINE cu_bool_t cugra_graph_is_directed(cugra_graph_t G)
{ return !(G->options & cugra_opt_undirected); }

/*!Construct and insert \a v into \a G. This variant is useful when \a v is
 * inherited in another struct, otherwise see \ref cugra_graph_vertex_new. */
void cugra_graph_vertex_init(cugra_graph_t G, cugra_vertex_t v);

/*!Insert a vertex into \a G and return a referece to it. If you need to store
 * data on the vertex, use \ref cugra_graph_vertex_new_mem or \ref
 * cugra_graph_vertex_new_ptr. */
cugra_vertex_t cugra_graph_vertex_new(cugra_graph_t G);

/*!Insert a vertex into \a G with slot size \a size and return it. */
cugra_vertex_t cugra_graph_vertex_new_mem(cugra_graph_t G, size_t size);

/*!Insert a vertex into \a G, with \a ptr stored in its slot, and return it. */
cugra_vertex_t cugra_graph_vertex_new_ptr(cugra_graph_t G, void *ptr);

/*!A pointer to the slot of \a v. */
CU_SINLINE void *cugra_vertex_mem(cugra_vertex_t v) { return v + 1; }

/*!Return an assumed pointer stored in the slot of \a v. */
CU_SINLINE void *cugra_vertex_ptr(cugra_vertex_t v) { return *(void **)(v+1); }

/*!True iff \a v is isolated. */
CU_SINLINE cu_bool_t cugra_vertex_is_isolated(cugra_vertex_t v)
{ return cu_dlink_is_singular(&v->adj_link[0])
      && cu_dlink_is_singular(&v->adj_link[1]); }

/*!True iff \a v is a sink, that is, it has out-degree 0. */
CU_SINLINE cu_bool_t cugra_vertex_is_sink(cugra_vertex_t v)
{ return cu_dlink_is_singular(&v->adj_link[0]); }

/*!True iff \a v is a source, that is, it has in-degree 0. */
CU_SINLINE cu_bool_t cugra_vertex_is_source(cugra_vertex_t v)
{ return cu_dlink_is_singular(&v->adj_link[1]); }

/*!True iff \a v has out-degree 1. */
CU_SINLINE cu_bool_t cugra_vertex_is_convergency(cugra_vertex_t v)
{ return cu_dlink_is_2node(&v->adj_link[0]); }

/*!True iff \a v has in-degree 1. */
CU_SINLINE cu_bool_t cugra_vertex_is_divergency(cugra_vertex_t v)
{ return cu_dlink_is_2node(&v->adj_link[1]); }

/*!True iff \a v is a sink or a convergency. */
CU_SINLINE cu_bool_t cugra_vertex_outdegree_leq_1(cugra_vertex_t v)
{ return cu_dlink_count_leq_2(&v->adj_link[0]); }

/*!True iff \a v is a source or a divergency. */
CU_SINLINE cu_bool_t cugra_vertex_indegree_leq_1(cugra_vertex_t v)
{ return cu_dlink_count_leq_2(&v->adj_link[1]); }

/*!True iff \a v has a loop. */
cu_bool_t cugra_vertex_has_loop(cugra_vertex_t v);

/*!Insert an arc from \a tail to \a head and return a reference to it. */
cugra_arc_t cugra_graph_arc_new(cugra_vertex_t tail, cugra_vertex_t head);

/*!Insert an arc from \a tail to \a head with slot size \a size and return
 * it. */
cugra_arc_t cugra_graph_arc_new_mem(cugra_vertex_t tail, cugra_vertex_t head,
				    size_t size);

/*!Insert an arc from \a tail to \a head with slot initialised to \a ptr and
 * return it. */
cugra_arc_t cugra_graph_arc_new_ptr(cugra_vertex_t tail, cugra_vertex_t head,
				    void *ptr);

/*!Erase \a a from its graph. */
void cugra_erase_arc(cugra_arc_t a);

/*!Erase \a v and all incident arcs from its graph. */
void cugra_erase_vertex(cugra_vertex_t v);
extern cu_clop(cugra_erase_vertex_clop, void, cugra_vertex_t);

/*!Make arcs from each in-neightbour of \a v to each out-neighbour of \a v,
 * and erase \a v and all its incident arcs. */
void cugra_eliminate_vertex(cugra_vertex_t v);

/*!Erase all loops on \a v. */
void cugra_vertex_erase_loops(cugra_vertex_t v);

/*!Erase loops on all vertices of \a G. */
void cugra_graph_erase_loops(cugra_graph_t G);

/*!A pointer to the slot of \a a. */
CU_SINLINE void *cugra_arc_mem(cugra_arc_t a) { return a + 1; }

/*!Assume the slot of \a a contains a pointer and return it. */
CU_SINLINE void *cugra_arc_ptr(cugra_arc_t a) { return *(void **)(a + 1); }

/*!Given that we are iterating over incedent arcs of direction \a dir from a
 * vertex \e v, this returns the vertex adjacent to \e v due to \a e. */
CU_SINLINE cugra_vertex_t
cugra_arc_adjacent(cugra_direction_t dir, cugra_arc_t e)
{ return e->adj[dir].vertex; }

/*!The head of \a e. */
CU_SINLINE cugra_vertex_t
cugra_arc_head(cugra_arc_t e) { return e->adj[cugra_direction_out].vertex; }

/*!The tail of \a e. */
CU_SINLINE cugra_vertex_t
cugra_arc_tail(cugra_arc_t e) { return e->adj[cugra_direction_in].vertex; }

/*!True iff \a a is a loop (head = tail). */
CU_SINLINE cu_bool_t
cugra_arc_is_loop(cugra_arc_t a)
{ return cugra_arc_head(a) == cugra_arc_tail(a); }

/*!The first incident arc with direction \a dir from \a v. */
CU_SINLINE cugra_arc_t
cugra_vertex_arcs_begin(cugra_direction_t dir, cugra_vertex_t v)
{
    return cu_ptr_add(v->adj_link[dir].next,
		      -offsetof(struct cugra_arc_s, adj[dir].link));
}

/*!A past-the-end mark for incident arcs with direction \a dir from \a v. */
CU_SINLINE cugra_arc_t
cugra_vertex_arcs_end(cugra_direction_t dir, cugra_vertex_t v)
{
    return cu_ptr_add(&v->adj_link[dir],
		      -offsetof(struct cugra_arc_s, adj[dir].link));
}

/*!The next incident arc with direction \a dir from the terminal of \a e
 * where \a e has direction \a dir. */
CU_SINLINE cugra_arc_t
cugra_vertex_arcs_next(cugra_direction_t dir, cugra_arc_t e)
{
    return cu_ptr_add(e->adj[dir].link.next,
		      -offsetof(struct cugra_arc_s, adj[dir].link));
}

/*!The first out-arc of \a v. */
CU_SINLINE cugra_arc_t cugra_vertex_outarcs_begin(cugra_vertex_t v)
{ return cugra_vertex_arcs_begin(cugra_direction_out, v); }

/*!An end-of-list marker for the out-arcs of \a v. */
CU_SINLINE cugra_arc_t cugra_vertex_outarcs_end(cugra_vertex_t v)
{ return cugra_vertex_arcs_end(cugra_direction_out, v); }

/*!The next out-arc from the tail of \a e. */
CU_SINLINE cugra_arc_t cugra_vertex_outarcs_next(cugra_arc_t e)
{ return cugra_vertex_arcs_next(cugra_direction_out, e); }

/*!The first in-arc of \a v. */
CU_SINLINE cugra_arc_t cugra_vertex_inarcs_begin(cugra_vertex_t v)
{ return cugra_vertex_arcs_begin(cugra_direction_in, v); }

/*!An end-of-list marker for the in-arcs of \a v. */
CU_SINLINE cugra_arc_t cugra_vertex_inarcs_end(cugra_vertex_t v)
{ return cugra_vertex_arcs_end(cugra_direction_in, v); }

/*!The next in-arc to the head of \a e. */
CU_SINLINE cugra_arc_t cugra_vertex_inarcs_next(cugra_arc_t e)
{ return cugra_vertex_arcs_next(cugra_direction_in, e); }

/*!The first of the sequence of all vertices of \a G. */
CU_SINLINE cugra_vertex_t cugra_graph_vertices_begin(cugra_graph_t G)
{
    return cu_ptr_add(G->vertices.next,
		      -offsetof(struct cugra_vertex_s, in_graph));
}

/*!An end marker of the sequence of all vertices in \a G. */
CU_SINLINE cugra_vertex_t cugra_graph_vertices_end(cugra_graph_t G)
{
    return cu_ptr_add(&G->vertices,
		      -offsetof(struct cugra_vertex_s, in_graph));
}

/*!The vertex after \a v in the sequence of all vertices of a graph. */
CU_SINLINE cugra_vertex_t cugra_graph_vertices_next(cugra_vertex_t v)
{
    return cu_ptr_add(v->in_graph.next,
		      -offsetof(struct cugra_vertex_s, in_graph));
}

/*!The first of the sequence of all arcs of \a G. */
cugra_arc_t cugra_graph_arcs_begin(cugra_graph_t G);

/*!The end marker of the sequence of all arcs of \a G. */
CU_SINLINE cugra_arc_t cugra_graph_arcs_end(cugra_graph_t G) { return NULL; }

/*!The arc after \a a in the sequence of all arcs of \a G. */
cugra_arc_t cugra_graph_arcs_next(cugra_graph_t G, cugra_arc_t a);


#define cugra_vertex_for_inarcs(a, v)					\
    for (a = cugra_vertex_inarcs_begin(v);				\
	 a != cugra_vertex_inarcs_end(v);				\
	 a = cugra_vertex_inarcs_next(a))

#define cugra_vertex_for_outarcs(a, v)					\
    for (a = cugra_vertex_outarcs_begin(v);				\
	 a != cugra_vertex_outarcs_end(v);				\
	 a = cugra_vertex_outarcs_next(a))

#define cugra_vertex_for_arcs(dir, a, v)				\
    for (a = cugra_vertex_arcs_begin(dir, v);				\
	 a != cugra_vertex_arcs_end(dir, v);				\
	 a = cugra_vertex_arcs_next(dir, a))

#define cugra_vertex_for_inoutarcs(dir, a, v)				\
    for (dir = cugra_direction_BEGIN; dir < cugra_direction_END; ++dir)	\
    for (a = cugra_vertex_arcs_begin(dir, v);				\
	 a != cugra_vertex_arcs_end(dir, v);				\
	 a = cugra_vertex_arcs_next(dir, a))

#define cugra_graph_for_vertices(v, G)					\
    for (v = cugra_graph_vertices_begin(G);				\
	 v != cugra_graph_vertices_end(G);				\
	 v = cugra_graph_vertices_next(v))

/*!@}*/

#if CU_COMPAT < 20080210
#  define cugra_vertex_inarcs_for	cugra_vertex_for_inarcs
#  define cugra_vertex_outarcs_for	cugra_vertex_for_outarcs
#  define cugra_vertex_arcs_for	 	cugra_vertex_for_arcs
#  define cugra_vertex_inoutarcs_for	cugra_vertex_for_inoutarcs
#  define cugra_graph_vertices_for	cugra_graph_for_vertices
#endif
CU_END_DECLARATIONS

#endif
