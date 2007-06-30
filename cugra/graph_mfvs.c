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
#include <cugra/bdd_buddy.h>
#include <cucon/pmap.h>
#include <cucon/pset.h>
#include <cucon/stack.h>
#include <cucon/list.h>
#include <bdd.h>


/* Feedback vertex set preserving graph reductions
 * ----------------------------------------------- */

static void
push_neighbourhood(cucon_stack_t KV, cugra_vertex_t v)
{
    cugra_arc_t a;
    cugra_vertex_outarcs_for(a, v)
	CUCON_STACK_PUSH(KV, cugra_vertex_t, cugra_arc_head(a));
    cugra_vertex_inarcs_for(a, v)
	CUCON_STACK_PUSH(KV, cugra_vertex_t, cugra_arc_tail(a));
}

static void
MFVS_reduce_graph(cugra_graph_t G, cucon_pset_t V, cucon_pmap_t vdst_to_vsrc)
{
    cugra_vertex_t v;
    struct cucon_stack_s KV;
    cucon_stack_cct(&KV);
    cugra_graph_vertices_for(v, G)
	CUCON_STACK_PUSH(&KV, cugra_vertex_t, v);
    while (!cucon_stack_is_empty(&KV)) {
	v = CUCON_STACK_TOP(&KV, cugra_vertex_t);
	CUCON_STACK_POP(&KV, cugra_vertex_t);
	if (cugra_vertex_is_sink(v) || cugra_vertex_is_source(v)) {
	    push_neighbourhood(&KV, v);
	    cugra_erase_vertex(v);
	}
	else if (cugra_vertex_has_loop(v)) {
	    cugra_vertex_t v_dst = cucon_pmap_find_ptr(vdst_to_vsrc, v);
	    cucon_pset_insert(V, v_dst);
	    push_neighbourhood(&KV, v);
	    cugra_erase_vertex(v);
	}
	else if (cugra_vertex_outdegree_leq_1(v)
		 || cugra_vertex_indegree_leq_1(v)) {
	    push_neighbourhood(&KV, v);
	    cugra_eliminate_vertex(v);
	}
    }
}


/* Building acyclicity condition for graph as a BDD
 * ------------------------------------------------ */

typedef struct MFVS_vinfo_s *MFVS_vinfo_t;
struct MFVS_vinfo_s
{
    int index;
    int it_num;
    /* Takes v=0 for uncut vertices and v=1 for cut vertices and returns
     * true iff the graph is acyclic. */
    BDD f;
};

static void
init_first_approx(cugra_vertex_t v, cucon_pmap_t vinfo_map,
		  cucon_umap_t index_to_vertex, cucon_list_t B)
{
    MFVS_vinfo_t vinfo;
    if (cucon_pmap_insert_mem(vinfo_map, v,
			      sizeof(struct MFVS_vinfo_s), &vinfo)) {
	cugra_arc_t a;
	vinfo->index = cucon_umap_size(index_to_vertex);
	cucon_umap_insert_ptr(index_to_vertex, vinfo->index, v);
	vinfo->f = bddfalse;
	vinfo->it_num = -1;
	cugra_vertex_outarcs_for(a, v)
	    init_first_approx(cugra_arc_head(a), vinfo_map, index_to_vertex, B);
	cu_dprintf("cugra.graph_mfvs",
		   "Init %p: f(%d) := %d", v, vinfo->index, vinfo->f);
	vinfo->it_num = 0;
    }
    else if (vinfo->it_num == -1) { /* visiting */
	vinfo->f = bddfalse;
	cucon_list_append_ptr(B, v);
    }
}

static BDD
MFVS_iterate(cugra_vertex_t v, cucon_pmap_t vinfo_map, int cur_it_num,
	     int *changes)
{
    cugra_arc_t a;
    MFVS_vinfo_t vinfo = cucon_pmap_find_mem(vinfo_map, v);
    cu_debug_assert(vinfo);
    if (vinfo->it_num != cur_it_num) {
	cu_dprintf("cugra.graph_mfvs", "Entering vertex %p", v);
	BDD tmp, g;
	vinfo->it_num = cur_it_num;

	/* g = ∧ {f(u) | (u, v) ∈ G} */
	g = bddtrue;
	cugra_vertex_inarcs_for(a, v) {
	    cugra_vertex_t u = cugra_arc_tail(a);
	    BDD f_u = MFVS_iterate(u, vinfo_map, cur_it_num, changes);
	    tmp = g;
	    g = bdd_addref(bdd_apply(g, f_u, bddop_and));
	    bdd_delref(tmp);
	}
	tmp = vinfo->f;
	vinfo->f = bdd_addref(bdd_apply(cugra_bdd_ithvar(vinfo->index), g,
					bddop_or));
	if (vinfo->f != tmp)
	    ++*changes;
	bdd_delref(tmp);
	bdd_delref(g);
	if (cu_debug_key("cugra.graph_mfvs")) {
	    cu_dprintf("cugra.graph_mfvs", "Leaving vertex %p: f=%d",
		       v, vinfo->f);
	    if (cu_debug_key("cugra.graph_mfvs.verbose"))
		bdd_printtable(vinfo->f);
	}
    }
    else
	cu_dprintf("cugra.graph_mfvs", "Catched vertex %p", v);
    return vinfo->f;
}

