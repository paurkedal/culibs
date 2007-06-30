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

/* XXX Loop detection and handling is not debugged.
 */

/* #define CHI_EX_ALGO_VERBOSE_DEBUG 1 */

#include <cuex/ex.h>
#include <cuex/opn.h>
#include <cuex/algo.h>
#include <cuex/oprdefs.h>
#include <cuex/aci.h>
#include <cuex/subst.h>
#include <cu/memory.h>
#include <cu/idr.h>
#include <cu/int.h>
#include <cucon/list.h>
#include <cucon/pmap.h>
#include <cucon/pset.h>
#include <string.h>
#include <assert.h>
#include <ffi.h>

#undef cuex_binary_inject_left


cu_rank_t
cuex_max_arity(cuex_t e)
{
    cuex_meta_t meta = cuex_meta(e);
    if (cuex_meta_is_opr(meta)) {
	cu_rank_t r = cuex_opr_r(meta);
	CUEX_OPN_ITER(meta, e, ep, r = cu_uint_max(r, cuex_max_arity(ep)));
	return r;
    }
    else
	return 0;
}

int
cuex_max_depth(cuex_t e)
{
    cuex_meta_t meta = cuex_meta(e);
    if (cuex_meta_is_opr(meta)) {
	int d = 0;
	CUEX_OPN_ITER(meta, e, ep, d = cu_uint_max(d, cuex_max_depth(ep)));
	return d + 1;
    }
    else
	return 0;
}

cu_bool_t
cuex_match_pmap(cuex_t patn, cuex_t ex, cucon_pmap_t vm)
{
    cuex_meta_t patn_meta = cuex_meta(patn);
    if (cuex_meta_is_opr(patn_meta)) {
	size_t i, n;
	cuex_opn_t opn_patn;
	cuex_opn_t opn_ex;
	assert(cuex_is_opn(patn));
	if (!cuex_is_opn(ex))
	    return 0;
	opn_patn = cuex_opn_from_ex(patn);
	opn_ex = cuex_opn_from_ex(ex);
	n = cuex_opr_r(patn_meta);
	if (n != cuex_opr_r(cuex_meta(opn_ex)))
	    return 0;
	for (i = 0; i < n; ++i)
	    if (!cuex_match_pmap(cuex_opn_at(opn_patn, i),
				 cuex_opn_at(opn_ex, i), vm))
		return 0;
	return 1;
    }
    else if (cuex_is_varmeta(patn_meta)) {
	cuex_t *slot;
	if (cucon_pmap_insert_mem(vm, patn, sizeof(cuex_t), &slot)) {
	    *slot = ex;
	    return cu_true;
	}
	else
	    return *slot == ex;
    }
    else
	return patn == ex;
}

cuex_t
cuex_unify(cuex_t ex0, cuex_t ex1)
{
    if (ex0 == ex1)
	return ex0;
    if (cuex_is_unknown(ex0))
	return ex1;
    if (cuex_is_unknown(ex1))
	return ex0;
    return cuex_null();
}

static cuex_t
fallback_tran_aci(cuex_meta_t join, cuex_opn_t idy, cuex_opn_t e,
		  cuex_opn_t acc, cu_clop(cb, cuex_t, cuex_t e))
{
    /* Basically a specialised version of "aci.c":aci_tran, for efficiency
     * since cuex_fallback_tran is very general. */
    while (e != idy) {
        cuex_opn_t v;
        cu_debug_assert(cuex_meta(e) == join);
        acc = fallback_tran_aci(join, idy, cuex_aci_left(e), acc, cb);
        if (!acc)
            return NULL;
        v = cu_call(cb, cuex_aci_rebranch(e, idy, idy));
        if (!v)
            return NULL;
        acc = cuex_aci_join(join, acc, v);
        e = cuex_aci_right(e);
    }
    return acc;
}
cuex_t
cuex_fallback_tran(cuex_t e, cu_clop(cb, cuex_t, cuex_t))
{
    cuex_t r = cu_call(cb, e);
    cuex_meta_t meta;
    if (r)
	return r;
    meta = cuex_meta(e);
    if (cuex_meta_is_opr(meta)) {
	if (cuex_opr_is_aci(meta)) {
	    cuex_opn_t idy = cuex_aci_identity(meta);
	    return fallback_tran_aci(meta, idy, e, idy, cb);
	}
	else
	    CUEX_OPN_TRAN(meta, e, se, cuex_fallback_tran(se, cb));
    }
    return e;
}

