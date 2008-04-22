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

#include <cuex/atree.h>
#include <cuex/oprdefs.h>
#include <cuex/opn.h>
#include <cudyn/misc.h>
#include <cu/test.h>
#include <cucon/pset.h>

//#define o2_tuple(x, y) cuex_opn(CUEX_OR_TUPLE(2), x, y)
#define o2_tuple(x, y) cuex_o2_apply(x, y)

cu_clop_def(opn0word, cu_word_t, cuex_t e)
{
    return (cu_word_t)cuex_opn_at(e, 0);
}

cu_clop_def(incr1, cuex_t, cuex_t e)
{
    int rhs_index = cudyn_to_uint(cuex_opn_at(e, 1));
    return o2_tuple(cuex_opn_at(e, 0), cudyn_uint(rhs_index));
}

void
test(int N, cu_bool_t print)
{
    struct cucon_pset_s keys;
    int i;
    cuex_t e = cuex_atree_empty();

    cucon_pset_init(&keys);
    for (i = 0; i < N; ++i) {
	cuex_t key;
	cuex_t val;
	cuex_t ep;
	key = cudyn_uint(lrand48() % (i + 1));
	val = o2_tuple(key, key);
	ep = cuex_atree_insert(opn0word, e, val);
	if (cucon_pset_insert(&keys, val)) {
	    cu_test_assert(e != ep);
	    cu_test_assert(cuex_atree_erase(opn0word, ep, (cu_word_t)key) == e);
	}
	else
	    cu_test_assert(e == ep);
	cu_test_assert(cuex_atree_find(opn0word, ep, (cu_word_t)key) == val);
	if (print)
	    cu_fprintf(stdout, "%! ∪ {%!}\n  = %! (depth = %d)\n", e, val, ep,
		       cuex_atree_depth(ep));
	e = ep;
    }

    {
	void *itr = cu_salloc(cuex_atree_itr_size(e));
	cuex_t ep = cuex_atree_empty();
	cuex_t val;
	cuex_atree_itr_init(itr, e);
	while ((val = cuex_atree_itr_get(itr)))
	    ep = cuex_atree_insert(opn0word, ep, val);
	cu_test_assert(cuex_atree_itr_get(itr) == NULL);
	cu_test_assert(e == ep);
    }
}

void
test_union_isecn(int N, cu_bool_t print)
{
    int i;
    cuex_t e0 = cuex_atree_empty();
    cuex_t e1 = cuex_atree_empty();
    cuex_t eU = cuex_atree_empty();
    cuex_t eI = cuex_atree_empty();
    cuex_t img0, img1, imgU;
    for (i = 0; i < N; ++i) {
	unsigned int r = lrand48();
	cuex_t key = cudyn_uint((r >> 3) % N);
	cuex_t val = o2_tuple(key, key);
	eU = cuex_atree_insert(opn0word, eU, val);
	switch (r & 7) {
	    case 0: case 1: case 2:
		e0 = cuex_atree_insert(opn0word, e0, val);
		if (cuex_atree_find(opn0word, e1, (cu_word_t)key))
		    eI = cuex_atree_insert(opn0word, eI, val);
		break;
	    case 3: case 4: case 5:
		e1 = cuex_atree_insert(opn0word, e1, val);
		if (cuex_atree_find(opn0word, e0, (cu_word_t)key))
		    eI = cuex_atree_insert(opn0word, eI, val);
		break;
	    case 6: case 7:
		e0 = cuex_atree_insert(opn0word, e0, val);
		e1 = cuex_atree_insert(opn0word, e1, val);
		eI = cuex_atree_insert(opn0word, eI, val);
		break;
	}
    }
    if (print)
	cu_fprintf(stdout,
		   ":== LHS: %!\n    RHS: %!\n  union: %!\n  isecn: %!\n",
		   e0, e1, eU, eI);
    cu_test_assert(cuex_atree_left_union(opn0word, e0, e1) == eU);
    cu_test_assert(cuex_atree_left_isecn(opn0word, e0, e1) == eI);

    /* Images */
    img0 = cuex_atree_image(e0, incr1, opn0word);
    img1 = cuex_atree_isokey_image(e1, incr1);
    imgU = cuex_atree_isokey_image(eU, incr1);
    cu_test_assert(cuex_atree_left_union(opn0word, img0, img1) == imgU);

    /* Ordering relations. */
    cu_test_assert(cuex_atree_subseteq(opn0word, e0, eU));
    if (e0 != eU) {
	cu_test_assert(!cuex_atree_subseteq(opn0word, eU, e0));
	cu_test_assert(cuex_atree_order(opn0word, e0, eU) == cu_order_lt);
	cu_test_assert(cuex_atree_order(opn0word, eU, e0) == cu_order_gt);
    }
    if (e0 != e1)
	cu_test_assert(cuex_atree_subseteq(opn0word, e0, e1)
		       == (cuex_atree_order(opn0word, e0, e1) == cu_order_lt));
}

int
main()
{
    int i;
    cuex_init();
    printf("Testing insert, erase, and find.\n");
    test(0, cu_true);
    for (i = 1; i < 80000; i *= 2)
	test(i, i < 16);
    printf("Testing union, intersection, and image.\n");
    for (i = 0; i < 400; ++i)
	test_union_isecn(i, i < 8);
    return 2*!!cu_test_bug_count();
}
