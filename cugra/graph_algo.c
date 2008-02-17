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

#include <cugra/graph.h>
#include <cugra/graph_algo.h>
#include <cucon/pmap.h>

void
cugra_graph_copy(cugra_graph_t G_src, cugra_graph_t G_dst,
		 cucon_pmap_t fv_src_to_dst, cucon_pmap_t fv_dst_to_src)
{
    cugra_vertex_t v_src;
    if (!fv_src_to_dst)
	fv_src_to_dst = cucon_pmap_new();
    cugra_graph_for_vertices(v_src, G_src) {
	cugra_vertex_t v_dst = cugra_graph_vertex_new(G_dst);
	cucon_pmap_insert_ptr(fv_src_to_dst, v_src, v_dst);
	if (fv_dst_to_src)
	    cucon_pmap_insert_ptr(fv_dst_to_src, v_dst, v_src);
    }
    cugra_graph_for_vertices(v_src, G_src) {
	cugra_arc_t a;
	cugra_vertex_t v_dst = cucon_pmap_find_ptr(fv_src_to_dst, v_src);
	cu_debug_assert(v_dst);
	cugra_vertex_for_outarcs(a, v_src) {
	    cugra_vertex_t v_src_head = cugra_arc_head(a);
	    cugra_vertex_t v_dst_head;
	    v_dst_head = cucon_pmap_find_ptr(fv_src_to_dst, v_src_head);
	    cu_debug_assert(v_dst_head);
	    cugra_connect(G_dst, v_dst, v_dst_head);
	}
    }
}

static cu_bool_t
detect_cycle(cugra_vertex_t v, cucon_pmap_t vinfo_map)
{
    int *vinfo;
    if (cucon_pmap_insert_mem(vinfo_map, v, sizeof(int), &vinfo)) {
	cugra_arc_t a;
	*vinfo = 1;
	cugra_vertex_for_outarcs(a, v) {
	    cugra_vertex_t u = cugra_arc_head(a);
	    if (detect_cycle(u, vinfo_map))
		return cu_true;
	}
	*vinfo = 0;
	return cu_false;
    }
    else
	return *vinfo;
}

cu_bool_t
cugra_graph_is_acyclic(cugra_graph_t G)
{
    cugra_vertex_t v;
    struct cucon_pmap_s vinfo_map;
    cucon_pmap_cct(&vinfo_map);
    cugra_graph_for_vertices(v, G)
	if (detect_cycle(v, &vinfo_map))
	    return cu_false;
    return cu_true;
}

void
cugra_erase_vertex_to_vsetcompl_arcs(cugra_graph_t G, cugra_vertex_t v,
				     cucon_pmap_t V)
{
    cugra_direction_t dir;
    cugra_arc_t a;
    for (dir = 0; dir < 2; ++dir) {
	a = cugra_vertex_arcs_begin(dir, v);
	while (a != cugra_vertex_arcs_end(dir, v)) {
	    cugra_vertex_t vp = cugra_arc_adjacent(dir, a);
	    if (!cucon_pmap_find_void(V, vp)) {
		cugra_arc_t a_next = cugra_vertex_arcs_next(dir, a);
		cugra_erase_arc(G, a);
		a = a_next;
	    }
	    else
		a = cugra_vertex_arcs_next(dir, a);
	}
    }
}

cu_clos_def(move_subgraph_cb,
	    cu_prot(cu_bool_t, void const *v),
    ( cugra_graph_t G_src, G_dst;
      cucon_pmap_t V_move; ))
{
#define v ((cugra_vertex_t)v)
    cu_clos_self(move_subgraph_cb);
    cugra_erase_vertex_to_vsetcompl_arcs(self->G_src, v, self->V_move);
    cu_dlink_erase(&v->in_graph);
    cu_dlink_insert_before(&self->G_dst->vertices, &v->in_graph);
    return cu_true;
#undef v
}

void
cugra_move_induced_subgraph(cucon_pmap_t V_move,
			    cugra_graph_t G_src, cugra_graph_t G_dst)
{
    move_subgraph_cb_t cb;
    cb.G_src = G_src;
    cb.G_dst = G_dst;
    cb.V_move = V_move;
    cucon_pmap_conj_keys(V_move, move_subgraph_cb_prep(&cb));
}

void
cugra_graph_erase_isolated(cugra_graph_t G)
{
    cugra_vertex_t v;
    cugra_graph_for_vertices(v, G)
	if (cugra_vertex_is_isolated(v))
	    cugra_erase_vertex(G, v);
}