cu_bool_t
cuex_depth_conj_vars(cuex_t ex, cu_clop(cb, cu_bool_t, cuex_var_t))
{
    cuex_meta_t meta;
recurse:
    meta = cuex_meta(ex);
    if (cuex_meta_is_opr(meta)) {
	size_t N, n;
	N = cuex_opr_r(meta);
	if (N > 0) {
	    --N;
	    for (n = 0; n < N; ++n)
		if (!cuex_depth_conj_vars(cuex_opn_at(cuex_opn_from_ex(ex), n),
					  cb))
		    return cu_false;
	    ex = cuex_opn_at(cuex_opn_from_ex(ex), N);
	    goto recurse;
	}
	return cu_true;
    }
    else if (cuex_is_varmeta(meta))
	return cu_call(cb, cuex_var_from_ex(ex));
    else
	return cu_true;
}

cu_clop_def(var_is_nonweak_clop, cu_bool_t, cuex_var_t ex)
{
    return !cuex_is_varmeta_w(cuex_meta(ex));
}

cu_bool_t
cuex_has_weak_var(cuex_t ex)
{
    return !cuex_depth_conj_vars(ex, var_is_nonweak_clop);
}

cu_bool_t
cuex_depthout_conj(cuex_t ex, cu_clop(cb, cu_bool_t, cuex_t))
{
    cuex_meta_t meta = cuex_meta(ex);
    if (cuex_meta_is_opr(meta)) {
	size_t i, r;
	r = cuex_opr_r(meta);
	for (i = 0; i < r; ++i)
	    if (!cuex_depthout_conj(cuex_opn_at(cuex_opn_from_ex(ex), i), cb))
		return cu_false;
    }
    return cu_call(cb, ex);
}

cu_clos_def(depthout_tran_aci_cb,
	    cu_prot(cuex_opn_t, cuex_opn_t opn),
    ( cu_clop(cb, cuex_t, cuex_t); ))
{
    cu_clos_self(depthout_tran_aci_cb);
    return cuex_depthout_tran(opn, self->cb);
}
cuex_t
cuex_depthout_tran(cuex_t ex, cu_clop(cb, cuex_t, cuex_t))
{
    cuex_meta_t meta = cuex_meta(ex);
    if (cuex_meta_is_opr(meta)) {
	if (cuex_opr_is_aci(meta)) {
	    depthout_tran_aci_cb_t aci_cb;
	    aci_cb.cb = cb;
	    ex = cuex_aci_tran(meta, ex, depthout_tran_aci_cb_prep(&aci_cb));
	}
	else
	    CUEX_OPN_TRAN(meta, ex, subex, cuex_depthout_tran(subex, cb));
    }
    return cu_call(cb, ex);
}

cuex_t
cuex_depth_tran_leaves(cuex_t ex, cu_clop(cb, cuex_t, cuex_t))
{
    cuex_meta_t meta = cuex_meta(ex);
    if (cuex_meta_is_opr(meta)) {
	CUEX_OPN_TRAN(meta, ex, subex, cuex_depth_tran_leaves(subex, cb));
	return ex;
    }
    else
	return cu_call(cb, ex);
}

cuex_t
cuex_depth_tran_vars(cuex_t ex, cu_clop(cb, cuex_t, cuex_var_t))
{
    cuex_meta_t meta = cuex_meta(ex);
    if (cuex_meta_is_opr(meta))
	CUEX_OPN_TRAN(meta, ex, subex, cuex_depth_tran_vars(subex, cb));
    else if (cuex_is_varmeta(meta))
	ex = cu_call(cb, ex);
    return ex;
}

