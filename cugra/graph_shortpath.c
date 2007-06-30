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
#include <cucon/algo_colour.h>
#include <cucon/priq.h>
#include <cucon/pmap.h>
#include <cucon/list.h>
#include <math.h>

typedef struct dij_vertex_s *dij_vertex_t;
struct dij_vertex_s
{
    cucon_algo_colour_t colour;
    double distance;
    cugra_vertex_t vertex;
    cugra_arc_t arc;
    dij_vertex_t prev;
};

cu_clop_def(dij_vertex_prior, cu_bool_t, dij_vertex_t v0, dij_vertex_t v1)
{
    return v0->distance < v1->distance;
}

double
cugra_shortest_path(cugra_direction_t dir, cugra_vertex_t v_start,
		    cu_clop(vertex_test, cu_bool_t, cugra_vertex_t),
		    cu_clop(arc_distance, double, cugra_arc_t),
		    cu_clop(path_unwind, void, cugra_arc_t))
{
    struct cucon_priq_s q;
    struct cucon_pmap_s vprop;
    dij_vertex_t dij_v;
    cucon_pmap_cct(&vprop);
    cucon_pmap_insert_mem(&vprop, v_start, sizeof(struct dij_vertex_s), &dij_v);
    dij_v->colour = cucon_algo_colour_grey;
    dij_v->distance = 0.0;
    dij_v->vertex = v_start;
    dij_v->arc = NULL;
    dij_v->prev = NULL;
    cucon_priq_cct(&q,
		   (cu_clop(, cu_bool_t, void *, void *))dij_vertex_prior);
    cucon_priq_insert(&q, dij_v);
    while (!cucon_priq_is_empty(&q)) {
	cugra_vertex_t v;
	cugra_arc_t a;
	dij_v = cucon_priq_pop_front(&q);
	v = dij_v->vertex;
	if (cu_call(vertex_test, v)) {
	    double distance = 0.0;
	    while (dij_v->arc != NULL) {
		cu_call(path_unwind, dij_v->arc);
		distance += dij_v->distance;
		dij_v = dij_v->prev;
	    }
	    return distance;
	}

	/* Relax v */
	cugra_vertex_arcs_for(dir, a, v) {
	    dij_vertex_t dij_u;
	    cugra_vertex_t u = cugra_arc_adjacent(dir, a);
	    double s_uv, s_u_try;
	    if (u == v)
		continue;
	    s_uv = cu_call(arc_distance, a);
	    if (isinf(s_uv))
		continue;
	    s_u_try = dij_v->distance + s_uv;
	    if (cucon_pmap_insert_mem(&vprop, u, sizeof(struct dij_vertex_s),
				      &dij_u)) {
		dij_u->colour = cucon_algo_colour_grey;
		dij_u->vertex = u;
		dij_u->arc = a;
		dij_u->prev = dij_v;
		dij_u->distance = s_u_try;
		cucon_priq_insert(&q, dij_u);
	    }
	    else if (dij_u->colour == cucon_algo_colour_grey &&
		     dij_u->distance < s_u_try) {
		dij_u->arc = a;
		dij_u->distance = s_u_try;
		dij_u->prev = dij_v;
		cucon_priq_insert(&q, dij_u);
	    }
	}
	dij_v->colour = cucon_algo_colour_black;
    }
    return INFINITY;
}
