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

#include <cucon/bigraph.h>
#include <cu/memory.h>
#include <cu/thread.h>

cucon_bigraph_t
cucon_bigraph_new(cucon_bigraph_opt_t options)
{
    cucon_bigraph_t g = cu_gnew(struct cucon_bigraph_s);
    g->options = options;
    cucon_list_cct(&g->vertices);
    cucon_list_cct(&g->edges);
    cu_mutex_cct(&g->mutex);
    return g;
}

cucon_bigraph_vertex_t
cucon_bigraph_insert_vertex_mem(cucon_bigraph_t g, size_t value_size)
{
    cucon_bigraph_vertex_t v;
    if (g->options & cucon_bigraph_opt_list_of_vertices) {
	cucon_list_it_t it
	    = cucon_list_append_mem(
		&g->vertices,
		CU_ALIGNED_SIZEOF(struct cucon_bigraph_vertex_s) + value_size);
	v = cucon_list_it_get_mem(it);
    }
    else
	v = cu_galloc(CU_ALIGNED_SIZEOF(struct cucon_bigraph_vertex_s)
		       + value_size);
    cucon_list_cct(&v->input_edges);
    cucon_list_cct(&v->output_edges);
    return v;
}

cucon_bigraph_vertex_t
cucon_bigraph_insert_vertex_ptr(cucon_bigraph_t g, void *value_ptr)
{
    cucon_bigraph_vertex_t v = cucon_bigraph_insert_vertex_mem(g, sizeof(void *));
    cucon_bigraph_vertex_set_ptr(v, value_ptr);
    return v;
}

cucon_bigraph_edge_t
cucon_bigraph_insert_edge_mem(cucon_bigraph_t g,
			    cucon_bigraph_vertex_t src,
			    cucon_bigraph_vertex_t dst,
			    size_t value_size)
{
    cucon_bigraph_edge_t e;
    if (g->options & cucon_bigraph_opt_list_of_edges) {
	cucon_list_it_t it = cucon_list_append_mem(
	    &g->edges,
	    CU_ALIGNED_SIZEOF(struct cucon_bigraph_edge_s) + value_size);
	e = cucon_list_it_get_mem(it);
    }
    else
	e = cu_galloc(CU_ALIGNED_SIZEOF(struct cucon_bigraph_edge_s)
		       + value_size);
    e->src = src;
    e->dst = dst;
    cucon_list_append_node_cct(&src->output_edges, &e->as_output);
    cucon_list_append_node_cct(&dst->input_edges, &e->as_input);
    return e;
}

cucon_bigraph_edge_t
cucon_bigraph_insert_edge_ptr(cucon_bigraph_t g,
			    cucon_bigraph_vertex_t src,
			    cucon_bigraph_vertex_t dst,
			    cu_ptr_t ptr)
{
    cucon_bigraph_edge_t e
	= cucon_bigraph_insert_edge_mem(g, src, dst, sizeof(cu_ptr_t));
    cucon_bigraph_edge_set_ptr(e, ptr);
    return e;
}

#define CUCON_T_NS(name) cucon_bigraph_##name
#define CUCON_T_ACCESS_OUTPUT_EDGES 1
#define CUCON_T_ACCESS_INPUT_EDGES 1
#define CUCON_T_ACCESS_ALL_EDGES 1
#define CUCON_T_ACCESS_ALL_VERTICES 1
#  include "graph_algo.c"
