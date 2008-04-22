/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2004--2007  Petter Urkedal <urkedal@nbi.dk>
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

#define CHI_SUBST_C
#include <cuex/subst.h>
#include <cucon/slink.h>
#include <cucon/pmap.h>
#include <cuex/ex.h>
#include <cuex/opn.h>
#include <cuex/oprdefs.h>
#include <cuex/algo.h>
#include <cuex/tvar.h>


/* Expensive debugging */
/* #define CHECK_SUBST 1 */


/* Variable Equivalence Classes
 * ============================ */

cuex_veqv_t
cuex_veqv_new(cuex_var_t var)
{
    cuex_veqv_t veqv = cu_gnew(struct cuex_veqv_s);
    veqv->qcode = cuex_varmeta_qcode(cuex_meta(var));
    veqv->var_link = cucon_slink_prepend_ptr(cucon_slink_empty(), var);
    veqv->value = NULL;
    return veqv;
}

cuex_veqv_t
cuex_veqv_new_copy(cuex_veqv_t src_veqv)
{
    cuex_veqv_t veqv = cu_gnew(struct cuex_veqv_s);
    veqv->qcode = src_veqv->qcode;
    veqv->var_link = src_veqv->var_link;
    veqv->value = src_veqv->value;
    return veqv;
}

/* Return a veqv whose variables are the union of variables of veqv0
 * and veqv1 with the head variable of veqv1 as the head variable,
 * and the value being either that of veqv0 or veqv1, whichever is
 * set.  It is an error if both have a value. */
cuex_veqv_t
cuex_veqv_unify(cuex_veqv_t veqv0, cuex_veqv_t veqv1, cuex_t new_value)
{
    cuex_var_t head_var;
    cucon_slink_t var_link;
    cucon_slink_t dst_var_link;
    struct cucon_pmap_s varset;
/*     cu_debug_assert(veqv0->qcode == veqv1->qcode); /\* XXX *\/ */
    if (veqv1->qcode == cuex_qcode_active_w)
	veqv1->qcode = veqv0->qcode;

    /* varset := vars(veqv1) */
    cucon_pmap_init(&varset);
    var_link = veqv1->var_link;
    do {
	cucon_pmap_insert_void(&varset, cucon_slink_get_ptr(var_link));
	var_link = cucon_slink_next(var_link);
    } while (var_link);

    /* vars(veqv1) := vars(veqv1) ∪ vars(veqv0).
     * Preserve head var of veqv1. */
    var_link = veqv0->var_link;
    head_var = cucon_slink_get_ptr(veqv1->var_link);
    dst_var_link = cucon_slink_next(veqv1->var_link);
    do {
	cuex_var_t var = cucon_slink_get_ptr(var_link);
	if (!cucon_pmap_find_mem(&varset, var))
	    dst_var_link = cucon_slink_prepend_ptr(dst_var_link, var);
	var_link = cucon_slink_next(var_link);
    } while (var_link);
    veqv1->var_link = cucon_slink_prepend_ptr(dst_var_link, head_var);

    veqv1->value = new_value;
    return veqv1;
}


/* Substitutions
 * ============= */

#ifdef CHECK_SUBST
static void cuexP_subst_check(cuex_subst_t subst);
#endif


/* Construction
 * ------------ */

CU_SINLINE void
cuexP_subst_cct(cuex_subst_t subst, cuex_subst_t src_subst,
		cuex_qcset_t qcset, cu_bool_t is_idem)
{
    subst->clone_cnt = 1;
    subst->shadow_access_cnt = 0;
    subst->shadowed = src_subst;
    subst->qcset = qcset;
    subst->is_idem = is_idem;
    cucon_pmap_init(&subst->var_to_veqv);
}

void
cuex_subst_cct(cuex_subst_t subst, cuex_qcset_t qcset)
{
    cuexP_subst_cct(subst, NULL, qcset, cu_true);
}

cuex_subst_t
cuex_subst_new(cuex_qcset_t qcset)
{
    cuex_subst_t subst = cu_gnew(struct cuex_subst_s);
    cuexP_subst_cct(subst, NULL, qcset, cu_true);
    return subst;
}

void
cuex_subst_cct_nonidem(cuex_subst_t subst, cuex_qcset_t qcset)
{
    cuexP_subst_cct(subst, NULL, qcset, cu_false);
}

cuex_subst_t
cuex_subst_new_nonidem(cuex_qcset_t qcset)
{
    cuex_subst_t subst = cu_gnew(struct cuex_subst_s);
    cuexP_subst_cct(subst, NULL, qcset, cu_false);
    return subst;
}

cuex_subst_t
cuex_subst_new_clone(cuex_subst_t src_subst, cuex_qcset_t qcset)
{
    cuex_subst_t subst = cu_gnew(struct cuex_subst_s);
    while (src_subst && cucon_pmap_size(&src_subst->var_to_veqv) == 0)
	src_subst = src_subst->shadowed;
    if (src_subst) {
	cuexP_subst_cct(subst, src_subst, qcset, src_subst->is_idem);
	++src_subst->clone_cnt;
	cu_debug_assert(qcset == src_subst->qcset);
    }
    else
	cuexP_subst_cct(subst, src_subst, qcset, cu_true);
    return subst;
}

