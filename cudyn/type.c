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
#include <cuex/algo.h>
#include <cuex/type.h>
#include <cuex/labelling.h>
#include <cuex/opn.h>
#include <cuex/set.h>
#include <cuex/monoid.h>
#include <cuoo/intf.h>


static cu_mutex_t type_glck = CU_MUTEX_INITIALISER;


/* Pointer Types
 * ============= */

/* May consider to add the deref type to the structure, but this means it must
 * be computed in case of cudyn_ptrtype_from_ex. */

cudyn_ptrtype_t
cudyn_ptrtype_from_ex(cuex_t ex)
{
    cudyn_ptrtype_t type;
    cuoo_hctem_decl(cudyn_ptrtype, tem);

    cuoo_hctem_init(cudyn_ptrtype, tem);
    type = cuoo_hctem_get(cudyn_ptrtype, tem);
    /*cu_debug_assert(cuex_meta(ex) == CUEX_O1_PTR_TO);*/
    cuoo_type_init_general_hcs(cu_to2(cuoo_type, cudyn_inltype, type),
			       cuoo_typekind_ptrtype, cuoo_impl_none, ex,
			       sizeof(void *));
    cu_to(cudyn_inltype, type)->layout = (AO_t)cuoo_layout_ptr;
    cu_to(cudyn_inltype, type)->ffitype = (AO_t)&ffi_type_pointer;
    return cuoo_hctem_new(cudyn_ptrtype, tem);
}

cudyn_ptrtype_t
cudyn_ptrtype(cuex_t deref)
{
    cuex_t ex;
    cudyn_ptrtype_t type;
    cuoo_hctem_decl(cudyn_ptrtype, tem);

    cuoo_hctem_init(cudyn_ptrtype, tem);
    type = cuoo_hctem_get(cudyn_ptrtype, tem);
    ex = cuex_o1_ptr_to(cuoo_type_as_expr(deref));
    cuoo_type_init_general_hcs(cu_to2(cuoo_type, cudyn_inltype, type),
			       cuoo_typekind_ptrtype, cuoo_impl_none, ex,
			       sizeof(void *));
    cu_to(cudyn_inltype, type)->layout = (AO_t)cuoo_layout_ptr;
    cu_to(cudyn_inltype, type)->ffitype = (AO_t)&ffi_type_pointer;
    return cuoo_hctem_new(cudyn_ptrtype, tem);
}


/* Elementary Types
 * ================ */

cudyn_elmtype_t
cudyn_elmtype_new(cuoo_typekind_t kind, cuoo_impl_t impl,
		  cu_offset_t size, cu_offset_t alignment, ffi_type *ffitype)
{
    cu_offset_t bitoffset;
    cudyn_elmtype_t t = cuoo_onew(cudyn_elmtype);
    cu_offset_t wsize;
    wsize = (size + sizeof(cu_word_t) - 1)/sizeof(cu_word_t)*sizeof(cu_word_t);
    cuoo_type_init_general_hcs(cu_to2(cuoo_type, cudyn_inltype, t), kind,
			       impl, NULL, wsize);
    cu_to(cudyn_inltype, t)->layout
	= (AO_t)cuoo_layout_pack_bits(NULL, size*8, alignment*8, &bitoffset);
    cu_to(cudyn_inltype, t)->ffitype = (AO_t)ffitype;
    return t;
}


/* Array Types
 * =========== */

static void
arrtype_cct_glck(cudyn_arrtype_t t)
{
    cu_offset_t bitoffset;
    size_t elt_bitsize, elt_bitalign;
    size_t arr_bitsize, arr_bitalign;
    cuoo_layout_t lyo, sub_lyo;
    cuex_t ex;

    ex = cudyn_arrtype_to_type(t)->as_expr;
    cu_debug_assert(cuex_meta(ex) == CUEX_O2_GEXPT);
    if (!cudyn_is_int(cuex_opn_at(ex, 1)))
	return;
    t->elt_type = cuoo_type_glck(cuex_opn_at(ex, 0));
    if (!t->elt_type)
	return;
    t->elt_cnt = cudyn_to_int(cuex_opn_at(ex, 1));
    sub_lyo = cuoo_type_layout(t->elt_type);
    elt_bitsize = cuoo_layout_bitsize(sub_lyo);
    elt_bitalign = cuoo_layout_bitalign(sub_lyo);
    arr_bitsize = elt_bitsize*t->elt_cnt;
    arr_bitalign = elt_bitalign;
    lyo = cuoo_layout_pack_bits(NULL, arr_bitsize, arr_bitalign, &bitoffset);
    cuoo_type_init_general_hcs(cu_to2(cuoo_type, cudyn_inltype, t),
			       cuoo_typekind_arrtype, cuoo_impl_none, ex,
			       cuoo_layout_size(lyo));
    AO_store_release_write(&cu_to(cudyn_inltype, t)->layout, (AO_t)lyo);
}

