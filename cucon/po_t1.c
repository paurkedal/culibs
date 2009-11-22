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

#include <cucon/po.h>
#include <cucon/pmap.h>
#include <cu/str.h>
#include <cu/test.h>

static cucon_po_t
po_of_random_ints(unsigned int mask, unsigned int n)
{
    unsigned int i;
    cucon_po_t po;
    struct cucon_list lst;
    po = cucon_po_new_mem(sizeof(unsigned int), sizeof(unsigned int));
    *(unsigned int *)cucon_poelt_get_mem(cucon_po_bot(po)) = 0;
    *(unsigned int *)cucon_poelt_get_mem(cucon_po_top(po)) = mask;
    cucon_list_init(&lst);
    for (i = 0; i < n; ++i) {
	cucon_listnode_t it;
	unsigned int k0 = lrand48() % (mask - 2) + 1;
	cucon_poelt_t e0;
	for (it = cucon_list_begin(&lst); it != cucon_list_end(&lst);
		it = cucon_listnode_next(it)) {
	    cucon_poelt_t e1 = cucon_listnode_ptr(it);
	    if (*(unsigned int *)cucon_poelt_get_mem(e1) == k0)
		break;
	}
	if (it != cucon_list_end(&lst))
	    continue;
	e0 = cucon_po_insert_mem(po, sizeof(unsigned int));
	*(unsigned int *)cucon_poelt_get_mem(e0) = k0;
	for (it = cucon_list_begin(&lst); it != cucon_list_end(&lst);
		it = cucon_listnode_next(it)) {
	    cucon_poelt_t e1 = cucon_listnode_ptr(it);
	    unsigned int k1 = *(unsigned int *)cucon_poelt_get_mem(e1);
	    if (k0 != k1) {
		unsigned int k01 = k0 & k1;
		if (k0 == k01)
		    cucon_po_constrain_prec(po, e0, e1);
		else if (k1 == k01)
		    cucon_po_constrain_prec(po, e1, e0);
	    }
	}
	cucon_list_append_ptr(&lst, e0);
    }
    return po;
}

CU_SINLINE cucon_pocmp_t
bitset_cmp(unsigned int k0, unsigned int k1)
{
    unsigned int k01;
    if (k0 == k1)
	return cucon_pocmp_eq;
    k01 = k0 & k1;
    if (k0 == k01)
	return cucon_pocmp_prec;
    if (k1 == k01)
	return cucon_pocmp_succ;
    return cucon_pocmp_unord;
}

CU_SINLINE unsigned int
elt_val(cucon_poelt_t e)
{ return *(unsigned int *)cucon_poelt_get_mem(e); }

cu_clos_def(elt_cmp,
	    cu_prot(cucon_pocmp_t, cucon_poelt_t elt),
	    ( unsigned int k0; ))
{
    cu_clos_self(elt_cmp);
    unsigned int k0 = self->k0;
    unsigned int k1 = *(unsigned int *)cucon_poelt_get_mem(elt);
    return bitset_cmp(k0, k1);
}

cu_clop_def(elt_print, cu_str_t, cucon_poelt_t e)
{
    return cu_str_new_fmt("%x", *(unsigned int *)cucon_poelt_get_mem(e));
}

cu_clos_def(pick_first,
	    cu_prot(cu_bool_t, void const *val),
	( cucon_poelt_t val; ))
{
    cu_clos_self(pick_first);
    self->val = (void *)val;
    return cu_false;
}

static cucon_poelt_t
random_elt(cucon_po_t po, unsigned int mask)
{
    unsigned int k = lrand48() & mask;
    elt_cmp_t cmp;
    struct cucon_pmap range, preds, succs;
    pick_first_t first;
    cu_clop(first_clop, cu_bool_t, void const *) = pick_first_prep(&first);
    cmp.k0 = k;
    cucon_pmap_init(&range);
    cucon_pmap_init(&preds);
    cucon_pmap_init(&succs);
    cucon_po_range_and_bounds_of_fn(cucon_po_bot(po), cucon_po_top(po),
				    elt_cmp_prep(&cmp),
				    &range, &preds, &succs);
    if (!cucon_pmap_is_empty(&range)) {
	cu_test_assert(cucon_pmap_size(&range) == 1);
	cucon_pmap_conj_keys(&range, first_clop);
	cu_test_assert(k == elt_val(first.val));
	return first.val;
    }
    if (cucon_pmap_size(&preds) > 0 && lrand48() % 2) {
	cucon_pmap_conj_keys(&preds, first_clop);
	cu_test_assert(bitset_cmp(k, elt_val(first.val)) == cucon_pocmp_succ);
	return first.val;
    }
    else {
	cu_test_assert(cucon_pmap_size(&succs) > 0);
	cucon_pmap_conj_keys(&succs, first_clop);
	cu_test_assert(bitset_cmp(k, elt_val(first.val)) == cucon_pocmp_prec);
	return first.val;
    }
}

void
check_closed_range_and_succs()
{
    unsigned int i;
    unsigned int mask = 0xff;
    unsigned int po_size = 128;
    cucon_po_t po = po_of_random_ints(mask, po_size);
    FILE *gvizout = fopen("tmp.gviz", "w");
    if (gvizout)
	cucon_po_print_gviz(po, elt_print, gvizout);
    for (i = 0; i < 100; ++i) {
	struct cucon_pmap range, succs;
	cucon_poelt_t e0 = random_elt(po, mask);
	cucon_poelt_t e1 = random_elt(po, mask);
	cucon_pmap_init(&range);
	cucon_pmap_init(&succs);
	printf("[%2x, %2x] ",
	       *(unsigned int *)cucon_poelt_get_mem(e0),
	       *(unsigned int *)cucon_poelt_get_mem(e1));
	if (cucon_po_closed_range_and_succs(e0, e1, &range, &succs)) {
	    printf("%ld %ld\n",
		   (long)cucon_pmap_size(&range),
		   (long)cucon_pmap_size(&succs));
	    /* TODO */
	}
	else {
	    cu_test_assert(!cucon_po_prec(e0, e1));
	    cu_test_assert(cucon_pmap_size(&range) == 0);
	    cu_test_assert(cucon_pmap_size(&succs) == 0);
	    printf("empty\n");
	}
    }
}

int
main()
{
    cu_init();
    check_closed_range_and_succs();
    return 2*!!cu_test_bug_count();
}
