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

#include <cuex/semilattice.h>
#include <cuex/atree.h>
#include <cuex/oprinfo.h>
#include <cuoo/intf.h>
#include <inttypes.h>

typedef struct cuex_semilattice_s *cuex_semilattice_t;
struct cuex_semilattice_s
{
    CUOO_HCOBJ
    cuex_meta_t meet;
    cuex_t atree;
};

#define SL(x) ((cuex_semilattice_t)(x))

#define atree_insert(A, x) cuex_atree_insert(semilattice_key, A, x)
#define atree_find(A, x) cuex_atree_find(semilattice_key, A, (cu_word_t)(x))
#define atree_union(A0, A1) cuex_atree_left_union(semilattice_key, A0, A1)
#define atree_isecn(A0, A1) cuex_atree_left_isecn(semilattice_key, A0, A1)
#define atree_order(A0, A1) cuex_atree_order(semilattice_key, A1, A0)
#define atree_leq(A0, A1) cuex_atree_subseteq(semilattice_key, A1, A0)

cuoo_type_t cuexP_semilattice_type;
#define sl_type cuexP_semilattice_type
#define sl_meta cuoo_type_to_meta(cuexP_semilattice_type)

cu_clop_def(semilattice_key, cu_word_t, cuex_t e)
{
    return (cu_word_t)e;
}

static cuex_t
sl_new(cuex_meta_t meet, cuex_t atree)
{
    if (cuex_atree_is_singleton(atree))
	return atree;
    cuoo_hctem_decl(cuex_semilattice, tem);
    cuoo_hctem_init(cuex_semilattice, tem);
    cuoo_hctem_get(cuex_semilattice, tem)->meet = meet;
    cuoo_hctem_get(cuex_semilattice, tem)->atree = atree;
    return cuoo_hctem_new(cuex_semilattice, tem);
}

cuex_t
cuex_meetlattice_top(cuex_meta_t meet)
{
    cuoo_hctem_decl(cuex_semilattice, tem);
    cuoo_hctem_init(cuex_semilattice, tem);
    cuoo_hctem_get(cuex_semilattice, tem)->meet = meet;
    cuoo_hctem_get(cuex_semilattice, tem)->atree = cuex_atree_empty();
    return cuoo_hctem_new(cuex_semilattice, tem);
}

cuex_t
cuex_meetlattice_meet(cuex_meta_t meet, cuex_t x0, cuex_t x1)
{
    cuex_meta_t meta0 = cuex_meta(x0);
    cuex_meta_t meta1 = cuex_meta(x1);
    if (x0 == x1)
	return x0;
    else if (meta0 == sl_meta && SL(x0)->meet == meet) {
	if (meta1 == sl_meta && SL(x1)->meet == meet)
	    return sl_new(meet, atree_union(SL(x0)->atree, SL(x1)->atree));
	else
	    return sl_new(meet, atree_insert(SL(x0)->atree, x1));
    }
    else if (meta1 == sl_meta && SL(x1)->meet == meet)
	return sl_new(meet, atree_insert(SL(x1)->atree, x0));
    else
	return sl_new(meet, atree_insert(x0, x1));
}

cuex_t
cuex_meetlattice_semijoin(cuex_meta_t meet, cuex_t x0, cuex_t x1)
{
    cuex_meta_t meta0 = cuex_meta(x0);
    cuex_meta_t meta1 = cuex_meta(x1);
    if (x0 == x1)
	return x0;
    else if (meta0 == sl_meta && SL(x0)->meet == meet) {
	if (meta1 == sl_meta && SL(x1)->meet == meet)
	    return sl_new(meet, atree_isecn(SL(x0)->atree, SL(x1)->atree));
	else
	    return sl_new(meet, atree_find(SL(x0)->atree, x1));
    }
    else if (meta1 == sl_meta && SL(x1)->meet == meet)
	return sl_new(meet, atree_find(SL(x1)->atree, x0));
    else
	return sl_new(meet, cuex_atree_empty());
}

cu_order_t
cuex_meetlattice_order(cuex_meta_t meet, cuex_t x0, cuex_t x1)
{
    cuex_meta_t meta0 = cuex_meta(x0);
    cuex_meta_t meta1 = cuex_meta(x1);
    if (x0 == x1)
	return cu_order_eq;
    else if (meta0 == sl_meta && SL(x0)->meet == meet) {
	if (meta1 == sl_meta && SL(x1)->meet == meet)
	    return atree_order(SL(x0)->atree, SL(x1)->atree);
	else
	    return atree_find(SL(x0)->atree, x1)? cu_order_lt : cu_order_none;
    }
    else if (meta1 == sl_meta && SL(x1)->meet == meet)
	return atree_find(SL(x1)->atree, x0)? cu_order_gt : cu_order_none;
    else
	return cu_order_none;
}

cu_bool_t
cuex_meetlattice_leq(cuex_meta_t meet, cuex_t x0, cuex_t x1)
{
    cuex_meta_t meta0 = cuex_meta(x0);
    cuex_meta_t meta1 = cuex_meta(x1);
    if (x0 == x1)
	return cu_true;
    else if (meta0 == sl_meta && SL(x0)->meet == meet) {
	if (meta1 == sl_meta && SL(x1)->meet == meet)
	    return atree_leq(SL(x0)->atree, SL(x1)->atree);
	else
	    return atree_find(SL(x0)->atree, x1) != NULL;
    }
    else if (meta1 == sl_meta && SL(x1)->meet == meet)
	return SL(x1)->atree == NULL;
    else
	return cu_false;
}

cu_clos_def(semilattice_print_elt, cu_prot(void, cuex_t e),
	    (FILE *out; int index;))
{
    cu_clos_self(semilattice_print_elt);
    if (self->index++)
	fputs(", ", self->out);
    cu_fprintf(self->out, "%!", e);
}

static void
semilattice_print(cuex_t x, FILE *out)
{
    cuex_meta_t opr = SL(x)->meet;
    cuex_oprinfo_t info = cuex_oprinfo(opr);
    semilattice_print_elt_t cb;
    fputc('(', out);
    if (info)
	fputs(cuex_oprinfo_name(info), out);
    else
	fprintf(out, "__O%d_0x%"CUEX_PRIxMETA, cuex_opr_r(opr), opr);
    fputs(" {", out);
    cb.out = out;
    cb.index = 0;
    cuex_atree_iter(SL(x)->atree, semilattice_print_elt_prep(&cb));
    fputs("})", out);
}

static cu_word_t
semilattice_dispatch(cu_word_t intf_number, ...)
{
    switch (intf_number) {
	case CUOO_INTF_PRINT_FN:
	    return (cu_word_t)semilattice_print;
	default:
	    return CUOO_IMPL_NONE;
    }
}

void
cuexP_semilattice_init(void)
{
    cuexP_semilattice_type
	= cuoo_type_new_opaque_hcs(
	    semilattice_dispatch,
	    sizeof(struct cuex_semilattice_s) - CUOO_HCOBJ_SHIFT);
}
