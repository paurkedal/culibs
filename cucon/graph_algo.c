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

#include <cucon/priq.h>
#include <cucon/pmap.h>
#include <cu/memory.h>

#ifndef CUCON_T_NS
#  error This is a template file which requires certain CPP definitions
#  error as parameters.
#endif


/* Shortest Path
 * ------------- */

typedef struct dij_vertex_s *dij_vertex_t;

struct dij_vertex_s
{
    cucon_algo_colour_t color;
    double dist;
    CUCON_T_NS(vertex_t) vertex;
    CUCON_T_NS(edge_t) edge;
    dij_vertex_t prev;
};

cu_clop_def(dij_vertex_prior, cu_bool_t, void *v0, void *v1)
{
    return ((dij_vertex_t)v0)->dist < ((dij_vertex_t)v1)->dist;
}


cucon_list_t
CUCON_T_NS(shortest_path_if)(
    CUCON_T_NS(t) g, CUCON_T_NS(vertex_t) v_s,
    cu_clop(vertex_test, cu_bool_t, CUCON_T_NS(vertex_t)),
    cu_clop(edge_distance, double, CUCON_T_NS(edge_t)))
{
    cucon_priq_t q;
    cucon_pmap_t vprop = cucon_pmap_new();
    dij_vertex_t dij_v = cu_gnew(struct dij_vertex_s);
    q = cucon_priq_new(dij_vertex_prior);
    dij_v->color = cucon_algo_colour_gray;
    dij_v->dist = 0.0;
    dij_v->vertex = v_s;
    dij_v->edge = NULL;
    dij_v->prev = NULL;
    cucon_priq_insert(q, dij_v);
    cucon_pmap_replace_ptr(vprop, v_s, dij_v);
    while (!cucon_priq_is_empty(q)) {
	CUCON_T_NS(vertex_t) v;
	CUCON_T_NS(edge_t) e;
	dij_v = cucon_priq_pop_front(q);
	v = dij_v->vertex;
	if (cu_call(vertex_test, v)) {
	    cucon_list_t lst = cucon_list_new();
	    while (dij_v->edge != NULL) {
		cucon_list_prepend_ptr(lst, dij_v->edge);
		dij_v = dij_v->prev;
	    }
	    return lst;
	}

	/* Relax v */
	for (e = CUCON_T_NS(edge_first_from_vertex)(v);
	     e != CUCON_T_NS(edge_stop_from_vertex)(v);
	     e = CUCON_T_NS(edge_next_from_vertex)(e)) {
	    dij_vertex_t *dij_u;
	    CUCON_T_NS(vertex_t) u = CUCON_T_NS(edge_dst)(e);
	    double edge_dist = cu_call(edge_distance, e);
	    double pot_new_dist = dij_v->dist + edge_dist;
	    if (u == v) {
		u = CUCON_T_NS(edge_src)(e);
		if (u == v)
		    continue;
	    }
	    if (cucon_pmap_insert_mem(vprop, u, sizeof(void*),
				    (void *)&dij_u)) {
		(*dij_u) = cu_gnew(struct dij_vertex_s);
		(*dij_u)->color = cucon_algo_colour_gray;
		(*dij_u)->vertex = u;
		(*dij_u)->edge = e;
		(*dij_u)->prev = dij_v;
		(*dij_u)->dist = pot_new_dist;
		cucon_priq_insert(q, *dij_u);
	    }
	    else if ((*dij_u)->color == cucon_algo_colour_gray &&
		     (*dij_u)->dist < pot_new_dist) {
		(*dij_u)->edge = e;
		(*dij_u)->dist = pot_new_dist;
		(*dij_u)->prev = dij_v;
		cucon_priq_insert(q, *dij_u);
	    }
	}
	dij_v->color = cucon_algo_colour_black;
    }
    return NULL;
}


cu_clos_def(vertex_eq,
	    cu_prot(cu_bool_fast_t, CUCON_T_NS(vertex_t) v),
	    (CUCON_T_NS(vertex_t) v_to_find;))
{
    cu_clos_self(vertex_eq);
    return self->v_to_find == v;
}