static void cuexP_subst_merge(cuex_subst_t dst);
cuex_subst_t
cuex_subst_new_copy(cuex_subst_t src_subst)
{
    cuex_subst_t subst;
    if (!src_subst)
	return NULL;
    subst = cuex_subst_new_clone(src_subst, src_subst->qcset);
    while (subst->shadowed)
	cuexP_subst_merge(subst);
    return subst;
}

void
cuex_subst_flatten(cuex_subst_t subst)
{
    if (!subst)
	return;
    while (subst->shadowed)
	cuexP_subst_merge(subst);
}

void
cuex_subst_delete(cuex_subst_t subst)
{
    if (subst->shadowed)
	--subst->shadowed->clone_cnt;
}


/* Lookup
 * ------ */

cu_clos_def(cuexP_subst_merge_cb_move,
	    cu_prot(void, void const *var, void *veqv),
	    (cucon_pmap_t dst;))
{
    cu_clos_self(cuexP_subst_merge_cb_move);
    cucon_pmap_replace_ptr(self->dst, var, veqv);
}
cu_clos_def(cuexP_subst_merge_cb_copy,
	    cu_prot(void, void const *var0, void *veqv0),
	    (cucon_pmap_t dst;))
{
#define var ((cuex_var_t)var0)
#define veqv ((cuex_veqv_t)veqv0)
    cu_clos_self(cuexP_subst_merge_cb_copy);
    cucon_slink_t var_cur = veqv->var_link;
    if (var == cucon_slink_get_ptr(var_cur)) {
	cuex_veqv_t *slot;
	if (cucon_pmap_insert_mem(self->dst, var,
				sizeof(struct cuex_veqv_s), &slot)) {
	    veqv0 = cuex_veqv_new_copy(veqv);
	    *slot = veqv;
	    var_cur = cucon_slink_next(var_cur);
	    while (var_cur) {
		var0 = cucon_slink_get_ptr(var_cur);
		cucon_pmap_replace_ptr(self->dst, var, veqv);
		var_cur = cucon_slink_next(var_cur);
	    }
	}
    }
#undef var
#undef veqv
};  

static void
cuexP_subst_merge(cuex_subst_t dst)
{
    cuex_subst_t src = dst->shadowed;
    if (src->clone_cnt == 1) {
	if (cucon_pmap_size(&src->var_to_veqv)
	    > cucon_pmap_size(&dst->var_to_veqv))
	    cucon_pmap_swap(&src->var_to_veqv, &dst->var_to_veqv);
	cuexP_subst_merge_cb_move_t cb;
	cb.dst = &dst->var_to_veqv;
	cucon_pmap_iter_ptr(&src->var_to_veqv,
			    cuexP_subst_merge_cb_move_prep(&cb));
    }
    else {
	cuexP_subst_merge_cb_copy_t cb;
	cb.dst = &dst->var_to_veqv;
	cucon_pmap_iter_ptr(&src->var_to_veqv,
			    cuexP_subst_merge_cb_copy_prep(&cb));
    }
    dst->shadowed = src->shadowed;
#ifdef CHECK_SUBST
    cuexP_subst_check(dst);
#endif
}

size_t
cuex_subst_size(cuex_subst_t subst)
{
    size_t sum = 0;
    while (subst) {
	sum += cucon_pmap_size(&subst->var_to_veqv);
	subst = subst->shadowed;
    }
    return sum;
}

cuex_veqv_t
cuex_subst_cref(cuex_subst_t subst, cuex_var_t var)
{
    cuex_veqv_t veqv;
    if (!subst)
	return NULL;
    for (;;) {
	cuex_subst_t subst_sh;
	veqv = cucon_pmap_find_ptr(&subst->var_to_veqv, var);
	if (veqv)
	    return veqv;
	subst_sh = subst->shadowed;
	if (!subst_sh)
	    return NULL;
	if (++subst->shadow_access_cnt
	    > 4*cucon_pmap_size(&subst_sh->var_to_veqv))
	    cuexP_subst_merge(subst);
	else
	    subst = subst_sh;
    }
    return NULL;
}

cuex_veqv_t
cuex_subst_mref(cuex_subst_t subst, cuex_var_t var)
{
    cuex_veqv_t *slot;
    cu_debug_assert(subst);
    if (cucon_pmap_insert_mem(&subst->var_to_veqv, var,
			      sizeof(cuex_veqv_t), &slot)) {
	cuex_subst_t shadowed = subst->shadowed;
	cuex_veqv_t veqv_src;
	if (shadowed && (veqv_src = cuex_subst_cref(shadowed, var)) != NULL) {
	    cucon_slink_t var_link;
	    *slot = cuex_veqv_new_copy(veqv_src);
	    var_link = veqv_src->var_link;
	    do {
		cuex_var_t var0 = cucon_slink_get_ptr(var_link);
		if (var0 != var)
		    cucon_pmap_replace_ptr(&subst->var_to_veqv, var0, *slot);
		var_link = cucon_slink_next(var_link);
	    } while (var_link);
	}
	else
	    *slot = cuex_veqv_new(var);
    }
    cu_debug_assert(*slot);
#ifdef CHECK_SUBST
    cuexP_subst_check(subst);
#endif
    return *slot;
}

