/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2011  Petter Urkedal <paurkedal@gmail.com>
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

#include <cuex/type.h>
#include <cuex/opn.h>
#include <cudyn/misc.h>
#include <cu/test.h>

int incr(int i) { return i + 1; }

int
main()
{
    cuex_t incr_ty, incr_ex;

    cuex_init();
    incr_ty = cuex_o2_farrow_native(cudyn_int_type(), cudyn_int_type());
    incr_ex = cuex_of_fnptr(incr_ty, (cu_fnptr_t)incr);
    cu_test_assert(cuex_is_fnptr(incr_ex));
    return 0;
}
