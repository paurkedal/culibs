/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2002--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUCON_USE_DEPRECATED_DIGRAPH_H
#  error Don't use this header, it's replaced by cugra/graph.h
#endif

#ifndef CUCON_DIGRAPH_H
#define CUCON_DIGRAPH_H

#include <cucon/fwd.h>
#include <cucon/list.h>
#include <cucon/algo_colour.h>
#include <cu/thread.h>
#include <cu/ptr.h>
#include <stdio.h>

#ifdef CU_NDEBUG
#  define CUCON_DIGRAPH_NDEBUG
#endif
#ifdef CU_NDEBUG_CLIENT
#  define CUCON_DIGRAPH_NDEBUG_CLIENT
#endif


CU_BEGIN_DECLARATIONS
/*!\defgroup cucon_digraph cucon/digraph.h: Directed and Undirected Graphs (deprecated)
 * @{\ingroup cucon_deprecated_mod
 * \deprecated Replaced by the \ref cugra_mod "cugra" library.
 */

struct cucon_digraph_vertex_s
{
    struct cucon_list_s output_edges;
};

struct cucon_digraph_edge_s
{
    struct cucon_list_node_s as_output;
    cucon_digraph_vertex_t src;
    cucon_digraph_vertex_t dst;
};


/* digraph: constructor
 * ------------------ */

/* Options to 'cucon_digraph_new'. */
typedef unsigned int cucon_digraph_opt_t;

#define cucon_digraph_opt_list_of_vertices	0x01
#define cucon_digraph_opt_list_of_edges		0x02
#define cucon_digraph_opt_lists \
	cucon_digraph_opt_list_of_vertices | cucon_digraph_opt_list_of_edges

/* The digraph has no edges with both endpoints at the same vertex. */
#define cucon_digraph_opt_no_oneedge_loops	0x10

/* The digraph has no loop of two or more edges. */
#define cucon_digraph_opt_no_multiedge_loops	0x20

/* The digraph has no loops (a binary or of the two above). */
#define cucon_digraph_opt_no_loops		0x30

struct cucon_digraph_s
{
    cucon_digraph_opt_t options;
    struct cucon_list_s vertices;	/* of struct cucon_digraph_vertex_s */
    struct cucon_list_s edges;	/* of struct cucon_digraph_edge_s */
#ifdef CUCONF_ENABLE_THREADS
    cu_mutex_t mutex;
#endif
};

/* Return an empty digraph with the given 'options' and with
 * 'vertex_value_size' as the default size of vertex values and
 * 'edge_value_size' as the default size of edge values.  */
cucon_digraph_t cucon_digraph_new(cucon_digraph_opt_t options);


/* digraph: mutations
 * ---------------- */

/*!Insert a new vertex with \a value_size allocated byes for the vertex
 * value. */
cucon_digraph_vertex_t
cucon_digraph_insert_vertex_mem(cucon_digraph_t, size_t size);

/*!Insert a new vertex with a pointer-sized value set to \a value_ptr. */
cucon_digraph_vertex_t
cucon_digraph_insert_vertex_ptr(cucon_digraph_t, void *ptr);

/*!Construct and insert \a node into \a G.
 * \pre \a G must not have the cucon_digraph_opt_list_of_vertices option. */
void cucon_digraph_insert_vertex_node_cct(cucon_digraph_t G,
					cucon_digraph_vertex_t node);

/*!Insert an edge from \a v0 to \a v1 into \a G, with a \a value_size bytes
 * value slot.  Use \a value_size = 0 for a plain edge. */
cucon_digraph_edge_t
cucon_digraph_insert_edge_mem(cucon_digraph_t G,
			      cucon_digraph_vertex_t v0,
			      cucon_digraph_vertex_t v1, size_t value_size);

/*!Insert an edge from \a v0 to \a v1 into \a G, with a value \a value_ptr. */
cucon_digraph_edge_t
cucon_digraph_insert_edge_ptr(cucon_digraph_t G,
			      cucon_digraph_vertex_t v0,
			      cucon_digraph_vertex_t v1, cu_ptr_t value_ptr);


/* digraph: The range of all edges
 * ------------------------------- */
typedef cucon_listnode_t cucon_digraph_all_edges_it_t;
#define cucon_digraph_all_edges_it_next cucon_listnode_next
#define cucon_digraph_all_edges_it_prev cucon_listnode_prev
#define cucon_digraph_all_edges_it_get cucon_listnode_mem
#define cucon_digraph_all_edges_begin(g) cucon_list_begin(&(g)->edges)
#define cucon_digraph_all_edges_end(g) cucon_list_end(&(g)->edges)

/* digraph: The range of all vertices
 * ---------------------------------- */
typedef cucon_listnode_t cucon_digraph_all_vertices_it_t;
#define cucon_digraph_all_vertices_it_next cucon_listnode_next
#define cucon_digraph_all_vertices_it_prev cucon_listnode_prev
#define cucon_digraph_all_vertices_it_get cucon_listnode_mem
#define cucon_digraph_all_vertices_begin(g) cucon_list_begin(&(g)->vertices)
#define cucon_digraph_all_vertices_end(g) cucon_list_end(&(g)->vertices)

