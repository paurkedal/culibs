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

#include <cu/idr.h>
#include <cu/test.h>
#include <stdlib.h>

#define N_IDR 100000
int
main()
{
    int j;
    char buf[sizeof(int)*2+1];
    cu_idr_t idr_arr[N_IDR];
    cu_init();
    memset(idr_arr, 0, sizeof(idr_arr));
    for (j = 0; j < 4*N_IDR; ++j) {
	cu_idr_t idr;
	int i = lrand48() % N_IDR;
	sprintf(buf, "%x", i);
	idr = cu_idr_by_cstr(buf);
	cu_test_assert(idr == cu_idr_by_charr(buf, strlen(buf)));
	if (idr_arr[i])
	    cu_test_assert(idr == idr_arr[i]);
	idr_arr[i] = idr;
    }
    return 0;
}
