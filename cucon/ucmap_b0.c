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
#include <cucon/ucmap.h>
#include <time.h>
#include <stdio.h>

#define NINS 2000
#define NFIND 2000
#define MOD 4000
#define REPEAT 500
#define ENABLE_UMAP 0
#define ENABLE_UCMAP 1

void
bench(cu_bool_t enable_umap, cu_bool_t enable_ucmap)
{
    int i, j;
    clock_t ubt_tins = 0, umap_tins = 0;
    clock_t ubt_tfind = 0, umap_tfind = 0;
    for (j = 0; j < REPEAT; ++j) {
	cucon_ucmap_t tree = NULL;
	cucon_umap_t umap = cucon_umap_new();
	for (i = 0; i < NINS; ++i) {
	    int j = lrand48() % MOD;
	    if (enable_ucmap) {
		ubt_tins -= clock();
		tree = cucon_ucmap_insert_int(tree, j, j);
		ubt_tins += clock();
	    }
	    if (enable_umap) {
		umap_tins -= clock();
		cucon_umap_insert_void(umap, j);
		umap_tins += clock();
	    }
	}
	for (i = 0; i < NFIND; ++i) {
	    int j = lrand48() % MOD;
	    if (enable_ucmap) {
		ubt_tfind -= clock();
		cucon_ucmap_find_ptr(tree, j);
		ubt_tfind += clock();
	    }
	    if (enable_umap) {
		umap_tfind -= clock();
		cucon_umap_find_mem(umap, j);
		umap_tfind += clock();
	    }
	}
    }
    printf("ubt_tins  = %lf\numap_tins = %lf\n",
	   ubt_tins/(double)CLOCKS_PER_SEC,
	   umap_tins/(double)CLOCKS_PER_SEC);
    printf("ubt_tfind  = %lf\numap_tfind = %lf\n",
	   ubt_tfind/(double)CLOCKS_PER_SEC,
	   umap_tfind/(double)CLOCKS_PER_SEC);
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
