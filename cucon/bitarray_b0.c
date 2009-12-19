/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2009  Petter Urkedal <urkedal@nbi.dk>
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

#include <cucon/bitarray.h>
#include <cu/test.h>
#include <time.h>
#include <stdio.h>

#define REPEAT 100
#define ARR_SIZE 1000000

double
_time_per_call(clock_t t)
{
    return t/((double)CLOCKS_PER_SEC*REPEAT*ARR_SIZE);
}

void
_test()
{
    size_t i;
    size_t repeat = REPEAT;
    clock_t t_set = 0, t_get = 0;
    struct cucon_bitarray ba;
    cucon_bitarray_init(&ba, ARR_SIZE);
    while (repeat--) {
	t_set -= clock();
	for (i = 0; i < ARR_SIZE; ++i)
	    cucon_bitarray_set_at(&ba, i, i & 1);
	t_set += clock();
	t_get -= clock();
	for (i = 0; i < ARR_SIZE; ++i) {
	    cu_bool_t val = cucon_bitarray_at(&ba, i);
	    cu_test_assert(val == (i & 1));
	}
	t_get += clock();
    }
    printf("t_set: %12.2lg\n"
	   "t_get: %12.2lg\n",
	   _time_per_call(t_set), _time_per_call(t_get));
}

int
main()
{
    cucon_init();
    _test();
    return 2*!!cu_test_bug_count();
}