cu_bool_t
cuex_maxtrees_of_leaftest_iter(cuex_t ex,
			       cu_clop(test, cu_bool_t, cuex_t leaf),
			       cu_clop(recv, void, cuex_t subtree))
{
    cuex_meta_t meta = cuex_meta(ex);
    if (cuex_meta_is_opr(meta)) {
	size_t i, r = cuex_opr_r(meta);
	cu_bool_t acc = cu_true;
	cu_bool_t *c = cu_salloc(sizeof(cu_bool_t)*r);
	for (i = 0; i < r; ++i) {
	    c[i] = cuex_maxtrees_of_leaftest_iter(cuex_opn_at(ex, i),
						  test, recv);
	    acc &= c[i];
	}
	if (!acc)
	    for (i = 0; i < r; ++i)
		if (c[i])
		    cu_call(recv, cuex_opn_at(ex, i));
	return acc;
    }
    else
	return cu_call(test, ex);
}

static int
maxtrees_of_leaftest_tran(cuex_t *ex,
			  cu_clop(test, cu_bool_t, cuex_t leaf),
			  cu_clop(tran, cuex_t, cuex_t subtree))
{
    cuex_meta_t meta = cuex_meta(*ex);
    if (cuex_meta_is_opr(meta)) {
	size_t i, r = cuex_opr_r(meta);
	int acc = 0;
	int *c = cu_salloc(sizeof(cu_bool_t)*r);
	cuex_t *x = cu_salloc(sizeof(cuex_t)*r);
	for (i = 0; i < r; ++i) {
	    x[i] = cuex_opn_at(*ex, i);
	    acc += c[i] = maxtrees_of_leaftest_tran(&x[i], test, tran);
	}
	if (acc == r)
	    return 1;
	else {
	    if (acc > 0) {
		for (i = 0; i < r; ++i)
		    if (c[i])
			x[i] = cu_call(tran, x[i]);
	    }
	    *ex = cuex_opn_by_arr(meta, x);
	    return 0;
	}
    }
    else
	return !!cu_call(test, *ex);
}

cuex_t
cuex_maxtrees_of_leaftest_tran(cuex_t ex,
			       cu_clop(test, cu_bool_t, cuex_t leaf),
			       cu_clop(tran, cuex_t, cuex_t subtree))
{
    if (maxtrees_of_leaftest_tran(&ex, test, tran))
	return cu_call(tran, ex);
    else
	return ex;
}

cu_clos_def(cuex_lgr_block_cb,
	    cu_prot(cu_bool_t, cuex_var_t var),
	    (cuex_subst_t subst;))
{
    cu_clos_self(cuex_lgr_block_cb);
    cuex_subst_t subst = self->subst;
    cuex_meta_t meta = cuex_meta(var);
    if (cuex_subst_is_active_varmeta(subst, meta)
	&& cuex_subst_lookup(subst, var) == NULL)
	cuex_subst_block(subst, var);
    return cu_true;
}

cu_bool_t
cuex_lgr(cuex_t ex0, cuex_t ex1, cuex_subst_t subst)
{
    cuex_lgr_block_cb_t cb;
    cb.subst = subst;
    cuex_depth_conj_vars(ex0, cuex_lgr_block_cb_prep(&cb));
    return cuex_subst_unify(subst, ex1, ex0);
}

cuex_t
cuex_msg_unify(cuex_t e0, cuex_t e1, cu_clop(f, cuex_t, cuex_t e0, cuex_t e1))
{
    cuex_meta_t e0_meta = cuex_meta(e0);
    if (e0 == e1)
	return e0;
    else if (e0_meta == cuex_meta(e1) && cuex_meta_is_opr(e0_meta)) {
	cu_rank_t r, j;
	cuex_t *tpl_arr;
	r = cuex_opr_r(e0_meta);
	tpl_arr = cu_salloc(r*sizeof(cuex_t));
	for (j = 0; j < r; ++j)
	    tpl_arr[j] = cuex_msg_unify(cuex_opn_at(e0, j),
					cuex_opn_at(e1, j), f);
	return cuex_opn_by_arr(e0_meta, tpl_arr);
    }
    else
	return cu_call(f, e0, e1);
}

