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


#include <cudyn/proto.h>
#include <cudyn/type.h>
#include <cu/memory.h>
#include <cucon/layout.h>
#include <string.h>
#include <assert.h>

#if 0
#  include <stdio.h>
#  define D_PRINTF(args...) ((void)chi_fprintf(stderr, args))
#else
#  define D_PRINTF(args...) ((void)0)
#endif

static cu_mutex_t cif_mutex = CU_MUTEX_INITIALISER;

struct fake_ffi_type_s
{
    ffi_type ffitype;
    ffi_type *elements[3];
};

static ffi_type *
layout_ffitype_ciflck(cucon_layout_t lyo)
{
    static struct fake_ffi_type_s *prebuilt_arr = NULL;
    static size_t prebuilt_cnt;
    size_t size = cucon_layout_size(lyo);
    if (cucon_layout_prefix(lyo)) {
	size_t cnt = (size + sizeof(long) - 1)/sizeof(long);
	size_t build_index;
	cu_debug_assert(cucon_layout_align(lyo) <= sizeof(long));
	if (cnt >= prebuilt_cnt) {
	    if (!prebuilt_arr) {
		prebuilt_arr = cu_galloc(sizeof(struct fake_ffi_type_s)*cnt);
		prebuilt_arr[0].ffitype.elements = prebuilt_arr[0].elements;
		prebuilt_arr[0].elements[0] = &ffi_type_ulong;
		prebuilt_arr[0].elements[1] = &ffi_type_ulong;
		build_index = 1;
		prebuilt_cnt = cnt;
	    } else {
		size_t size;
		build_index = prebuilt_cnt;
		prebuilt_cnt = prebuilt_cnt*2;
		if (prebuilt_cnt < cnt)
		    prebuilt_cnt = cnt;
		size = sizeof(struct fake_ffi_type_s)*cnt;
		struct fake_ffi_type_s *old_arr = prebuilt_arr;
		prebuilt_arr = cu_galloc(size);
		memcpy(prebuilt_arr, old_arr, size);
	    }
	    while (prebuilt_cnt < cnt) {
		prebuilt_arr[prebuilt_cnt].ffitype.elements
		    = prebuilt_arr[prebuilt_cnt].elements;
		prebuilt_arr[prebuilt_cnt].elements[0]
		    = &prebuilt_arr[prebuilt_cnt - 1].ffitype;
		prebuilt_arr[prebuilt_cnt].elements[1] = &ffi_type_ulong;
		++prebuilt_cnt;
	    }
	}
	return &prebuilt_arr[cnt - 1].ffitype;
    }
    else {
	if (size <= 1)
	    return &ffi_type_uint8;
	else if (size <= 2)
	    return &ffi_type_uint16;
	else if (size <= 4)
	    return &ffi_type_uint32;
	else if (size <= 8)
	    return &ffi_type_uint64;
	else
	    cu_bugf("Unexpected layout size %ld", size);
    }
}


ffi_type *
cudynP_type_ffitype_ciflck(cudyn_type_t type)
{
    if (cudyn_type_is_nonptr_inltype(type)) {
	cudyn_inltype_t t;
	t = cu_from(cudyn_inltype, cudyn_type, type);
	if (!t->ffitype) {
	    switch (cudyn_type_typekind(type)) {
		case cudyn_typekind_arrtype:
		    /* Passed as pointer when an argument, but what to do
		     * when inlining in a struct?  We could maybe fake the
		     * ffi_type by setting 'size' and 'alignment', but
		     * what about 'type'? */
		    t->ffitype = (AO_t)
			layout_ffitype_ciflck((cucon_layout_t)t->layout);
		    break;
		case cudyn_typekind_tuptype:
#if 0 /* only applies if tuptype layout is not packed */
		    ffitype0 = cudynP_type_ffitype_ciflck(
			    t->u0.tuptype.sans_last);
		    ffitype1 = cudynP_type_ffitype_ciflck(
			    t->u0.tuptype.last);
		    ffitype = cu_galloc(sizeof(ffi_type) +
					sizeof(ffi_type*)*3);
		    ffitype->elements = (ffi_type **)(ffitype + 1);
		    ffitype->elements[0] = ffitype0;
		    ffitype->elements[1] = ffitype1;
		    ffitype->elements[2] = NULL;
		    t->ffitype = ffitype;
#endif
		    t->ffitype = (AO_t)
			layout_ffitype_ciflck((cucon_layout_t)t->layout);
		    break;
		case cudyn_typekind_duntype:
		    cu_debug_assert(!"unimplemented");
		    break;
		default:
		    cu_debug_unreachable();
	    }
	}
	return (ffi_type *)t->ffitype;
    }
    else
	return &ffi_type_pointer;
}

cu_clos_def(proto_init_cif, cu_prot(void, void *proto), (int r;))
{
    cu_clos_self(proto_init_cif);
#define proto ((cudyn_proto_t)proto)
    size_t i;
    ffi_type **arg_ffi_arr;
    ffi_type *res_ffi;
    ffi_status err;
    arg_ffi_arr = (void *)(proto + 1);
    ffi_type **arg_ffi_cur = arg_ffi_arr;
    cu_mutex_lock(&cif_mutex);
    for (i = 0; i < self->r; ++i) {
	cudyn_type_t t = cudyn_tuptype_at(proto->arg_type, i);
	*arg_ffi_cur = cudynP_type_ffitype_ciflck(t);
    }
    res_ffi = cudynP_type_ffitype_ciflck(proto->res_type);
    cu_mutex_unlock(&cif_mutex);
    err = ffi_prep_cif(&proto->cif, FFI_DEFAULT_ABI,
		       self->r, res_ffi, arg_ffi_arr);
    cu_debug_assert(err == FFI_OK);
#undef proto
}

