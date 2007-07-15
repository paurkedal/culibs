/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2005--2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cuex/opntpl.h>

cuex_opn_t
cuex_opntpl_new(cuex_meta_t opr)
{
    cu_rank_t r = cuex_opr_r(opr);
    size_t size = cuex_sizeof_opn(r);
    cuex_opn_t opn = cuexP_oalloc(opr, size);
    return opn;
}

cuex_t
cuex_of_opntpl(cuex_t ex)
{
    cuex_meta_t meta = cuex_meta(ex);
    if (cuex_meta_is_opr(meta)) {
	cu_rank_t r = cuex_opr_r(meta);
	cu_rank_t i;
	cuex_t *arr = cu_salloc(r*sizeof(cuex_t));
	for (i = 0; i < r; ++i) {
	    cuex_t old_sub = cuex_opn_at(ex, i);
	    cu_debug_assert(old_sub);
	    arr[i] = cuex_of_opntpl(old_sub);
	}
	return cuex_opn_by_arr(meta, arr);
    }
    else
	return ex;
}
