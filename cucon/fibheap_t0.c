/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2009  Petter Urkedal <urkedal@nbi.dk>
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

#include <cucon/fibheap.h>
#include <cucon/fibheap_test.h>
#include <cu/test.h>
#include <cu/memory.h>
#include <cu/int.h>
#include <limits.h>
#include <string.h>

#define MAXP_VALUE 256

cu_clos_def(_fibheap_pick_helper, cu_prot(cu_bool_t, cucon_fibnode_t node),
    ( int i; cucon_fibnode_t r; ))
{
    cu_clos_self(_fibheap_pick_helper);
    if (self->i == 0) {
	self->r = node;
	return cu_false;
    }
    else {
	--self->i;
	return cu_true;
    }
}

static cucon_fibnode_t
_fibheap_pick(cucon_fibheap_t H)
{
    _fibheap_pick_helper_t cl;
    cl.i = lrand48() % cucon_fibheap_card(H);
    cucon_fibheap_iterA(H, _fibheap_pick_helper_prep(&cl));
    return cl.r;
}

static int
_random_inserts(int maxp_value, int count, cucon_fibheap_t H, int *counts)
{
    int i, j_min = INT_MAX;
    cu_test_assert(maxp_value <= MAXP_VALUE);
    for (i = 0; i < count; ++i) {
	int j = lrand48() % maxp_value;
	_fibheap_insert(H, j);
	if (j < j_min)
	    j_min = j;
	++counts[j];
    }
    return j_min;
}

static int
_pop_n(int count, cucon_fibheap_t H, int *counts, int j_min)
{
    while (count--) {
	int j = _fibheap_pop(H);
	cu_test_assert(j < MAXP_VALUE);
	cu_test_assert(j != -1);
	cu_test_assert(j >= j_min);
	j_min = j;
	cu_test_assert(counts[j]-- > 0);
    }
    return j_min;
}

static void
_test()
{
    cucon_fibheap_t H;
    int dim, round, *counts, j_min;

    H = cucon_fibheap_new(cu_clop_ref(_fibnode_prioreq));
    cu_debug_assert(cucon_fibheap_is_empty(H));

    /* Fixed Tests */

    _fibheap_insert(H, 10);
    cucon_fibheap_validate(H);
    cu_test_assert_size_eq(cucon_fibheap_card(H), 1);
    cu_test_assert_int_eq(_fibheap_pop(H), 10);
    cu_test_assert_size_eq(cucon_fibheap_card(H), 0);
    cu_test_assert_int_eq(_fibheap_pop(H), -1);

    _fibheap_insert(H, 10);
    _fibheap_insert(H, 11);
    cucon_fibheap_validate(H);
    cu_test_assert_size_eq(cucon_fibheap_card(H), 2);
    cu_test_assert_int_eq(_fibheap_pop(H), 10);
    cu_test_assert_int_eq(_fibheap_pop(H), 11);

    _fibheap_insert(H, 11);
    _fibheap_insert(H, 10);
    cucon_fibheap_validate(H);
    cu_test_assert_int_eq(_fibheap_pop(H), 10);
    cu_test_assert_int_eq(_fibheap_pop(H), 11);

    /* Random Tests */

    counts = cu_snewarr(int, MAXP_VALUE);
    memset(counts, 0, sizeof(int)*MAXP_VALUE);

    for (dim = 1; dim < 100; ++dim)
	for (round = 0; round < 10; ++round) {
	    int j_min = _random_inserts(dim, dim, H, counts);
	    cucon_fibheap_validate(H);
	    cu_test_assert_size_eq(cucon_fibheap_card(H), dim);
	    _pop_n(dim, H, counts, j_min);
	    cucon_fibheap_validate(H);
	    cu_test_assert_size_eq(cucon_fibheap_card(H), 0);
	}

    for (dim = 1; dim < 10; ++dim) {
	int j_min = INT_MAX;
	for (round = 0; round < 20000; ++round) {
	    int n_ins, n_pop, j_min_p;
	    n_ins = lrand48() % dim + 1;
	    j_min_p = _random_inserts(n_ins, n_ins % MAXP_VALUE, H, counts);
	    j_min = cu_int_min(j_min, j_min_p);
	    n_pop = lrand48() % cucon_fibheap_card(H);
	    j_min = _pop_n(n_pop, H, counts, INT_MIN);
	}
	_pop_n(cucon_fibheap_card(H), H, counts, j_min);
    }

    cu_test_assert(cucon_fibheap_card(H) == 0);
    j_min = INT_MAX;
    for (round = 0; round < 20000; ++round) {
	int choice = lrand48();
	switch (cucon_fibheap_card(H)? choice % 5 : 0) {
		int j;
		cucon_fibnode_t node;

	    case 0:
	    case 1:
		j = lrand48() % MAXP_VALUE;
		if (j < j_min)
		    j_min = j;
		_fibheap_insert(H, j);
		break;

	    case 2:
		j = _fibheap_pop(H);
		cu_test_assert(j >= j_min);
		j_min = j;
		break;

	    case 3:
		node = _fibheap_pick(H);
		j = _fibnode_value(node);
		if (j > 0)
		    j = lrand48() % j;
		if (j < j_min)
		    j_min = j;
		_fibnode_set_value(node, j);
		cucon_fibheap_prioritise(H, node);
		break;

	    case 4:
		node = _fibheap_pick(H);
		cucon_fibheap_remove(H, node);
		break;
	}
	cucon_fibheap_validate(H);
    }
    printf("card(H) = %zd\n", cucon_fibheap_card(H));
}

int
main()
{
    cu_init();
    _test();
    return 2*!!cu_test_bug_count();
}
