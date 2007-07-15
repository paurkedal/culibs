/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2005--2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cudyn/type.h>
#include <cudyn/misc.h>
#include <cucon/stack.h>
#include <cuex/oprdefs.h>
#include <cuex/aci.h>
#include <cuex/algo.h>
#include <cuex/type.h>
#include <cuex/sig.h>


static cu_mutex_t type_glck = CU_MUTEX_INITIALISER;


/* Pointer Types
 * ============= */

/* May consider to add the deref type to the structure, but this means it must
 * be computed in case of cudyn_ptrtype_from_ex. */

cudyn_ptrtype_t
cudyn_ptrtype_from_ex(cuex_t ex)
{
    cudyn_ptrtype_t type;
    cudyn_hctem_decl(cudyn_ptrtype, tem);

    cudyn_hctem_init(cudyn_ptrtype, tem);
    type = cudyn_hctem_get(cudyn_ptrtype, tem);
    /*cu_debug_assert(cuex_meta(ex) == CUEX_O1_PTR_TO);*/
    cudynP_hctype_cct_hcs(cu_to2(cudyn_hctype, cudyn_inltype, type), ex,
			  cudyn_typekind_ptrtype, sizeof(void *));
    cu_to(cudyn_inltype, type)->layout = (AO_t)cucon_layout_ptr;
    cu_to(cudyn_inltype, type)->ffitype = (AO_t)&ffi_type_pointer;
    return cudyn_hctem_new(cudyn_ptrtype, tem);
}

cudyn_ptrtype_t
cudyn_ptrtype(cuex_t deref)
{
    cuex_t ex;
    cudyn_ptrtype_t type;
    cudyn_hctem_decl(cudyn_ptrtype, tem);

    cudyn_hctem_init(cudyn_ptrtype, tem);
    type = cudyn_hctem_get(cudyn_ptrtype, tem);
    ex = cuex_o1_ptr_to(cudyn_type_as_expr(deref));
    cudynP_hctype_cct_hcs(cu_to2(cudyn_hctype, cudyn_inltype, type), ex,
			  cudyn_typekind_ptrtype, sizeof(void *));
    cu_to(cudyn_inltype, type)->layout = (AO_t)cucon_layout_ptr;
    cu_to(cudyn_inltype, type)->ffitype = (AO_t)&ffi_type_pointer;
    return cudyn_hctem_new(cudyn_ptrtype, tem);
}


/* Elementary Types
 * ================ */

cudyn_elmtype_t
cudyn_elmtype_new(cudyn_typekind_t kind,
		  cu_offset_t size, cu_offset_t alignment, ffi_type *ffitype)
{
    cu_offset_t bitoffset;
    cudyn_elmtype_t t = cudyn_onew(cudyn_elmtype);
    cu_offset_t wsize;
    wsize = (size + sizeof(cu_word_t) - 1)/sizeof(cu_word_t)*sizeof(cu_word_t);
    cudynP_hctype_cct_hcs(cu_to2(cudyn_hctype, cudyn_inltype, t), NULL,
			  kind, wsize);
    cu_to(cudyn_inltype, t)->layout
	= (AO_t)cucon_layout_pack_bits(NULL, size*8, alignment*8, &bitoffset);
    cu_to(cudyn_inltype, t)->ffitype = (AO_t)ffitype;
    return t;
}


/* Array Types
 * =========== */

