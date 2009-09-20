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
#include <cu/test.h>
#include <cu/memory.h>
#include <cu/int.h>
#include <limits.h>
#include <string.h>

#define MAXP_VALUE 256

typedef struct _fibnode *_fibnode_t;
struct _fibnode
{
    cu_inherit (cucon_fibnode);
    int value;
};

CU_SINLINE int
_fibnode_value(cucon_fibnode_t node)
{
    return cu_downcast(_fibnode, cucon_fibnode, node)->value;
}

cu_clop_def(_fibnode_prioreq, cu_bool_t,
	    cucon_fibnode_t lhs, cucon_fibnode_t rhs)
{
    return _fibnode_value(lhs) <= _fibnode_value(rhs);
}

static cucon_fibnode_t
_fibnode_new(int i)
{
    _fibnode_t node = cu_gnew(struct _fibnode);
    node->value = i;
    return cu_upcast(cucon_fibnode, node);
}

static void
_fibheap_insert(cucon_fibheap_t H, int i)
{
    cucon_fibheap_insert(H, _fibnode_new(i));
}

static int
_fibheap_pop(cucon_fibheap_t H)
{
    cucon_fibnode_t node = cucon_fibheap_pop_front(H);
    if (node)
	return _fibnode_value(node);
    else
	return -1;
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
    int dim, round, *counts;

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
}

int
main()
{
    cu_init();
    _test();
    return 2*!!cu_test_bug_count();
}
