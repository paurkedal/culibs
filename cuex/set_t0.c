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

#include <cuex/set.h>
#include <cuex/test.h>
#include <cuex/compound.h>
#include <cuex/intf.h>
#include <cuoo/type.h>
#include <cudyn/misc.h>
#include <cu/ptr_seq.h>

cu_ptr_source_t
set_source(cuex_t S)
{
    cuex_intf_compound_t S_c;
    cu_ptr_source_t source;

    S_c = cuoo_impl_ptr(S, CUEX_INTF_COMPOUND);
    cu_test_assert(S_c);
    source = cuex_compound_pref_iter_source(S_c, S);
    cu_test_assert(source);
    return source;
}

void
test()
{
    int i;
    cuex_t S0, S1, S2;
    cu_ptr_source_t source;

    S0 = cuex_empty_set();
    S1 = cuex_set_insert(S0, cudyn_int(0));
    S2 = cuex_set_insert(S1, cudyn_int(1));
    cuex_test_assert_eq(cuex_set_erase(S1, cudyn_int(0)), S0);
    cuex_test_assert_eq(cuex_set_erase(S2, cudyn_int(1)), S1);
    cuex_test_assert_eq(cuex_set_erase(S0, cudyn_int(0)), S0);
    cuex_test_assert_eq(cuex_set_erase(S2, cudyn_int(2)), S2);
    cuex_test_assert_eq(cuex_set_insert(S1, cudyn_int(0)), S1);
    cuex_test_assert_eq(cuex_set_insert(S2, cudyn_int(1)), S2);

    S0 = cuex_empty_set();
    for (i = 0; i < 100; ++i)
	S0 = cuex_set_insert(S0, cudyn_int(i));
    source = set_source(S0);
    cu_test_assert(cu_ptr_source_count(source) == 100);

    S1 = cuex_empty_set();
    for (i = 80; i < 120; ++i)
	S1 = cuex_set_insert(S1, cudyn_int(i));
    source = set_source(cuex_set_union(S0, S1));
    cu_test_assert(cu_ptr_source_count(source) == 120);
    source = set_source(cuex_set_isecn(S0, S1));
    cu_test_assert(cu_ptr_source_count(source) == 20);
}

int main()
{
    cuex_init();
    test();
    return 0;
}
