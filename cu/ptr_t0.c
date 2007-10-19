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

#include <cu/ptr.h>
#include <cu/test.h>

int
main()
{
    int arr[8];
    int i, j;
    void *p0, *p1;
    p0 = arr;
    for (i = 0; i < 16; ++i) {
	p1 = cu_ptr_add(p0, i);
	cu_test_assert(cu_ptr_sub(p1, i) == p0);
	cu_test_assert(cu_ptr_diff(p1, p0) == i);
	cu_test_assert(cu_ptr_alignfloor(p1) == cu_ptr_alignfloor(p1));
	if ((uintptr_t)p1 % CUCONF_MAXALIGN == 0)
	    cu_test_assert(cu_ptr_alignceil(p1) == cu_ptr_alignfloor(p1));
	else
	    cu_test_assert(cu_ptr_diff(cu_ptr_alignceil(p1),
				       cu_ptr_alignfloor(p1))
			   == CUCONF_MAXALIGN);
	for (j = 0; j < 4; ++j) {
	    cu_test_assert(cu_ptr_mulceil(p1, 1 << j)
			   == cu_ptr_scal2ceil(p1, j));
	    cu_test_assert(cu_ptr_mulfloor(p1, 1 << j)
			   == cu_ptr_scal2floor(p1, j));
	}
    }
    return 0;
}
