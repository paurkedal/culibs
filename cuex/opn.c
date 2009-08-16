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
#include <cuex/compound.h>
#include <cuex/intf.h>
#include <cuex/oprdefs.h>
#include <cudyn/misc.h>
#include <cuoo/halloc.h>
#include <cu/ptr_seq.h>
#include <cu/ptr.h>

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
	    CUOO_HCOBJ_KEY_SIZEW(N*sizeof(cuex_t) + CUOO_HCOBJ_SHIFT),
	    operand_arr);
    else {
	cct_cached_t cct;
	cuex_oprinfo_t oi = cuex_oprinfo(opr);
	cct.oi = oi;
	return cuexP_hxalloc_raw(
	    opr,
	    CUOO_HCOBJ_ALLOC_SIZEG(N*sizeof(cuex_t) + CUOO_HCOBJ_SHIFT
				   + oi->cache_size),
	    CUOO_HCOBJ_KEY_SIZEW(N*sizeof(cuex_t) + CUOO_HCOBJ_SHIFT),
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
	    CUOO_HCOBJ_KEY_SIZEW(N*sizeof(cuex_t) + CUOO_HCOBJ_SHIFT),
	    operand_arr);
    else {
	cct_cached_t cct;
	cuex_oprinfo_t oi = cuex_oprinfo(opr);
	cct.oi = oi;
	return cuexP_hxalloc_raw(
	    opr,
	    CUOO_HCOBJ_ALLOC_SIZEG(N*sizeof(cuex_t) + CUOO_HCOBJ_SHIFT
				   + oi->cache_size),
	    CUOO_HCOBJ_KEY_SIZEW(N*sizeof(cuex_t) + CUOO_HCOBJ_SHIFT),
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
    return cuexP_hxalloc_raw(
	opr,
	CUOO_HCOBJ_ALLOC_SIZEG(N*sizeof(cuex_t) + CUOO_HCOBJ_SHIFT
			       + oi->cache_size),
	CUOO_HCOBJ_KEY_SIZEW(N*sizeof(cuex_t) + CUOO_HCOBJ_SHIFT),
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

cu_rank_t
cuex_arity(cuex_t e)
{
    cuex_meta_t e_meta = cuex_meta(e);
    if (cuex_meta_is_opr(e_meta))
	return cuex_opr_r(e_meta);
    else if (cuex_meta_is_type(e_meta)) {
	cuex_intf_compound_t ci;
	ci = cuex_type_compound(cuoo_type_from_meta(e_meta));
	if (ci)
	    return cuex_compound_size(ci, e);
    }
    return 0;
}

static void *
_opn_ncomm_source_get(cu_ptr_source_t base)
{
    cuex_opn_source_t src = cu_from(cuex_opn_source, cu_ptr_source, base);
    if (src->i >= cuex_opn_arity(src->e))
	return NULL;
    else
	return cuex_opn_at(src->e, src->i++);
}

void
cuex_opn_ncomm_source_init(cuex_opn_source_t src, cuex_t e)
{
    cu_debug_assert(cuex_meta_is_opr(cuex_meta(e)));
    src->i = 0;
    src->e = e;
    cu_ptr_source_init(cu_to(cu_ptr_source, src), _opn_ncomm_source_get);
}

cu_ptr_source_t
cuex_opn_ncomm_source(cuex_t e)
{
    cuex_opn_source_t src;
    src = cu_gnew(struct cuex_opn_source_s);
    cuex_opn_ncomm_source_init(src, e);
    return cu_to(cu_ptr_source, src);
}

static void *
_opn_comm_source_get(cu_ptr_source_t base)
{
    cuex_opn_source_t src = cu_from(cuex_opn_source, cu_ptr_source, base);
    if (src->i >= cuex_opn_arity(src->e))
	return NULL;
    else {
	int i = src->i++;
	return cuex_o2_metapair(cudyn_int(i), cuex_opn_at(src->e, i));
    }
}

void
cuex_opn_comm_source_init(cuex_opn_source_t src, cuex_t e)
{
    cu_debug_assert(cuex_meta_is_opr(cuex_meta(e)));
    src->i = 0;
    src->e = e;
    cu_ptr_source_init(cu_to(cu_ptr_source, src), _opn_comm_source_get);
}

cu_ptr_source_t
cuex_opn_comm_source(cuex_t e)
{
    cuex_opn_source_t src;
    src = cu_gnew(struct cuex_opn_source_s);
    cuex_opn_comm_source_init(src, e);
    return cu_to(cu_ptr_source, src);
}
