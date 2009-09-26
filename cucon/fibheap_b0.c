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

#include <cucon/fibheap.h>
#include <cucon/fibheap_test.h>
#include <cu/test.h>
#include <limits.h>

#define REPEAT 100000
#define N_INS_POP 16 /* power of 2 */
#define VMAX INT_MAX
#define MAX_CARD (1 << 20)

static void
_bench()
{
    cucon_fibheap_t H = cucon_fibheap_new(cu_clop_ref(_fibnode_prioreq));
    size_t avg_card;

    for (avg_card = N_INS_POP/2.0; avg_card <= MAX_CARD; avg_card *= 2) {
	int round;
	double scl = 1.0/((double)CLOCKS_PER_SEC*REPEAT*N_INS_POP);
	clock_t t_ins = 0, t_pop = 0;

	while (cucon_fibheap_card(H) < avg_card - N_INS_POP/2)
	    _fibheap_insert(H, lrand48() % VMAX);
	for (round = 0; round < REPEAT; ++round) {
	    int i, values[N_INS_POP];

	    for (i = 0; i < N_INS_POP; ++i)
		values[i] = lrand48() % VMAX;
	    t_ins -= clock();
	    for (i = 0; i < N_INS_POP; ++i)
		_fibheap_insert(H, values[i]);
	    t_ins += clock();

	    t_pop -= clock();
	    for (i = 0; i < N_INS_POP; ++i)
		_fibheap_pop(H);
	    t_pop += clock();
	}
	printf("%8zd %12lg %12lg\n", avg_card, t_ins*scl, t_pop*scl);
    }
}

int
main()
{
    cucon_init();
    _bench();
    return 2*!!cu_test_bug_count();
}
