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

#include <cucon/parray.h>
#include <cu/test.h>

void
append_n(cucon_parray_t arr, size_t n)
{
    while (n--)
	cucon_parray_append_gp(arr, NULL);
}

int main()
{
    cucon_parray_t arr;
    int x0, x1, x2;
    cucon_init();
    arr = cucon_parray_new_empty();
    cucon_parray_append_gp(arr, &x0);
    cucon_parray_append_gp(arr, &x1);
    cucon_parray_append_gp(arr, &x2);
    cu_test_assert_int_eq(cucon_parray_size(arr), 3);
    cu_test_assert_ptr_eq(*(int **)cucon_parray_ref_at(arr, 1), &x1);
    append_n(arr, 100);
    cu_test_assert_int_eq(cucon_parray_size(arr), 103);
    cu_test_assert_ptr_eq(*(int **)cucon_parray_ref_at(arr, 2), &x2);
    return 0;
}