cu_clop_def(subst_is_identity_cb, cu_bool_t, void const *var, void *veqv)
{
#define veqv ((cuex_veqv_t)veqv)
    return veqv->value == NULL && cucon_slink_next(veqv->var_link) == NULL;
#undef veqv
}

cu_bool_t
cuex_subst_is_identity(cuex_subst_t subst)
{
    while (subst)
	if (!cucon_pmap_conj_ptr(&subst->var_to_veqv, subst_is_identity_cb))
	    return cu_false;
    return cu_true;
}


/* Unification
 * ----------- */

static cu_bool_t
cuex_subst_ex_contains_veqv(cuex_subst_t subst, cuex_t ex, cuex_veqv_t veqv)
{
    cuex_meta_t meta;
tailrec:
    meta = cuex_meta(ex);
    switch (cuex_meta_kind(meta)) {
	cu_rank_t n;
	cu_rank_t i;
	cuex_veqv_t subveqv;
    case cuex_meta_kind_opr:
	n = cuex_opr_r(meta);
	if (n > 0) {
	    --n;
	    for (i = 0; i < n; ++i) {
		if (cuex_subst_ex_contains_veqv(
			subst, cuex_opn_at(cuex_opn_from_ex(ex), i), veqv))
		    return cu_true;
	    }
	    ex = cuex_opn_at(cuex_opn_from_ex(ex), i);
	    goto tailrec;
	}
	return cu_false;
    default:
	if (cuex_is_varmeta(meta)) {
	    subveqv = cuex_subst_cref(subst, cuex_var_from_ex(ex));
	    if (subveqv == veqv)
		return cu_true;
	    else if (subveqv && subveqv->value)
		return cuex_subst_ex_contains_veqv(subst, subveqv->value,
						   veqv);
	    else
		return cu_false;
	}
	else
	    return cu_false;
    }
}

static cu_bool_t
occur_check_and_set_var(cuex_subst_t subst, cuex_veqv_t veqv, cuex_t ex)
{
    if (subst->is_idem && cuex_subst_ex_contains_veqv(subst, ex, veqv))
	return cu_false;
    else {
	veqv->value = ex;
	return cu_true;
    }
}

cuex_t
cuex_subst_unify_aux(cuex_subst_t subst, cuex_t ex0, cuex_t ex1,
		     cu_clop(aux_unify, cuex_t, cuex_subst_t, cuex_t, cuex_t))
{
    cuex_meta_t meta0, meta1;
    cuex_qcset_t qcset = subst->qcset;
    cu_debug_assert(subst->clone_cnt == 1);
    if (ex0 == ex1)
	return ex0;
    meta0 = cuex_meta(ex0);
    meta1 = cuex_meta(ex1);
    if (cuex_is_varmeta(meta0)
	&& cuex_qcset_contains(qcset, cuex_varmeta_qcode(meta0))) {
	cuex_veqv_t vq0 = cuex_subst_mref(subst, cuex_var_from_ex(ex0));
	if (cuex_is_varmeta(meta1)
	    && cuex_qcset_contains(qcset, cuex_varmeta_qcode(meta1))) {
	    /* We have two variables */
	    cuex_veqv_t vq1 = cuex_subst_mref(subst, cuex_var_from_ex(ex1));
	    cuex_veqv_t vq2;
	    cucon_slink_t var_cur;
	    cuex_t value;
	    if (vq0 == vq1)
		return ex0; /* or head var or value when present? */

	    /* Require that the result subst will be idempotent. */
	    if (subst->is_idem &&
		((vq0->value
		  && cuex_subst_ex_contains_veqv(subst, vq0->value, vq1)) ||
		 (vq1->value
		  && cuex_subst_ex_contains_veqv(subst, vq1->value, vq0))))
		return NULL;

	    if (vq0->value) {
		if (vq1->value) {
		    value = cuex_subst_unify_aux(subst, vq0->value, vq1->value,
						 aux_unify);
		    if (!value)
			return NULL;
		} else
		    value = vq0->value;
	    } else
		value = vq1->value;
	    vq2 = cuex_veqv_unify(vq0, vq1, value);

	    /* Update var_to_veqv for variables in vq2. */
	    var_cur = vq2->var_link;
	    do {
		cuex_var_t var = cucon_slink_get_ptr(var_cur);
		if (cuex_subst_cref(subst, var) != vq2)
		    cucon_pmap_replace_ptr(&subst->var_to_veqv, var, vq2);
		var_cur = cucon_slink_next(var_cur);
	    } while (var_cur);

	    if (value)
		return value;
	    else
		return ex0;
	}
	else if (vq0->value) {
	    /* We have one variable, ex0 */
	    cuex_t value;
	    value = cuex_subst_unify_aux(subst, vq0->value, ex1, aux_unify);
	    if (!value)
		return NULL;
	    vq0->value = value;
	    return value;
	}
	else if (occur_check_and_set_var(subst, vq0, ex1))
	    return ex0;
	else
	    return NULL;
    }
    else if (cuex_is_varmeta(meta1)
	     && cuex_qcset_contains(qcset, cuex_varmeta_qcode(meta1))) {
	/* We have one variable, ex1 */
	cuex_veqv_t vq1 = cuex_subst_mref(subst, cuex_var_from_ex(ex1));
	if (vq1->value) {
	    cuex_t value;
	    value = cuex_subst_unify_aux(subst, ex0, vq1->value, aux_unify);
	    if (!value)
		return NULL;
	    vq1->value = value;
	    return value;
	}
	else if (occur_check_and_set_var(subst, vq1, ex0))
	    return ex1;
	else
	    return NULL;
    }

    if (meta0 == meta1 && cuex_meta_is_opr(meta0)) {
	cu_rank_t i, r = cuex_opr_r(meta0);
	cuex_t *arr = cu_salloc(sizeof(cuex_t)*r);
	for (i = 0; i < r; ++i) {
	    arr[i] = cuex_subst_unify_aux(subst,
				cuex_opn_at(cuex_opn_from_ex(ex0), i),
				cuex_opn_at(cuex_opn_from_ex(ex1), i),
				aux_unify);
	    if (!arr[i])
		goto normal_fail;
	}
	return cuex_opn_by_arr(meta0, arr);
    }
normal_fail:
    if (cu_clop_is_null(aux_unify))
	return NULL;
    else
	return cu_call(aux_unify, subst, ex0, ex1);
}

