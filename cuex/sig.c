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

#include <cuex/sig.h>
#include <cuex/type.h>
#include <cuex/acidefs.h>
#include <cu/idr.h>
#include <cuex/opr_priv.h>
#include <cucon/priq.h>

#define MAX_BITSIZE ((cu_offset_t)1 << (sizeof(cu_offset_t)*8 - 3))

CU_SINLINE void
clear_offsets(cu_offset_t *dst)
{
    int i;
    for (i = 0; i < CUCONF_LOG2_MAXALIGN + 2; ++i)
	dst[i] = 0;
}

CU_SINLINE void
copy_offsets(cu_offset_t *dst, cu_offset_t *src)
{
    int i;
    for (i = 0; i < CUCONF_LOG2_MAXALIGN + 2; ++i)
	dst[i] = src[i];
}

CU_SINLINE void
add_to_offsets(cu_offset_t *dst, cu_offset_t *src)
{
    int i;
    for (i = 0; i < CUCONF_LOG2_MAXALIGN + 2; ++i)
	dst[i] += src[i];
}

CU_SINLINE int
alignment_index(cu_offset_t bitalign)
{
    int i = 0;
    bitalign /= 8;
    while (bitalign) {
	++i;
	bitalign /= 2;
    }
    return i;
}

void
cuex_o4aci_sigprod_cache_cct(cuex_opn_t e)
{
    int i;
    cuex_opr_sigprod_cache_t cache = cuex_opn_cache_r(e, 4);
    cuex_opn_t left = cuex_aci_left(e);
    cuex_opn_t right = cuex_aci_right(e);
    cuex_t type;
    cu_dprintf("cuex.sig", "Constructing %!", e);
    cu_debug_assert(cuex_meta(e) == CUEX_O4ACI_SIGPROD);
    if (cuex_is_aci_identity(CUEX_O4ACI_SIGPROD, left)) {
	if (cuex_is_aci_identity(CUEX_O4ACI_SIGPROD, right))
	    clear_offsets(cache->alloc_size);
	else {
	    cuex_opr_sigprod_cache_t cache_right = cuex_opn_cache_r(right, 4);
	    copy_offsets(cache->alloc_size, cache_right->alloc_size);
	}
    }
    else if (cuex_is_aci_identity(CUEX_O4ACI_SIGPROD, right)) {
	cuex_opr_sigprod_cache_t cache_left = cuex_opn_cache_r(left, 4);
	copy_offsets(cache->alloc_size, cache_left->alloc_size);
    }
    else {
	cuex_opr_sigprod_cache_t cache_left = cuex_opn_cache_r(left, 4);
	cuex_opr_sigprod_cache_t cache_right = cuex_opn_cache_r(right, 4);
	for (i = 0; i < CUCONF_LOG2_MAXALIGN + 2; ++i)
	    cache->alloc_size[i] = cache_left->alloc_size[i]
				 + cache_right->alloc_size[i];
    }
    type = cuex_aci_at(e, 1);
    i = alignment_index(cuex_type_bitalign(type));
    cache->alloc_size[i] += cuex_type_bitsize(type);

    /* overflow check */
    cu_debug_assert(cuex_type_bitsize(e) < MAX_BITSIZE);
}

void
cuex_o2_sigbase_cache_cct(cuex_opn_t e)
{
    cuex_opn_t eB = cuex_opn_at(e, 0);
    cuex_opn_t eD = cuex_opn_at(e, 1);
    cuex_opr_sigprod_cache_t e_cache;
    int eB_aindex;

    /* The operands of sigbase/2 must be sigbase/2 or sigprod/2aci2
     * expressions. */
    cu_debug_assert(cuex_meta(eB) == CUEX_O2_SIGBASE ||
		    cuex_is_aci(CUEX_O4ACI_SIGPROD, eB));
    cu_debug_assert(cuex_meta(eD) == CUEX_O2_SIGBASE ||
		    cuex_is_aci(CUEX_O4ACI_SIGPROD, eD));
    e_cache = cuex_opn_cache_r(e, 2);
    if (cuex_is_aci_identity(CUEX_O4ACI_SIGPROD, eD))
	memset(e_cache->alloc_size, 0, sizeof(e_cache->alloc_size));
    else {
	cuex_opr_sigprod_cache_t eD_cache = cuex_opn_cache(eD);
	memcpy(e_cache->alloc_size, eD_cache->alloc_size,
	       sizeof(e_cache->alloc_size));
    }
    eB_aindex = alignment_index(cuex_type_bitalign(eB));
    e_cache->alloc_size[eB_aindex] += cuex_type_bitsize(eB);

    /* overflow check */
    cu_debug_assert(cuex_type_bitsize(e) < MAX_BITSIZE);
}

