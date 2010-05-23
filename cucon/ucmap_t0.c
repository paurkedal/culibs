/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2008--2010  Petter Urkedal <paurkedal@eideticdew.org>
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

#include <cucon/ucmap.h>
#include <cucon/pcmap.h>
#include <cucon/ucset.h>
#include <cu/test.h>
#include <cu/int.h>
#include <cu/size.h>

#define REPEAT 40
#define N_INS 0x400
#define MAX_KEY N_INS

static cucon_ucmap_t
_random_map(size_t size,
	    uintptr_t min_key, uintptr_t maxp_key,
	    uintptr_t min_val, uintptr_t maxp_val)
{
    cucon_ucmap_t M = cucon_ucmap_empty();
    while (size--) {
	uintptr_t key = lrand48() % (maxp_key - min_key) + min_key;
	uintptr_t val = lrand48() % (maxp_val - min_val) + min_val;
	M = cucon_ucmap_insert_int(M, key, val);
    }
    return M;
}

cu_clos_def(_rewrite_map_cb, cu_prot(void, uintptr_t key, int val),
    ( cucon_ucmap_t M; ))
{
    cu_clos_self(_rewrite_map_cb);
    self->M = cucon_ucmap_insert_int(self->M, key, val);
}

static cucon_ucmap_t
_rewrite_map(cucon_ucmap_t M)
{
    _rewrite_map_cb_t cb;
    cb.M = cucon_ucmap_empty();
    cucon_ucmap_iter_int(M, _rewrite_map_cb_prep(&cb));
    return cb.M;
}

cu_clop_def(_intptr_cmp, int, void const *ptr0, void const *ptr1)
{
    return (intptr_t)ptr0 < (intptr_t)ptr1 ? -1
	 : (intptr_t)ptr0 > (intptr_t)ptr1 ?  1 : 0;
}
cu_clop_def(_intptr_eq, cu_bool_t, void const *ptr0, void const *ptr1)
{
    return ptr0 == ptr1;
}

static void
_test_cmp()
{
    int i, j;
    printf("Comparison tests.\n");
    for (j = 0; j < N_INS; ++j)
    for (i = 0; i < REPEAT/(j + 1) + 1; ++i) {
	int c0, c1, c2;
	int k = lrand48() % (j + 1) + 1;
	uintptr_t dist = (j + k - 1)/k;
	cucon_ucmap_t M0, M1, M2;

	M0 = _random_map(j, 0, dist, 0, dist);
	M1 = _rewrite_map(M0);
	c0 = cucon_ucmap_cmp(M0, M1);
	cu_test_assert(c0 == 0);

	M1 = _random_map(j, 0, dist, 0, dist);
	c0 = cucon_ucmap_cmp(M0, M1);
#if CUCONP_UCMAP_ENABLE_HCONS
	cu_test_assert((M0 == M1) == (c0 == 0));
#endif
	c2 = cucon_ucmap_cmp_ptr(cu_clop_ref(_intptr_cmp), M0, M1);
	cu_test_assert(c0 == c2);

	M2 = _random_map(j, 0, dist, 0, dist);
	c1 = cucon_ucmap_cmp(M1, M2);
	c2 = cucon_ucmap_cmp(M2, M0);
	cu_test_assert(cu_int_abs(c0 + c1 + c2) <= 1);
	cu_test_assert(c0 != 0 || c1 == -c2);
	cu_test_assert(c1 != 0 || c2 == -c0);
	cu_test_assert(c2 != 0 || c0 == -c1);

	cu_test_assert(!c0 == cucon_ucmap_eq(M0, M1));
	cu_test_assert(!c1 == cucon_ucmap_eq(M1, M2));
	cu_test_assert(!c2 == cucon_ucmap_eq(M2, M0));
	cu_test_assert(!c0 ==
		       cucon_ucmap_eq_ptr(cu_clop_ref(_intptr_eq), M0, M1));
    }
}

cu_clos_def(_iter_cb, cu_prot(void, uintptr_t key, int val),
    ( cucon_ucset_t S;
      size_t count; ))
{
    cu_clos_self(_iter_cb);
    cu_test_assert(cucon_ucset_find(self->S, key));
    cu_test_assert_int_eq((int)key, val);
    ++self->count;
}

