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

#include <cu/int.h>
#include <time.h>
#include <string.h>

#define N_FLOOR 100000000

int target_time = 1;

int f_table[0x10000];

CU_SINLINE int f(int x)
{
    return f_table[x % 0x10000];
}

#define def_bench(f, min, max)						\
    int									\
    bench_##f(char *select)						\
    {									\
	int i, sum = 0, n = 0;						\
	clock_t t;							\
	if (select && strcmp(select, #f))				\
	    return 0;							\
	t = -clock();							\
	while (t + clock() < target_time*CLOCKS_PER_SEC) {		\
	    for (i = min; i < max; ++i)					\
		sum += f(i);						\
	    ++n;							\
	}								\
	t += clock();							\
	printf("%30s %12lg s\n", #f, t/((double)CLOCKS_PER_SEC*(max - min)*n));\
	return sum;							\
    }

#define MAX 10000000
def_bench(cu_uint32_dcover, 1, MAX)
def_bench(cu_uint64_dcover, 1, MAX)
def_bench(cu_ulong_ucover, 1, MAX)
def_bench(cu_uint32_floor_log2, 1, MAX)
def_bench(cu_uint64_floor_log2, 1, MAX)
def_bench(cu_uint32_bit_count, 1, MAX)
def_bench(cu_uint64_bit_count, 1, MAX)
def_bench(f, 1, MAX)

int
main(int argc, char **argv)
{
    char *select = NULL;
    cu_init();
    if (argc == 3) {
	target_time = atoi(argv[1]);
	select = argv[2];
    }

    bench_cu_uint32_dcover(select);
    bench_cu_uint64_dcover(select);
    bench_cu_ulong_ucover(select);
    bench_cu_uint32_floor_log2(select);
    bench_cu_uint64_floor_log2(select);
    bench_cu_uint32_bit_count(select);
    bench_cu_uint64_bit_count(select);
    bench_f(select);
    return 0;
}
