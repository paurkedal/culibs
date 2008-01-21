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

#include <cuex/fwd.h>
#include <cudyn/misc.h>
#include <cu/test.h>
#include <stdint.h>

int main()
{
    uint64_t n;
    cuex_t x;
    cuex_init();
    x = cudyn_uint(792);
    cu_test_assert(cudyn_is_uint(x));
    cu_test_assert(cudyn_to_uint(x) == 792);
    cu_test_assert(cudyn_uint(792) == x);
    x = cudyn_uint8(111);
    cu_test_assert(cudyn_is_uint8(x));
    cu_test_assert(cudyn_to_uint8(x) == 111);
    cu_test_assert(cudyn_uint8(111) == x);
    n = UINT64_C(0xf39a241788bed001);
    x = cudyn_uint64(n);
    cu_test_assert(cudyn_is_uint64(x));
    cu_test_assert(cudyn_to_uint64(x) == n);
    cu_test_assert(cudyn_uint64(n) == x);
    return 2*!!cu_test_bug_count();
}