cu_clop_def(free_bdd_cb, cu_bool_t, void const *v, void *vinfo)
{
    bdd_delref(((MFVS_vinfo_t)vinfo)->f);
    return cu_true;
}

void
cugra_MFVS(cugra_graph_t G_src, cucon_pset_t cutset)
{
    struct cugra_graph_s G_dst;
    cugra_graph_t G;
    struct cucon_stack_s KG;
    struct cucon_pmap_s vdst_to_vsrc;
    cugra_graph_cct(&G_dst, 0);
    cucon_pmap_cct(&vdst_to_vsrc);
    cugra_graph_copy(G_src, &G_dst, NULL, &vdst_to_vsrc);
    MFVS_reduce_graph(&G_dst, cutset, &vdst_to_vsrc);
    cucon_stack_cct(&KG);
    cugra_move_MSC_subgraphs(&G_dst, &KG);
    cu_dprintf("cugra.graph_mfvs", "Start MFVS.");
    while (!cucon_stack_is_empty(&KG)) {
	cucon_listnode_t it_B;
	struct cucon_pmap_s vinfo_map;
	cugra_vertex_t v;
	int changes, cur_it_num;
	BDD g, partial_cutset;
	struct cucon_umap_s index_to_vertex;
	struct cucon_list_s B;
	int B_cnt;
	cu_dprintf("cugra.graph_mfvs", "MFVS: %d components on stack",
		   CUCON_STACK_SIZE(&KG, cugra_graph_t));

	G = CUCON_STACK_TOP(&KG, cugra_graph_t);
	CUCON_STACK_POP(&KG, cugra_graph_t);
	v = cugra_graph_vertices_begin(G);

	cucon_pmap_cct(&vinfo_map);
	cucon_umap_cct(&index_to_vertex);
	cucon_list_cct(&B);
	init_first_approx(v, &vinfo_map, &index_to_vertex, &B);
	cur_it_num = 1;
	B_cnt = cucon_list_count(&B);
	do {
	    changes = 0;
#if 0
	    cu_debug_assert(cur_it_num <= B_cnt + 3);
	    for (it_B = cucon_list_begin(&B); it_B != cucon_list_end(&B);
		    it_B = cucon_listnode_next(it_B))
		g = MFVS_iterate(cucon_listnode_ptr(it_B), &vinfo_map,
				 cur_it_num, &changes);
#else
	    g = MFVS_iterate(v, &vinfo_map, cur_it_num, &changes);
#endif
	    cu_dprintf("cugra.graph_mfvs", "%d changes at iteration %d.",
		       changes, cur_it_num);
	    ++cur_it_num;
	} while (changes);

	g = bddtrue;
	for (it_B = cucon_list_begin(&B); it_B != cucon_list_end(&B);
		it_B = cucon_listnode_next(it_B)) {
	    MFVS_vinfo_t vinfo;
	    BDD tmp;
	    vinfo = cucon_pmap_find_mem(&vinfo_map,
					cucon_listnode_ptr(it_B));
	    tmp = g;
	    g = bdd_addref(bdd_apply(g, vinfo->f, bddop_and));
	    bdd_delref(tmp);
	}
	cucon_pmap_conj_mem(&vinfo_map, free_bdd_cb);

	/* add partial cutset to fill cutset */
	//partial_cutset = bdd_satoneset(g, bddfalse, bddtrue);
	partial_cutset = bdd_satone(g);
	if (cu_debug_key("cugra.graph_mfvs")) {
	    fputs("\n******* partial solution\n", stderr);
	    bdd_fprintset(stderr, partial_cutset);
	    fputs("\n\n", stderr);
	}
	while (partial_cutset != bddtrue) {
	    BDD low, high;
	    cu_debug_assert(partial_cutset != bddfalse);
	    low = bdd_low(partial_cutset);
	    high = bdd_high(partial_cutset);
	    if (low == bddfalse) {
		int index = bdd_var(partial_cutset);
		cugra_vertex_t v;
		cugra_vertex_t v_src;
		v = cucon_umap_find_ptr(&index_to_vertex, index);
		cu_debug_assert(v);
		v_src = cucon_pmap_find_ptr(&vdst_to_vsrc, v);
		cu_debug_assert(v_src);
		cu_dprintf("cugra.graph_mfvs", "VERTEX %p", v_src);
		cucon_pset_insert(cutset, v_src);
		partial_cutset = high;
	    }
	    else {
		cu_debug_assert(high == bddfalse);
		partial_cutset = low;
	    }
	}
	bdd_delref(g);
    }
    cu_dprintf("cugra.graph_mfvs", "MFVS has %d vertices",
	       cucon_pset_size(cutset));
}


/* References
 *
 * [1] Pranav Ashar and Sharad Malik: "Implicit Computation of Minimum-Cost
 * Feedback-Vertex Sets for Partial Scan and Other Applications"
 */
