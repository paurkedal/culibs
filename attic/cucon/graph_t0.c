/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cucon/digraph.h>
#include <stdio.h>
#include <cu/clos.h>

void
print_graphviz(cucon_digraph_t g, cucon_list_t path, FILE* out)
{
    cucon_digraph_all_vertices_it_t it_v;
    cucon_digraph_all_edges_it_t it_e;
    cucon_list_it_t it_path;
    fprintf(out, "digraph test {\n");
    for (it_v = cucon_digraph_all_vertices_begin(g);
	 it_v != cucon_digraph_all_vertices_end(g);
	 it_v = cucon_digraph_all_vertices_it_next(it_v)) {
	cucon_digraph_vertex_t v = cucon_digraph_all_vertices_it_get(it_v);
	fprintf(out, "\tx%p [label=v%d];\n",
		v, *(int*)cucon_digraph_vertex_get_mem(v));
    }
    for (it_e = cucon_digraph_all_edges_begin(g);
	 it_e != cucon_digraph_all_edges_end(g);
	 it_e = cucon_digraph_all_edges_it_next(it_e)) {
	cucon_digraph_edge_t e = cucon_digraph_all_edges_it_get(it_e);
	cucon_digraph_vertex_t v0 = cucon_digraph_edge_src(e);
	cucon_digraph_vertex_t v1 = cucon_digraph_edge_dst(e);
/* 	fprintf(out, "\tx%p -> x%p [label=e%d];\n", */
/* 		v0, v1, *(int*)cucon_digraph_edge_value(e)); */
	fprintf(out, "\tx%p -> x%p;\n", v0, v1);
    }
    if (path != NULL)
	for (it_path = cucon_list_begin(path); it_path != cucon_list_end(path);
	     it_path = cucon_list_it_next(it_path)) {
	    cucon_digraph_edge_t e = cucon_list_it_get_ptr(it_path);
	    cucon_digraph_vertex_t v0 = cucon_digraph_edge_src(e);
	    cucon_digraph_vertex_t v1 = cucon_digraph_edge_dst(e);
	    fprintf(out, "\tx%p -> x%p [style=dashed];\n", v0, v1);
	}
    fprintf(out, "}\n");
}

#define VERTEX_COUNT 20
#define EDGE_COUNT 50

double
edge_distance(cucon_digraph_edge_t e)
{
    return 1.0;
}

cu_clos_def(vertex_test,
	    cu_bool_fast_t cu_proto(cucon_digraph_vertex_t v),
	    (cucon_digraph_vertex_t v_to_find;))
{
    cu_clos_self(vertex_test);
    return self->v_to_find == v;
}

int
real_main()
{
    int i;
    cucon_digraph_t g = cucon_digraph_new(cucon_digraph_opt_list_of_vertices);
    cucon_digraph_vertex_t v[VERTEX_COUNT];
    cucon_digraph_edge_t e[EDGE_COUNT];

    vertex_test_t vtest;
    double cu_nonclos(edist)(cucon_digraph_edge_t);
    cu_nonclos_cct(&edist, edge_distance);
    cu_clos_cct(&vtest, vertex_test);

    for (i = 0; i < VERTEX_COUNT; ++i) {
	v[i] = cucon_digraph_insert_vertex_mem(g, sizeof(int));
	*(int*)cucon_digraph_vertex_get_mem(v[i]) = i;
    }
    for (i = 0; i < EDGE_COUNT; ++i) {
	int i0 = lrand48() % VERTEX_COUNT;
	int i1 = lrand48() % VERTEX_COUNT;
	e[i] = cucon_digraph_insert_edge_mem(g, v[i0], v[i1], sizeof(int));
	*(int*)cucon_digraph_edge_get_mem(e[i]) = i;
    }

    vtest.v_to_find = v[1];
    cucon_list_t path
	= cucon_digraph_shortest_path_if(g, v[0],
				       cu_clos_ref(&vtest),
				       cu_nonclos_ref(&edist));
    if (path == NULL)
	fprintf(stderr, "No path.\n");
	
    print_graphviz(g, path, stdout);

    return 0;
}

int
main()
{
    cu_init();
    return real_main();
}
