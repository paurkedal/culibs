/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2006--2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cuex/recursion.h>
#include <cuex/ex.h>
#include <cuex/oprdefs.h>
#include <cuex/opn.h>
#include <cuex/var.h>

static cuex_t
mu_unfold(cuex_t e, int l_top, cuex_t e_top)
{
    cuex_meta_t e_meta = cuex_meta(e);
    if (cuex_meta_is_opr(e_meta)) {
	if (cuex_og_hole_contains(e_meta)) {
	    int l = cuex_oa_hole_index(e_meta);
	    if (l == l_top)
		return cuex_bfree_adjusted(e_top, l_top);
	    else if (l > l_top)
		return cuex_hole(l - 1);
	}
	else {
	    if (cuex_og_binder_contains(e_meta))
		++l_top;
	    CUEX_OPN_TRAN(e_meta, e, ep, mu_unfold(ep, l_top, e_top));
	}
    }
    return e;
}

cuex_t
cuex_mu_unfold(cuex_t e)
{
    cu_debug_assert(cuex_meta(e) == CUEX_O1_MU);
    return mu_unfold(cuex_opn_at(e, 0), 0, e);
}