cu_bool_t
cuex_subst_unify(cuex_subst_t subst, cuex_t ex0, cuex_t ex1)
{
    return !!cuex_subst_unify_aux(subst, ex0, ex1, cu_clop_null);
}


/* -- cuex_subst_block
 * -- cuex_subst_unblock */

void
cuex_subst_block(cuex_subst_t subst, cuex_var_t v)
{
    cuex_veqv_t vq = cuex_subst_mref(subst, v);
    if (vq->value) {
	cu_debug_assert(cuex_meta(vq->value) == CUEX_O1_SUBST_BLOCK);
	return;
    }
    vq->value = cuex_opn_to_ex(
	cuex_opn(CUEX_O1_SUBST_BLOCK, cuex_var_to_ex(cuex_var_new_n())));
}

void
cuex_subst_unblock(cuex_subst_t subst, cuex_var_t v)
{
    cuex_veqv_t vq = cuex_subst_mref(subst, v);
    cu_debug_assert(vq->value &&
		     cuex_meta(vq->value) == CUEX_O1_SUBST_BLOCK);
    vq->value = NULL;
}


/* -- cuex_subst_unblock_all */

cu_clos_def(unblock_all_cb0,
	    cu_prot(void, void const *var, void *veqv),
	    ( struct cucon_pmap_s visited; ) )
{
#define veqv ((cuex_veqv_t)veqv)
    cu_clos_self(unblock_all_cb0);
    if (!cucon_pmap_insert_void(&self->visited, (cuex_var_t)var))
	return;
    if (veqv->value && cuex_meta(veqv->value) == CUEX_O1_SUBST_BLOCK)
	veqv->value = NULL;
#undef veqv
}
cu_clos_def(unblock_all_cb1,
	    cu_prot(void, void const *var, void *veqv),
	    ( cucon_pmap_t visited;
	      cuex_subst_t subst; ) )
{
#define veqv ((cuex_veqv_t)veqv)
    cu_clos_self(unblock_all_cb1);
    if (!cucon_pmap_insert_void(self->visited, var))
	return;
    if (veqv->value && cuex_meta(veqv->value) == CUEX_O1_SUBST_BLOCK)
	cuex_subst_mref(self->subst, (cuex_var_t)var)->value = NULL;
#undef veqv
}
void
cuex_subst_unblock_all(cuex_subst_t subst)
{
    if (subst) {
	unblock_all_cb0_t cb0;
	cucon_pmap_init(&cb0.visited);
	cucon_pmap_iter_ptr(&subst->var_to_veqv, unblock_all_cb0_prep(&cb0));
	subst = subst->shadowed;
	while (subst) {
	    unblock_all_cb1_t cb1;
	    cb1.subst = subst;
	    cb1.visited = &cb0.visited;
	    cucon_pmap_iter_ptr(&subst->var_to_veqv,
				unblock_all_cb1_prep(&cb1));
	    subst = subst->shadowed;
	}
    }
}


/* -- cuex_subst_freshen_and_block_vars_in */

cu_clos_def(subst_freshen_var_cb,
	    cu_prot(cu_bool_t, cuex_var_t var),
	    (cuex_subst_t subst;))
{
    cu_clos_self(subst_freshen_var_cb);
    cuex_subst_t subst = self->subst;
    cuex_meta_t var_meta = cuex_meta(var);
    if (cuex_is_varmeta(var_meta)
	&& cuex_qcset_contains(self->subst->qcset,
			       cuex_varmeta_qcode(var_meta))
	&& cuex_subst_lookup(self->subst, var) == NULL) {
	cuex_qcode_t qcode = cuex_var_qcode(var);
	cuex_var_t new_var = cuex_var_new(qcode);
	cuex_subst_block(subst, new_var);
	if (!cuex_subst_unify(subst, cuex_var_to_ex(var),
			      cuex_var_to_ex(new_var)))
	    cu_debug_unreachable();
    }
    return cu_true;
}
void
cuex_subst_freshen_and_block_vars_in(cuex_subst_t subst, cuex_t ex)
{
    subst_freshen_var_cb_t cb;
    cb.subst = subst;
    cuex_depth_conj_vars(ex, subst_freshen_var_cb_prep(&cb));
}