static void
_test_ief()
{
    int i, j;
    printf("Insert, erase, and find tests.\n");
    for (i = 0; i < REPEAT; ++i) {
	cucon_ucmap_t M = NULL, Mp, Mpp;
	cucon_ucset_t S = NULL;
	uintptr_t key;
	_iter_cb_t iter_cb;
	size_t count = 0;

	for (j = 0; j < N_INS; ++j) {
	    key = lrand48() % MAX_KEY;
	    Mp = cucon_ucmap_insert_int(M, key, key);
	    if (!cucon_ucset_find(S, key)) {
		++count;
		Mpp = cucon_ucmap_erase(Mp, key);
		cu_test_assert(cucon_ucmap_eq(M, Mpp));
	    }
	    M = Mp;
	    S = cucon_ucset_insert(S, key);
	}
	for (key = 0; key < MAX_KEY; ++key) {
	    cu_bool_t pres;
	    uintptr_t val, valp;

	    val = cucon_ucmap_find_int(M, key);
	    pres = val != cucon_ucmap_int_none;
	    cu_test_assert(pres == cucon_ucmap_contains(M, key));
	    cu_test_assert(pres == cucon_ucmap_find(M, key, &valp));

	    if (val == cucon_ucmap_int_none) {
		cu_test_assert(!cucon_ucset_find(S, key));
		cu_test_assert(cucon_ucmap_erase(M, key) == M);
	    }
	    else {
		cu_test_assert(val == valp);
		cu_test_assert(cucon_ucset_find(S, key));
		cu_test_assert(val == key);
		Mp = cucon_ucmap_erase(M, key);
		cu_test_assert(cucon_ucmap_find_int(Mp, key) ==
			       cucon_ucmap_int_none);
	    }
	}

	for (j = 0; j < 4; ++j) {
	    key = lrand48() % MAX_KEY;
	    Mp = cucon_ucmap_erase(M, key);
	    if (Mp != M)
		cu_test_assert(cucon_ucmap_card(Mp) == count - 1);
	}

	cu_test_assert_size_eq(count, cucon_ucmap_card(M));

	iter_cb.S = S;
	iter_cb.count = 0;
	cucon_ucmap_iter_int(M, _iter_cb_prep(&iter_cb));
	cu_test_assert(iter_cb.count == count);
    }
}

cu_clos_def(_check_union_member, cu_prot(cu_bool_t, uintptr_t k, uintptr_t v),
    ( cucon_ucmap_t M0, M1, Mc;
      size_t card0, card1;
      uintptr_t clip_min, clip_max; ))
{
    cu_clos_self(_check_union_member);
    cu_bool_t pres0, pres1, in_clip;
    uintptr_t v0, v1, vc;
    pres0 = cucon_ucmap_find(self->M0, k, &v0);
    pres1 = cucon_ucmap_find(self->M1, k, &v1);
    cu_test_assert(pres0 || pres1);
    cu_test_assert(v == (pres0? v0 : v1));
    if (pres0) ++self->card0;
    if (pres1) ++self->card1;
    if (self->clip_min <= self->clip_max)
	in_clip = self->clip_min <= k && k <= self->clip_max;
    else
	in_clip = k <= self->clip_max || self->clip_min <= k;
    cu_test_assert(in_clip == cucon_ucmap_find(self->Mc, k, &vc));
    if (in_clip)
	cu_test_assert(v == vc);
    return cu_true;
}

static void
_test_union()
{
    int i;
    cucon_ucmap_t M;
    _check_union_member_t f_ckm;

    printf("Union tests.\n");
    _check_union_member_init(&f_ckm);
    for (i = 0; i < REPEAT; ++i) {
	size_t n0 = lrand48() % (1 << (lrand48() % 16));
	size_t n1 = lrand48() % (1 << (lrand48() % 16));
	size_t n = cu_size_max(n0, n1);
	f_ckm.card0 = 0;
	f_ckm.card1 = 0;
	f_ckm.M0 = _random_map(n0, 0, n0, 0, n0);
	f_ckm.M1 = _random_map(n1, 0, n1, 0, n1);
	M = cucon_ucmap_left_union(f_ckm.M0, f_ckm.M1);
	f_ckm.clip_min = lrand48() % (n + 1);
	f_ckm.clip_max = lrand48() % (n + 1);
	f_ckm.Mc = cucon_ucmap_clip_corange(M, f_ckm.clip_min, f_ckm.clip_max);
	cucon_ucmap_iterA(_check_union_member_ref(&f_ckm), M);
	cu_test_assert(f_ckm.card0 == cucon_ucmap_card(f_ckm.M0));
	cu_test_assert(f_ckm.card1 == cucon_ucmap_card(f_ckm.M1));
    }
}

int
main()
{
    cu_init();
    _test_ief();
    _test_cmp();
    _test_union();
    return 2*!!cu_test_bug_count();
}
