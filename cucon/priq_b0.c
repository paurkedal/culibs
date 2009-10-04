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

/* The result of this benchmark is comparable to those of cucon/fibq_b0.c and
 * cucon/fibheap_b0.c */

#include <cucon/priq.h>
#include <cu/test.h>
#include <cu/memory.h>
#include <limits.h>

#define REPEAT 100000
#define N_INS_POP 16 /* power of 2 */
#define VMAX INT_MAX
#define MAX_CARD (1 << 20)

cu_clop_def(_priq_prioreq, cu_bool_t, void *x, void *y)
{
    return *(int *)x <= *(int *)y;
}

static void
_priq_insert(cucon_priq_t Q, int v)
{
    int *p = cu_gnew(int);
    *p = v;
    cucon_priq_insert(Q, p);
}

static int
_priq_pop(cucon_priq_t Q)
{
    int *p = cucon_priq_pop_front(Q);
    return p? *p : -1;
}

static void
_bench()
{
    cucon_priq_t Q = cucon_priq_new(cu_clop_ref(_priq_prioreq));
    size_t avg_card;

    for (avg_card = N_INS_POP/2.0; avg_card <= MAX_CARD; avg_card *= 2) {
	int round;
	double scl = 1.0/((double)CLOCKS_PER_SEC*REPEAT*N_INS_POP);
	clock_t t_ins = 0, t_pop = 0;

	while (cucon_priq_count(Q) < avg_card - N_INS_POP/2)
	    _priq_insert(Q, lrand48() % VMAX);
	for (round = 0; round < REPEAT; ++round) {
	    int i, values[N_INS_POP];

	    for (i = 0; i < N_INS_POP; ++i)
		values[i] = lrand48() % VMAX;
	    t_ins -= clock();
	    for (i = 0; i < N_INS_POP; ++i)
		_priq_insert(Q, values[i]);
	    t_ins += clock();

	    t_pop -= clock();
	    for (i = 0; i < N_INS_POP; ++i)
		_priq_pop(Q);
	    t_pop += clock();
	}
	printf("%8zd %12lg %12lg\n", avg_card, t_ins*scl, t_pop*scl);
    }
}

int
main()
{
    cu_init();
    _bench();
    return 2*!!cu_test_bug_count();
}
