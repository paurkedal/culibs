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

#include <cu/wstring.h>
#include <cu/test.h>

int
main()
{
    cu_wstring_t s0, s1, s2, s3;
    int i;
    s0 = CU_WSTRING_C("Break me ");
    s1 = CU_WSTRING_C("not.");
    s2 = cu_wstring_concat(s0, s1);
    s3 = cu_wstring_slice(s0, 2, 5);
    cu_test_assert(cu_wstring_cmp(s2, CU_WSTRING_C("Break me not.")) == 0);
    cu_test_assert(cu_wstring_cmp(s3, CU_WSTRING_C("eak")) == 0);
    for (i = 0; i < cu_wstring_length(s3); ++i)
	cu_test_assert(cu_wstring_at(s3, i) ==
		       cu_wstring_at(cu_wstring_slice_o(s3, i, i + 1), 0));
    return 0;
}