static cudyn_arrtype_t
arrtype(cuex_t ex, cu_bool_t have_lock)
{
    cudyn_arrtype_t t;
    t = cuoo_hxnew_setao(cudyn_arrtype, sizeof(cuex_t), &ex,
			 offsetof(struct cudyn_inltype_s, layout), 0);
    if (!AO_load_acquire_read(&cu_to(cudyn_inltype, t)->layout)) {
	if (!have_lock)
	    cu_mutex_lock(&type_glck);
	if (!cu_to(cudyn_inltype, t)->layout)
	    arrtype_cct_glck(t);
	if (!have_lock)
	    cu_mutex_unlock(&type_glck);
	if (!cu_to(cudyn_inltype, t)->layout)
	    return NULL;
    }
    return t;
}

cudyn_arrtype_t
cudyn_arrtype(cuoo_type_t elt_type, size_t cnt)
{
    return arrtype(cuex_o2_gexpt(elt_type, cudyn_int(cnt)), cu_false);
}


/* Product Types
 * ============= */

static cuoo_layout_t
tuptype_finish_gprod_glck(cudyn_tuptype_t t, cuex_t ex, int i)
{
    cuoo_type_t subt;
    cuoo_layout_t lyo;
    if (cuex_meta(ex) == CUEX_O2_GPROD) {
	cu_debug_assert(i > 0);
	subt = cuoo_type_glck(cuex_opn_at(ex, 1));
	if (!subt)
	    return NULL;
	lyo = tuptype_finish_gprod_glck(t, cuex_opn_at(ex, 0), i - 1);
	if (!lyo)
	    return NULL;
	t->tcomp_arr[i].type = subt;
	return cuoo_layout_product(lyo, cuoo_type_layout(subt),
				    &t->tcomp_arr[i].bitoffset);
    }
    else {
	cu_debug_assert(i == 0);
	subt = cuoo_type_glck(ex);
	if (!subt)
	    return NULL;
	t->tcomp_arr[0].type = subt;
	t->tcomp_arr[0].bitoffset = 0;
	return cuoo_type_layout(subt);
    }
}

#if 0
cu_clos_def(tuptype_finish_sigprod_cb,
	    cu_prot(cu_bool_t, cuex_t idr, cuex_t subt),
	( cuoo_layout_t lyo;
	  cudyn_tuptype_t t; ))
{
    cu_clos_self(tuptype_finish_sigprod_cb);
    struct cudyn_tupcomp_s *comp;
    cu_debug_assert(cu_is_idr(idr));
    cu_debug_assert(cuoo_is_type(subt));
    if (!cucon_pmap_insert_mem(&self->t->scomp_map, idr,
			       sizeof(struct cudyn_tupcomp_s), &comp))
	cu_debug_unreachable();
    subt = cuoo_type_glck(subt);
    if (!subt)
	return cu_false;
    comp->type = subt;
    self->lyo = cuoo_layout_product(self->lyo, cuoo_type_layout(subt),
				    &comp->bitoffset);
    return cu_true;
}

static cuoo_layout_t
tuptype_finish_sigprod_glck(cudyn_tuptype_t t, cuex_t ex, cuoo_layout_t lyo)
{
    tuptype_finish_sigprod_cb_t cb;
    cb.lyo = lyo;
    cb.t = t;
    if (cuex_labelling_conj_kv(ex, tuptype_finish_sigprod_cb_prep(&cb)))
	return cb.lyo;
    else
	return NULL;
}
#endif

static void
tuptype_cct_glck(cudyn_tuptype_t t)
{
    cuoo_layout_t lyo;
    cuex_t ex;

    ex = cudyn_tuptype_to_type(t)->as_expr;
    cucon_pmap_init(&t->scomp_map);
#if 0
    if (ex == cuex_o0_gunit()) { /* XXX */
	lyo = NULL;
	t->tcomp_cnt = 0;
	t->tcomp_arr = NULL;
    }
    else
#endif
    if (cuex_meta(ex) == CUEX_O2_GPROD) {
#if 0
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
	else
#endif
	{
	    t->tcomp_cnt = cuex_binary_left_depth(CUEX_O2_GPROD, ex) + 1;
	    t->tcomp_arr =
		cu_galloc(t->tcomp_cnt*sizeof(struct cudyn_tupcomp_s));
	    lyo = tuptype_finish_gprod_glck(t, ex, t->tcomp_cnt - 1);
	    if (!lyo)
		return;
	}
    }
#if 0
    else if (cuex_meta(ex) == CUEX_O4ACI_SIGPROD) {
	t->tcomp_cnt = 0;
	t->tcomp_arr = NULL;
	lyo = tuptype_finish_sigprod_glck(t, ex, NULL);
	if (!lyo)
	    return;
    }
#endif
    else {
	cuoo_type_t t0 = cuoo_type_glck(ex);
	if (!t0)
	    return;
	t->tcomp_cnt = 1;
	t->tcomp_arr = cu_galloc(sizeof(struct cudyn_tupcomp_s));
	t->tcomp_arr[0].type = t0;
	t->tcomp_arr[0].bitoffset = 0;
	lyo = cuoo_type_layout(t0);
	cu_debug_assert(lyo);
    }
    cuoo_type_init_general_hcs(cu_to2(cuoo_type, cudyn_inltype, t),
			       cuoo_typekind_tuptype, cuoo_impl_none, ex,
			       cuoo_layout_size(lyo));
    AO_store_release_write(&cu_to(cudyn_inltype, t)->layout, (AO_t)lyo);
}

