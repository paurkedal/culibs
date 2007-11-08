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

#include <cuex/aci.h>
#include <cuex/oprdefs.h>
#include <cudyn/misc.h>
#include <time.h>
#include <math.h>

#define S 2
#define KEY_CNT 1000000
#define FIND_MULT 10
#define OPR CUEX_O3ACI_PROD
#define cuex_aci_insert cuex_aci_join
cuex_t key_arr[KEY_CNT];
FILE *out;

cuex_opn_t
bench_N_to_SN(size_t N, cuex_opn_t eN)
{
    cuex_opn_t e;
    size_t i;
    size_t K;
    size_t k;
    clock_t t0, t1;
    double c;
    struct cuex_aci_stats_s stats;
    K = (int)(1e7/(N*S * log2(N*S))) + 1;
    printf("insert\r"); fflush(stdout);
    t0 = -clock();
    for (k = 0; k < K; ++k) {
	e = eN;
	for (i = N; i < N*S; ++i)
	    e = cuex_aci_insert(OPR, e, key_arr[i]);
    }
    t0 += clock();
    printf("insert, find\n");
    t1 = -clock();
    for (k = 0; k < K*FIND_MULT; ++k) {
	for (i = N; i < N*S; ++i)
	    cuex_aci_find(e, key_arr[i]);
    }
    t1 += clock();
    c = 1.0/((double)CLOCKS_PER_SEC*K*N*9);
    printf("%6ld %12lg %12lg K=%ld\n", (long)N*S, t0*c, t1*c/FIND_MULT,
	   (long)K);
    fprintf(out, "%6ld %12lg %12lg\n", (long)N*S, t0*c, t1*c/FIND_MULT);
    fflush(out);

    cuex_aci_stats(e, &stats);
    printf("%d depth_rng=[%d, %d] avg_node_depth=%lg avg_leaf_depth=%lg\n",
	   (int)stats.node_cnt,
	   stats.min_depth, stats.max_depth,
	   stats.node_depth_sum/(double)stats.node_cnt,
	   stats.leaf_depth_sum/(double)stats.leaf_cnt);
    if (N < 8)
	cuex_aci_dump(e, stdout);

    return e;
}

int
main(int argc, char **argv)
{
    size_t N;
    cuex_opn_t eN;
    size_t i;
    cuex_init();
    out = fopen("aci_b0.out", "w");
    if (!out) {
	perror(argv[0]);
	return 2;
    }
    for (i = 0; i < KEY_CNT; ++i)
	key_arr[KEY_CNT-i-1] = cudyn_int(KEY_CNT-i-1);
    eN = cuex_aci_insert(OPR, cuex_aci_identity(OPR), key_arr[0]);
    for (N = 1; N < KEY_CNT/S; N *= S)
	eN = bench_N_to_SN(N, eN);
    return 0;
}