cu_clop_def(arrtype_cct_glck, void, void *t)
{
#define t ((cudyn_arrtype_t)t)
    cu_offset_t bitoffset;
    size_t elt_bitsize, elt_bitalign;
    size_t arr_bitsize, arr_bitalign;
    cucon_layout_t lyo, sub_lyo;
    cuex_t ex;

    ex = cudyn_arrtype_to_type(t)->as_expr;
    cu_debug_assert(cuex_meta(ex) == CUEX_O2_GEXPT);
    if (!cudyn_is_int(cuex_opn_at(ex, 1)))
	return;
    t->elt_type = cudyn_type_glck(cuex_opn_at(ex, 0));
    if (!t->elt_type)
	return;
    t->elt_cnt = cudyn_to_int(cuex_opn_at(ex, 1));
    sub_lyo = cudyn_type_layout(t->elt_type);
    elt_bitsize = cucon_layout_bitsize(sub_lyo);
    elt_bitalign = cucon_layout_bitalign(sub_lyo);
    arr_bitsize = elt_bitsize*t->elt_cnt;
    arr_bitalign = elt_bitalign;
    lyo = cucon_layout_pack_bits(NULL, arr_bitsize, arr_bitalign, &bitoffset);
    cudynP_hctype_cct_hcs(cu_to2(cudyn_hctype, cudyn_inltype, t), ex,
			  cudyn_typekind_arrtype, cucon_layout_size(lyo));
    AO_store_release_write(&cu_to(cudyn_inltype, t)->layout, (AO_t)lyo);
#undef t
}

cudyn_arrtype_t
cudyn_arrtype_glck(cuex_t ex)
{
    cudyn_arrtype_t t;
    t = cudyn_hnew_extra(cudyn_arrtype, sizeof(cuex_t), &ex, arrtype_cct_glck);
    if (!cu_to(cudyn_inltype, t)->layout)
	return NULL;
    return t;
}

cudyn_arrtype_t
cudyn_arrtype(cudyn_type_t elt_type, size_t cnt)
{
    cudyn_arrtype_t t;
    cu_mutex_lock(&type_glck);
    t = cudyn_arrtype_glck(cuex_o2_gexpt(elt_type, cudyn_int(cnt)));
    cu_mutex_unlock(&type_glck);
    return t;
}


/* Product Types
 * ============= */

static cucon_layout_t
tuptype_finish_gprod_glck(cudyn_tuptype_t t, cuex_t ex, int i)
{
    cudyn_type_t subt;
    cucon_layout_t lyo;
    if (cuex_meta(ex) == CUEX_O2_GPROD) {
	cu_debug_assert(i > 0);
	subt = cudyn_type_glck(cuex_opn_at(ex, 1));
	if (!subt)
	    return NULL;
	lyo = tuptype_finish_gprod_glck(t, cuex_opn_at(ex, 0), i - 1);
	if (!lyo)
	    return NULL;
	t->tcomp_arr[i].type = subt;
	return cucon_layout_product(lyo, cudyn_type_layout(subt),
				    &t->tcomp_arr[i].bitoffset);
    }
    else {
	cu_debug_assert(i == 0);
	subt = cudyn_type_glck(ex);
	if (!subt)
	    return NULL;
	t->tcomp_arr[0].type = subt;
	t->tcomp_arr[0].bitoffset = 0;
	return cudyn_type_layout(subt);
    }
}

cu_clos_def(tuptype_finish_sigprod_cb,
	    cu_prot(cu_bool_t, cuex_opn_t e),
	( cucon_layout_t lyo;
	  cudyn_tuptype_t t; ))
{
    cu_clos_self(tuptype_finish_sigprod_cb);
    cudyn_type_t subt;
    cu_idr_t idr = cuex_aci_at(e, 0);
    struct cudyn_tupcomp_s *comp;
    if (!cucon_pmap_insert_mem(&self->t->scomp_map, idr,
			       sizeof(struct cudyn_tupcomp_s), &comp))
	cu_debug_unreachable();
    subt = cudyn_type_glck(cuex_aci_at(e, 1));
    if (!subt)
	return cu_false;
    comp->type = subt;
    self->lyo = cucon_layout_product(self->lyo, cudyn_type_layout(subt),
				     &comp->bitoffset);
    return cu_true;
}

static cucon_layout_t
tuptype_finish_sigprod_glck(cudyn_tuptype_t t, cuex_t ex, cucon_layout_t lyo)
{
    tuptype_finish_sigprod_cb_t cb;
    cb.lyo = lyo;
    cb.t = t;
    if (cuex_aci_conj(CUEX_O4ACI_SIGPROD, ex,
		      tuptype_finish_sigprod_cb_prep(&cb)))
	return cb.lyo;
    else
	return NULL;
}

