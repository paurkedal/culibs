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

#include <cuex/tmonoid.h>
#include <cuex/oprdefs.h>
#include <cudyn/misc.h>
#include <cu/test.h>

#define OPR CUEX_O2_TUPLE
#define N 500

int
main()
{
    cuex_t x;
    int i;
    int j;

    cuex_init();

    /* Check nesting. */
    x = cuex_tmonoid_identity(OPR);
    for (j = 0; j < N; ++j) {
	cuex_t y, z;
	y = cuex_tmonoid_identity(OPR);
	for (i = j; i < N; ++i) {
	    z = cuex_tmonoid_rightmult(x, y);
	    if (i < 4)
		cu_verbf(0, "%!\n", z);
	    cu_test_assert_size_eq(cuex_tmonoid_length(z), j + 1);
	    y = cuex_tmonoid_rightmult(y, cudyn_int(i));
	}
	x = cuex_tmonoid_rightmult(x, cudyn_int(j));
    }

    return 2*!!cu_test_bug_count();
}
