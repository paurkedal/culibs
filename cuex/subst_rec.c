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

#include <cuex/subst.h>
#include <cuex/algo.h>
#include <cuex/opn.h>
#include <cuex/oprdefs.h>
#include <cuex/var.h>
#include <cucon/stack.h>
#include <cucon/pmap.h>
#include <cucon/pset.h>
#include <cugra/graph.h>
#include <cugra/graph_algo.h>

cu_dlog_def(_file, "dtag=cuex.subst.rec");

/* -- cuex_subst_render_idempotent */

cu_clos_def(_build_graph_vertex_cb,
	    cu_prot(void, cuex_veqv_t vq),
    ( cugra_graph_t G;
      cucon_pmap_t var_to_vertex; ))
{
    cugra_vertex_t *vtx;
    cu_clos_self(_build_graph_vertex_cb);
    vq->is_feedback_var = 0;
    if (cucon_pmap_insert_mem(self->var_to_vertex, cuex_veqv_primary_var(vq),
			      sizeof(cugra_vertex_t), &vtx)) {
	*vtx = cugra_graph_vertex_new_ptr(self->G, vq);
    }
}

cu_clos_def(_build_graph_connect_cb,
	    cu_prot(cu_bool_t, cuex_var_t var),
    ( cuex_subst_t sig;
      cugra_graph_t G;
      cucon_pmap_t var_to_vertex;
      cugra_vertex_t vtx_tail; ))
{
    cu_clos_self(_build_graph_connect_cb);
    cugra_vertex_t vtx_head = cucon_pmap_find_ptr(self->var_to_vertex, var);
    cuex_veqv_t vq = cuex_subst_cref(self->sig, var);
    if (vq) {
	var = cuex_veqv_primary_var(vq);
	cu_debug_assert(vtx_head);
	cugra_connect(self->G, self->vtx_tail, vtx_head);
    }
    return cu_true;
}

cu_clos_def(_build_graph_arc_cb,
	    cu_prot(void, cuex_veqv_t vq),
    ( cuex_subst_t sig;
      cugra_graph_t G;
      cucon_pmap_t var_to_vertex; ))
{
    cu_clos_self(_build_graph_arc_cb);
    cuex_t val = cuex_veqv_value(vq);
    if (val) {
	_build_graph_connect_cb_t cb;
	cuex_var_t var = cuex_veqv_primary_var(vq);
	cb.sig = self->sig;
	cb.G = self->G;
	cb.vtx_tail = cucon_pmap_find_ptr(self->var_to_vertex, var);
	cb.var_to_vertex = self->var_to_vertex;
	cu_debug_assert(cb.vtx_tail);
	cuex_depth_conj_vars(val, _build_graph_connect_cb_prep(&cb));
    }
}

cu_clos_def(_mark_veqv_cb,
	    cu_prot(void, void const *vertex),
    ( cucon_pmap_t var_to_rvar;
      int i;
      cuex_veqv_t *veqv_arr; ))
{
#define vertex ((cugra_vertex_t)vertex)
    cu_clos_self(_mark_veqv_cb);
    cuex_veqv_t vq = cugra_vertex_ptr(vertex);
    cuex_veqv_it_t it;
    cuex_var_t rvar = cuex_rvar(self->i);
    cu_debug_assert(vq);
    cu_debug_assert(vq->value);
    vq->is_feedback_var = 1;
    for (it = cuex_veqv_begin(vq); it != cuex_veqv_end(vq);
	    it = cuex_veqv_it_next(it)) {
	cu_dlogf(_file, "%! ↦ %!", cuex_veqv_it_get(it), vq->value),
	cucon_pmap_insert_ptr(self->var_to_rvar, cuex_veqv_it_get(it), rvar);
    }
    self->veqv_arr[self->i++] = vq;
#undef vertex
}

static cuex_t
render_idempotent_expand(cuex_subst_t sig, cucon_pset_t msc_vars,
			 cucon_pmap_t var_to_rvar, cuex_t e)
{
    cuex_meta_t meta = cuex_meta(e);
    if (cuex_meta_is_opr(meta))
	CUEX_OPN_TRAN(meta, e, ep,
		render_idempotent_expand(sig, msc_vars, var_to_rvar, ep));
    else if (cuex_is_varmeta(meta)) {
	cuex_var_t rvar = cucon_pmap_find_ptr(var_to_rvar, e);
	if (rvar) {
	    cu_dlogf(_file, "(expand) feedback: %! ↦ %!", e, rvar);
	    return rvar;
	}
	else if (cucon_pset_find(msc_vars, e)) {
	    cu_dlogf(_file, "(expand) strongly connected: %!", e);
	    e = cuex_subst_lookup(sig, e);
	    cu_debug_assert(e);
	    return render_idempotent_expand(sig, msc_vars, var_to_rvar, e);
	}
	cu_dlogf(_file, "(expand) independent: %!", e);
    }
    return e;
}

