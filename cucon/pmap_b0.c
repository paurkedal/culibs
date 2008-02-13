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

#include <cucon/pmap.h>
#include <cu/memory.h>
#include <time.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

void
bench_em(int N)
{
    int j;
    int J = 0x40000/N + 1;
    int data;
    clock_t t0 = 0, t1 = 0;
/* #define STORE_KEYS */

#ifdef STORE_KEYS
    void **keys = cu_galloc((N + N/2)*sizeof(void*));
    for (j = 0; j < N + N/2; ++j)
/* 	keys[j] = &keys[(j*j) % N]; */
	keys[j] = cu_gnew(char);
#endif

    GC_gcollect();
    for (j = 0; j < J; ++j) {
	int i;
	cucon_pmap_t pmap;
	int count = 0;

	/* Bench pmap */
	pmap = cucon_pmap_new();
	t0 -= clock();
	for (i = 0; i < N; ++i)
#ifdef STORE_KEYS
	    cucon_pmap_replace_ptr(pmap, keys[i], &data);
#else
	    cucon_pmap_replace_ptr(pmap, cu_gnew(char), &data);
#endif
	t0 += clock();
	t1 -= clock();
	for (i = 0; i < N; ++i)
#ifdef STORE_KEYS
	    count += !!cucon_pmap_find_mem(pmap, keys[i + N/2]);
#else
	    count += !!cucon_pmap_find_mem(pmap, &data + j*sizeof(void*));
#endif
	t1 += clock();
	if (j == 0)
	    cucon_pmap_dump_stats(pmap, stdout);
    }
    printf("%8d%16lg%16lg\n", N,
	   t0/((double)CLOCKS_PER_SEC * N * J),
	   t1/((double)CLOCKS_PER_SEC * N * J));
}


int
main()
{
    int i;
    cu_init();
    printf("\t%16s%16s\n", "insert", "find");
    for (i = 0; i < 22; ++i)
	bench_em(1 << i);
    return 0;
}
