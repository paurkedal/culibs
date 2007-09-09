/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cuex/labelling.h>
#include <cuex/opn.h>
#include <cuex/oprdefs.h>
#include <cuex/atree.h>
#include <cuoo/halloc.h>
#include <cuoo/properties.h>
#include <cuoo/intf.h>

#define tuple(l, e) cuex_opn(CUEX_OR_TUPLE(2), l, e)

cu_clop_def(get_key, cu_word_t, cuex_t e)
{ return (cu_word_t)cuex_opn_at(e, 0); }

#define atree_find(T, l) cuex_atree_find(get_key, T, (cu_word_t)(l))
#define atree_insert(T, e) cuex_atree_insert(get_key, T, e)
#define atree_erase(T, l) cuex_atree_erase(get_key, T, (cu_word_t)(l))
#define atree_union(T0, T1) cuex_atree_union(get_key, T0, T1)
#define atree_isecn(T0, T1) cuex_atree_isecn(get_key, T0, T1)
#define atree_order(T0, T1) cuex_atree_order(get_key, T0, T1)
#define atree_subseteq(T0, T1) cuex_atree_subseteq(get_key, T0, T1)

cuoo_stdtype_t cuexP_labelling_type;

typedef struct cuex_labelling_s *cuex_labelling_t;
struct cuex_labelling_s
{
    CUOO_HCOBJ
    cuex_t atree;
};

#define LABELLING(e) ((cuex_labelling_t)(e))

static cuex_labelling_t
labelling(cuex_t atree)
{
    cuoo_hctem_decl(cuex_labelling, key);
    cuoo_hctem_init(cuex_labelling, key);
    cuoo_hctem_get(cuex_labelling, key)->atree = atree;
    return cuoo_hctem_new(cuex_labelling, key);
}

cuex_t
cuex_labelling_empty(void)
{
    return labelling(NULL);
}

cuex_t
cuex_labelling_singleton(cuex_t l, cuex_t e)
{
    return labelling(tuple(l, e));
}

cuex_t
cuex_labelling_by_arglist(cuex_t l, cuex_t e, ...)
{
    cuex_t L;
    va_list va;
    va_start(va, e);
    L = cuex_labelling_insert_valist(cuex_labelling_singleton(l, e), va);
    va_end(va);
    return L;
}

cuex_t
cuex_labelling_insert(cuex_t L, cuex_t l, cuex_t e)
{
    if (!cuex_is_labelling(L))
	cu_bugf("First argument of cuex_labelling_insert must be a "
		"labelling.");
    return labelling(atree_insert(LABELLING(L)->atree, tuple(l, e)));
}

cuex_t
cuex_labelling_insert_valist(cuex_t L, va_list va)
{
    cuex_t l, v;
    while ((l = va_arg(va, cuex_t)) != NULL) {
	v = va_arg(va, cuex_t);
	cu_debug_assert(v);
	L = cuex_labelling_insert(L, l, v);
    }
    return L;
}

cuex_t
cuex_labelling_insert_arglist(cuex_t L, ...)
{
    va_list va;
    va_start(va, L);
    L = cuex_labelling_insert_valist(L, va);
    va_end(va);
    return L;
}

cuex_t
cuex_labelling_deep_insert(cu_clop(merge, cuex_t, cuex_t, cuex_t),
			   cuex_t L, cuex_t l, cuex_t e)
{
    if (!cuex_is_labelling(L))
	cu_bugf("Second argument of cuex_labelling_insert must be a "
		"labelling.");
    return labelling(cuex_atree_deep_insert(get_key, merge, L, tuple(l, e)));
}

cuex_t
cuex_labelling_find(cuex_t L, cuex_t l)
{
    cuex_t e;
    if (!cuex_is_labelling(L))
	cu_bugf("First argument of cuex_labelling_find must be a labelling.");
    e = atree_find(LABELLING(L)->atree, l);
    if (e)
	return cuex_opn_at(e, 1);
    else
	return NULL;
}

cuex_t
cuex_labelling_left_union(cuex_t L0, cuex_t L1)
{
    if (!cuex_is_labelling(L0))
	cu_bugf("First argument of cuex_labelling_union must be a labelling.");
    if (!cuex_is_labelling(L1))
	cu_bugf("Second argument of cuex_labelling_union must be a "
		"labelling.");
    return labelling(cuex_atree_left_union(get_key,
					   LABELLING(L0)->atree,
					   LABELLING(L1)->atree));
}

