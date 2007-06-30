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

#include <cudyn/type.h>
#include <cudyn/misc.h>
#include <cuex/oprdefs.h>
#include <cuex/ex.h>
#include <cuex/sig.h>

void
test_type(cuex_t ex)
{
    cudyn_type_t t = cudyn_type(ex);
    cu_debug_assert(t);
    cu_fprintf(stdout, "%! ↦ %! (%d bits aligned on %d bits)\n",
	       ex, t, cudyn_type_bitsize(t), cudyn_type_bitalign(t));
}

void
test()
{
    test_type(cuex_o2_gexpt(cudyn_int_type(), cudyn_int(4)));
    test_type(cuex_o2_gexpt(cudyn_bool_type(), cudyn_int(33)));
    test_type(cuex_o2_gprod(cudyn_int8_type(), cudyn_int16_type()));
    test_type(cuex_o2_gprod(cudyn_int8_type(), cudyn_int32_type()));
    test_type(cuex_sig_v(3,
		"x", cudyn_int8_type(), 
		"y", cudyn_float_type(),
		"cond", cudyn_bool_type()));
}


int
main()
{
    cuex_init();
    cu_debug_assert(cudyn_int_type());
    test();
    return 0;
}
