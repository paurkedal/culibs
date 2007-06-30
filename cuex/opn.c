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
#include <cu/hcons.h>

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

#if CUCONF_SIZEOF_LONG != CUCONF_SIZEOF_VOID_P
#  error FIXME calculate correct size for cu_hc_key_hash in cuex_opn,
#  error cuex_opn_by_va, and cuex_opn_by_arr.
#endif

CU_SINLINE void
opn_cct_cached(cuex_opn_t opn, cuex_oprinfo_t oi,
	       cuex_t *operand_arr, cuex_t *operand_arr_end)
{
    wordaligned_copy(operand_arr, operand_arr_end, opn->operand_arr);
    (*oi->cache_ctor)(opn);
}

cuex_opn_t
cuex_opn(cuex_meta_t opr, ...)
{
    cu_rank_t N = cuex_opr_r(opr), n;
    size_t key_size = N*sizeof(cuex_t);
    va_list va;
    cu_hash_t hash;
    cuex_t *operand_arr = cu_salloc(key_size);
    cuex_t *operand_arr_end = operand_arr + N;

    /* TODO: Hack for platforms with stacks growing towards NULL.  Need
     * Autoconf macro. */
    va_start(va, opr);
    for (n = 0; n < N; ++n) {
	operand_arr[n] = va_arg(va, cuex_t);
	cu_debug_assert(operand_arr[n]);
    }
    va_end(va);

    cu_debug_assert(key_size == cuex_key_size(opr, NULL));
    hash = cu_hc_key_hash(N, (cu_word_t *)operand_arr, opr);
    if (!cuex_opr_has_ctor(opr)) {
	CU_HC(cuex_opn_t, opn, opr, key_size + CU_HCOBJ_SHIFT, hash,
	      wordaligned_eq(operand_arr, operand_arr_end, opn->operand_arr),
	      wordaligned_copy(operand_arr, operand_arr_end, opn->operand_arr));
    }
    else {
	cuex_oprinfo_t oi = cuex_oprinfo(opr);
	CU_HC(cuex_opn_t, opn, opr,
	      key_size + CU_HCOBJ_SHIFT + oi->cache_size, hash,
	      wordaligned_eq(operand_arr, operand_arr_end, opn->operand_arr),
	      opn_cct_cached(opn, oi, operand_arr, operand_arr_end));
    }
}

cuex_opn_t
cuex_opn_by_valist(cuex_meta_t opr, va_list va)
{
    cu_rank_t N = cuex_opr_r(opr), n;
    size_t key_size = N*sizeof(cuex_t);
    cu_hash_t hash;
    cuex_t *operand_arr = cu_salloc(key_size);
    cuex_t *operand_arr_end = operand_arr + N;

    /* TODO: Hack for platforms with stacks growing towards NULL.  Need
     * Autoconf macro. */
    for (n = 0; n < N; ++n) {
	operand_arr[n] = va_arg(va, cuex_t);
	cu_debug_assert(operand_arr[n]);
    }

    cu_debug_assert(key_size == cuex_key_size(opr, NULL));
    hash = cu_hc_key_hash(N, (cu_word_t *)operand_arr, opr);
    if (!cuex_opr_has_ctor(opr)) {
	CU_HC(cuex_opn_t, opn, opr, key_size + CU_HCOBJ_SHIFT, hash,
	      wordaligned_eq(operand_arr, operand_arr_end, opn->operand_arr),
	      wordaligned_copy(operand_arr, operand_arr_end, opn->operand_arr));
    }
    else {
	cuex_oprinfo_t oi = cuex_oprinfo(opr);
	CU_HC(cuex_opn_t, opn, opr,
	      key_size + CU_HCOBJ_SHIFT + oi->cache_size, hash,
	      wordaligned_eq(operand_arr, operand_arr_end, opn->operand_arr),
	      opn_cct_cached(opn, oi, operand_arr, operand_arr_end));
    }
}

cuex_opn_t
cuex_opn_by_arr(cuex_meta_t opr, void **operand_arr)
{
    cu_rank_t N = cuex_opr_r(opr);
    size_t key_size = N*sizeof(cuex_t);
    cu_hash_t hash;
    cuex_t *operand_arr_end = operand_arr + N;

    hash = cu_hc_key_hash(N, (cu_word_t *)operand_arr, opr);
    if (!cuex_opr_has_ctor(opr)) {
	CU_HC(cuex_opn_t, opn, opr, key_size + CU_HCOBJ_SHIFT, hash,
	      wordaligned_eq(operand_arr, operand_arr_end, opn->operand_arr),
	      wordaligned_copy(operand_arr, operand_arr_end, opn->operand_arr));
    }
    else {
	cuex_oprinfo_t oi = cuex_oprinfo(opr);
	CU_HC(cuex_opn_t, opn, opr,
	      key_size + CU_HCOBJ_SHIFT + oi->cache_size, hash,
	      wordaligned_eq(operand_arr, operand_arr_end, opn->operand_arr),
	      opn_cct_cached(opn, oi, operand_arr, operand_arr_end));
    }
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