static cu_bool_t
sigprod_conj(cuex_opn_t e, cu_offset_t *bitoff_arr,
	     cu_clop(cb, cu_bool_t, cu_idr_t, cuex_t, cu_offset_t))
{
    cuex_t type;
    int aindex;
    cuex_t eL;
tailcall:
    if (cuex_is_aci_identity(CUEX_O4ACI_SIGPROD, e))
	return cu_true;
    cu_debug_assert(cuex_meta(e) == CUEX_O4ACI_SIGPROD);
    eL = cuex_aci_left(e);
    if (!cuex_is_aci_identity(CUEX_O4ACI_SIGPROD, eL))
	if (!sigprod_conj(eL, bitoff_arr, cb))
	    return cu_false;
    type = cuex_aci_at(e, 1);
    aindex = alignment_index(cuex_type_bitalign(type));
    if (!cu_call(cb, cuex_aci_at(e, 0), type, bitoff_arr[aindex]))
	return cu_false;
    bitoff_arr[aindex] += cuex_type_bitsize(type);
    e = cuex_aci_right(e);
    goto tailcall;
}

static cu_bool_t
sigbase_conj(cuex_opn_t e, cu_offset_t *bitoff_arr,
	     cu_clop(cb, cu_bool_t, cu_idr_t, cuex_t, cu_offset_t))
{
    cuex_meta_t e_meta;
    while ((e_meta = cuex_meta(e)) == CUEX_O2_SIGBASE) {
	cuex_t eB = cuex_opn_at(e, 0);
	if (!cuex_is_aci_identity(CUEX_O4ACI_SIGPROD, eB)) {
	    int i;
	    cu_offset_t eB_bitalign = cuex_type_bitalign(eB);
	    cu_offset_t eB_aindex = alignment_index(eB_bitalign);
	    cu_offset_t sub_bitoff[CUCONF_LOG2_MAXALIGN + 2];
	    cuex_opr_sigprod_cache_t eB_cache;
	    eB_cache = cuex_opn_cache(eB);
	    sub_bitoff[CUCONF_LOG2_MAXALIGN + 1] = bitoff_arr[eB_aindex];
	    i = CUCONF_LOG2_MAXALIGN + 2;
	    while (--i > 0)
		sub_bitoff[i - 1] = sub_bitoff[i] + eB_cache->alloc_size[i];
	    if (!sigbase_conj(eB, sub_bitoff, cb))
		return cu_false;
	    bitoff_arr[eB_aindex] += cuex_type_bitsize(eB);
	}
	e = cuex_opn_at(e, 1);
    }
    cu_debug_assert(cuex_is_aci(CUEX_O4ACI_SIGPROD, e));
    return sigprod_conj(e, bitoff_arr, cb);
}

cu_bool_t
cuex_sig_conj(cuex_opn_t e,
	      cu_clop(cb, cu_bool_t, cu_idr_t, cuex_t, cu_offset_t))
{
    cu_offset_t bitoff_arr[CUCONF_LOG2_MAXALIGN + 2];
    int i;
    cuex_opr_sigprod_cache_t cache;
    if (cuex_is_aci_identity(CUEX_O4ACI_SIGPROD, e))
	return cu_true;
    cu_debug_assert(cuex_meta(e) == CUEX_O4ACI_SIGPROD ||
		    cuex_meta(e) == CUEX_O2_SIGBASE);
    cache = cuex_opn_cache(e);
    bitoff_arr[CUCONF_LOG2_MAXALIGN + 1] = 0;
    i = CUCONF_LOG2_MAXALIGN + 2;
    while (--i > 0)
	bitoff_arr[i - 1] = bitoff_arr[i] + cache->alloc_size[i];
    return sigbase_conj(e, bitoff_arr, cb);
}