cu_clop_def(tuptype_cct_glck, void, void *t)
{
#define t ((cudyn_tuptype_t)t)
    cucon_layout_t lyo;
    cuex_t ex;

    ex = cudyn_tuptype_to_type(t)->as_expr;
    cucon_pmap_cct(&t->scomp_map);
    if (ex == cuex_sig_identity()) {
	lyo = 0;
	t->tcomp_cnt = 0;
	t->tcomp_arr = NULL;
    }
    else if (cuex_meta(ex) == CUEX_O2_GPROD) {
	cuex_t ex0 = cuex_opn_at(ex, 0);
	cuex_t ex1 = cuex_opn_at(ex, 1);
	if (cuex_meta(ex1) == CUEX_O4ACI_SIGPROD) {
	    t->tcomp_cnt = cuex_binary_left_depth(CUEX_O2_GPROD, ex0) + 1;
	    t->tcomp_arr =
		cu_galloc(t->tcomp_cnt*sizeof(struct cudyn_tupcomp_s));
	    lyo = tuptype_finish_gprod_glck(t, ex0, t->tcomp_cnt - 1);
	    if (!lyo)
		return;
	    lyo = tuptype_finish_sigprod_glck(t, ex1, lyo);
	    if (!lyo)
		return;
	}
	else {
	    t->tcomp_cnt = cuex_binary_left_depth(CUEX_O2_GPROD, ex) + 1;
	    t->tcomp_arr =
		cu_galloc(t->tcomp_cnt*sizeof(struct cudyn_tupcomp_s));
	    lyo = tuptype_finish_gprod_glck(t, ex, t->tcomp_cnt - 1);
	    if (!lyo)
		return;
	}
    }
    else if (cuex_meta(ex) == CUEX_O4ACI_SIGPROD) {
	t->tcomp_cnt = 0;
	t->tcomp_arr = NULL;
	lyo = tuptype_finish_sigprod_glck(t, ex, NULL);
	if (!lyo)
	    return;
    }
    else {
	cudyn_type_t t0 = cudyn_type_glck(ex);
	if (!t0)
	    return;
	t->tcomp_cnt = 1;
	t->tcomp_arr = cu_galloc(sizeof(struct cudyn_tupcomp_s));
	t->tcomp_arr[0].type = t0;
	t->tcomp_arr[0].bitoffset = 0;
	lyo = cudyn_type_layout(t0);
	cu_debug_assert(lyo);
    }
    cudynP_hctype_cct_hcs(cu_to2(cudyn_hctype, cudyn_inltype, t), ex,
			  cudyn_typekind_tuptype, cucon_layout_size(lyo));
    AO_store_release_write(&cu_to(cudyn_inltype, t)->layout, (AO_t)lyo);
#undef t
}

cudyn_tuptype_t
cudyn_tuptype_glck(cuex_t ex)
{
    cudyn_tuptype_t t;
    t = cudyn_hnew_extra(cudyn_tuptype, sizeof(cuex_t), &ex, tuptype_cct_glck);
    if (!cu_to(cudyn_inltype, t)->layout)
	return NULL;
    return t;
}

cu_clop_def(tuptype_cct, void, void *t)
{
    cu_mutex_lock(&type_glck);
    cu_call(tuptype_cct_glck, t);
    cu_mutex_unlock(&type_glck);
}

cudyn_tuptype_t
cudyn_tuptype(cuex_t ex)
{
    cudyn_tuptype_t t;
    t = cudyn_hnew_extra(cudyn_tuptype, sizeof(cuex_t), &ex, tuptype_cct);
    if (!cu_to(cudyn_inltype, t)->layout)
	return NULL;
    return t;
}

cudyn_tuptype_t
cudyn_tuptype_by_valist(cu_offset_t cnt, va_list vl)
{
    cuex_t e;
    cu_debug_assert(cnt > 0);
    e = va_arg(vl, cuex_t);
    while (--cnt)
	e = cuex_o2_gprod(e, va_arg(vl, cuex_t));
    return cudyn_tuptype(e);
}

