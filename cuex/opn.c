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

#include <cuex/opn.h>
#include <cuex/oprinfo.h>
#include <cu/halloc.h>

CU_SINLINE cu_bool_t
wordaligned_eq(void *arr0, void *arr0_end, void *arr1)
{
    while (arr0 < arr0_end) {
	if (*(cu_word_t *)arr0 != *(cu_word_t *)arr1)
	    return cu_false;
	arr0 += sizeof(cu_word_t);
	arr1 += sizeof(cu_word_t);
    }
    return cu_true;
}

CU_SINLINE void
wordaligned_copy(void *src, void *src_end, void *dst)
{
    while (src < src_end) {
	*(cu_word_t *)dst = *(cu_word_t *)src;
	src += sizeof(cu_word_t);
	dst += sizeof(cu_word_t);
    }
}

cu_clos_def(cct_cached, cu_prot(void, void *e), (cuex_oprinfo_t oi;))
{
    cu_clos_self(cct_cached);
    (*self->oi->cache_ctor)((cuex_opn_t)e);
}

cuex_opn_t
cuex_opn(cuex_meta_t opr, ...)
{
    cu_rank_t N = cuex_opr_r(opr), n;
    size_t key_size = N*sizeof(cuex_t);
    va_list va;
    cuex_t *operand_arr = cu_salloc(key_size);

    /* TODO: Hack for platforms with stacks growing towards NULL.  Need
     * Autoconf macro. */
    va_start(va, opr);
    for (n = 0; n < N; ++n) {
	operand_arr[n] = va_arg(va, cuex_t);
	cu_debug_assert(operand_arr[n]);
    }
    va_end(va);
    if (!cuex_opr_has_ctor(opr))
	return cuexP_halloc_raw(
	    opr,
	    CUDYN_HCOBJ_KEY_SIZEW(N*sizeof(cuex_t) + CU_HCOBJ_SHIFT),
	    operand_arr);
    else {
	cct_cached_t cct;
	cuex_oprinfo_t oi = cuex_oprinfo(opr);
	cct.oi = oi;
	return cuexP_halloc_extra_raw(
	    opr,
	    CUDYN_HCOBJ_ALLOC_SIZEG(N*sizeof(cuex_t) + CU_HCOBJ_SHIFT
				    + oi->cache_size),
	    CUDYN_HCOBJ_KEY_SIZEW(N*sizeof(cuex_t) + CU_HCOBJ_SHIFT),
	    operand_arr, cct_cached_prep(&cct));
    }
}

cuex_opn_t
cuex_opn_by_valist(cuex_meta_t opr, va_list va)
{
    cu_rank_t N = cuex_opr_r(opr), n;
    size_t key_size = N*sizeof(cuex_t);
    cuex_t *operand_arr = cu_salloc(key_size);

    /* TODO: Hack for platforms with stacks growing towards NULL.  Need
     * Autoconf macro. */
    for (n = 0; n < N; ++n) {
	operand_arr[n] = va_arg(va, cuex_t);
	cu_debug_assert(operand_arr[n]);
    }
    if (!cuex_opr_has_ctor(opr))
	return cuexP_halloc_raw(
	    opr,
	    CUDYN_HCOBJ_KEY_SIZEW(N*sizeof(cuex_t) + CU_HCOBJ_SHIFT),
	    operand_arr);
    else {
	cct_cached_t cct;
	cuex_oprinfo_t oi = cuex_oprinfo(opr);
	cct.oi = oi;
	return cuexP_halloc_extra_raw(
	    opr,
	    CUDYN_HCOBJ_ALLOC_SIZEG(N*sizeof(cuex_t) + CU_HCOBJ_SHIFT
				    + oi->cache_size),
	    CUDYN_HCOBJ_KEY_SIZEW(N*sizeof(cuex_t) + CU_HCOBJ_SHIFT),
	    operand_arr, cct_cached_prep(&cct));
    }
}

cuex_opn_t
cuexP_opn_by_arr_with_ctor(cuex_meta_t opr, cuex_t *operand_arr)
{
    cu_rank_t N = cuex_opr_r(opr);
    cct_cached_t cct;
    cuex_oprinfo_t oi = cuex_oprinfo(opr);
    cct.oi = oi;
    return cuexP_halloc_extra_raw(
	opr,
	CUDYN_HCOBJ_ALLOC_SIZEG(N*sizeof(cuex_t) + CU_HCOBJ_SHIFT
				+ oi->cache_size),
	CUDYN_HCOBJ_KEY_SIZEW(N*sizeof(cuex_t) + CU_HCOBJ_SHIFT),
	operand_arr, cct_cached_prep(&cct));
}

cuex_opn_t
cuex_opn2_left(cuex_meta_t opr, cuex_t x, cuex_t y)
{
    while (cuex_meta(y) == opr) {
	x = cuex_opn2_left(opr, x, cuex_opn_at(y, 0));
	y = cuex_opn_at(y, 1);
    }
    return cuex_opn(opr, x, y);
}

cuex_opn_t
cuex_opn2_right(cuex_meta_t opr, cuex_t x, cuex_t y)
{
    while (cuex_meta(x) == opr) {
	y = cuex_opn2_right(opr, cuex_opn_at(x, 1), y);
	x = cuex_opn_at(x, 0);
    }
    return cuex_opn(opr, x, y);
}
