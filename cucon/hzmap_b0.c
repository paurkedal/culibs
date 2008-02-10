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

/* This test is comparable to cucon/uset_b0 */

#include <cucon/hzmap.h>
#include <cu/memory.h>
#include <time.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

clock_t avg_t_insert = 0, avg_t_find = 0;
long avg_count = 0;

void
bench_em(int N)
{
    int j;
    int J = 0x80000/N + 1;
    clock_t t_insert = 0, t_find = 0;

    /* GC_gcollect(); */
    for (j = 0; j < J; ++j) {
	int i;
	cucon_hzmap_t hzmap;

	hzmap = cucon_hzmap_new(1);
	t_insert -= clock();
	for (i = 0; i < N; ++i) {
	    cu_word_t key = i*i%N;
	    cucon_hzmap_insert_void(hzmap, &key);
	}
	t_insert += clock();
	t_find -= clock();
	for (i = 0; i < N; ++i) {
	    cu_word_t key = i;
	    cucon_hzmap_1w_find(hzmap, &key);
	}
	t_find += clock();
    }
    printf("%8d%16lg%16lg\n", N,
	   t_insert/((double)CLOCKS_PER_SEC * N * J),
	   t_find/((double)CLOCKS_PER_SEC * N * J));
    avg_t_insert += t_insert;
    avg_t_find += t_find;
    avg_count += N*J;
}

int
main()
{
    int i;
    cucon_init();
    printf("\t%16s%16s\n", "insert", "find");
    for (i = 0; i < 22; ++i)
	bench_em(1 << i);
    printf("---------------------------------------------"
	   "\n     avg %16lg%16lg\n",
	   avg_t_insert/((double)CLOCKS_PER_SEC*avg_count),
	   avg_t_find/((double)CLOCKS_PER_SEC*avg_count));
    return 0;
}