cuex_t
cuex_msg_unify_by_arr(size_t cnt, cuex_t *arr, cu_clop(f, cuex_t, cuex_t *arr))
{
    size_t i;
    cuex_meta_t meta;
    cu_rank_t r, j;
    cuex_t *sub_arr;
    cuex_t *tpl_arr;
    cu_debug_assert(cnt > 0);
    meta = cuex_meta(arr[0]);
    if (cuex_meta_is_opr(meta)) {
	r = cuex_opr_r(meta);
	sub_arr = cu_salloc(cnt*r*sizeof(cuex_t));
	for (i = 0; i < cnt; ++i) {
	    if (cuex_meta(arr[i]) != meta)
		return cu_call(f, arr);
	    for (j = 0; j < r; ++j)
		sub_arr[j*cnt + i] = cuex_opn_at(arr[i], j);
	}
	tpl_arr = cu_salloc(r*sizeof(cuex_t));
	for (j = 0; j < r; ++j)
	    tpl_arr[j] = cuex_msg_unify_by_arr(cnt, sub_arr + j*cnt, f);
	return cuex_opn_by_arr(meta, tpl_arr);
    } else {
	for (i = 1; i < cnt; ++i)
	    if (arr[i] != arr[0])
		return cu_call(f, arr);
	return arr[0];
    }
}

size_t
cuex_count_unique_nodes_except(cuex_t ex, cucon_pmap_t cache)
{
    void *ignore;
    if (!cucon_pmap_insert_mem(cache, ex, 0, &ignore))
	return 0;
    if (cuex_is_opn(ex)) {
	size_t count = 1;
	size_t i;
	size_t n = cuex_opn_r(cuex_opn_from_ex(ex));
	for (i = 0; i < n; ++i)
	    count += cuex_count_unique_nodes_except(
		cuex_opn_at(cuex_opn_from_ex(ex), i), cache);
	return count;
    }
    else
	return 1;
}

size_t
cuex_count_unique_nodes(cuex_t ex)
{
    cucon_pmap_t cache = cucon_pmap_new();
    return cuex_count_unique_nodes_except(ex, cache);
}

cu_clos_def(cuexP_contains_var_in_pmap_h0,
	    cu_prot(cu_bool_t, cuex_var_t var),
	    (cucon_pmap_t pmap;))
{
    cu_clos_self(cuexP_contains_var_in_pmap_h0);
    return !cucon_pmap_find_mem(self->pmap, var);
}
cu_bool_t
cuex_contains_var_in_pmap(cuex_t ex, cucon_pmap_t pmap)
{
    cuexP_contains_var_in_pmap_h0_t cb;
    cb.pmap = pmap;
    return !cuex_depth_conj_vars(ex, cuexP_contains_var_in_pmap_h0_prep(&cb));
}


cu_bool_t
cuex_contains_ex(cuex_t ex, cuex_t sub)
{
    if (ex == sub)
	return cu_true;
    else if (cuex_is_opn(ex)) {
	size_t i;
	size_t arity = cuex_opn_r(cuex_opn_from_ex(ex));
	for (i = 0; i < arity; ++i)
	    if (cuex_contains_ex(cuex_opn_at(cuex_opn_from_ex(ex), i), sub))
		return cu_true;
	return cu_false;
    }
    else
	return cu_false;
}

cuex_t
cuex_substitute_ex(cuex_t ex, cuex_t var, cuex_t value)
{
    if (ex == var)
	return value;
    else {
	cuex_meta_t meta = cuex_meta(ex);
	if (cuex_meta_is_opr(meta))
	    CUEX_OPN_TRAN(meta, ex, subex,
			    cuex_substitute_ex(subex, var, value));
	return ex;
    }
}

cuex_t
cuex_substitute_pmap(cuex_t ex, cucon_pmap_t pmap)
{
    cuex_t repl = cucon_pmap_find_ptr(pmap, ex);
    if (repl != NULL)
	return repl;
    else {
	cuex_meta_t meta = cuex_meta(ex);
	if (cuex_meta_is_opr(meta))
	    CUEX_OPN_TRAN(meta, ex, subex,
			    cuex_substitute_pmap(subex, pmap));
	return ex;
    }
}

