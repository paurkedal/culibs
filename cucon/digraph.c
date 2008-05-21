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

#define CUCON_USE_DEPRECATED_DIGRAPH_H
#include <cucon/digraph.h>
#include <cu/memory.h>

cucon_digraph_t
cucon_digraph_new(cucon_digraph_opt_t options)
{
    cucon_digraph_t g = cu_gnew(struct cucon_digraph_s);
    g->options = options;
    cucon_list_init(&g->vertices);
    cucon_list_init(&g->edges);
#ifdef CUCONF_ENABLE_THREADS
    cu_mutex_init(&g->mutex);
#endif
    return g;
}

cucon_digraph_vertex_t
cucon_digraph_insert_vertex_mem(cucon_digraph_t g, size_t value_size)
{
    cucon_digraph_vertex_t v;
    if (g->options & cucon_digraph_opt_list_of_vertices) {
	cucon_listnode_t it
	    = cucon_list_append_mem(
		&g->vertices,
		CU_ALIGNED_SIZEOF(struct cucon_digraph_vertex_s) + value_size);
	v = cucon_listnode_mem(it);
    }
    else
	v = cu_galloc(CU_ALIGNED_SIZEOF(struct cucon_digraph_vertex_s)
		       + value_size);
    cucon_list_init(&v->output_edges);
    return v;
}

cucon_digraph_vertex_t
cucon_digraph_insert_vertex_ptr(cucon_digraph_t g, void *value_ptr)
{
    cucon_digraph_vertex_t v = cucon_digraph_insert_vertex_mem(g, sizeof(void *));
    cucon_digraph_vertex_set_ptr(v, value_ptr);
    return v;
}

void
cucon_digraph_insert_vertex_node_cct(cucon_digraph_t g, cucon_digraph_vertex_t v)
{
#ifndef CUCON_DIGRAPH_NDEBUG_CLIENT
    if (g->options & cucon_digraph_opt_list_of_vertices) {
	fprintf(stderr,
		"cucon_digraph_insert_vertex_node_cct: Not valid call on "
		"graph with option cucon_digraph_opt_list_of_vertices.");
	abort();
    }
#endif
    cucon_list_init(&v->output_edges);
}

cucon_digraph_edge_t
cucon_digraph_insert_edge_mem(cucon_digraph_t g,
			    cucon_digraph_vertex_t src,
			    cucon_digraph_vertex_t dst,
			    size_t value_size)
{
    cucon_digraph_edge_t e;
    if (g->options & cucon_digraph_opt_list_of_edges) {
	cucon_listnode_t it = cucon_list_append_mem(
	    &g->edges,
	    CU_ALIGNED_SIZEOF(struct cucon_digraph_edge_s)
	    + value_size);
	e = cucon_listnode_mem(it);
    }
    else
	e = cu_galloc(CU_ALIGNED_SIZEOF(struct cucon_digraph_edge_s)
		       + value_size);
    e->src = src;
    e->dst = dst;
    cucon_list_append_init_node(&src->output_edges, &e->as_output);
    return e;
}

cucon_digraph_edge_t
cucon_digraph_insert_edge_ptr(cucon_digraph_t g,
			    cucon_digraph_vertex_t src,
			    cucon_digraph_vertex_t dst,
			    cu_ptr_t ptr)
{
    cucon_digraph_edge_t e
	= cucon_digraph_insert_edge_mem(g, src, dst, sizeof(cu_ptr_t));
    cucon_digraph_edge_set_ptr(e, ptr);
    return e;
}

#define CUCON_T_NS(name) cucon_digraph_##name
#define CUCON_T_ACCESS_OUTPUT_EDGES 1
#define CUCON_T_ACCESS_INPUT_EDGES 1
#define CUCON_T_ACCESS_ALL_EDGES 1
#define CUCON_T_ACCESS_ALL_VERTICES 1
#  include "graph_algo.c"
