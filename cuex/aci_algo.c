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

#include <cuex/aci_priv.h>
#include <cucon/pmap.h>

static cuex_opn_t
aci_meet_spill(cuex_meta_t join, cuex_opn_t x, cuex_t y_key, cuex_opn_t xy)
{
    while (!aci_is_idy(x)) {
	aci_key_t x_key = aci_key(x);
	if (x_key == y_key) {
	    cuex_t idy = cuex_aci_identity(join);
	    return cuex_aci_join(join, xy, cuex_aci_rebranch(x, idy, idy));
	}
	if (x_key > y_key)
	    x = aci_left(x);
	else
	    x = aci_right(x);
    }
    return xy;
}

static cuex_t
aci_meet(cuex_meta_t join, cuex_t x, cuex_t y, cuex_t xy)
{
    aci_key_t x_llim, x_rlim, y_llim, y_rlim;
    int key_cmp;
tailcall:
    if (aci_is_idy(x) || aci_is_idy(y))
	return xy;

    key_cmp = cuex_cmp(aci_key(x), aci_key(y));
    if (!key_cmp) {
	cuex_t idy = cuex_aci_identity(join);
	xy = cuex_aci_join(join, xy, cuex_aci_rebranch(x, idy, idy));
    }

    aci_tree_lrlim(x, &x_llim, &x_rlim);
    aci_tree_lrlim(y, &y_llim, &y_rlim);
    if (x_rlim <= y_llim || x_llim >= y_rlim)
	return xy;
    if (x_llim == y_llim) {
	if (x_rlim == y_rlim) {
	    xy = aci_meet(join, aci_left(x), aci_left(y), xy);
	    switch (key_cmp) {
		case -1:
		    xy = aci_meet_spill(join, aci_right(x), aci_key(y), xy);
		    break;
		case 1:
		    xy = aci_meet_spill(join, aci_right(y), aci_key(x), xy);
		    break;
	    }
	    x = aci_right(x);
	    y = aci_right(y);
	    goto tailcall;
	}
	else if (x_rlim < y_rlim) {
	    cu_debug_assert(aci_key(y) >= x_rlim); /* Ie no spill */
	    y = aci_left(y);
	    goto tailcall;
	}
	else { /* x_rlim > y_rlim */
	    cu_debug_assert(aci_key(x) >= y_rlim); /* Ie no spill */
	    x = aci_left(x);
	    goto tailcall;
	}
    }
    else if (x_llim < y_llim) {
	cu_debug_assert(x_rlim >= y_rlim);
	if (y_rlim <= aci_tree_clim(x))
	    x = aci_left(x);
	else {
	    cu_debug_assert(y_llim >= aci_tree_clim(x));
	    xy = aci_meet_spill(join, y, aci_key(x), xy);
	    x = aci_right(x);
	}
	goto tailcall;
    }
    else { /* y_llim < x_llim */
	cu_debug_assert(x_rlim <= y_rlim);
	if (x_rlim <= aci_tree_clim(y))
	    y = aci_left(y);
	else {
	    cu_debug_assert(x_llim >= aci_tree_clim(y));
	    xy = aci_meet_spill(join, x, aci_key(y), xy);
	    y = aci_right(y);
	}
	goto tailcall;
    }
}

cuex_opn_t
cuex_aci_meet(cuex_meta_t join, cuex_t x, cuex_t y)
{
    if (cuex_is_aci(join, x))
	if (cuex_is_aci(join, y))
	    return aci_meet(join, x, y, cuex_aci_identity(join));
	else {
	    if (cuex_aci_find(x, y))
		return y;
	    else
		return cuex_aci_identity(join);
	}
    else if (cuex_is_aci(join, y)) {
	if (cuex_aci_find(y, x))
	    return x;
	else
	    return cuex_aci_identity(join);
    }
    else {
	if (x == y)
	    return x;
	else
	    return cuex_aci_identity(join);
    }
}
