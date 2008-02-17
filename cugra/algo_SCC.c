/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2008  Petter Urkedal <urkedal@nbi.dk>
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

#include <cugra/algo_SCC.h>
#include <cugra/graph.h>
#include <cucon/pmap.h>
#include <cucon/stack.h>
#include <cucon/list.h>
#include <cu/int.h>
#include <cu/inherit.h>
#include <limits.h>


/* cugra_detect_SCC
 * ---------------- */

typedef struct SCC_vinfo_s *SCC_vinfo_t;
struct SCC_vinfo_s {
    cu_inherit (cucon_pmap_node_s);
    int index; /* Set to INT_MAX when popped from stack */
    SCC_vinfo_t next_vertex;
    void *cpt;
};

static int
detect_SCC(cugra_detect_SCC_t cb,
	   cucon_pmap_t vinfo_map, cugra_vertex_t tail, int *index_pool,
	   SCC_vinfo_t *vertex_stack, cucon_stack_t cpt_stack)
{
    SCC_vinfo_t tail_info;
    if (cucon_pmap_insert_new_node(vinfo_map, tail,
				   sizeof(struct SCC_vinfo_s), &tail_info)) {
	cucon_stack_mark_t cpt_stack_mark = cucon_stack_mark(cpt_stack);
	cugra_arc_t arc;
	int tail_min_reach, head_min_reach;

	/* Create metadata for the tail vertex and process all adjacent head
	 * vertices. */
	tail_info->index = tail_min_reach = (*index_pool)++;
	tail_info->next_vertex = *vertex_stack;
	*vertex_stack = tail_info;

	cugra_vertex_for_outarcs(arc, tail) {
	    cugra_vertex_t head = cugra_arc_head(arc);
	    head_min_reach = detect_SCC(cb, vinfo_map, head, index_pool,
					vertex_stack, cpt_stack);
	    tail_min_reach = cu_int_min(tail_min_reach, head_min_reach);
	}

	/* If this is the root of a strong component, process the component. */
	if (tail_info->index == tail_min_reach) {
	    /* Pop off vertices up to the root of the component. */
	    void *cpt = (*cb->cpt_new)(cb);
	    cugra_vertex_t v;
	    do {
		SCC_vinfo_t vinfo = *vertex_stack;
		*vertex_stack = vinfo->next_vertex;
		v = cucon_pmap_node_key(cu_to(cucon_pmap_node, vinfo));
		(*cb->cpt_insert)(cb, cpt, v);
		vinfo->cpt = cpt;
	    } while (v != tail);

	    /* Connect component to all subcomponents, and push it. */
	    while (cucon_stack_mark(cpt_stack) != cpt_stack_mark) {
		SCC_vinfo_t sub_cpt = cucon_stack_pop_ptr(cpt_stack);
		(*cb->cpt_connect)(cb, cpt, sub_cpt);
	    }
	    cucon_stack_push_ptr(cpt_stack, cpt);
	}
	tail_info->index = INT_MAX;
	return tail_min_reach;
    } else {
	if (tail_info->cpt)
	    cucon_stack_push_ptr(cpt_stack, tail_info->cpt);
	return tail_info->index;
    }
}

void
cugra_detect_SCC(cugra_graph_t G, cugra_detect_SCC_t cb)
{
    cugra_vertex_t v;
    struct cucon_pmap_s vinfo_map;
    int index_pool = 0;
    SCC_vinfo_t vertex_stack = NULL;
    struct cucon_stack_s cpt_stack;

    cucon_stack_cct(&cpt_stack);
    cucon_pmap_cct(&vinfo_map);
    cugra_graph_for_vertices(v, G)
	detect_SCC(cb, &vinfo_map, v, &index_pool, &vertex_stack, &cpt_stack);
}


/* cugra_SCC_graph_of_lists
 * ------------------------ */

typedef struct graph_of_lists_state_s *graph_of_lists_state_t;
struct graph_of_lists_state_s
{
    cu_inherit (cugra_detect_SCC_s);
    size_t vslot_size;
    cu_clop(vslot_copy, void, void *src, void *dst);
    cugra_graph_t A;
};

static void *
graph_of_lists_cpt_new(cugra_detect_SCC_t base)
{
    cugra_vertex_t v;
    graph_of_lists_state_t self;
    self = cu_from(graph_of_lists_state, cugra_detect_SCC, base);
    v = cugra_graph_vertex_new_mem(self->A, sizeof(struct cucon_list_s));
    cucon_list_cct(cugra_vertex_mem(v));
    return v;
}

static void
graph_of_lists_cpt_insert(cugra_detect_SCC_t base, void *v_new,
			  cugra_vertex_t v)
{
    graph_of_lists_state_t self;
    cucon_listnode_t node;
    self = cu_from(graph_of_lists_state, cugra_detect_SCC, base);
    node = cucon_list_append_mem(cugra_vertex_mem(v_new), self->vslot_size);
    cu_call(self->vslot_copy, cugra_vertex_mem(v), cucon_listnode_mem(node));
}

static void
graph_of_lists_cpt_connect(cugra_detect_SCC_t base, void *tail, void *head)
{
    graph_of_lists_state_t self;
    self = cu_from(graph_of_lists_state, cugra_detect_SCC, base);
    cugra_connect(self->A, tail, head);
}

cugra_graph_t
cugra_SCC_graph_of_lists(cugra_graph_t G, size_t vslot_size,
			 cu_clop(vslot_copy, void, void *src, void *dst))
{
    struct graph_of_lists_state_s state;
    cu_to(cugra_detect_SCC, &state)->cpt_new = graph_of_lists_cpt_new;
    cu_to(cugra_detect_SCC, &state)->cpt_insert = graph_of_lists_cpt_insert;
    cu_to(cugra_detect_SCC, &state)->cpt_connect = graph_of_lists_cpt_connect;
    state.vslot_size = vslot_size;
    state.vslot_copy = vslot_copy;
    state.A = cugra_graph_new(0);
    cugra_detect_SCC(G, cu_to(cugra_detect_SCC, &state));
    return state.A;
}
