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

static void
_test_simple()
{
    cucon_ucmultimap_t M0, M1, M2;
    cucon_ucset_t S1, S2;
    M0 = cucon_ucmultimap_empty();
    cu_test_assert(cucon_ucmultimap_is_empty(M0));
    cu_test_assert(cucon_ucmultimap_corange_card(M0) == 0);

    S1 = cucon_ucset_singleton(80);
    M1 = cucon_ucmultimap_isokey_singleton(100, S1);
    cu_test_assert(M1 == cucon_ucmultimap_singleton(100, 80));
    cu_test_assert(!cucon_ucmultimap_is_empty(M1));
    cu_test_assert(cucon_ucmultimap_corange_card(M1) == 1);
    cu_test_assert(cucon_ucmultimap_isokey_find(M1, 100) == S1);
    cu_test_assert(cucon_ucmultimap_isokey_insert(M0, 100, S1) == M1);
    cu_test_assert(cucon_ucmultimap_isokey_erase(M1, 100) == M0);

    S2 = cucon_ucset_insert(S1, 81);
    M2 = cucon_ucmultimap_insert(M1, 100, 81);
    cu_test_assert(M2 == cucon_ucmultimap_isokey_singleton(100, S2));
    cu_test_assert(cucon_ucmultimap_corange_card(M2) == 1);
    cu_test_assert(cucon_ucmultimap_card(M2) == 2);
    cu_test_assert(cucon_ucmultimap_isokey_find(M2, 100) == S2);
    cu_test_assert(cucon_ucmultimap_contains(M2, 100, 80));
    cu_test_assert(cucon_ucmultimap_contains(M2, 100, 81));
}

static void
_test_unary()
{
    cucon_ucmultimap_t M0, M1a, M1b, M2;
    int round, subround;
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
	    cu_test_assert(M2 == cucon_ucmultimap_insert(M1b, ka, va));
	    if (ka == kb && va == vb)
		cu_test_assert(M1a == M1b && M1a == M2);
	    else if (cucon_ucmultimap_contains(M0, ka, va))
		cu_test_assert(M1b == M2);
	    else
		cu_test_assert(cucon_ucmultimap_erase(M2, ka, va) == M1b);
	}
    }
}

int
main()
{
    cu_init();
    _test_simple();
    _test_unary();
    return 2*!!cu_test_bug_count();
}