/* -- cuex_subst_lookup */

cuex_t
cuex_subst_lookup(cuex_subst_t subst, cuex_var_t var)
{
    cuex_veqv_t vq = cuex_subst_cref(subst, var);
    if (!vq)
	return NULL;
    else if (vq->value && cuex_meta(vq->value) != CUEX_O1_SUBST_BLOCK)
	return vq->value;
    else if (cucon_slink_next(vq->var_link))
	return cucon_slink_get_ptr(vq->var_link);
    else
	return NULL;
}


/* -- cuex_subst_apply */

static cuex_t cuexP_subst_apply(cuex_subst_t, cuex_t);
cu_clos_def(subst_apply_cb,
	    cu_prot(cuex_opn_t, cuex_opn_t node),
	( cuex_subst_t subst; ))
{
    cu_clos_self(subst_apply_cb);
    cuex_meta_t meta = cuex_meta(node);
    CUEX_OPN_TRAN(meta, node, subex, cuexP_subst_apply(self->subst, subex));
    return node;
}

static cuex_t
cuexP_subst_apply(cuex_subst_t subst, cuex_t ex)
{
    cuex_meta_t meta;
tailcall:
    while (cuex_is_varmeta(meta = cuex_meta(ex))) {
	cuex_veqv_t vq;
	vq = cuex_subst_cref(subst, cuex_var_from_ex(ex));
	if (vq) {
	    if (vq->value) {
		if (cuex_meta(vq->value) == CUEX_O1_SUBST_BLOCK)
		    return ex;
		ex = vq->value;
		goto tailcall;
	    }
	    else
		return cucon_slink_get_ptr(vq->var_link);
	}
	return ex;
    }
    if (cuex_meta_is_opr(meta)) {
#if 0
	if (cuex_opr_is_aci(meta)) {
	    subst_apply_cb_t cb;
	    cb.subst = subst;
	    return cuex_aci_tran(meta, ex, subst_apply_cb_prep(&cb));
	}
	else
#endif
	    CUEX_OPN_TRAN(meta, ex, subex, cuexP_subst_apply(subst, subex));
    }
    return ex;
}

cuex_t
cuex_subst_apply(cuex_subst_t subst, cuex_t ex)
{
    if (subst) {
	if (subst->is_idem)
	    return cuexP_subst_apply(subst, ex);
	else
	    cu_bugf("Apply is undefined for non-idempotent substitution.");
    }
    else
	return ex;
}


/* -- cuex_subst_update_tvar_types */

cu_clos_def(subst_update_tvar_types_cb,
	    cu_prot(cu_bool_t, cuex_var_t var),
	( cuex_subst_t subst; ))
{
    cu_clos_self(subst_update_tvar_types_cb);
    if (cuex_is_tvar(var)) {
	cuex_t t = cuex_tvar_type(cuex_tvar_from_var(var));
	if (t)
	    cuex_tvar_set_type(cuex_tvar_from_var(var),
			       cuex_subst_apply(self->subst, t));
    }
    return cu_true;
}

void
cuex_subst_update_tvar_types(cuex_subst_t subst, cuex_t ex)
{
    subst_update_tvar_types_cb_t cb;
    cb.subst = subst;
    cuex_depth_conj_vars(ex, subst_update_tvar_types_cb_prep(&cb));
}


/* -- cuex_subst_expand */

static cuex_t
cuexP_subst_expand(cuex_subst_t subst, cuex_t ex)
{
    cuex_meta_t meta;
    if (cuex_is_varmeta(meta = cuex_meta(ex))) {
	cuex_veqv_t vq = cuex_subst_cref(subst, cuex_var_from_ex(ex));
	if (vq) {
	    if (vq->value) {
		if (cuex_meta(vq->value) == CUEX_O1_SUBST_BLOCK)
		    return ex;
		else
		    return vq->value;
	    }
	    else
		return cucon_slink_get_ptr(vq->var_link);
	}
	else
	    return ex;
    }
    if (cuex_meta_is_opr(meta))
	CUEX_OPN_TRAN(meta, ex, subex, cuexP_subst_apply(subst, subex));
    return ex;
}

cuex_t
cuex_subst_expand(cuex_subst_t subst, cuex_t ex)
{
    if (subst)
	return cuexP_subst_expand(subst, ex);
    else
	return ex;
}


/* -- cuex_subst_collect_veqvset */

