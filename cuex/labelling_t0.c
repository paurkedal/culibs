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

#include <cuex/iteration.h>
#include <cuex/labelling.h>
#include <cufo/stream.h>
#include <cuex/opn.h>
#include <cudyn/misc.h>
#include <cu/test.h>

void
test()
{
    cuex_t L0, L1, L1p, L2, L3;
    cuex_t LC;
    L0 = cuex_labelling_empty();
    cufo_oprintf("Empty labelling: %!\n", L0);
    L1 = cuex_labelling_singleton(cu_idr_by_cstr("label_a"), cudyn_int(8));
    cufo_oprintf("A singleton labelling: %!\n", L1);
    cu_test_assert(cuex_labelling_left_union(L0, L0) == L0);
    cu_test_assert(cuex_labelling_left_isecn(L0, L0) == L0);
    cu_test_assert(cuex_labelling_left_union(L1, L1) == L1);
    cu_test_assert(cuex_labelling_left_isecn(L1, L1) == L1);
    cu_test_assert(cuex_labelling_left_union(L0, L1) == L1);
    cu_test_assert(cuex_labelling_left_isecn(L0, L1) == L0);
    cu_test_assert(cuex_labelling_left_union(L1, L0) == L1);
    cu_test_assert(cuex_labelling_left_isecn(L1, L0) == L0);
    L2 = cuex_labelling_insert(L1, cu_idr_by_cstr("label_b"), cudyn_int(9));
    cufo_oprintf("Added element to singleton labelling: %!\n", L2);
    cu_test_assert(cuex_labelling_left_union(L1, L2) == L2);
    cu_test_assert(cuex_labelling_left_isecn(L1, L2) == L1);
    L1p = cuex_labelling_singleton(cu_idr_by_cstr("label_c"), cudyn_int(10));
    cu_test_assert(cuex_labelling_left_isecn(L1p, L2) == L0);
    L3 = cuex_labelling_left_union(L2, L1p);
    cufo_oprintf("L3 = %!\n", L3);
    LC = cuex_labelling_by_arglist(cu_idr_by_cstr("label_a"), cudyn_int(-8),
				   cu_idr_by_cstr("label_b"), cudyn_int(-9),
				   NULL);
    cufo_oprintf("LC = %!\n"
		 "left_union(LC, L3) = %!\n"
		 "left_isecn(LC, L3) = %!\n"
		 "left_union(L3, LC) = %!\n"
		 "left_isecn(L3, LC) = %!\n",
		 LC,
		 cuex_labelling_left_union(LC, L3),
		 cuex_labelling_left_isecn(LC, L3),
		 cuex_labelling_left_union(L3, LC),
		 cuex_labelling_left_isecn(L3, LC));
}

cu_clos_def(ladd, cu_prot(cuex_t, cuex_t e), (int diff;))
{
    cu_clos_self(ladd);
    cu_test_assert(cudyn_is_int(e));
    return cudyn_int(cudyn_to_int(e) + self->diff);
}

cu_clos_def(lsum, cu_prot(cu_bool_t, cuex_t e), (int sum;))
{
    cu_clos_self(lsum);
    cu_test_assert(cudyn_is_int(e));
    self->sum += cudyn_to_int(e);
    return cu_true;
}

void
test_image(int N, cu_bool_t do_print)
{
    cuex_t L = cuex_labelling_empty();
    cuex_t L0;
    int i;
    int sum = 0;
    lsum_t lsum;
    ladd_t ladd;
    for (i = 0; i < N; ++i) {
	char ls[6 + sizeof(int)*3];
	int j = lrand48();
	sum += j;
	sprintf(ls, "label%d", i);
	L = cuex_labelling_insert(L, cu_idr_by_cstr(ls), cudyn_int(j));
    }
    ladd.diff = -60;
    L0 = cuex_img_operands(ladd_prep(&ladd), L);
    if (do_print)
	cufo_oprintf("Original: %!\n   Image: %!\n", L, L0);
    lsum.sum = 0;
    cuex_iterA_operands(lsum_prep(&lsum), L);
    cu_test_assert(lsum.sum == sum);
    lsum.sum = 0;
    cuex_iterA_operands(lsum_prep(&lsum), L0);
    cu_test_assert(lsum.sum == sum - 60*N);
}

void
test_expand(int N, cu_bool_t do_print)
{
    int n;
    cuex_t L = cuex_labelling_empty();
    cuex_t L_e;
    for (n = 0; n < N; ++n) {
	char ls[6 + sizeof(int)*3];
	int j = lrand48();
	sprintf(ls, "l%d", n);
	L = cuex_labelling_insert(L, cu_idr_by_cstr(ls), cudyn_int(j));
    }
    L_e = cuex_labelling_expand_all(L);
    if (do_print)
	cufo_oprintf("        L = %!\nexpand(L) = %!\n", L, L_e);
    cu_test_assert(cuex_labelling_contract_all(L_e) == L);
}

void
test_find_index(int N, cu_bool_t do_print)
{
    int n;
    cuex_t L = cuex_labelling_empty();
    char *found_arr;
    for (n = 0; n < N; ++n) {
	char ls[6 + sizeof(int)*3];
	sprintf(ls, "l%d", n);
	L = cuex_labelling_insert(L, cu_idr_by_cstr(ls), cudyn_int(n));
    }
    cu_test_assert(cuex_labelling_card(L) == N);
    found_arr = cu_salloc(sizeof(char)*N);
    memset(found_arr, 0, N);
    if (do_print)
	printf("cuex_labelling_find_index check, N = %d\n", N);
    for (n = 0; n < N; ++n) {
	size_t index;
	char ls[6 + sizeof(int)*3];
	sprintf(ls, "l%d", n);
	index = cuex_labelling_find_index(L, cu_idr_by_cstr(ls));
	cu_test_assert(index != (size_t)-1);
	cu_test_assert(index < N);
	cu_debug_assert(found_arr[index] == 0);
	found_arr[index] = 1;
	if (do_print)
	    printf("\t%s at index %zd\n", ls, index);
    }
}

int
main()
{
    cuex_init();
    test();
    test_image(10, cu_true);
    test_image(1000, cu_false);
    test_expand(0, cu_true);
    test_expand(1, cu_true);
    test_expand(2, cu_true);
    test_expand(8, cu_true);
    test_expand(1000, cu_false);
    test_find_index(0, cu_false);
    test_find_index(1, cu_false);
    test_find_index(2, cu_true);
    test_find_index(3, cu_false);
    test_find_index(6, cu_true);
    test_find_index(10, cu_false);
    test_find_index(100, cu_false);
    test_find_index(1000, cu_false);
    return cu_test_bug_count()? 2 : 0;
}