cuex_t
cuex_sig_find_type(cuex_opn_t sig, cu_idr_t label)
{
tailcall:
    switch (cuex_meta(sig)) {
	    cuex_t e;
	case CUEX_O2_SIGBASE:
	    e = cuex_sig_find_type(cuex_opn_at(sig, 1), label);
	    if (e)
		return e;
	    else {
		sig = cuex_opn_at(sig, 0);
		goto tailcall;
	    }
	case CUEX_O4ACI_SIGPROD:
	    e = cuex_aci_find(sig, label);
	    if (e)
		return cuex_aci_at(e, 1);
	    else
		return NULL;
	case CUEX_O0ACI_SIGPROD:
	    return NULL;
	default:
	    cu_debug_unreachable();
	    return NULL;
    }
}


cu_bool_t
cuex_sig_find(cuex_opn_t e, cu_idr_t idr,
	      cuex_t *type_out, cu_offset_t *bitoff_out)
{
    int aindex;
    cu_offset_t bitoff_arr[CUCONF_LOG2_MAXALIGN + 2];
    cu_offset_t bitoff;
    cuex_opr_sigprod_cache_t etop_cache;
    if (cuex_is_aci_identity(CUEX_O4ACI_SIGPROD, e))
	return cu_false;
    clear_offsets(bitoff_arr);
    etop_cache = cuex_opn_cache(e);
    for (;;) {
	cuex_meta_t e_meta = cuex_meta(e);
	if (e_meta == CUEX_O2_SIGBASE) {
	    cuex_t eB = cuex_opn_at(e, 0);
	    aindex = alignment_index(cuex_type_bitalign(eB));
	    if (cuex_sig_find(eB, idr, type_out, &bitoff)) {
		bitoff += bitoff_arr[aindex];
		break;
	    }
	    else {
		bitoff_arr[aindex] += cuex_type_bitsize(eB);
		e = cuex_opn_at(e, 1);
	    }
	}
	else if (cuex_lt(idr, cuex_aci_key(e)))
	    e = cuex_aci_left(e);
	else {
	    cuex_t eL = cuex_aci_left(e);
	    cuex_t type;
	    if (!cuex_is_aci_identity(CUEX_O4ACI_SIGPROD, eL)) {
		cuex_opr_sigprod_cache_t eL_cache = cuex_opn_cache_r(eL, 4);
		add_to_offsets(bitoff_arr, eL_cache->alloc_size);
	    }
	    type = cuex_aci_at(e, 1);
	    aindex = alignment_index(cuex_type_bitalign(type));
	    if (cuex_eq(idr, cuex_aci_key(e))) {
		bitoff = bitoff_arr[aindex];
		*type_out = type;
		break;
	    }
	    else {
		bitoff_arr[aindex] += cuex_type_bitsize(type);
		e = cuex_aci_right(e);
	    }
	}
	if (cuex_is_aci_identity(CUEX_O4ACI_SIGPROD, e))
	    return cu_false;
    }
    /* 'aindex' is index of relevant alignment
     * 'bitoff' is set relative to start of relevant alignment
     * '*type_out' is set */
    while (++aindex < CUCONF_LOG2_MAXALIGN + 2)
	bitoff += etop_cache->alloc_size[aindex];
    *bitoff_out = bitoff;
    return cu_true;
}

cu_offset_t
cuex_sig_bitsize(cuex_opn_t e)
{
    if (cuex_is_aci_identity(CUEX_O4ACI_SIGPROD, e))
	return 0;
    else {
	int i;
	cuex_opr_sigprod_cache_t cache;
	cu_offset_t *top_bitsizes;
	cu_offset_t bitsize;
	cu_offset_t bitalign;
	cu_debug_assert(cuex_meta(e) == CUEX_O4ACI_SIGPROD ||
			cuex_meta(e) == CUEX_O2_SIGBASE);
	cache = cuex_opn_cache(e);
	top_bitsizes = cache->alloc_size;
	bitsize = top_bitsizes[0];
	for (i = 1; i < CUCONF_LOG2_MAXALIGN + 2; ++i)
	    bitsize += top_bitsizes[i];
	bitalign = cuex_sig_bitalign(e);
	return bitsize + bitalign - 1 - (bitsize - 1)%bitalign;;
    }
}

