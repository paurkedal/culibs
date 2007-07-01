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

#ifndef CUCON_BIGRAPH_H
#define CUCON_BIGRAPH_H

#include <cucon/fwd.h>
#include <cucon/list.h>
#include <cucon/algo_colour.h>
#include <cu/thread.h>
#include <stdio.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cucon_bigraph cucon/bigraph.h: Bidirectional graphs
 * @{ \ingroup cucon_mod */

struct cucon_bigraph_vertex_s
{
    struct cucon_list_s input_edges;
    struct cucon_list_s output_edges;
};

struct cucon_bigraph_edge_s
{
    struct cucon_list_node_s as_input;
    struct cucon_list_node_s as_output;
    cucon_bigraph_vertex_t src;
    cucon_bigraph_vertex_t dst;
};


/* bigraph: constructor
 * ------------------ */

/* Options to 'cucon_bigraph_new'. */
typedef unsigned int cucon_bigraph_opt_t;

#define cucon_bigraph_opt_list_of_vertices	0x01
#define cucon_bigraph_opt_list_of_edges		0x02
#define cucon_bigraph_opt_lists \
	cucon_bigraph_opt_list_of_vertices | cucon_bigraph_opt_list_of_edges

/* The bigraph has no edges with both endpoints at the same vertex. */
#define cucon_bigraph_opt_no_oneedge_loops	0x10

/* The bigraph has no loop of two or more edges. */
#define cucon_bigraph_opt_no_multiedge_loops	0x20

/* The bigraph has no loops (a binary or of the two above). */
#define cucon_bigraph_opt_no_loops		0x30

#define cucon_bigraph_opt_undirected		0x40

struct cucon_bigraph_s
{
    cucon_bigraph_opt_t options;
    struct cucon_list_s vertices;	/* of struct cucon_bigraph_vertex_s */
    struct cucon_list_s edges;	/* of struct cucon_bigraph_edge_s */
    cu_mutex_t mutex;
};

/* Return an empty bigraph with the given 'options' and with
 * 'vertex_value_size' as the default size of vertex values and
 * 'edge_value_size' as the default size of edge values.  */
cucon_bigraph_t cucon_bigraph_new(cucon_bigraph_opt_t options);

CU_SINLINE cu_bool_t
cucon_bigraph_is_directed(cucon_bigraph_t G)
{
    return !(G->options & cucon_bigraph_opt_undirected);
}

/*FIXME*/
#define cucon_bigraph_edge_is_directed(e) ((e), cu_true)


/* bigraph: mutations
 * ---------------- */

/* Insert a new vertex with 'value_size' allocated byes for the vertex
 * value. */
cucon_bigraph_vertex_t cucon_bigraph_insert_vertex_mem(cucon_bigraph_t,
					       size_t value_size);

/* Insert a new vertex with a suitably sized value set to 'value_ptr'. */
cucon_bigraph_vertex_t cucon_bigraph_insert_vertex_ptr(cucon_bigraph_t, void *value_ptr);

/* Insert an edge from 'v0' to 'v1'.  Intrinsic and extrinsic versions. */
cucon_bigraph_edge_t
cucon_bigraph_insert_edge_mem(cucon_bigraph_t,
			  cucon_bigraph_vertex_t v0, cucon_bigraph_vertex_t v1,
			  size_t value_size);
cucon_bigraph_edge_t
cucon_bigraph_insert_edge_ptr(cucon_bigraph_t,
			  cucon_bigraph_vertex_t v0, cucon_bigraph_vertex_t v1,
			  cu_ptr_t value_ptr);


/* bigraph: The range of all edges
 * ----------------------------- */
typedef cucon_list_it_t cucon_bigraph_all_edges_it_t;
#define cucon_bigraph_all_edges_it_next cucon_list_it_next
#define cucon_bigraph_all_edges_it_prev cucon_list_it_prev
#define cucon_bigraph_all_edges_it_get cucon_list_it_get_mem
#define cucon_bigraph_all_edges_begin(g) cucon_list_begin(&(g)->edges)
#define cucon_bigraph_all_edges_end(g) cucon_list_end(&(g)->edges)

/* bigraph: The range of all vertices
 * -------------------------------- */
typedef cucon_list_it_t cucon_bigraph_all_vertices_it_t;
#define cucon_bigraph_all_vertices_it_next cucon_list_it_next
#define cucon_bigraph_all_vertices_it_prev cucon_list_it_prev
#define cucon_bigraph_all_vertices_it_get cucon_list_it_get_mem
#define cucon_bigraph_all_vertices_begin(g) cucon_list_begin(&(g)->vertices)
#define cucon_bigraph_all_vertices_end(g) cucon_list_end(&(g)->vertices)

/* vertex: value
 * ------------- */
#define cucon_bigraph_vertex_get_mem(v) \
	((void*)CU_ALIGNED_MARG_END(cucon_bigraph_vertex_t, (v)))
#define cucon_bigraph_vertex_from_mem(mem) \
	CU_ALIGNED_PTR_FROM_END(cucon_bigraph_t, mem)
#define cucon_bigraph_vertex_get_ptr(v) \
	(*(void**)CU_ALIGNED_MARG_END(cucon_bigraph_vertex_t, (v)))
#define cucon_bigraph_vertex_set_ptr(v, x) \
	(*(void**)CU_ALIGNED_MARG_END(cucon_bigraph_vertex_t, (v)) = (x))

/* vertex: The range of input and/or output edges of a vertex
 * ---------------------------------------------------------- */
#define cuconP_OUTEDGE(e)							\
	((cucon_bigraph_edge_t)						\
	 ((void *)e							\
	  - offsetof(struct cucon_bigraph_edge_s, as_output)))
#define cuconP_OUTEDGE_TO_IT(e)						\
	(&CU_MARG(cucon_bigraph_edge_t, e)->as_output)

#define cucon_bigraph_edge_first_from_vertex(v) \
	cuconP_OUTEDGE(			\
	    cucon_list_begin(&CU_MARG(cucon_bigraph_vertex_t, v)->output_edges))
#define cucon_bigraph_edge_last_from_vertex(v)	\
	cuconP_OUTEDGE(			\
	    cucon_list_it_prev(		\
		cucon_list_end(&CU_MARG(cucon_bigraph_vertex_t, v)->output_edges)))
#define cucon_bigraph_edge_stop_from_vertex(v) \
	cuconP_OUTEDGE( \
	    cucon_list_end(&CU_MARG(cucon_bigraph_vertex_t, v)->output_edges))
#define cucon_bigraph_edge_next_from_vertex(e) \
	cuconP_OUTEDGE(cucon_list_it_next(cuconP_OUTEDGE_TO_IT(e)))
#define cucon_bigraph_edge_prev_from_vertex(e) \
	cuconP_OUTEDGE(cucon_list_it_prev(cuconP_OUTEDGE_TO_IT(e)))


/* edge: value
 * ----------- */
#define cucon_bigraph_edge_get_mem(e) \
	((void*)CU_ALIGNED_MARG_END(cucon_bigraph_edge_t, e))
#define cucon_bigraph_edge_from_mem(mem) \
	CU_ALIGNED_PTR_FROM_END(cucon_bigraph_edge_t, mem)
#define cucon_bigraph_edge_get_ptr(e) \
	(*(void**)CU_ALIGNED_MARG_END(cucon_bigraph_edge_t, e))
#define cucon_bigraph_edge_set_ptr(e, x) \
	(*(void**)CU_ALIGNED_MARG_END(cucon_bigraph_edge_t, e) = (x))

/* edge: endpoints
 * --------------- */
#define cucon_bigraph_edge_src(e) ((e)->src)
#define cucon_bigraph_edge_dst(e) ((e)->dst)

/* bigraph_algo.c
 * ------------ */

/* Shortest path.
 *
 * Prototype for the arguments:
 *     double #[closure edge_distance](cucon_bigraph_edge_t);
 *     cu_bool_fast_t #[closure vertex_test](cucon_bigraph_vertex_t);
 *
 * Returs a 'cucon_list_t' of 'cucon_bigraph_edge_t'.
 *
 * XX This currently treats bidirectional graphs as directional.
 */
cucon_list_t
cucon_bigraph_shortest_path_if(
    cucon_bigraph_t g,
    cucon_bigraph_vertex_t v_start,
    cu_bool_t cu_clptr(vertex_test)(cucon_bigraph_vertex_t),
    double cu_clptr(edge_distance)(cucon_bigraph_edge_t));

cucon_list_t
cucon_bigraph_shortest_path(
    cucon_bigraph_t g,
    cucon_bigraph_vertex_t v_start,
    cucon_bigraph_vertex_t v_final,
    double cu_clptr(edge_distance)(cucon_bigraph_edge_t));

/* Representation of bigraph in bigraphviz format.
 *     char const *#[closure vertex_label](cucon_bigraph_vertex_t);
 *     char const *#[closure edge_label](cucon_bigraph_edge_t);
 */
void cucon_bigraph_write_graphviz(
    cucon_bigraph_t g,
    char const *cu_clptr(vertex_label)(cucon_bigraph_vertex_t),
    char const *cu_clptr(edge_label)(cucon_bigraph_edge_t),
    FILE* out);
void
cucon_bigraph_write_graphviz_props(
    cucon_bigraph_t g,
    void cu_clptr(vertex_print_props)(cucon_bigraph_vertex_t, FILE *),
    void cu_clptr(edge_print_props)(cucon_bigraph_edge_t, FILE *),
    FILE *out);

/*!@}*/
CU_END_DECLARATIONS

#endif