void
cuex_subst_collect_veqvset(cuex_subst_t subst, cuex_t ex, cucon_pmap_t veqvset)
{
    cuex_meta_t meta;
recurse:
    meta = cuex_meta(ex);
    switch (cuex_meta_kind(meta)) {
	    cu_rank_t i, n;
	case cuex_meta_kind_opr:
	    n = cuex_opn_r(cuex_opn_from_ex(ex));
	    if (n == 0)
		return;
	    for (i = 1; i < n; ++i)
		cuex_subst_collect_veqvset(
		    subst, cuex_opn_at(cuex_opn_from_ex(ex), i), veqvset);
	    ex = cuex_opn_at(cuex_opn_from_ex(ex), 0);
	    goto recurse;
	default:
	    if (cuex_is_varmeta(meta)
		&& cuex_qcset_contains(subst->qcset, cuex_varmeta_qcode(meta)))
		cucon_pmap_insert_void(veqvset,
				       cuex_subst_mref(subst,
						       cuex_var_from_ex(ex)));
	    return;
    }
}


/* -- cuex_subst_free_vars_conj
 * -- cuex_subst_free_vars_insert
 * -- cuex_subst_free_vars_erase */

cu_bool_t
cuex_subst_free_vars_conj(cuex_subst_t subst, cuex_t ex,
			  cu_clop(cb, cu_bool_t, cuex_var_t))
{
    cuex_meta_t meta;
recurse:
    meta = cuex_meta(ex);
    switch (cuex_meta_kind(meta)) {
	    cu_rank_t i, n;
	case cuex_meta_kind_opr:
	    n = cuex_opn_r(cuex_opn_from_ex(ex));
	    if (n == 0)
		return cu_true;
	    for (i = 1; i < n; ++i)
		if (!cuex_subst_free_vars_conj(
			subst, cuex_opn_at(cuex_opn_from_ex(ex), i), cb))
		    return cu_false;
	    ex = cuex_opn_at(cuex_opn_from_ex(ex), 0);
	    goto recurse;
	default:
	    if (cuex_is_varmeta(meta)
		&& cuex_qcset_contains(subst->qcset,
				       cuex_varmeta_qcode(meta))) {
		cuex_veqv_t veqv;
		veqv = cuex_subst_cref(subst, cuex_var_from_ex(ex));
		if (veqv) {
		    if (veqv->value &&
			cuex_meta(veqv->value) != CUEX_O1_SUBST_BLOCK) {
			ex = veqv->value;
			goto recurse;
		    }
		    else {
			cuex_var_t rv = cucon_slink_get_ptr(veqv->var_link);
			return cu_call(cb, rv);
		    }
		}
		else
		    return cu_call(cb, cuex_var_from_ex(ex));
	    }
	    else
		return cu_true;
    }
}

void
cuex_subst_free_vars_insert(cuex_subst_t subst, cuex_t ex, cucon_pset_t accu)
{
    cuex_pset_curried_insert_ex_t cb;
    cb.accu = accu;
    cuex_subst_free_vars_conj(subst, ex,
			      (cu_clop(, cu_bool_t, cuex_var_t))
				    cuex_pset_curried_insert_ex_prep(&cb));
}

void
cuex_subst_free_vars_erase(cuex_subst_t subst, cuex_t ex, cucon_pset_t accu)
{
    cuex_pset_curried_erase_ex_t cb;
    cb.accu = accu;
    cuex_subst_free_vars_conj(subst, ex,
			      (cu_clop(, cu_bool_t, cuex_var_t))
				    cuex_pset_curried_erase_ex_prep(&cb));
}


/* -- cuex_subst_dump */

cu_clos_def(cuex_subst_dump_cb,
	    cu_prot(void, void const *var, void *veqv),
	    (FILE *file;))
{
#define var ((cuex_var_t)var)
#define veqv ((cuex_veqv_t)veqv)
    cu_clos_self(cuex_subst_dump_cb);
    cucon_slink_t var_cur;
    FILE *file = self->file;
    if (cucon_slink_get_ptr(veqv->var_link) != var)
	return;
    cu_fprintf(file, "\t[%!", cuex_var_to_ex(var));
    for (var_cur = cucon_slink_next(veqv->var_link); var_cur;
	 var_cur = cucon_slink_next(var_cur))
	cu_fprintf(file, ", %!", cuex_var_to_ex(cucon_slink_get_ptr(var_cur)));
    fprintf(file, "]");
    if (veqv->value) {
	if (cuex_meta(veqv->value) == CUEX_O1_SUBST_BLOCK)
	    fprintf(file, " = __blocked_%p", veqv->value);
	else
	    cu_fprintf(file, " = %!", veqv->value);
    }
    fputc('\n', file);
#undef var
#undef veqv
}
void
cuex_subst_dump(cuex_subst_t subst, FILE *file)
{
    cuex_subst_dump_cb_t cb;
    cu_clop(cb_clop, void, void const *, void *)
	= cuex_subst_dump_cb_prep(&cb);
    cb.file = file;
    fprintf(file, "subst @ %p (qcset = %x, %s)\n", subst,
	    subst->qcset, subst->is_idem? "idem" : "nonidem");
    while (subst) {
	fprintf(file, "    frame @ %p\n", subst);
	cucon_pmap_iter_ptr(&subst->var_to_veqv, cb_clop);
	subst = subst->shadowed;
    }
}


/* -- cuex_subst_iter_veqv */