static cudyn_tuptype_t
tuptype(cuex_t ex, cu_bool_t have_lock)
{
    cudyn_tuptype_t t;
    t = cuoo_hxnew_setao(cudyn_tuptype, sizeof(cuex_t), &ex,
			 offsetof(struct cudyn_inltype_s, layout), 0);
    if (!AO_load_acquire_read(&cu_to(cudyn_inltype, t)->layout)) {
	if (!have_lock)
	    cu_mutex_lock(&type_glck);
	if (!cu_to(cudyn_inltype, t)->layout)
	    tuptype_cct_glck(t);
	if (!have_lock)
	    cu_mutex_unlock(&type_glck);
	if (!cu_to(cudyn_inltype, t)->layout)
	    return NULL;
    }
    return t;
}

cudyn_tuptype_t
cudyn_tuptype(cuex_t ex)
{
    return tuptype(ex, cu_false);
}

cudyn_tuptype_t
cudyn_tuptype_by_valist(cu_offset_t cnt, va_list vl)
{
    cuex_t e;
    cu_debug_assert(cnt > 0);
    e = va_arg(vl, cuex_t);
    while (--cnt)
	e = cuex_o2_gprod(e, va_arg(vl, cuex_t));
    return tuptype(e, cu_false);
}

cu_clos_def(tuptype_conj_cb,
	    cu_prot(cu_bool_t, void const *idr, void *slot),
	( cu_clop(cb, cu_bool_t, cu_idr_t, cu_offset_t, cuoo_type_t); ))
{
    cu_clos_self(tuptype_conj_cb);
    struct cudyn_tupcomp_s *comp = slot;
    return cu_call(self->cb, (cu_idr_t)idr, comp->bitoffset, comp->type);
}

cu_bool_t
cudyn_tuptype_conj(cudyn_tuptype_t t,
		   cu_clop(cb, cu_bool_t, cu_idr_t, cu_offset_t, cuoo_type_t))
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

cu_clos_def(tuptype_print_elt,
            cu_prot(cu_bool_t, cu_idr_t label, cu_offset_t bitoff,
                               cuoo_type_t t),
    ( int i;
      FILE *out; ))
{
    cu_clos_self(tuptype_print_elt);
    if (self->i++)
        fputs(" × ", self->out);
    if (label) {
        fputc('~', self->out);
        fputs(cu_idr_to_cstr(label), self->out);
    }
    cu_fprintf(self->out, "@%d:%!", bitoff, t);
    ++self->i;
    return cu_true;
}

void
cudyn_tuptype_print(void *t, FILE *out)
{
    tuptype_print_elt_t cb;
    cb.i = 0;
    cb.out = out;
    fprintf(out, "#[cudyn_tuptype_t size=%ld: ", (long)cuoo_type_size(t));
    cudyn_tuptype_conj(t, tuptype_print_elt_prep(&cb));
    fputc(']', out);
}

static cu_word_t
tuptype_impl(cu_word_t intf_number, ...)
{
    switch (intf_number) {
	case CUOO_INTF_PRINT_FN: return (cu_word_t)cudyn_tuptype_print;
	default: return CUOO_IMPL_NONE;
    }
}


/* Union Types
 * =========== */

