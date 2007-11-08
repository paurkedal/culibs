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

#include <cuex/dunion.h>
#include <cuex/aci.h>
#include <cuex/oprdefs.h>
#include <cuex/opr_priv.h>

static int
dunion_count(cuex_opn_t e)
{
    if (cuex_is_aci_identity(CUEX_O4ACI_DUNION, e))
	return 0;
    else {
	cuex_opr_dunion_cache_t cache = cuex_opn_cache_r(e, 4);
	return cache->count;
    }
}

void
cuex_opr_dunion_2aci2_cache_cct(cuex_opn_t e)
{
    cuex_opr_dunion_cache_t cache = cuex_opn_cache_r(e, 4);
    cu_debug_assert(cuex_meta(e) == CUEX_O4ACI_DUNION);
    cache->count = dunion_count(cuex_aci_left(e))
		 + dunion_count(cuex_aci_right(e));
}

cu_bool_t
cuex_dunion_find(cuex_opn_t e, cu_idr_t label,
		 cuex_t *type_out, int *index_out)
{
    int index = 0;
    if (cuex_is_aci_identity(CUEX_O4ACI_DUNION, e))
	return cu_false;
    for (;;) {
	int cmp = cuex_cmp(label, cuex_aci_key(e));
	if (cmp <= 0) {
	    index += dunion_count(cuex_aci_left(e));
	    if (cmp == 0) {
		*type_out = cuex_aci_at(e, 1);
		*index_out = index;
	    }
	    e = cuex_aci_right(e);
	} else
	    e = cuex_aci_left(e);
    }
}