cu_clos_def(cuex_subst_iter_cb,
	    cu_prot(void, void const *var, void *veqv),
	    (cu_clop(cb, void, cuex_veqv_t); struct cucon_pmap_s visited;))
{
#define veqv ((cuex_veqv_t)veqv)
    cu_clos_self(cuex_subst_iter_cb);
    if (cucon_pmap_insert_void(&self->visited, var)
	&& var == cucon_slink_get_ptr(veqv->var_link)
	&& (veqv->value || cucon_slink_next(veqv->var_link)))
	cu_call(self->cb, veqv);
#undef veqv
}
void
cuex_subst_iter_veqv(cuex_subst_t subst, cu_clop(cb, void, cuex_veqv_t))
{
    cuex_subst_iter_cb_t cb0;
    cu_clop(cb0_clop, void, void const *, void *)
	= cuex_subst_iter_cb_prep(&cb0);
    cb0.cb = cb;
    cucon_pmap_init(&cb0.visited);
    while (subst) {
	cucon_pmap_iter_ptr(&subst->var_to_veqv, cb0_clop);
	subst = subst->shadowed;
    }
}


/* -- cuex_subst_iter_veqv_cache
 * -- cuex_subst_iter_veqv_cache_sub */

cu_clos_def(subst_iter_veqv_cache_cb,
	    cu_prot(void, void const *var, void *veqv),
    (	cu_clop(f, void, cuex_veqv_t veqv, void *cache, void *sub_data);
	struct cucon_pmap_s cache;
	size_t cache_size;
	cuex_subst_t sig;
    ))
{
#define veqv ((cuex_veqv_t)veqv)
    cu_clos_self(subst_iter_veqv_cache_cb);
    if (veqv->value || cucon_slink_next(veqv->var_link)) {
	void *cache;
	if (cucon_pmap_insert_mem(&self->cache, veqv,
				  self->cache_size, &cache))
	    cu_call(self->f, veqv, cache, self);
    }
#undef veqv
}

void
cuex_subst_iter_veqv_cache(cuex_subst_t sig, size_t cache_size,
			   cu_clop(f, void, cuex_veqv_t veqv, void *cache,
					    void *sub_data))
{
    subst_iter_veqv_cache_cb_t cb0;
    cu_clop(cb0_clop, void, void const *, void *)
	= subst_iter_veqv_cache_cb_prep(&cb0);
    if (!sig) return;
    cb0.f = f;
    cucon_pmap_init(&cb0.cache);
    cb0.cache_size = cache_size;
    cb0.sig = sig;
    do {
	cucon_pmap_iter_ptr(&sig->var_to_veqv, cb0_clop);
	sig = sig->shadowed;
    } while (sig);
}

void *
cuex_subst_iter_veqv_cache_sub(void *sub_data, cuex_t var)
{
#define self ((subst_iter_veqv_cache_cb_t *)sub_data)
    cuex_veqv_t veqv = cuex_subst_cref(self->sig, var);
    if (veqv && (veqv->value || cucon_slink_next(veqv->var_link))) {
	void *cache;
	if (cucon_pmap_insert_mem(&self->cache, veqv,
				  self->cache_size, &cache))
	    cu_call(self->f, veqv, cache, self);
	return cache;
    }
    else
	return NULL;
#undef self
}


/* -- cuex_subst_tran_cache
 * -- cuex_subst_tran_cache_sub */

/* If non-zero, use a clone of the source subtitution for transformed
 * substitutions, else use a fresh substitution. */
#define CUEXOPT_SUBST_TRAN_CLONE 1

cu_clos_def(subst_tran_cache_cb,
	    cu_prot(void, void const *var, void *veqv),
    (	cu_clop(f, cuex_t, cuex_t value, void *cache, void *sub_data);
	struct cucon_pmap_s cache;
	size_t cache_size;
	cuex_subst_t sig;
	cuex_subst_t tau;
    ))
{
#define veqv ((cuex_veqv_t)veqv)
#define var ((cuex_t)var)
    cu_clos_self(subst_tran_cache_cb);
    if (veqv->value) {
	void *cache;
	if (cucon_pmap_insert_mem(&self->cache, veqv,
				  self->cache_size, &cache)) {
	    cuex_t e = cu_call(self->f, veqv->value, cache, self);
#if CUEXOPT_SUBST_TRAN_CLONE
	    if (veqv->value != e) {
		cuex_veqv_t new_veqv = cuex_subst_mref(self->tau, var);
		new_veqv->value = e;
	    }
#else
	    if (e) {
		cuex_veqv_t new_veqv = cuex_subst_mref(self->tau, var);
		new_veqv->value = e;
	    }
#endif
	}
    }
#undef var
#undef veqv
}

cuex_subst_t
cuex_subst_tran_cache(cuex_subst_t sig, size_t cache_size,
		      cu_clop(f, cuex_t, cuex_t value, void *cache,
					 void *sub_data))
{
    subst_tran_cache_cb_t cb;
    cu_clop(cb_clop, void, void const *, void *)
	= subst_tran_cache_cb_prep(&cb);
    if (!sig) return NULL;
    cb.f = f;
    cucon_pmap_init(&cb.cache);
    cb.cache_size = cache_size;
    cb.sig = sig;
#if CUEXOPT_SUBST_TRAN_CLONE
    cb.tau = cuex_subst_new_clone(sig, cuex_subst_active_qcset(sig));
#else
    cb.tau = cuex_subst_new(cuex_subst_active_qcset(sig));
#endif
    do {
	cucon_pmap_iter_ptr(&sig->var_to_veqv, cb_clop);
	sig = sig->shadowed;
    } while (sig);
    return cb.tau;
}