/* vertex: value
 * ------------- */
#define cucon_digraph_vertex_get_mem(v) \
	((void*)CU_ALIGNED_MARG_END(cucon_digraph_vertex_t, (v)))
#define cucon_digraph_vertex_from_mem(mem) \
	CU_ALIGNED_PTR_FROM_END(cucon_digraph_t, mem)
#define cucon_digraph_vertex_get_ptr(v) \
	(*(void**)CU_ALIGNED_MARG_END(cucon_digraph_vertex_t, (v)))
#define cucon_digraph_vertex_set_ptr(v, x) \
	(*(void**)CU_ALIGNED_MARG_END(cucon_digraph_vertex_t, (v)) = (x))

/* vertex: The range of input and/or output edges of a vertex
 * ---------------------------------------------------------- */
#define cuconP_OUTEDGE(e)						\
	((cucon_digraph_edge_t)						\
	 (cu_ptr_sub((void *)e,						\
		     offsetof(struct cucon_digraph_edge_s, as_output))))
#define cuconP_OUTEDGE_TO_IT(e)						\
	(&CU_MARG(cucon_digraph_edge_t, e)->as_output)

#define cucon_digraph_edge_first_from_vertex(v) \
	cuconP_OUTEDGE(			\
	    cucon_list_begin(&CU_MARG(cucon_digraph_vertex_t, v)->output_edges))
#define cucon_digraph_edge_last_from_vertex(v)	\
	cuconP_OUTEDGE(			\
	    cucon_listnode_prev(		\
		cucon_list_end(&CU_MARG(cucon_digraph_vertex_t, v)->output_edges)))
#define cucon_digraph_edge_stop_from_vertex(v) \
	cuconP_OUTEDGE( \
	    cucon_list_end(&CU_MARG(cucon_digraph_vertex_t, v)->output_edges))
#define cucon_digraph_edge_next_from_vertex(e) \
	cuconP_OUTEDGE(cucon_listnode_next(cuconP_OUTEDGE_TO_IT(e)))
#define cucon_digraph_edge_prev_from_vertex(e) \
	cuconP_OUTEDGE(cucon_listnode_prev(cuconP_OUTEDGE_TO_IT(e)))


/* edge: value
 * ----------- */
#define cucon_digraph_edge_get_mem(e) \
	((void*)CU_ALIGNED_MARG_END(cucon_digraph_edge_t, e))
#define cucon_digraph_edge_from_mem(mem) \
	ALIGNED_PTR_FROM_END(cucon_digraph_edge_t, mem)
#define cucon_digraph_edge_get_ptr(e) \
	(*(void**)CU_ALIGNED_MARG_END(cucon_digraph_edge_t, e))
#define cucon_digraph_edge_set_ptr(e, x) \
	(*(void**)CU_ALIGNED_MARG_END(cucon_digraph_edge_t, e) = (x))

/* edge: endpoints
 * --------------- */
#define cucon_digraph_edge_src(e) ((e)->src)
#define cucon_digraph_edge_dst(e) ((e)->dst)
#define cucon_digraph_edge_is_directed(e) cu_false


/* digraph_algo.c
 * ============== */

/* Shortest path.
 *
 * Prototype for the arguments:
 *     double #[closure edge_distance](cucon_digraph_edge_t);
 *     cu_bool_fast_t #[closure vertex_test](cucon_digraph_vertex_t);
 *
 * Returs a 'cucon_list_t' of 'cucon_digraph_edge_t'.
 */
cucon_list_t
cucon_digraph_shortest_path_if(
    cucon_digraph_t g,
    cucon_digraph_vertex_t v_start,
    cu_clop(vertex_test, cu_bool_t, cucon_digraph_vertex_t),
    cu_clop(edge_distance, double, cucon_digraph_edge_t));

cucon_list_t
cucon_digraph_shortest_path(
    cucon_digraph_t g,
    cucon_digraph_vertex_t v_start,
    cucon_digraph_vertex_t v_final,
    cu_clop(edge_distance, double, cucon_digraph_edge_t));

/* Representation of digraph in digraphviz format.
 *     char const *#[closure vertex_label](cucon_digraph_vertex_t);
 *     char const *#[closure edge_label](cucon_digraph_edge_t);
 */
void
cucon_digraph_write_graphviz(
    cucon_digraph_t g,
    cu_clop(vertex_label, char const *, cucon_digraph_vertex_t),
    cu_clop(edge_label, char const *, cucon_digraph_edge_t),
    FILE* out);
void
cucon_digraph_write_graphviz_props(
    cucon_digraph_t g,
    cu_clop(vertex_print_props, void, cucon_digraph_vertex_t, FILE *),
    cu_clop(edge_print_props, void, cucon_digraph_edge_t, FILE *),
    FILE *out);


/*!@}*/
CU_END_DECLARATIONS

#endif
