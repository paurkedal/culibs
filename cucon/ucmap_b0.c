/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2007--2010  Petter Urkedal <paurkedal@eideticdew.org>
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
#include <cucon/ucmap.h>
#include <time.h>
#include <stdio.h>

#define NINS 2000
#define NFIND 2000
#define MOD 4000
#define REPEAT 500
#define ENABLE_UMAP 0
#define ENABLE_UCMAP 1

#define SCALE(x, N) ((x)/((double)CLOCKS_PER_SEC*N*REPEAT))

void
bench(cu_bool_t enable_umap, cu_bool_t enable_ucmap)
{
    int i, j;
    clock_t ucmap_tins = 0, umap_tins = 0;
    clock_t ucmap_tfind = 0, umap_tfind = 0;
    long int seed = lrand48();

    printf("_________insert______find\n");
    if (enable_umap) {
	srand48(seed);
	for (j = 0; j < REPEAT; ++j) {
	    cucon_umap_t umap = cucon_umap_new();
	    for (i = 0; i < NINS; ++i) {
		int j = lrand48() % MOD;
		umap_tins -= clock();
		cucon_umap_insert_int(umap, j, j);
		umap_tins += clock();
	    }
	    for (i = 0; i < NFIND; ++i) {
		int j = lrand48() % MOD;
		umap_tfind -= clock();
		cucon_umap_find_mem(umap, j);
		umap_tfind += clock();
	    }
	}
	printf(" umap%#10.3lg%#10.3lg\n",
	       SCALE(umap_tins, NINS), SCALE(umap_tfind, NFIND));
    }
    if (enable_ucmap) {
	srand48(seed);
	for (j = 0; j < REPEAT; ++j) {
	    cucon_ucmap_t ucmap = NULL;
	    for (i = 0; i < NINS; ++i) {
		int j = lrand48() % MOD;
		ucmap_tins -= clock();
		ucmap = cucon_ucmap_insert_int(ucmap, j, j);
		ucmap_tins += clock();
	    }
	    for (i = 0; i < NFIND; ++i) {
		int j = lrand48() % MOD;
		ucmap_tfind -= clock();
		cucon_ucmap_find_ptr(ucmap, j);
		ucmap_tfind += clock();
	    }
	}
	printf("ucmap%#10.3lg%#10.3lg\n",
	       SCALE(ucmap_tins, NINS), SCALE(ucmap_tfind, NFIND));
    }
}

int
main(int argc, char **argv)
{
    int enable = 3;
    cu_init();
    if (argc > 1)
	sscanf(argv[1], "%d", &enable);
    bench(enable & 1, enable & 2);
    return 0;
}
