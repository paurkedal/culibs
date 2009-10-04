/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2008  Petter Urkedal <urkedal@nbi.dk>
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

#include <cuex/occurtree.h>
#include <cuex/binding.h>
#include <cuex/oprdefs.h>
#include <cuex/set.h>
#include <cuex/labelling.h>
#include <cudyn/misc.h>
#include <cufo/stream.h>
#include <cufo/tagdefs.h>
#include <cu/test.h>

#define _apply cuex_o2_apply
#define _ident cuex_o1_ident
#define _lambda cuex_o1_lambda
#define _mu cuex_o1_mu
#define _set1(x0) cuex_singleton_set(x0)
#define _set2(x0, x1) cuex_set_insert(_set1(x0), x1)
#define _set3(x0, x1, x2) cuex_set_insert(_set2(x0, x1), x2)

static void
_show(cuex_t e)
{
    cuex_occurtree_t ot;
    cufo_oprintf("\n%<orig%> = %!\n", cufoT_variable, e);
    ot = cuex_unfolded_occurtree(e, cu_true);
    cufo_flush(cufo_stdout);
    cuex_occurtree_dump(ot, stdout);
    fflush(stdout);

    ot = cuex_occurtree_prune_mu(ot, CUEX_SCOMM_VIEW);
    if (e != ot->e)
	cufo_oprintf("%<simp%> = %!\n", cufoT_variable, ot->e);
}

void
test()
{
    cuex_t _0 = cuex_hole(0);
    cuex_t _1 = cuex_hole(1);
    cuex_t _2 = cuex_hole(2);
    cuex_t _3 = cuex_hole(3);

    cuex_t ident = _lambda(_0);
    cuex_t konst = _lambda(_lambda(_1));
    cuex_t scomb = _lambda(_lambda(_lambda(_apply(_apply(_2, _0),
						  _apply(_1, _0)))));

    _show(ident);
    _show(konst);
    _show(scomb);
    _show(_ident(_mu(_apply(_0, _3))));
    _show(_mu(_lambda(_mu(_apply(_0, _2)))));
    _show(_set3(ident, konst, _3));
    _show(_mu(_set3(_lambda(_0), _1, _2)));
    _show(_mu(_ident(_lambda(_mu(_apply(_1, _2))))));
    _show(cuex_labelling_by_arglist(cudyn_int(0), _lambda(_ident(_0)),
				    cudyn_int(1), cuex_empty_set(), NULL));
}

int
main()
{
    cuex_init();
    test();
    return 2*!!cu_test_bug_count();
}