void *
cuex_subst_tran_cache_sub(void *sub_data, cuex_t var)
{
#define self ((subst_tran_cache_cb_t *)sub_data)
    cuex_veqv_t veqv = cuex_subst_cref(self->sig, var);
    if (veqv && veqv->value) {
	void *cache;
	if (cucon_pmap_insert_mem(&self->cache, veqv,
				  self->cache_size, &cache)) {
	    cuex_t e = cu_call(self->f, veqv->value, cache, self);
#if CUEXOPT_SUBST_TRAN_CLONE
	    if (veqv->value != e) {
		cuex_veqv_t new_veqv = cuex_subst_mref(self->tau, var);
		new_veqv->value = e;
	    }
#else
	    if (e) {
		cuex_veqv_t new_veqv = cuex_subst_mref(self->tau, var);
		new_veqv->value = e;
	    }
#endif
	}
	return cache;
    }
    else
	return NULL;
#undef self
}



/* -- cuex_subst_print */

cu_clos_def(cuex_subst_print_cb,
	    cu_prot(void, cuex_veqv_t veqv),
	    ( FILE *file;
	      char const *sep;
	      cu_bool_t is_first; ))
{
    cu_clos_self(cuex_subst_print_cb);
    cucon_slink_t var_cur;
    char *sep;
    if (self->is_first)
	self->is_first = cu_false;
    else
	fputs(self->sep, self->file);
    sep = "";
    for (var_cur = veqv->var_link; var_cur;
	 var_cur = cucon_slink_next(var_cur)) {
	cu_fprintf(self->file, "%s%!",
		   sep, cuex_var_to_ex(cucon_slink_get_ptr(var_cur)));
	sep = " = ";
    }
    if (veqv->value) {
	if (cuex_meta(veqv->value) == CUEX_O1_SUBST_BLOCK)
	    fprintf(self->file, " __blocked_%p", veqv->value);
	else
	    cu_fprintf(self->file, " ↦ %!", veqv->value);
    }
}

void
cuex_subst_print(cuex_subst_t subst, FILE *file, char const *sep)
{
    cuex_subst_print_cb_t cb;
    cb.file = file;
    cb.sep = sep;
    cb.is_first = cu_true;
    cuex_subst_iter_veqv(subst, cuex_subst_print_cb_prep(&cb));
}


/* -- cuexP_subst_check */

#ifdef CHECK_SUBST
static void
cuexP_subst_check_ex(cuex_subst_t subst, cuex_t ex, cucon_pmap_t seen)
{
    if (cuex_is_opn(ex)) {
	cu_rank_t n = cuex_opn_r(cuex_opn_from_ex(ex));
	cu_rank_t i;
	for (i = 0; i < n; ++i)
	    cuexP_subst_check_ex(subst,
				cuex_opn_at(cuex_opn_from_ex(ex), i),
				seen);
    }
    else if (cuex_is_var(ex)) {
	cuex_veqv_t veqv = cuex_subst_cref(subst, cuex_var_from_ex(ex));
	if (veqv) {
	    if (!cucon_pmap_insert_void(seen, veqv))
		cu_bugf("Detected loopy substitution.");
	    if (veqv->value)
		cuexP_subst_check_ex(subst, veqv->value, seen);
	    cucon_pmap_erase(seen, veqv);
	}
    }
}

cu_clos_def(subst_check_cb,
	    cu_prot(void, void const *var, void *veqv),
    ( cuex_subst_t subst; ) )
{
#define var ((cuex_var_t)var)
#define veqv ((cuex_veqv_t)veqv)
    cu_clos_self(subst_check_cb);
    cuex_subst_t subst = self->subst;
    cucon_slink_t var_cur;

    /* Check that 'var' is in 'veqv'. */
    var_cur = veqv->var_link;
    assert(var_cur);
    do {
	if (cucon_slink_get_ptr(var_cur) == var)
	    break;
	var_cur = cucon_slink_next(var_cur);
    } while (var_cur);
    assert(var_cur);

    var_cur = veqv->var_link;
    if (var == cucon_slink_get_ptr(var_cur)) {
	/* Check that all vars in 'veqv' maps to 'veqv'. */
	do {
	    cuex_var_t v = cucon_slink_get_ptr(var_cur);
	    assert(cuex_subst_cref(subst, v) == veqv);
	    var_cur = cucon_slink_next(var_cur);
	} while (var_cur);

	/* Occur check. */
	if (veqv->value) {
	    struct cucon_pmap_s seen;
	    cucon_pmap_init(&seen);
	    cucon_pmap_insert_void(&seen, veqv);
	    cuexP_subst_check_ex(subst, veqv->value, &seen);
	}
    }
#undef var
#undef veqv
}

static void
cuexP_subst_check(cuex_subst_t subst)
{
    if (subst) {
	subst_check_cb_t cb;
	cb.subst = subst;
	cucon_pmap_iter_ptr(&subst->var_to_veqv, subst_check_cb_prep(&cb));
    }
}
#endif