cuex_t
cuex_leftmost_with_meta(cuex_t ex, cuex_meta_t search_meta)
{
    cuex_meta_t meta = cuex_meta(ex);
    if (cuex_meta_is_opr(meta)) {
	cuex_t *it;
	cuex_t *it_end = cuex_opn_end(ex);
	for (it = cuex_opn_begin(ex); it < it_end; ++it) {
	    cuex_t res = cuex_leftmost_with_meta(*it, search_meta);
	    if (res)
		return res;
	}
    }
    if (meta == search_meta)
	return ex;
    return NULL;
}

cuex_t
cuex_rightmost_with_meta(cuex_t ex, cuex_meta_t search_meta)
{
    cuex_meta_t meta = cuex_meta(ex);
    if (cuex_meta_is_opr(meta)) {
	cuex_t *it_begin = cuex_opn_begin(ex);
	cuex_t *it;
	for (it = cuex_opn_end(ex); it >= it_begin; --it) {
	    cuex_t res = cuex_rightmost_with_meta(*it, search_meta);
	    if (res)
		return res;
	}
    }
    if (meta == search_meta)
	return ex;
    return NULL;
}

cuex_var_t
cuex_leftmost_var(cuex_t ex, cuex_qcset_t qcset)
{
    cuex_meta_t meta = cuex_meta(ex);
    if (cuex_meta_is_opr(meta)) {
	cuex_t *it;
	cuex_t *it_end = cuex_opn_end(ex);
	for (it = cuex_opn_begin(ex); it < it_end; ++it) {
	    cuex_var_t res = cuex_leftmost_var(*it, qcset);
	    if (res)
		return res;
	}
    }
    else if (cuex_is_varmeta(meta)) {
	if (cuex_qcset_contains(qcset, cuex_varmeta_qcode(meta)))
	    return ex;
    }
    return NULL;
}

cuex_var_t
cuex_rightmost_var(cuex_t ex, cuex_qcset_t qcset)
{
    cuex_meta_t meta = cuex_meta(ex);
    if (cuex_meta_is_opr(meta)) {
	cuex_t *it_begin = cuex_opn_begin(ex);
	cuex_t *it;
	for (it = cuex_opn_end(ex) - 1; it >= it_begin; --it) {
	    cuex_var_t res = cuex_rightmost_var(*it, qcset);
	    if (res)
		return res;
	}
    }
    else if (cuex_is_varmeta(meta)) {
	if (cuex_qcset_contains(qcset, cuex_varmeta_qcode(meta)))
	    return ex;
    }
    return NULL;
}

int
cuex_binary_left_depth(cuex_meta_t opr, cuex_t ex)
{
    int cnt = 0;
    cu_debug_assert(cuex_meta_is_opr(opr));
    cu_debug_assert(cuex_opr_r(opr) >= 1);
    while (cuex_meta(ex) == opr) {
	++cnt;
	ex = cuex_opn_at(ex, 0);
    }
    return cnt;
}

int
cuex_binary_right_depth(cuex_meta_t opr, cuex_t ex)
{
    int cnt = 0;
    cu_debug_assert(cuex_meta_is_opr(opr));
    cu_debug_assert(cuex_opr_r(opr) == 2);
    while (cuex_meta(ex) == opr) {
	++cnt;
	ex = cuex_opn_at(ex, 1);
    }
    return cnt;
}

cuex_t
cuex_binary_left_subex(cuex_meta_t opr, cuex_t ex, int depth)
{
    while (depth) {
	cu_debug_assert(cuex_meta(ex) == opr);
	ex = cuex_opn_at(ex, 0);
	--depth;
    }
    return ex;
}

cuex_t
cuex_binary_right_subex(cuex_meta_t opr, cuex_t ex, int depth)
{
    while (depth) {
	cu_debug_assert(cuex_meta(ex) == opr);
	ex = cuex_opn_at(ex, 1);
	--depth;
    }
    return ex;
}

