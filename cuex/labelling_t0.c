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

#include <cuex/labelling.h>
#include <cudyn/misc.h>
#include <cu/test.h>

void
test()
{
    cuex_t L0, L1, L1p, L2, L3;
    cuex_t LC;
    L0 = cuex_labelling_empty();
    cu_fprintf(stdout, "Empty labelling: %!\n", L0);
    L1 = cuex_labelling_singleton(cu_idr_by_cstr("label_a"), cudyn_int(8));
    cu_fprintf(stdout, "A singleton labelling: %!\n", L1);
    cu_test_assert(cuex_labelling_left_union(L0, L0) == L0);
    cu_test_assert(cuex_labelling_left_isecn(L0, L0) == L0);
    cu_test_assert(cuex_labelling_left_union(L1, L1) == L1);
    cu_test_assert(cuex_labelling_left_isecn(L1, L1) == L1);
    cu_test_assert(cuex_labelling_left_union(L0, L1) == L1);
    cu_test_assert(cuex_labelling_left_isecn(L0, L1) == L0);
    cu_test_assert(cuex_labelling_left_union(L1, L0) == L1);
    cu_test_assert(cuex_labelling_left_isecn(L1, L0) == L0);
    L2 = cuex_labelling_insert(L1, cu_idr_by_cstr("label_b"), cudyn_int(9));
    cu_fprintf(stdout, "Added element to singleton labelling: %!\n", L2);
    cu_test_assert(cuex_labelling_left_union(L1, L2) == L2);
    cu_test_assert(cuex_labelling_left_isecn(L1, L2) == L1);
    L1p = cuex_labelling_singleton(cu_idr_by_cstr("label_c"), cudyn_int(10));
    cu_test_assert(cuex_labelling_left_isecn(L1p, L2) == L0);
    L3 = cuex_labelling_left_union(L2, L1p);
    cu_fprintf(stdout, "L3 = %!\n", L3);
    LC = cuex_labelling_by_arglist(cu_idr_by_cstr("label_a"), cudyn_int(-8),
				   cu_idr_by_cstr("label_b"), cudyn_int(-9),
				   NULL);
    cu_fprintf(stdout,
	       "LC = %!\n"
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

int
main()
{
    cuex_init();
    test();
    return cu_test_bug_count()? 2 : 0;
}