cuex_t
cuex_labelling_deep_union(cu_clop(merge, cuex_t, cuex_t, cuex_t),
			  cuex_t L0, cuex_t L1)
{
    if (!cuex_is_labelling(L0))
	cu_bugf("First argument of cuex_labelling_deep_union must be a "
		"labelling.");
    if (!cuex_is_labelling(L1))
	cu_bugf("Second argument of cuex_labelling_deep_union must be a "
		"labelling.");
    return labelling(cuex_atree_deep_union_iv(get_key, 1, merge,
					      LABELLING(L0)->atree,
					      LABELLING(L1)->atree));
}

cuex_t
cuex_labelling_left_isecn(cuex_t L0, cuex_t L1)
{
    if (!cuex_is_labelling(L0))
	cu_bugf("First argument of cuex_labelling_left_isecn must be a "
		"labelling.");
    if (!cuex_is_labelling(L1))
	cu_bugf("Second argument of cuex_labelling_left_isecn must be a "
		"labelling.");
    return labelling(cuex_atree_left_isecn(get_key,
					   LABELLING(L0)->atree,
					   LABELLING(L1)->atree));
}

cuex_t
cuex_labelling_deep_isecn(cu_clop(merge, cuex_t, cuex_t, cuex_t),
			  cuex_t L0, cuex_t L1)
{
    if (!cuex_is_labelling(L0))
	cu_bugf("First argument of cuex_labelling_deep_isecn must be a "
		"labelling.");
    if (!cuex_is_labelling(L1))
	cu_bugf("Second argument of cuex_labelling_deep_isecn must be a "
		"labelling.");
    return labelling(cuex_atree_deep_isecn_iv(get_key, 1, merge,
					      LABELLING(L0)->atree,
					      LABELLING(L1)->atree));
}

cu_bool_t
cuex_labelling_conj_kv(cuex_t L, cu_clop(f, cu_bool_t, cuex_t l, cuex_t e))
{
    if (!cuex_is_labelling(L))
	cu_bugf("First argument of cuex_labelling_conj must be a labelling.");
    return cuex_atree_conj_kv(LABELLING(L)->atree, f);
}

cuex_t
cuex_labelling_image(cuex_t L, cu_clop(f, cuex_t, cuex_t v))
{
    if (!cuex_is_labelling(L))
	cu_bugf("First argument of cuex_labelling_image must be a labelling.");
    return labelling(cuex_atree_isokey_image_iv(LABELLING(L)->atree, 1, f));
}

cuex_t
cuex_labelling_image_kv(cuex_t L, cu_clop(f, cuex_t, cuex_t l, cuex_t e))
{
    if (!cuex_is_labelling(L))
	cu_bugf("First argument of cuex_labelling_image must be a labelling.");
    return labelling(cuex_atree_isokey_image_kv(LABELLING(L)->atree, f));
}

cu_clop_def(labelling_conj, cu_bool_t, cuex_t L, cu_clop(f, cu_bool_t, cuex_t))
{
    return cuex_atree_conj_iv(LABELLING(L)->atree, 1, f);
}

cu_clop_def(labelling_tran, cuex_t, cuex_t L, cu_clop(f, cuex_t, cuex_t))
{
    return labelling(cuex_atree_isokey_image_iv(LABELLING(L)->atree, 1, f));
}

cu_clos_def(labelling_print_elt, cu_prot(void, cuex_t e),
  ( FILE *out;
    int index; ))
{
    cuex_t l, x;
    cu_clos_self(labelling_print_elt);
    if (self->index++)
	fputs(", ", self->out);
    cu_debug_assert(cuex_meta_is_opr(cuex_meta(e)) &&
		    cuex_opr_r(cuex_meta(e)) >= 2);
    l = cuex_opn_at(e, 0);
    x = cuex_opn_at(e, 1);
    cu_fprintf(self->out, "%!: %!", l, x);
}

static void
labelling_print(void *L, FILE *out)
{
    labelling_print_elt_t cl;
    fputc('(', out);
    cl.out = out;
    cl.index = 0;
    cuex_atree_iter(LABELLING(L)->atree, labelling_print_elt_prep(&cl));
    fputc(')', out);
}

static cu_word_t
labelling_impl(cu_word_t intf_number, ...)
{
    switch (intf_number) {
	case CUOO_INTF_PRINT_FN: return (cu_word_t)labelling_print;
	default: return CUOO_IMPL_NONE;
    }
}

void
cuexP_labelling_init(void)
{
    cuexP_labelling_type = cuoo_stdtype_new_hcs(
	labelling_impl, sizeof(struct cuex_labelling_s) - CUOO_HCOBJ_SHIFT);
    cuexP_labelling_type->conj = labelling_conj;
    cuexP_labelling_type->tran = labelling_tran;
}
