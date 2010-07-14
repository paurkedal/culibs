/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2010  Petter Urkedal <paurkedal@eideticdew.org>
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

#include <cucon/ucmultimap.h>
#include <cucon/ucset.h>
#include <cu/test.h>

#define REPEAT 200
#define SUBREPEAT 100

static cucon_ucmultimap_t
_random_ucmultimap(size_t n, int n_k, int n_v)
{
    cucon_ucmultimap_t M = cucon_ucmultimap_empty();
    while (n--)
	M = cucon_ucmultimap_insert(M, lrand48() % n_k, lrand48() % n_v);
    return M;
}

#define _assert_ucmultimap_eq(M0, M1) cu_test_assert(cucon_ucmultimap_eq(M0,M1))
#define _assert_ucset_eq(S0, S1) cu_test_assert(cucon_ucset_eq(S0, S1))

static void
_test_simple()
{
    cucon_ucmultimap_t M0, M1, M2;
    cucon_ucset_t S1, S2;

    printf("Testing simple operations.\n");

    M0 = cucon_ucmultimap_empty();
    cu_test_assert(cucon_ucmultimap_is_empty(M0));
    cu_test_assert(cucon_ucmultimap_corange_card(M0) == 0);

    S1 = cucon_ucset_singleton(80);
    M1 = cucon_ucmultimap_isokey_singleton(100, S1);
    _assert_ucmultimap_eq(M1, cucon_ucmultimap_singleton(100, 80));
    cu_test_assert(!cucon_ucmultimap_is_empty(M1));
    cu_test_assert(cucon_ucmultimap_corange_card(M1) == 1);
    _assert_ucset_eq(cucon_ucmultimap_isokey_find(M1, 100), S1);
    _assert_ucmultimap_eq(cucon_ucmultimap_isokey_insert(M0, 100, S1), M1);
    _assert_ucmultimap_eq(cucon_ucmultimap_isokey_erase(M1, 100), M0);

    S2 = cucon_ucset_insert(S1, 81);
    M2 = cucon_ucmultimap_insert(M1, 100, 81);
    _assert_ucmultimap_eq(M2, cucon_ucmultimap_isokey_singleton(100, S2));
    cu_test_assert_size_eq(cucon_ucmultimap_corange_card(M2), 1);
    cu_test_assert_size_eq(cucon_ucmultimap_card(M2), 2);
    _assert_ucset_eq(cucon_ucmultimap_isokey_find(M2, 100), S2);
    cu_test_assert(cucon_ucmultimap_contains(M2, 100, 80));
    cu_test_assert(cucon_ucmultimap_contains(M2, 100, 81));
}

static void
_test_unary()
{
    cucon_ucmultimap_t M0, M1a, M1b, M2;
    int round, subround;

    printf("Testing unary operations.\n");
    for (round = 0; round < REPEAT; ++round) {
	int en = lrand48() % 12;
	int n = 1 << en;
	int m = 1 << (lrand48() % (3*en/2 + 1));
	M0 = _random_ucmultimap(n, m, m);
#if 0
	cucon_ucmultimap_fprint_uintptr(M0, stdout);
	printf("\n");
#endif
	for (subround = 0; subround < SUBREPEAT; ++subround) {
	    uintptr_t ka = lrand48() % m;
	    uintptr_t va = lrand48() % m;
	    uintptr_t kb = lrand48() % m;
	    uintptr_t vb = lrand48() % m;

	    M1a = cucon_ucmultimap_insert(M0, ka, va);
	    M1b = cucon_ucmultimap_insert(M0, kb, vb);
	    M2 = cucon_ucmultimap_insert(M1a, kb, vb);
	    cu_test_assert(cucon_ucmultimap_contains(M1a, ka, va));
	    _assert_ucmultimap_eq(M2, cucon_ucmultimap_insert(M1b, ka, va));
	    if (ka == kb && va == vb) {
		_assert_ucmultimap_eq(M1a, M1b);
		_assert_ucmultimap_eq(M1a, M2);
	    }
	    else if (cucon_ucmultimap_contains(M0, ka, va))
		_assert_ucmultimap_eq(M1b, M2);
	    else
		_assert_ucmultimap_eq(cucon_ucmultimap_erase(M2, ka, va), M1b);
	}
    }
}

#define ARR_LOG2_MAX_LENGTH 12

static void
_test_array_ctor()
{
    cucon_ucmultimap_t M0, M1;
    int round;
    struct cucon_ucmultimap_element arr[1 << ARR_LOG2_MAX_LENGTH];

    printf("Testing array construction.\n");
    for (round = 0; round < REPEAT; ++round) {
	size_t i, n = lrand48() % (1 << lrand48() % (ARR_LOG2_MAX_LENGTH + 1));
	M0 = cucon_ucmultimap_empty();
	for (i = 0; i < n; ++i) {
	    uintptr_t key = mrand48();
	    arr[i].key = key;
	    arr[i].value = ~key;
	    M0 = cucon_ucmultimap_insert(M0, key, ~key);
	}
	qsort(arr, n, sizeof(struct cucon_ucmultimap_element),
	      cucon_ucmultimap_element_cmp);
	M1 = cucon_ucmultimap_from_sorted_array(arr, n);
	cu_test_assert(cucon_ucmultimap_eq(M0, M1));
    }
}

int
main()
{
    cu_init();
    _test_simple();
    _test_unary();
    _test_array_ctor();
    return 2*!!cu_test_bug_count();
}