cudyn_proto_t
cudyn_proto_by_tuptype(cudyn_tuptype_t arg_type, cudyn_type_t res_type)
{
    proto_init_cif_t init;
    size_t r = cudyn_tuptype_tcomp_cnt(arg_type);
    struct cudyn_proto_s key;
    cudyn_proto_t proto;
    size_t size = sizeof(struct cudyn_proto_s) + r*sizeof(ffi_type *);
    cudynP_type_cct_hcs(cu_to(cudyn_type, &key), NULL,
			cudyn_typekind_proto, sizeof(cu_fnptr_t));
    key.arg_type = arg_type;
    key.res_type = res_type;
    init.r = r;
    proto = cudyn_halloc_extra(cudyn_proto_type(), size,
			       CUDYN_PROTO_KEY_SIZE,
			       cu_ptr_add(&key, CU_HCOBJ_SHIFT),
			       proto_init_cif_prep(&init));
//XXX    cu_debug_assert(r != 0 || cudyn_type_is_sngtype(arg_type));
    return proto;
}

cudyn_proto_t
cudyn_proto(cu_rank_t r, ...)
{
    cudyn_tuptype_t arg_type;
    cudyn_type_t res_type;
    va_list va;
    va_start(va, r);
    arg_type = cudyn_tuptype_by_valist(r, va);
    res_type = va_arg(va, cudyn_type_t);
    va_end(va);
    return cudyn_proto_by_tuptype(arg_type, res_type);
}

#if 0
cu_bool_t
cudyn_proto_equal(cudyn_proto_t proto0, cudyn_proto_t proto1)
{
    cu_rank_t i;
    cu_rank_t type_count = cudyn_proto_type_count(proto0);
    if (!!proto0->non_closure_proto != !!proto1->non_closure_proto)
	return cu_false;
    if (cudyn_proto_type_count(proto1) != type_count)
	return cu_false;
    for (i = 0; i < type_count; ++i) {
	if (!cudyn_type_equal(cudyn_proto_arg_type(proto0, i),
			   cudyn_proto_arg_type(proto1, i)))
	    return cu_false;
    }
    return cu_true;
}

cu_bool_t
cudyn_proto_subeq(cudyn_proto_t proto0, cudyn_proto_t proto1)
{
    cu_rank_t i;
    cu_rank_t type_count = cudyn_proto_type_count(proto0);
    if (proto0->non_closure_proto && !proto1->non_closure_proto)
	return cu_false;
    if (cudyn_proto_type_count(proto1) != type_count)
	return cu_false;
    for (i = 0; i < type_count; ++i) {
	cudyn_type_t arg0 = cudyn_proto_arg_type(proto0, i);
	cudyn_type_t arg1 = cudyn_proto_arg_type(proto1, i);
	if (!cudyn_type_subeq(arg0, arg1))
	    return cu_false;
    }
    return cu_true;
}
#endif

static void
set_ffi_arg(void **ffi_arg, cuex_t *arg, cudyn_type_t arg_type)
{
    if (cudyn_type_is_inltype(arg_type)) {
	if (cuex_meta(*arg) != cudyn_type_to_meta(arg_type)) {
	    cu_errf("Mismatched argument type in function call.");
	    abort();
	}
	if (cudyn_type_is_hctype(arg_type))
	    *ffi_arg = (void *)*arg + CU_HCOBJ_SHIFT;
	else
	    *ffi_arg = (void *)*arg + CU_OBJ_SHIFT;
    }
    else {
	if (!cudyn_is_cuex_type(arg_type) &&
	    cuex_meta(*arg) != cudyn_type_to_meta(arg_type)) {
	    cu_errf("Mismatched argument type in function call.");
	    abort();
	}
	*ffi_arg = arg;
    }
}

cuex_t
cudyn_proto_apply_fn(cudyn_proto_t proto, cu_fnptr_t fn,
		     cuex_t *arg_arr)
{
    cu_rank_t r = cudyn_proto_r(proto);
    cu_rank_t i;
    void **ffi_arg_arr = cu_salloc(sizeof(void *)*r);
    cudyn_type_t res_type;
    for (i = 0; i < r; ++i) {
	cudyn_type_t t = cudyn_tuptype_at(proto->arg_type, i);
	cu_debug_assert(cudyn_type_is_inltype(t));
	set_ffi_arg(&ffi_arg_arr[i], &arg_arr[i], t);
    }
    res_type = proto->res_type;
    if (cudyn_type_is_inltype(res_type)) {
	size_t size = cudyn_type_size(res_type);
	if (cudyn_type_is_hctype(res_type)) {
	    void *res_data;
	    size_t key_sizew = CUDYN_HCOBJ_KEY_SIZEW(size + CU_HCOBJ_SHIFT);
	    res_data = cu_salloc(key_sizew*CU_WORD_SIZE);
	    ffi_call(&proto->cif, fn, res_data, ffi_arg_arr);
	    return cuexP_halloc_raw(cudyn_type_to_meta(res_type),
				    key_sizew, res_data);
	}
	else {
	    void *res = cudyn_oalloc(res_type, size + CU_OBJ_SHIFT);
	    ffi_call(&proto->cif, fn, res + CU_OBJ_SHIFT, ffi_arg_arr);
	    return res;
	}
    }
    else {
	cuex_t res;
	ffi_call(&proto->cif, fn, &res, ffi_arg_arr);
	return res;
    }
}


cudyn_stdtype_t cudynP_proto_type;

void
cudynP_proto_init()
{
    cudynP_proto_type = cudyn_stdtype_new_hcs(CUDYN_PROTO_KEY_SIZE);
}
