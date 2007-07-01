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

#include <cucon/bigraph.h>
#include <cucon/pmap.h>
#include <cu/cstr.h>
#include <cu/str.h>
#include <stdio.h>

struct cucon_umap_s vmap;
cucon_bigraph_t g;

cucon_bigraph_vertex_t
vertex(int i)
{
    cucon_bigraph_vertex_t *v;
    if (cucon_umap_insert_mem(&vmap, i, sizeof(cucon_bigraph_vertex_t), &v)) {
	*v = cucon_bigraph_insert_vertex_mem(g, sizeof(int));
	*(int *)cucon_bigraph_vertex_get_mem(*v) = i;
    }
    return *v;
}

cucon_bigraph_edge_t
edge(int n, int i, int j)
{
    cucon_bigraph_edge_t e;
    e = cucon_bigraph_insert_edge_mem(g, vertex(i), vertex(j), sizeof(int));
    *(int *)cucon_bigraph_edge_get_mem(e) = n;
    return e;
}

void
cycle(int n, int i, int j)
{
    int k;
    for (k = i; k < j; ++k)
	edge(n, k, k + 1);
    edge(n, j, i);
}

cu_clos_def(vertex_label,
	    char const *cu_proto(cucon_bigraph_vertex_t v),
	    ())
{
    cu_str_t s = cu_str_new_fmt("v%d", *(int *)cucon_bigraph_vertex_get_mem(v));
    return cu_str_to_cstr(s);
}
cu_clos_def(edge_label,
	    char const *cu_proto(cucon_bigraph_edge_t e),
	    ())
{
    cu_str_t s = cu_str_new_fmt("e%d", *(int *)cucon_bigraph_edge_get_mem(e));
    return cu_str_to_cstr(s);
}

cu_clos_def(edge_distance,
	    double cu_proto(cucon_bigraph_edge_t e),
	    ())
{
    return 1.0;
}

int
main(int argc, char **argv)
{
    vertex_label_t clos_vl;
    edge_label_t clos_el;
    FILE *out;
    cucon_list_t sp;
    cucon_list_it_t it;
    edge_distance_t edist;

    cu_init();
    cucon_umap_cct(&vmap);
    g = cucon_bigraph_new(cucon_bigraph_opt_list_of_vertices |
			cucon_bigraph_opt_list_of_edges);

    cycle(0, 0, 8);
    edge(1, 8, 9);
    edge(1, 9, 10);
    cycle(2, 10, 14);

    cu_clos_cct(&edist, edge_distance);
    sp = cucon_bigraph_shortest_path(g, vertex(2), vertex(11),
				   cu_clos_ref(&edist));
    assert(sp);
    for (it = cucon_list_begin(sp); it != cucon_list_end(sp);
	 it = cucon_list_it_next(it)) {
	cucon_bigraph_edge_t e = cucon_list_it_get_ptr(it);
	cucon_bigraph_vertex_t v = cucon_bigraph_edge_dst(e);
	printf(" v%d", *(int *)cucon_bigraph_vertex_get_mem(v));
    }
    printf("\n");

    out = fopen("tmp.bigraph_t0.gviz", "w");
    if (!out) {
	perror(argv[0]);
	exit(1);
    }
    cu_clos_cct(&clos_vl, vertex_label);
    cu_clos_cct(&clos_el, edge_label);
    fputs("digraph test {\n", out);
    cucon_bigraph_write_graphviz(g,
			       cu_clos_ref(&clos_vl),
			       cu_clos_ref(&clos_el), out);
    fputs("}\n", out);
    fclose(out);
    return 0;
}

