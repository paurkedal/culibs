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
#include <cu/memory.h>
#include <cucon/pset.h>

void
cugra_graph_cct(cugra_graph_t G, unsigned int options)
{
    G->options = options;
    cu_dlink_cct_singular(&G->vertices);
}

cugra_graph_t
cugra_graph_new(unsigned int options)
{
    cugra_graph_t G = cu_gnew(struct cugra_graph_s);
    cugra_graph_cct(G, options);
    return G;
}

void
cugra_graph_merge(cugra_graph_t G_dst, cugra_graph_t G_src)
{
    cu_dlink_insert_list_before(&G_dst->vertices, &G_src->vertices);
}

CU_SINLINE void
cct_vertex(cugra_graph_t G, cugra_vertex_t v)
{
    cu_dlink_insert_before(&G->vertices, &v->in_graph);
    cu_dlink_cct_singular(&v->adj_link[0]);
    cu_dlink_cct_singular(&v->adj_link[1]);
}

CU_SINLINE void
cct_arc(cugra_arc_t a, cugra_vertex_t v_tail, cugra_vertex_t v_head)
{
    cu_dlink_insert_before(&v_tail->adj_link[cugra_direction_out],
			   &a->adj[cugra_direction_out].link);
    a->adj[cugra_direction_out].vertex = v_head;
    cu_dlink_insert_before(&v_head->adj_link[cugra_direction_in],
			   &a->adj[cugra_direction_in].link);
    a->adj[cugra_direction_in].vertex = v_tail;
}

cugra_vertex_t
cugra_graph_vertex_new(cugra_graph_t G)
{
    cugra_vertex_t v = cu_gnew(struct cugra_vertex_s);
    cct_vertex(G, v);
    return v;
}

cugra_vertex_t
cugra_graph_vertex_new_mem(cugra_graph_t G, size_t size)
{
    cugra_vertex_t v = cu_galloc(sizeof(struct cugra_vertex_s) + size);
    cct_vertex(G, v);
    return v;
}

cugra_vertex_t
cugra_graph_vertex_new_ptr(cugra_graph_t G, void *ptr)
{
    cugra_vertex_t v;
    v = cu_galloc(sizeof(struct cugra_vertex_s) + sizeof(void *));
    *(void **)cugra_vertex_mem(v) = ptr;
    cct_vertex(G, v);
    return v;
}

cugra_arc_t
cugra_graph_arc_new(cugra_vertex_t v_tail, cugra_vertex_t v_head)
{
    cugra_arc_t a = cu_gnew(struct cugra_arc_s);
    cct_arc(a, v_tail, v_head);
    return a;
}

cugra_arc_t
cugra_graph_arc_new_mem(cugra_vertex_t v_tail, cugra_vertex_t v_head,
			size_t size)
{
    cugra_arc_t a = cu_galloc(sizeof(struct cugra_arc_s) + size);
    cct_arc(a, v_tail, v_head);
    return a;
}

cugra_arc_t
cugra_graph_arc_new_ptr(cugra_vertex_t v_tail, cugra_vertex_t v_head,
			void *ptr)
{
    cugra_arc_t a = cu_galloc(sizeof(struct cugra_arc_s) + sizeof(void *));
    *(void **)cugra_arc_mem(a) = ptr;
    cct_arc(a, v_tail, v_head);
    return a;
}

void
cugra_erase_arc(cugra_arc_t a)
{
    cu_dlink_erase(&a->adj[0].link);
    cu_dlink_erase(&a->adj[1].link);
}

void
cugra_erase_vertex(cugra_vertex_t v)
{
    cugra_direction_t dir;
    for (dir = 0; dir < 2; ++dir) {
	cugra_arc_t a;
	cu_dlink_erase(&v->in_graph);
	a = cugra_vertex_arcs_begin(dir, v);
	while (a != cugra_vertex_arcs_end(dir, v)) {
	    cugra_arc_t a_next = cugra_vertex_arcs_next(dir, a);
	    cugra_erase_arc(a);
	    a = a_next;
	}
    }
}
cu_clos_def(erase_vertex_clos, cu_prot(void, cugra_vertex_t v), ())
{
    cugra_erase_vertex(v);
}
cu_clop(cugra_erase_vertex_clop, void, cugra_vertex_t);

void
cugra_eliminate_vertex(cugra_vertex_t v)
{
    cugra_arc_t out, in;
    cugra_vertex_inarcs_for(in, v) {
	struct cucon_pset_s V;
	cugra_vertex_t v_tail = cugra_arc_tail(in);
	cucon_pset_cct(&V);
	cugra_vertex_outarcs_for(out, v_tail)
	    cucon_pset_insert(&V, cugra_arc_head(out));
	cugra_vertex_outarcs_for(out, v) {
	    cugra_vertex_t v_head = cugra_arc_head(out);
	    if (!cucon_pset_find(&V, v_head))
		cugra_graph_arc_new(v_tail, v_head);
	}
    }
    cugra_erase_vertex(v);
}

void
cugra_vertex_erase_loops(cugra_vertex_t v)
{
    cugra_arc_t a;
    a = cugra_vertex_outarcs_begin(v);
    while (a != cugra_vertex_outarcs_end(v)) {
	cugra_arc_t a_next = cugra_vertex_outarcs_next(a);
	if (cugra_arc_head(a) == v)
	    cugra_erase_arc(a);
	a = a_next;
    }
}

void
cugra_graph_erase_loops(cugra_graph_t G)
{
    cugra_vertex_t v;
    cugra_graph_vertices_for(v, G)
	cugra_vertex_erase_loops(v);
}

cu_bool_t
cugra_vertex_has_loop(cugra_vertex_t v)
{
    cugra_arc_t a;
    cugra_vertex_outarcs_for(a, v)
	if (cugra_arc_is_loop(a))
	    return cu_true;
    return cu_false;
}

cugra_arc_t
cugra_graph_arcs_begin(cugra_graph_t G)
{
    cugra_vertex_t v = cugra_graph_vertices_begin(G);
    cugra_arc_t a;
    for (;;) {
	if (v == cugra_graph_vertices_end(G))
	    return NULL;
	a = cugra_vertex_outarcs_begin(v);
	if (a != cugra_vertex_outarcs_end(v))
	    return a;
	v = cugra_graph_vertices_next(v);
    }
}

cugra_arc_t
cugra_graph_arcs_next(cugra_graph_t G, cugra_arc_t a)
{
    cugra_vertex_t v = cugra_arc_tail(a);
    a = cugra_vertex_outarcs_next(a);
    while (a == cugra_vertex_outarcs_end(v)) {
	v = cugra_graph_vertices_next(v);
	if (v == cugra_graph_vertices_end(G))
	    return NULL;
	a = cugra_vertex_outarcs_begin(v);
    }
    return a;
}

void
cugraP_graph_init()
{
    static erase_vertex_clos_t erase_vertex_clos;
    cugra_erase_vertex_clop = erase_vertex_clos_prep(&erase_vertex_clos);
}
