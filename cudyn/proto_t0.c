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

#include <cudyn/proto.h>
#include <cudyn/misc.h>
#include <cu/int.h>

int
main()
{
    cudyn_proto_t p_uint_uint;
    cuex_t res0, res1;
    cuex_t arg_arr[2];
    cuex_init();

    p_uint_uint = cudyn_proto(1, cudyn_uint_type(), cudyn_uint_type());
    arg_arr[0] = cudyn_uint(0x60);
    res0 = cudyn_proto_apply_fn(p_uint_uint, (cu_fnptr_t)cu_uint_log2_lowbit,
				arg_arr);
    arg_arr[0] = cudyn_uint(0x20);
    res1 = cudyn_proto_apply_fn(p_uint_uint, (cu_fnptr_t)cu_uint_log2_lowbit,
				arg_arr);
    cu_debug_assert(cudyn_is_uint(res0) && cudyn_is_uint(res1));
    cu_debug_assert(cudyn_to_uint(res0) == 5);
    cu_debug_assert(res0 == res1);
    return 0;
}