#if 0
cu_clos_def(duntype_cct_cb,
	    cu_prot(cu_bool_t, cuex_opn_t node),
	( cuoo_layout_t lyo;
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
    part->type = cuoo_type_glck(typeex);
    if (!part->type)
	return cu_false;
    self->lyo = cuoo_layout_union(self->lyo, cuoo_type_layout(part->type));
    return cu_true;
}

static void
duntype_cct_glck(cudyn_duntype_t duntype)
{
    duntype_cct_cb_t cb;
    cuex_t ex = cudyn_duntype_to_type(duntype)->as_expr;
    cu_debug_assert(cuex_meta(ex) == CUEX_O4ACI_DUNION);
    cucon_pmap_init(&duntype->idr_to_part);
    cb.lyo = NULL;
    cb.t = duntype;
    cb.cnum = 0;
    if (!cuex_aci_conj(CUEX_O4ACI_DUNION, ex, duntype_cct_cb_prep(&cb)))
	return;
    /* TODO. Hash cons option, variable size. */
    cuoo_type_init_general(cu_to2(cuoo_type, cudyn_inltype, duntype),
			   cuoo_typekind_duntype, cuoo_impl_none, ex);
    AO_store_release_write(&cu_to(cudyn_inltype, duntype)->layout,
			   (AO_t)cb.lyo);
}

static cudyn_duntype_t
duntype(cuex_t ex, cu_bool_t have_lock)
{
    cudyn_duntype_t t;
    t = cuoo_hxnew_setao(cudyn_duntype, sizeof(cuex_t), &ex,
			 offsetof(struct cudyn_inltype_s, layout), 0);
    if (!AO_load_acquire_read(&cu_to(cudyn_inltype, t)->layout)) {
	if (!have_lock)
	    cu_mutex_lock(&type_glck);
	if (!cu_to(cudyn_inltype, t)->layout)
	    duntype_cct_glck(t);
	if (!have_lock)
	    cu_mutex_unlock(&type_glck);
	if (!cu_to(cudyn_inltype, t)->layout)
	    return NULL;
    }
    return t;
}

cudyn_duntype_t
cudyn_duntype(cuex_t ex)
{
    return duntype(ex, cu_false);
}
#endif


/* Generic
 * ======= */

static cuoo_type_t
default_type(cuex_t ex)
{
    cuoo_type_t t;
    t = cuoo_hxalloc_setao(cuoo_type_type(),
			   sizeof(struct cuoo_type_s) + sizeof(AO_t),
			   sizeof(cuex_t), &ex,
			   sizeof(struct cuoo_type_s), 0);
    if (!AO_load_acquire_read((AO_t *)(t + 1)))
	cuoo_type_init_general_hcs(t, cuoo_typekind_by_expr,
				   cuoo_impl_none, ex, cuex_type_size(ex));
    return t;
}

cuoo_type_t
dispatch_type(cuex_t ex, cu_bool_t have_lock)
{
    if (cuoo_is_type(ex))
	return ex;
    switch (cuex_meta(ex)) {
#if 0
	case CUEX_O4ACI_DUNION:
	    return cudyn_duntype_to_type(duntype(ex, have_lock));
#endif
	case CUEX_O2_GEXPT:
	    return cudyn_arrtype_to_type(arrtype(ex, have_lock));
	case CUEX_O2_GPROD:
	    return cudyn_tuptype_to_type(tuptype(ex, have_lock));
	case CUEX_O2_FARROW:
	case CUEX_O2_FARROW_NATIVE:
	    /* TODO, for now. */
	    return cudyn_ptrtype_to_type(cudyn_ptrtype_from_ex(ex));
	default:
#if 0
	    cu_bugf("Invalid or unimplemented type expression.");
	    return NULL;
#else
	    return default_type(ex);
#endif
    }
}

cuoo_type_t
cuoo_type_glck(cuex_t ex)
{
    return dispatch_type(ex, cu_true);
}

cuoo_type_t
cuoo_type(cuex_t ex)
{
    return dispatch_type(ex, cu_false);
}


/* Init
 * ==== */

cuoo_type_t cudynP_cuex_type;
cuoo_type_t cudynP_ptrtype_type;
cuoo_type_t cudynP_elmtype_type;
cuoo_type_t cudynP_arrtype_type;
cuoo_type_t cudynP_tuptype_type;
cuoo_type_t cudynP_sngtype_type;
cuoo_type_t cudynP_duntype_type;
cuex_t cudynP_tup_null;
cudyn_sngtype_t cudynP_default_sngtype;

void
cudynP_type_init()
{
    cudynP_cuex_type = cuoo_type_new_opaque(cuoo_impl_none);
    cudynP_ptrtype_type = cuoo_type_new_metatype_hcs(
	cuoo_impl_none, sizeof(struct cudyn_ptrtype_s) - CUOO_HCOBJ_SHIFT);
    cudynP_elmtype_type = cuoo_type_new_metatype(cuoo_impl_none);
    cudynP_arrtype_type = cuoo_type_new_metatype_hce(cuoo_impl_none);
    cudynP_tuptype_type = cuoo_type_new_metatype_hce(tuptype_impl);
    cudynP_duntype_type = cuoo_type_new_metatype_hce(cuoo_impl_none);
    cudynP_sngtype_type = cuoo_type_new_metatype_hce(cuoo_impl_none);
    cudynP_tup_null = cuex_monoid_identity(CUEX_O2_TUPLE);
}
