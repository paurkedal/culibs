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

#include <cugra/fwd.h>
#include <cugra/graph.h>
#include <cugra/graph_algo.h>
#include <cucon/pmap.h>
#include <cucon/pset.h>
#include <cucon/stack.h>
#include <cu/str.h>
#include <cu/test.h>
#include <time.h>


cu_clos_def(vertex_label,
	    cu_prot(cu_str_t, cugra_vertex_t v),
    ( cucon_pmap_t msc_map;
      cucon_pset_t mfvs_set;
      struct cucon_pmap_s vertex_index_map;
      int current_vertex_index; ))
{
    cu_clos_self(vertex_label);
    int *i;
#if 0
    cu_str_t label;
    if (cucon_pmap_insert_mem(&self->vertex_index_map, v, sizeof(int), &i))
	*i = self->current_vertex_index++;
    label = cu_str_new_fmt("%d", *i);
    i = cucon_pmap_find_mem(self->msc_map, v);
    if (i)
	cu_str_append_fmt(label, ":%d", *i);
    return label;
#else
    i = cucon_pmap_find_mem(self->msc_map, v);
    if (cucon_pset_find(self->mfvs_set, v)) {
	cu_test_assert(i);
	return cu_str_new_fmt("%d cut", *i);
    }
    else if (i)
	return cu_str_new_fmt("%d", *i);
    else
	return cu_str_new();
#endif
}

void
test(int vertex_cnt, cu_bool_t do_save)
{
    int i, j;
    uint8_t *adj = cu_galloc(sizeof(uint8_t)*vertex_cnt*vertex_cnt);
    cugra_graph_t G = cugra_graph_new(0);
    cugra_vertex_t *v_arr = cu_galloc(sizeof(cugra_vertex_t)*vertex_cnt);
    cugra_vertex_t v;
    cugra_arc_t a;
    int arc_cnt = 0;
    struct cucon_stack_s msc_stack;
    struct cucon_pmap_s msc_pmap;
    struct cucon_pset_s V;

    for (i = 0; i < vertex_cnt; ++i)
	v_arr[i] = cugra_graph_vertex_new(G);
    for (i = 0; i < vertex_cnt; ++i)
    for (j = 0; j < vertex_cnt; ++j) {
	if (lrand48() % (2*vertex_cnt) < 3) {
	    cugra_graph_arc_new(v_arr[i], v_arr[j]);
	    adj[i*vertex_cnt + j] = 1;
	    ++arc_cnt;
	}
	else
	    adj[i*vertex_cnt + j] = 0;
    }
    i = 0;
    for (v = cugra_graph_vertices_begin(G); v != cugra_graph_vertices_end(G);
	    v = cugra_graph_vertices_next(v)) {
	++i;
	for (a = cugra_vertex_outarcs_begin(v);
		a != cugra_vertex_outarcs_end(v);
		a = cugra_vertex_outarcs_next(a))
	    cu_test_assert(cugra_arc_tail(a) == v);
	for (a = cugra_vertex_inarcs_begin(v);
		a != cugra_vertex_inarcs_end(v);
		a = cugra_vertex_inarcs_next(a))
	    cu_test_assert(cugra_arc_head(a) == v);
    }
    cu_test_assert(i == vertex_cnt);
    i = 0;
    for (a = cugra_graph_arcs_begin(G); a != cugra_graph_arcs_end(G);
	    a = cugra_graph_arcs_next(G, a))
	++i;
    cu_test_assert(i == arc_cnt);

    /* Simplyfy graph */
    cugra_graph_erase_loops(G);
    cugra_graph_erase_isolated(G);

    /* Compute MFVS */
    cucon_pset_cct(&V);
    cugra_MFVS(G, &V);

    /* Compute MSC subgraphs (for displaying) */
    cucon_stack_cct(&msc_stack);
    cucon_pmap_cct(&msc_pmap);
    cugra_identify_MSC(G, &msc_stack, &msc_pmap);

    if (do_save) {
	vertex_label_t vlabel_cb;
	vlabel_cb.msc_map = &msc_pmap;
	cucon_pmap_cct(&vlabel_cb.vertex_index_map);
	vlabel_cb.current_vertex_index = 0;
	vlabel_cb.mfvs_set = &V;
	cugra_graph_save_dot(G, vertex_label_prep(&vlabel_cb), cu_clop_null,
			     "tmp.graph_t0.dot");
    }

    /* Make graph ascyclic by removing the MFVS vertices. */
    cucon_pset_iter(&V,
		    (cu_clop(, void, void const *))cugra_erase_vertex_clop);
    cu_test_assert(cugra_graph_is_acyclic(G));
}

void stress_test()
{
    FILE *fout = fopen("tmp.graph_t0_bm.list", "w");
    int i;
    for (i = 0; i < 200; i += 1 + i/50) {
	clock_t t;
	int j, J = 1000/(i + 1) + 1;
	fprintf(stderr, "Stress test with %d elements.\n", i);
	t = -clock();
	for (j = 0; j < J; ++j)
	    test(i, cu_false);
	t += clock();
	fprintf(fout, "%d %lg\n", i, t/(J*(double)CLOCKS_PER_SEC));
    }
    fclose(fout);
}

int
main()
{
    clock_t t;
    cugra_init();
    test(50, cu_true);
    t = -clock();
    stress_test();
    t += clock();
    fprintf(stdout, "time: %lg\n", t/(double)CLOCKS_PER_SEC);
    return 2*!!cu_test_bug_count();
}