cuex_t
cuex_binary_inject_left(cuex_meta_t opr, cuex_t ex, cuex_t lhs)
{
    if (cuex_meta(ex) == opr) {
	cuex_t ex0 = cuex_binary_inject_left(opr, cuex_opn_at(ex, 0), lhs);
	return cuex_opn(opr, ex0, cuex_opn_at(ex, 1));
    }
    else
	return cuex_opn(opr, lhs, ex);
}

cuex_t
cuex_binary_inject_left_D(cuex_meta_t opr, cuex_t ex, cuex_t lhs)
{
    cu_debug_assert(cuex_meta_is_opr(opr) && cuex_opr_r(opr) == 2);
    return cuex_binary_inject_left(opr, ex, lhs);
}

static cu_bool_t
free_vars_conj(cuex_t ex, cuex_qcset_t qcset, cucon_pset_t excl,
	       cu_clop(fn, cu_bool_t, cuex_var_t, cucon_pset_t))
{
    cuex_meta_t meta = cuex_meta(ex);
    if (cuex_meta_is_opr(meta)) {
	if (cuex_og_scoping_contains(meta)) {
	    size_t i, r = cuex_opr_r(meta);
	    cuex_t var = cuex_opn_at(ex, 0);
	    cu_bool_t nonshadow;
	    cu_debug_assert(cuex_opr_r(meta) > 0);
	    nonshadow = cucon_pset_insert(excl, var);
	    for (i = 1; i < r; ++i)
		if (!free_vars_conj(cuex_opn_at(ex, i), qcset, excl, fn)) {
		    if (nonshadow)
			cucon_pset_erase(excl, var);
		    return cu_false;
		}
	    if (nonshadow)
		cucon_pset_erase(excl, var);
	}
	else
	    CUEX_OPN_CONJ_RETURN(meta, ex, subex,
		    free_vars_conj(subex, qcset, excl, fn));
    }
    else if (cuex_is_varmeta(meta) &&
	     cuex_qcset_contains(qcset, cuex_varmeta_qcode(meta))) {
	if (!cucon_pset_find(excl, ex))
	    return cu_call(fn, ex, excl);
    }
    return cu_true;
}

cu_bool_t
cuex_free_vars_conj(cuex_t ex, cuex_qcset_t qcset, cucon_pset_t excl,
		    cu_clop(fn, cu_bool_t, cuex_var_t, cucon_pset_t))
{
    if (!excl) {
	excl = cu_salloc(sizeof(struct cucon_pmap_s));
	cucon_pset_cct(excl);
    }
    return free_vars_conj(ex, qcset, excl, fn);
}

cu_clos_efun(cuex_pset_curried_insert_ex,
	     cu_prot(cu_bool_t, cuex_t var))
{
    cu_clos_self(cuex_pset_curried_insert_ex);
    cucon_pset_insert(self->accu, var);
    return cu_true;
}

cu_clos_efun(cuex_pset_curried_erase_ex,
	     cu_prot(cu_bool_t, cuex_t var))
{
    cu_clos_self(cuex_pset_curried_erase_ex);
    cucon_pset_erase(self->accu, var);
    return cu_true;
}

void
cuex_free_vars_insert(cuex_t e, cuex_qcset_t qcset, cucon_pset_t excl,
		      cucon_pset_t accu)
{
    cuex_pset_curried_insert_ex_t cb;
    cb.accu = accu;
    cuex_free_vars_conj(e, qcset, excl,
			(cu_clop(, cu_bool_t, cuex_var_t, cucon_pset_t))
				cuex_pset_curried_insert_ex_prep(&cb));
}

void
cuex_free_vars_erase(cuex_t e, cuex_qcset_t qcset, cucon_pset_t excl,
		     cucon_pset_t accu)
{
    cuex_pset_curried_erase_ex_t cb;
    cb.accu = accu;
    cuex_free_vars_conj(e, qcset, excl,
			(cu_clop(, cu_bool_t, cuex_var_t, cucon_pset_t))
				cuex_pset_curried_erase_ex_prep(&cb));
}

cu_clos_def(free_vars_count_cb,
	    cu_prot(cu_bool_t, cuex_var_t var, cucon_pset_t bound),
	( int cnt; ))
{
    cu_clos_self(free_vars_count_cb);
    ++self->cnt;
    return cu_true;
}

