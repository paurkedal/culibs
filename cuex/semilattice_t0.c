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

#include <cuex/semilattice.h>
#include <cuex/oprdefs.h>
#include <cufo/tagdefs.h>
#include <cufo/stream.h>
#include <cudyn/misc.h>
#include <cucon/pset.h>
#include <cu/test.h>
#include <math.h>

#define MEET CUEX_O2_MEET

cuex_t
build(int N, int J, cucon_pset_t set)
{
    if (N == 0)
	return cuex_meetlattice_top(MEET);
    else if (N == 1) {
	cuex_t je = cudyn_int(lrand48() % J);
	cucon_pset_insert(set, je);
	return je;
    }
    else {
	int M = lrand48() % N;
	cuex_t e0 = build(M, J, set);
	cuex_t e1 = build(N - M, J, set);
	cuex_t e = cuex_meetlattice_meet(MEET, e0, e1);
	cu_test_assert(cuex_meetlattice_semijoin(MEET, e, e1) == e1);
	return e;
    }
}

void
test(int N, int J, cu_bool_t do_print)
{
    struct cucon_pset_s set;
    cuex_t e0, e1, e_meet, e_join;
    cucon_pset_init(&set);
    e0 = build(N, J, &set);
    e1 = build(N, J, &set);
    e_meet = cuex_meetlattice_meet(MEET, e0, e1);
    e_join = cuex_meetlattice_semijoin(MEET, e0, e1);
    if (do_print)
	cufo_oprintf("e0 %<=%> %!\ne1 %<=%> %!\n"
		     "e0 %<∧%> e1 %<=%> %!\n"
		     "e0 %<∨%> e1 %<=%> %!\n\n",
		     cufoT_operator, e0, cufoT_operator, e1,
		     cufoT_operator, cufoT_operator, e_meet,
		     cufoT_operator, cufoT_operator, e_join);
}

int
main()
{
    double x;
    cuex_init();
    test(2, 2, cu_true);
    for (x = 1.0; x < 20.0; x *= M_SQRT2)
	test((int)x, (int)x, cu_true);
    for (x = 1.0; x < 1000.0; x *= M_SQRT2)
	test((int)x, (int)x, cu_false);
    return cu_test_bug_count()? 2 : 0;
}