cucon_list_t
CUCON_T_NS(shortest_path)(CUCON_T_NS(t) g,
			CUCON_T_NS(vertex_t) v_start,
			CUCON_T_NS(vertex_t) v_final,
			cu_clop(edge_distance, double, CUCON_T_NS(edge_t)))
{
    vertex_eq_t tst;
    tst.v_to_find = v_final;
    return CUCON_T_NS(shortest_path_if)(g, v_start, vertex_eq_prep(&tst),
				      edge_distance);
}


/* Graphviz Output
 * --------------- */

void
CUCON_T_NS(write_graphviz)(
    CUCON_T_NS(t) g,
    cu_clop(vertex_label, char const *, CUCON_T_NS(vertex_t)),
    cu_clop(edge_label, char const *, CUCON_T_NS(edge_t)),
    FILE *out)
{
    CUCON_T_NS(all_vertices_it_t) it_v;
    CUCON_T_NS(all_edges_it_t) it_e;
    if (!cu_clop_is_null(vertex_label))
	for (it_v = CUCON_T_NS(all_vertices_begin)(g);
	     it_v != CUCON_T_NS(all_vertices_end)(g);
	     it_v = CUCON_T_NS(all_vertices_it_next)(it_v)) {
	    CUCON_T_NS(vertex_t) v = CUCON_T_NS(all_vertices_it_get)(it_v);
	    fprintf(out, "v%p[label=%s];\n", v,
		    cu_call(vertex_label, v));
	}
    for (it_e = CUCON_T_NS(all_edges_begin)(g);
	 it_e != CUCON_T_NS(all_edges_end)(g);
	 it_e = CUCON_T_NS(all_edges_it_next)(it_e)) {
	CUCON_T_NS(edge_t) e = CUCON_T_NS(all_edges_it_get)(it_e);
	CUCON_T_NS(vertex_t) v0 = CUCON_T_NS(edge_src)(e);
	CUCON_T_NS(vertex_t) v1 = CUCON_T_NS(edge_dst)(e);
	if (cu_clop_is_null(edge_label)) {
	    if (CUCON_T_NS(edge_is_directed)(e))
		fprintf(out, "v%p -> v%p;\n", v0, v1);
	    else
		fprintf(out, "v%p -> v%p[dir=none];\n", v0, v1);
	}
	else {
	    if (CUCON_T_NS(edge_is_directed)(e))
		fprintf(out, "v%p -> v%p[label=%s];\n", v0, v1,
			cu_call(edge_label, e));
	    else
		fprintf(out, "v%p -> v%p[dir=none,label=%s];\n", v0, v1,
			cu_call(edge_label, e));
	}
    }
}

void
CUCON_T_NS(write_graphviz_props)(
    CUCON_T_NS(t) g,
    cu_clop(vertex_print_props, void, CUCON_T_NS(vertex_t), FILE *),
    cu_clop(edge_print_props, void, CUCON_T_NS(edge_t), FILE *),
    FILE *out)
{
    CUCON_T_NS(all_vertices_it_t) it_v;
    CUCON_T_NS(all_edges_it_t) it_e;
    for (it_v = CUCON_T_NS(all_vertices_begin)(g);
	 it_v != CUCON_T_NS(all_vertices_end)(g);
	 it_v = CUCON_T_NS(all_vertices_it_next)(it_v)) {
	CUCON_T_NS(vertex_t) v = CUCON_T_NS(all_vertices_it_get)(it_v);
	fprintf(out, "v%p[", v);
	cu_call(vertex_print_props, v, out);
	fputs("];\n", out);
    }
    for (it_e = CUCON_T_NS(all_edges_begin)(g);
	 it_e != CUCON_T_NS(all_edges_end)(g);
	 it_e = CUCON_T_NS(all_edges_it_next)(it_e)) {
	CUCON_T_NS(edge_t) e = CUCON_T_NS(all_edges_it_get)(it_e);
	CUCON_T_NS(vertex_t) v0 = CUCON_T_NS(edge_src)(e);
	CUCON_T_NS(vertex_t) v1 = CUCON_T_NS(edge_dst)(e);
	fprintf(out, "v%p -> v%p[", v0, v1);
	cu_call(edge_print_props, e, out);
	fputs("];\n", out);
    }
}