int
cuex_free_vars_count(cuex_t e, cuex_qcset_t qcset, cucon_pset_t excl)
{
    free_vars_count_cb_t cb;
    cb.cnt = 0;
    cuex_free_vars_conj(e, qcset, excl, free_vars_count_cb_prep(&cb));
    return cb.cnt;
}

cu_clos_def(outmost_quantify_vars_cb,
	    cu_prot(void, void const *var),
	( cuex_t e;
	  cuex_meta_t opr; ))
{
    cu_clos_self(outmost_quantify_vars_cb);
    self->e = cuex_opn(self->opr, var, self->e);
}

cuex_t
cuex_outmost_quantify_vars(cuex_meta_t opr, cucon_pset_t vars, cuex_t e)
{
    outmost_quantify_vars_cb_t cb;
    cu_debug_assert(cuex_opr_r(opr) == 2);
    cb.e = e;
    cb.opr = opr;
    cucon_pset_iter(vars, outmost_quantify_vars_cb_prep(&cb));
    return cb.e;
}

cu_clos_def(autoquantify_uvw_xyz_cb,
	    cu_prot(cuex_t, cuex_t ex),
	    (cucon_pmap_t env;))
{
    cu_clos_self(autoquantify_uvw_xyz_cb);
    assert(ex);
    if (cuex_is_idr(ex)) {
	cuex_pvar_t *var;
	char const *name = cu_idr_to_cstr(cuex_idr_from_ex(ex));
	char ch = name[0];
	cuex_qcode_t qcode;
	if ('u' <= ch && ch <= 'w')
	    qcode = cuex_qcode_active_w;
	else if ('x' <= ch && ch <= 'z')
	    qcode = cuex_qcode_active_s;
	else
	    return ex;
	if (cucon_pmap_insert_mem(self->env, ex, sizeof(cuex_pvar_t), &var))
	    *var = cuex_pvar_new(qcode);
//	    *var = cuex_pvar_new_named(NULL, qcode, name);
	return cuex_pvar_to_ex(*var);
    }
    else
	return ex;
}
cuex_t
cuex_autoquantify_uvw_xyz(cuex_t ex, cucon_pmap_t env)
{
    autoquantify_uvw_xyz_cb_t cb;
    cb.env = env;
    return cuex_depthout_tran(ex, autoquantify_uvw_xyz_cb_prep(&cb));
}

cu_clos_def(ex_stats_cb,
	    cu_prot(cu_bool_t, cuex_t ex),
	    (cuex_stats_t *stats;))
{
    cu_clos_self(ex_stats_cb);
    cuex_meta_t meta = cuex_meta(ex);
    switch (cuex_meta_kind(meta)) {
	case cuex_meta_kind_type:
	    ++self->stats->obj_cnt;
	    break;
	case cuex_meta_kind_opr:
	    ++self->stats->opn_cnt;
	    break;
	default:
	    if (cuex_is_varmeta(meta))
		switch (cuex_varmeta_qcode(meta)) {
		    case cuex_qcode_u:
			++self->stats->strong_var_cnt;
			break;
		    case cuex_qcode_e:
			++self->stats->passive_var_cnt;
			break;
		    case cuex_qcode_w:
			++self->stats->weak_var_cnt;
			break;
		    default:
			++self->stats->other_var_cnt;
			break;
		}
	    else
		++self->stats->other_node_cnt;
	    break;
    }
    return cu_true;
}

void
cuex_stats(cuex_t ex, cuex_stats_t *stats)
{
    ex_stats_cb_t cb;
    cb.stats = stats;
    memset(stats, 0, sizeof(cuex_stats_t));
    cuex_depthout_conj(ex, ex_stats_cb_prep(&cb));
    stats->var_cnt
	= stats->strong_var_cnt
	+ stats->weak_var_cnt
	+ stats->passive_var_cnt
	+ stats->other_var_cnt;
    stats->node_cnt
	= stats->var_cnt
	+ stats->opn_cnt
	+ stats->obj_cnt
	+ stats->other_node_cnt;
}