static void
create_MSC_graphs(cuex_subst_t sig, cucon_stack_t KG)
{
    struct cugra_graph_s G_sig;
    struct cucon_pmap_s var_to_vertex;
    _build_graph_vertex_cb_t vertex_cb;
    _build_graph_arc_cb_t arc_cb;

    /* Construct graph of variable dependencies. */
    cuex_subst_flatten(sig);
    cugra_graph_init(&G_sig, 0);
    cucon_pmap_init(&var_to_vertex);
    /* -- vertices */
    vertex_cb.G = &G_sig;
    vertex_cb.var_to_vertex = &var_to_vertex;
    cuex_subst_iter_veqv(sig, _build_graph_vertex_cb_prep(&vertex_cb));
    /* -- arcs */
    arc_cb.sig = sig;
    arc_cb.G = &G_sig;
    arc_cb.var_to_vertex = &var_to_vertex;
    cuex_subst_iter_veqv(sig, _build_graph_arc_cb_prep(&arc_cb));

    /* Process each maximally connected subgraph, corresponding to
     * interdependent sets of variables. */
    cucon_stack_init(KG);
    cugra_move_MSC_subgraphs(&G_sig, KG);
}

void
cuex_subst_render_idempotent(cuex_subst_t sig)
{
    struct cucon_stack_s KG;
    create_MSC_graphs(sig, &KG);
    while (!cucon_stack_is_empty(&KG)) {
	struct cucon_pset_s mfvs_vertices;
	struct cucon_pset_s msc_vars;
	_mark_veqv_cb_t mark_cb;
	int i, n_V;
	cuex_t *opd_arr;
	cuex_veqv_t *veqv_arr;
	cuex_t recv;
	struct cucon_pmap_s var_to_rvar;
	cugra_vertex_t vtx;
	cugra_graph_t G = CUCON_STACK_TOP(&KG, cugra_graph_t);
	CUCON_STACK_POP(&KG, cugra_graph_t);

	/* Collect maximally strongly connected variables. */
	cucon_pset_init(&msc_vars);
	cugra_graph_for_vertices(vtx, G) {
	    cuex_veqv_t vq = cugra_vertex_mem(vtx);
	    cuex_veqv_it_t it;
	    for (it = cuex_veqv_begin(vq); it != cuex_veqv_end(vq);
		    it = cuex_veqv_it_next(it))
		cucon_pset_insert(&msc_vars, cuex_veqv_it_get(it));
	}

	/* Mark variables in mimimum feedback vertex set, and associate
	 * cuex_rvar_t variables with them. */
	cucon_pset_init(&mfvs_vertices);
	cugra_MFVS(G, &mfvs_vertices);
	n_V = cucon_pset_size(&mfvs_vertices);
	veqv_arr = cu_salloc(sizeof(cuex_veqv_t)*n_V);
	mark_cb.veqv_arr = veqv_arr;
	mark_cb.i = 0;
	cucon_pmap_init(&var_to_rvar);
	mark_cb.var_to_rvar = &var_to_rvar;
	cucon_pset_iter(&mfvs_vertices, _mark_veqv_cb_prep(&mark_cb));

	/* Replace variable mappings with rbind constructs. */
	opd_arr = cu_salloc(sizeof(cuex_t)*n_V);
	cu_dlogf(_file, "%d MSC vars", cucon_pset_size(&msc_vars));
	for (i = 0; i < n_V; ++i)
	    opd_arr[i] = render_idempotent_expand(sig, &msc_vars, &var_to_rvar,
						  veqv_arr[i]->value);
	recv = cuex_opn_by_arr(CUEX_OR_TUPLE(n_V), opd_arr);
	for (i = 0; i < n_V; ++i)
	    veqv_arr[i]->value = cuex_o2_rbind(recv, cuex_rvar(i));
    }
    sig->is_idem = cu_true;
}


/* -- cuex_subst_mark_min_feedback */

cu_clop_def(mark_veqv_of_vertex, void, void const *vertex)
{
#define vertex ((cugra_vertex_t)vertex)
    ((cuex_veqv_t)cugra_vertex_mem(vertex))->is_feedback_var = 1;
#undef vertex
}

cu_bool_t
cuex_subst_mark_min_feedback(cuex_subst_t sig)
{
    struct cucon_stack_s KG;
    create_MSC_graphs(sig, &KG);
    if (cucon_stack_is_empty(&KG))
	return cu_false;
    do {
	cugra_graph_t G = cucon_stack_pop_ptr(&KG);
	struct cucon_pset_s mfvs_vertices;
	cugra_MFVS(G, &mfvs_vertices);
	cucon_pset_iter(&mfvs_vertices, mark_veqv_of_vertex);
    }
    while (!cucon_stack_is_empty(&KG));
    return cu_true;
}


/* -- cuex_subst_mark_all_feedback */

cu_bool_t
cuex_subst_mark_all_feedback(cuex_subst_t sig)
{
    struct cucon_stack_s KG;
    create_MSC_graphs(sig, &KG);
    if (cucon_stack_is_empty(&KG))
	return cu_false;
    do {
	cugra_graph_t G = cucon_stack_pop_ptr(&KG);
	cugra_vertex_t vtx;
	cugra_graph_for_vertices(vtx, G) {
	    cuex_veqv_t vq = cugra_vertex_mem(vtx);
	    vq->is_feedback_var = cu_true;
	}
    } while (!cucon_stack_is_empty(&KG));
    return cu_true;
}
