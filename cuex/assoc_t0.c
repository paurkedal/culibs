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

#include <cuex/assoc.h>
#include <cuex/oprdefs.h>
#include <cudyn/misc.h>
#include <cu/test.h>
#include <cucon/pset.h>

//#define o2_tuple(x, y) cuex_opn(CUEX_OR_TUPLE(2), x, y)
#define o2_tuple(x, y) cuex_o2_apply(x, y)

void
test(int N, cu_bool_t print)
{
    struct cucon_pset_s keys;
    int i;
    cuex_t e = cuex_assoc_empty();

    cucon_pset_cct(&keys);
    for (i = 0; i < N; ++i) {
	cuex_t key;
	cuex_t val;
	cuex_t ep;
	key = cudyn_uint(lrand48() % (i + 1));
	val = o2_tuple(key, key);
	ep = cuex_assoc_insert(e, val);
	if (cucon_pset_insert(&keys, val)) {
	    cu_test_assert(e != ep);
	    cu_test_assert(cuex_assoc_erase(ep, key) == e);
	}
	else
	    cu_test_assert(e == ep);
	cu_test_assert(cuex_assoc_find(ep, key) == val);
	if (print)
	    cu_fprintf(stdout, "%! ∪ {%!}\n  = %!\n", e, val, ep);
	e = ep;
    }
}

int
main()
{
    int i;
    cuex_init();
    for (i = 1; i < 100000; i *= 2)
	test(i, i < 16);
    return 2*!!cu_test_bug_count();
}
