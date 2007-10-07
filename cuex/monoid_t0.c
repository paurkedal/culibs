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

#include <cuex/monoid.h>
#include <cuex/oprdefs.h>
#include <cudyn/misc.h>

#define OPR CUEX_O2M_TUPLE

#define N 500

int
main()
{
    cuex_t x;
    int i;
    int j;
    cuex_monoid_it_t it;
    cuex_init();

    /* Check simple construction and iteration */
    cuex_monoid_it_cct(&it, OPR, cudyn_int(0));
    cu_debug_assert(cuex_monoid_it_read(&it) == cudyn_int(0));
    cu_debug_assert(cuex_monoid_it_read(&it) == NULL);
    x = cuex_monoid_product(OPR, cudyn_int(0), cudyn_int(1));
    cuex_monoid_it_cct(&it, OPR, x);
    cu_debug_assert(cuex_monoid_it_read(&it) == cudyn_int(0));
    cu_debug_assert(cuex_monoid_it_read(&it) == cudyn_int(1));
    cu_debug_assert(cuex_monoid_it_read(&it) == NULL);

    /* Check construction and iteration */
    x = cuex_monoid_identity(OPR);
    cu_debug_assert(cuex_is_monoid_identity(OPR, x));
    cu_debug_assert(cuex_is_monoid_product(OPR, x));
    x = cuex_monoid_product(OPR, x, cudyn_int(0));
    cu_debug_assert(!cuex_is_monoid_product(OPR, x));
    cu_debug_assert(x == cudyn_int(0));
    for (i = 1; i < N; ++i) {
	x = cuex_monoid_product(OPR, x, cudyn_int(i));
	cu_debug_assert(cuex_is_monoid_product(OPR, x));
	cu_debug_assert(cuex_monoid_factor_count(OPR, x) == i + 1);
	cuex_monoid_it_cct(&it, OPR, x);
	for (j = 0; j <= i; ++j) {
	    cuex_monoid_it_t it_r;
	    cu_debug_assert(cuex_monoid_it_read(&it) == cudyn_int(j));
	    cu_debug_assert(cuex_monoid_factor_at(OPR, x, j) == cudyn_int(j));
	    cuex_monoid_it_cct_at(&it_r, OPR, x, j);
	    cu_debug_assert(cuex_monoid_it_read(&it_r) == cudyn_int(j));
	}
	cu_debug_assert(cuex_monoid_it_read(&it) == NULL);
    }

    /* Check associativity */
    for (j = 0; j < N; ++j) {
	cuex_t y = cuex_monoid_identity(OPR);
	cuex_t z = cuex_monoid_identity(OPR);
	for (i = 0; i < j; ++i)
	    y = cuex_monoid_product(OPR, y, cudyn_int(i));
	for (i = j; i < N; ++i)
	    z = cuex_monoid_product(OPR, z, cudyn_int(i));
	cu_debug_assert(x == cuex_monoid_product(OPR, y, z));
	cu_debug_assert(cuex_monoid_factor_prefix(OPR, x, j) == y);
	cu_debug_assert(cuex_monoid_factor_range(OPR, x, j, N) == z);
    }

    return 0;
}
