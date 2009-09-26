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

#include <cucon/fibq_test.h>
#include <cu/int.h>
#include <string.h>

static int
_random_inserts(int maxp_value, int count, cucon_fibq_t Q, int *counts)
{
    int i, j_min = INT_MAX;
    cu_test_assert(maxp_value <= MAXP_VALUE);
    for (i = 0; i < count; ++i) {
	int j = lrand48() % maxp_value;
	_fibq_insert(Q, j);
	if (j < j_min)
	    j_min = j;
	++counts[j];
    }
    return j_min;
}

static int
_pop_n(int count, cucon_fibq_t Q, int *counts, int j_min)
{
    while (count--) {
	int j = _fibq_pop(Q);
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
    int dim, j_min, round, *counts;
    cucon_fibq_t Q = cucon_fibq_new(cu_clop_ref(_fibq_prioreq));
    size_t card = 0;

    /* Fixed Tests */

    _fibq_insert(Q, 10);
    cucon_fibq_validate(Q);
    cu_test_assert_size_eq(cucon_fibq_card(Q), 1);
    cu_test_assert_int_eq(_fibq_pop(Q), 10);
    cu_test_assert_size_eq(cucon_fibq_card(Q), 0);
    cu_test_assert_int_eq(_fibq_pop(Q), -1);

    _fibq_insert(Q, 10);
    _fibq_insert(Q, 11);
    cucon_fibq_validate(Q);
    cu_test_assert_size_eq(cucon_fibq_card(Q), 2);
    cu_test_assert_int_eq(_fibq_pop(Q), 10);
    cu_test_assert_int_eq(_fibq_pop(Q), 11);

    _fibq_insert(Q, 11);
    _fibq_insert(Q, 10);
    cucon_fibq_validate(Q);
    cu_test_assert_int_eq(_fibq_pop(Q), 10);
    cu_test_assert_int_eq(_fibq_pop(Q), 11);

    /* Random Tests */

    counts = cu_snewarr(int, MAXP_VALUE);
    memset(counts, 0, sizeof(int)*MAXP_VALUE);

    for (dim = 1; dim < 100; ++dim)
	for (round = 0; round < 10; ++round) {
	    int j_min = _random_inserts(dim, dim, Q, counts);
	    cucon_fibq_validate(Q);
	    cu_test_assert_size_eq(cucon_fibq_card(Q), dim);
	    _pop_n(dim, Q, counts, j_min);
	    cucon_fibq_validate(Q);
	    cu_test_assert_size_eq(cucon_fibq_card(Q), 0);
	}

    for (dim = 1; dim < 10; ++dim) {
	int j_min = INT_MAX;
	for (round = 0; round < 20000; ++round) {
	    int n_ins, n_pop, j_min_p;
	    n_ins = lrand48() % dim + 1;
	    j_min_p = _random_inserts(n_ins, n_ins % MAXP_VALUE, Q, counts);
	    j_min = cu_int_min(j_min, j_min_p);
	    n_pop = lrand48() % cucon_fibq_card(Q);
	    j_min = _pop_n(n_pop, Q, counts, INT_MIN);
	}
	_pop_n(cucon_fibq_card(Q), Q, counts, j_min);
    }

    cu_test_assert(cucon_fibq_card(Q) == 0);
    j_min = INT_MAX;
    for (round = 0; round < 20000; ++round) {
	int choice = lrand48();
	switch (cucon_fibq_card(Q)? choice % 2 : 0) {
		int j;

	    case 0:
		j = lrand48() % MAXP_VALUE;
		if (j < j_min)
		    j_min = j;
		_fibq_insert(Q, j);
		++card;
		break;

	    case 1:
		--card;
		j = _fibq_pop(Q);
		cu_test_assert(j >= j_min);
		j_min = j;
		break;
	}
	cucon_fibq_validate(Q);
	cu_test_assert(card == cucon_fibq_card(Q));
    }
    printf("card(Q) = %zd\n", cucon_fibq_card(Q));
}

int
main()
{
    cu_init();
    _test();
    return 2*!!cu_test_bug_count();
}