cu_clos_def(tuptype_conj_cb,
	    cu_prot(cu_bool_t, void const *idr, void *slot),
	( cu_clop(cb, cu_bool_t, cu_idr_t, cu_offset_t, cudyn_type_t); ))
{
    cu_clos_self(tuptype_conj_cb);
    struct cudyn_tupcomp_s *comp = slot;
    return cu_call(self->cb, (cu_idr_t)idr, comp->bitoffset, comp->type);
}

cu_bool_t
cudyn_tuptype_conj(cudyn_tuptype_t t,
		   cu_clop(cb, cu_bool_t, cu_idr_t, cu_offset_t, cudyn_type_t))
{
    tuptype_conj_cb_t scb;
    size_t i;
    struct cudyn_tupcomp_s *comp = t->tcomp_arr;
    for (i = 0; i < t->tcomp_cnt; ++i) {
	if (!cu_call(cb, NULL, comp->bitoffset, comp->type))
	    return cu_false;
	++comp;
    }
    scb.cb = cb;
    return cucon_pmap_conj_mem(&t->scomp_map, tuptype_conj_cb_prep(&scb));
}


/* Union Types
 * =========== */

cu_clos_def(duntype_cct_cb,
	    cu_prot(cu_bool_t, cuex_opn_t node),
	( cucon_layout_t lyo;
	  cudyn_cnum_t cnum;
	  cudyn_duntype_t t; ))
{
    cu_clos_self(duntype_cct_cb);
    cuex_t typeex;
    struct cudyn_dunpart_s *part;
    if (!cucon_pmap_insert_mem(&self->t->idr_to_part, cuex_aci_at(node, 0),
			       sizeof(struct cudyn_dunpart_s), &part))
	cu_debug_unreachable();
    typeex = cuex_binary_inject_left(CUEX_O2_GPROD, cuex_aci_at(node, 1),
				     cudyn_int_type());
    part->cnum = self->cnum++;
    part->type = cudyn_type_glck(typeex);
    if (!part->type)
	return cu_false;
    self->lyo = cucon_layout_union(self->lyo, cudyn_type_layout(part->type));
    return cu_true;
}

cu_clop_def(duntype_cct_glck, void, void *duntype)
{
#define duntype ((cudyn_duntype_t)duntype)
    duntype_cct_cb_t cb;
    cuex_t ex = cudyn_duntype_to_type(duntype)->as_expr;
    cu_debug_assert(cuex_meta(ex) == CUEX_O4ACI_DUNION);
    cucon_pmap_cct(&duntype->idr_to_part);
    cb.lyo = NULL;
    cb.t = duntype;
    cb.cnum = 0;
    if (!cuex_aci_conj(CUEX_O4ACI_DUNION, ex, duntype_cct_cb_prep(&cb)))
	return;
    /* TODO. Hash cons option, variable size. */
    cudynP_hctype_cct_nonhc(cu_to2(cudyn_hctype, cudyn_inltype, duntype),
			    ex, cudyn_typekind_duntype);
    AO_store_release_write(&cu_to(cudyn_inltype, duntype)->layout,
			   (AO_t)cb.lyo);
#undef duntype 
}

cudyn_duntype_t
cudyn_duntype_glck(cuex_t ex)
{
    cudyn_duntype_t t;
    t = cudyn_hnew_extra(cudyn_duntype, sizeof(cuex_t), &ex, duntype_cct_glck);
    if (!cu_to(cudyn_inltype, t)->layout)
	return NULL;
    return t;
}


/* Singular Types
 * ============== */

cu_clop_def(sngtype_cct_glck, void, void *sngtype)
{
#define sngtype ((cudyn_sngtype_t)sngtype)
    cuex_t ex = cudyn_sngtype_to_type(sngtype)->as_expr;
    cudynP_hctype_cct_nonhc(cu_to2(cudyn_hctype, cudyn_inltype, sngtype),
			    ex, cudyn_typekind_sngtype);
    AO_store_release_write(&cu_to(cudyn_inltype, sngtype)->ffitype,
			   (AO_t)&ffi_type_void);
#undef sngtype
}

cudyn_sngtype_t
cudyn_sngtype_glck(cuex_t ex)
{
    return cudyn_hnew_extra(cudyn_sngtype, sizeof(cuex_t), &ex,
			    sngtype_cct_glck);
}

