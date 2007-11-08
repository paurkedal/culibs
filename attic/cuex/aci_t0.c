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

#include <cuex/aci.h>
#include <cuex/oprdefs.h>
#include <cudyn/misc.h>
#include <cucon/pmap.h>
#include <time.h>

#if 1
#  define KEY_CNT 8000
#  define KEY_MAX 8000
#  define VERBOSE 0
#  define TEST2_REPEAT 1
#elif 1
#  define KEY_CNT 10
#  define KEY_MAX 10
#  define VERBOSE 0
#  define TEST2_REPEAT 1000
#else
#  define KEY_CNT 10
#  define KEY_MAX 10
#  define VERBOSE 1
#  define TEST2_REPEAT 1
#endif
#define REPEAT 2
#define OPR CUEX_O3ACI_PROD

#if 0
#  define INSERT(x, y) cuex_aci_insert(OPR, x, y)
#else
#  define INSERT(x, y) cuex_aci_join(OPR, x, y)
#endif

void
ins_find()
{
    int i, k;
    cuex_opn_t e;
    cu_bool_least_t present[KEY_MAX];
    clock_t t0;

    t0 = -clock();
    for (k = 0; k < REPEAT; ++k) {
	size_t cnt = 0;
	memset(present, 0, sizeof(present));
	e = cuex_aci_identity(OPR);
	for (i = 0; i < KEY_CNT; ++i) {
	    int j = lrand48() % KEY_MAX;
	    cuex_t e_old = e;
	    e = INSERT(e, cudyn_int(j));
	    if (present[j])
		cu_debug_assert(e == e_old);
	    else {
		cu_debug_assert(cuex_aci_order(OPR, e, e_old) == cu_order_gt);
		cu_debug_assert(cuex_aci_order(OPR, e_old, e) == cu_order_lt);
		cu_debug_assert(cuex_aci_count(e) == ++cnt);
	    }
	    present[j] = cu_true;
	}
	for (i = 0; i < KEY_MAX; ++i)
	    cu_debug_assert(present[i] == !!cuex_aci_find(e, cudyn_int(i)));
    }
    t0 += clock();
    printf("%lg s per operation\n",
	   t0/((double)CLOCKS_PER_SEC*REPEAT*KEY_CNT));
}

cuex_t
aci_from_arr(cuex_t *keys, size_t cnt)
{
    size_t i;
    cuex_t e = cuex_aci_identity(OPR);
    for (i = 0; i < cnt; ++i)
	e = INSERT(e, keys[i]);
    return e;
}

void
permute(cuex_t *arr, size_t cnt)
{
    size_t i;
    for (i = 1; i < cnt; ++i) {
	size_t j = lrand48() % (i + 1);
	cuex_t save = arr[i];
	arr[i] = arr[j];
	arr[j] = save;
    }
}

void
test2()
{
    size_t i;
    cuex_t keys[KEY_CNT];
    cuex_t e0, e;
    for (i = 0; i < KEY_CNT; ++i)
	keys[i] = cudyn_int(lrand48() % KEY_MAX);

    /* Test hashcons equality for permuted constructions */
    e0 = aci_from_arr(keys, KEY_CNT);
    for (i = 0; i < REPEAT; ++i) {
	permute(keys, KEY_CNT);
	if (VERBOSE)
	    printf("%p %p %p %p\n", keys[0], keys[1], keys[2], keys[3]);
	e = aci_from_arr(keys, KEY_CNT);
	cu_debug_assert(e == e0);
    }

    /* Test equality for equivalent merges. */
    if (!VERBOSE)
	printf("Merge test");
    for (i = 0; i < 16; ++i) {
	cuex_t eJ, eM;
	if (!VERBOSE) {
	    fputc('.', stdout);
	    fflush(stdout);
	}
	size_t j;
	cuex_t x = cuex_aci_identity(OPR);
	cuex_t y = cuex_aci_identity(OPR);
	cuex_t xy = cuex_aci_identity(OPR);
	struct cucon_pmap_s x_pmap;
	permute(keys, KEY_CNT);
	cucon_pmap_cct(&x_pmap);
	for (j = 0; j < 2*KEY_CNT/3; ++j)
	    cucon_pmap_insert_void(&x_pmap, keys[j]);
	for (j = 0; j < 2*KEY_CNT/3; ++j) {
	    x = INSERT(x, keys[j]);
	    y = INSERT(y, keys[KEY_CNT - j - 1]);
	    if (cucon_pmap_find_void(&x_pmap, keys[KEY_CNT - j - 1]))
		xy = INSERT(xy, keys[KEY_CNT - j - 1]);
	}
	eJ = cuex_aci_join(OPR, x, y);
	eM = cuex_aci_meet(OPR, x, y);
	cu_debug_assert(cuex_aci_leq(OPR, x, eJ));
	cu_debug_assert(cuex_aci_leq(OPR, y, eJ));
	cu_debug_assert(cuex_aci_leq(OPR, eM, x));
	cu_debug_assert(cuex_aci_leq(OPR, eM, y));
	if (VERBOSE) {
	    fputs("x = ", stdout);
	    cuex_aci_dump(x, stdout);
	    fputs("y = ", stdout);
	    cuex_aci_dump(y, stdout);
	    fputs("xy = ", stdout);
	    cuex_aci_dump(xy, stdout);
	    fputc('\n', stdout);
	    fputs("x ∨ y = ", stdout);
	    cuex_aci_dump(eJ, stdout);
	    fputc('\n', stdout);
	    fputs("x ∧ y = ", stdout);
	    cuex_aci_dump(eM, stdout);
	    fputc('\n', stdout);
	}
	cu_debug_assert(eJ == e0);
	cu_debug_assert(eM == xy);
    }
    fputc('\n', stdout);
}

int
main()
{
    int i;
    cuex_init();
    ins_find();
    for (i = 0; i < TEST2_REPEAT; ++i)
	test2();
    return 0;
}
