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

#include <cuex/tuple.h>
#include <cuex/fpvar.h>
#include <cuex/opn.h>


cuex_t
cuex_tuple_sub(cuex_t tuple, cu_rank_t begin, cu_rank_t end)
{
    cu_rank_t r = end - begin;
    cuex_t *argv = cu_salloc(sizeof(cuex_t)*r);
    cu_debug_assert(cuex_opn_r(tuple) <= end);
    while (begin < end) {
	*argv++ = cuex_opn_at(tuple, begin);
	++begin;
    }
    argv -= r;
    return cuex_opn_by_arr(CUEX_OR_TUPLE(r), argv);
}

cuex_t
cuex_tuple_sub_carg(cuex_t carg, cuex_t tuple, cu_rank_t begin, cu_rank_t end)
{
    cu_rank_t r = end - begin + 1;
    cuex_t *argv = cu_salloc(sizeof(cuex_t)*r);
    cu_debug_assert(cuex_opn_r(tuple) <= end);
    *argv++ = carg;
    while (begin < end) {
	*argv++ = cuex_opn_at(tuple, begin);
	++begin;
    }
    argv -= r;
    return cuex_opn_by_arr(CUEX_OR_TUPLE(r), argv);
}

cuex_t
cuex_tuple_append(cuex_t tuple0, cuex_t tuple1)
{
    cu_rank_t r0 = cuex_opn_r(tuple0);
    cu_rank_t r1 = cuex_opn_r(tuple1);
    cu_rank_t r = r0 + r1;
    cuex_t *argv = cu_salloc(sizeof(cuex_t)*r);
    memcpy(argv,      cuex_opn_begin(tuple0), sizeof(cuex_t)*r0);
    memcpy(argv + r0, cuex_opn_begin(tuple1), sizeof(cuex_t)*r1);
    return cuex_opn_by_arr(CUEX_OR_TUPLE(r), argv);
}