#define sngtype_cct sngtype_cct_glck

cudyn_sngtype_t
cudyn_sngtype(cuex_t ex)
{
    return cudyn_hnew_extra(cudyn_sngtype, sizeof(cuex_t), &ex, sngtype_cct);
}

cudyn_sngtype_t
cudyn_sngtype_of_elt(cuex_t elt)
{
    return cudyn_sngtype(cuex_aci_generator(CUEX_O3ACI_SETJOIN, elt));
}


/* Generic
 * ======= */

cu_clop_def(type_init_default, void, void *type)
{
#define type ((cudyn_hctype_t)type)
    cuex_t ex = cudyn_hctype_to_type(type)->as_expr;
    cudynP_hctype_cct_hcs(type, ex, cudyn_typekind_by_expr,
			  cuex_type_size(ex));
#undef type
}

cudyn_type_t
cudyn_type_glck(cuex_t ex)
{
    if (cudyn_is_type(ex))
	return ex;
    switch (cuex_meta(ex)) {
	case CUEX_O3ACI_SETJOIN:
	    return cudyn_sngtype_to_type(cudyn_sngtype_glck(ex));
	case CUEX_O2_GEXPT:
	    return cudyn_arrtype_to_type(cudyn_arrtype_glck(ex));
	case CUEX_O2_GPROD:
//	case CUEX_O4ACI_SIGPROD:
	    return cudyn_tuptype_to_type(cudyn_tuptype_glck(ex));
	case CUEX_O4ACI_DUNION:
	    return cudyn_duntype_to_type(cudyn_duntype_glck(ex));
	case CUEX_O2_FARROW:
	case CUEX_O2_FARROW_NATIVE:
	    /* TODO, for now. */
	    return cudyn_ptrtype_to_type(cudyn_ptrtype_from_ex(ex));
	default:
#if 0
	    cu_bugf("Invalid or unimplemented type expression.");
	    return NULL;
#else
	    {
		return cudyn_halloc_extra(cudyn_type_type(),
			    sizeof(struct cudyn_hctype_s),
			    sizeof(cuex_t), &ex,
			    type_init_default);
	    }
#endif
    }
}

cudyn_type_t
cudyn_type(cuex_t ex)
{
    cudyn_type_t t;
    cu_mutex_lock(&type_glck);
    t = cudyn_type_glck(ex);
    cu_mutex_unlock(&type_glck);
    return t;
}


/* Init
 * ==== */

cudyn_stdtype_t cudynP_cuex_type;
cudyn_stdtype_t cudynP_ptrtype_type;
cudyn_stdtype_t cudynP_elmtype_type;
cudyn_stdtype_t cudynP_arrtype_type;
cudyn_stdtype_t cudynP_tuptype_type;
cudyn_stdtype_t cudynP_sngtype_type;
cudyn_stdtype_t cudynP_duntype_type;
cuex_t cudynP_tup_null;
cudyn_sngtype_t cudynP_default_sngtype;
cudyn_stdtype_t cudynP_type_type;

void
cudynP_type_init()
{
    cudynP_cuex_type = cudyn_stdtype_new();
    cudynP_ptrtype_type = cudyn_stdtypeoftypes_new_hcs(
	    sizeof(struct cudyn_ptrtype_s) - CU_HCOBJ_SHIFT);
    cudynP_elmtype_type = cudyn_stdtypeoftypes_new();
    cudynP_arrtype_type = cudyn_stdtypeoftypes_new_hce();
    cudynP_tuptype_type = cudyn_stdtypeoftypes_new_hce();
    cudynP_duntype_type = cudyn_stdtypeoftypes_new_hce();
    cudynP_sngtype_type = cudyn_stdtypeoftypes_new_hce();
    cudynP_tup_null = cuex_aci_identity(CUEX_O4ACI_SIGPROD);
    cudynP_default_sngtype = cudyn_sngtype_of_elt(cudynP_tup_null);
    cudynP_type_type = cudyn_stdtypeoftypes_new_hce();
}