cu_offset_t
cuex_sig_bitalign(cuex_opn_t e)
{
    if (!cuex_is_aci_identity(CUEX_O4ACI_SIGPROD, e)) {
	cuex_opr_sigprod_cache_t cache;
	cu_offset_t *top_sizes;
	int i;
	cu_debug_assert(cuex_meta(e) == CUEX_O4ACI_SIGPROD ||
			cuex_meta(e) == CUEX_O2_SIGBASE);
	cache = cuex_opn_cache(e);
	top_sizes = cache->alloc_size;
	for (i = CUCONF_LOG2_MAXALIGN + 1; i > 0; --i)
	    if (top_sizes[i])
		return 4 << i;
    }
    return 1;
}

cuex_opn_t
cuex_sig_va(int n, va_list va)
{
    cuex_opn_t r = cuex_aci_identity(CUEX_O4ACI_SIGPROD);
    while (n) {
	char const *s_idr = va_arg(va, char const *);
	cuex_t type = va_arg(va, cuex_t);
	r = cuex_aci_insert(CUEX_O4ACI_SIGPROD, r,
			    cu_idr_by_cstr(s_idr), type);
	--n;
    }
    return r;
}

cuex_opn_t
cuex_sig_v(int n, ...)
{
    cuex_opn_t sig;
    va_list va;
    va_start(va, n);
    sig = cuex_sig_va(n, va);
    va_end(va);
    return sig;
}


typedef struct sigterm_data_s *sigterm_data_t;
struct sigterm_data_s
{
    cu_idr_t label;
    cuex_t type;
    cu_offset_t bitoff;
};

cu_clop_def(sigterm_loweroffset, cu_bool_t, void *x, void *y)
{
    return ((sigterm_data_t)x)->bitoff < ((sigterm_data_t)y)->bitoff;
}

cu_clos_def(enqueue_sig,
	    cu_prot(cu_bool_t, cu_idr_t label, cuex_t type, cu_offset_t bitoff),
    ( struct cucon_priq_s priq; ))
{
    cu_clos_self(enqueue_sig);
    sigterm_data_t comp = cu_galloc(sizeof(struct sigterm_data_s));
    comp->bitoff = bitoff;
    comp->label = label;
    comp->type = type;
    cucon_priq_insert(&self->priq, comp);
    return cu_true;
}

cu_bool_t
cuex_sig_conj_byoffset(cuex_t sig,
		       cu_clop(f, cu_bool_t, cu_idr_t label, cuex_t type,
					     cu_offset_t bitoff))
{
    enqueue_sig_t g;
    cucon_priq_cct(&g.priq, sigterm_loweroffset);
    cuex_sig_conj(sig, enqueue_sig_prep(&g));
    while (!cucon_priq_is_empty(&g.priq)) {
	sigterm_data_t comp = cucon_priq_pop_front(&g.priq);
	if (!cu_call(f, comp->label, comp->type, comp->bitoff))
	    return cu_false;
    }
    return cu_true;
}

cu_clos_def(sig_dump,
	    cu_prot(cu_bool_t, cu_idr_t label, cuex_t type, cu_offset_t bitoff),
    ( FILE *out; ))
{
    cu_clos_self(sig_dump);
    cu_debug_assert(label && type);
    fprintf(self->out, "%8x %6x %02d ",
	    bitoff, cuex_type_bitsize(type), cuex_type_bitalign(type));
    cu_fprintf(self->out, "%!: %!\n", label, type);
    return cu_true;
}

void
cuex_sig_dump(cuex_t sig, FILE *out)
{
    sig_dump_t dump;
    dump.out = out;
    cuex_sig_conj_byoffset(sig, sig_dump_prep(&dump));
}

cuex_opn_t cuexP_sig_identity;

void
cuexP_sig_init()
{
    cuexP_sig_identity = cuex_aci_identity(CUEX_O4ACI_SIGPROD);
}
