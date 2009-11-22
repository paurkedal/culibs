/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2008  Petter Urkedal <urkedal@nbi.dk>
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

/* Benchmark of cucon/stack.h.
 * See also cucon/frame_b0.c */

#include <cucon/stack.h>
#include <cu/test.h>
#include <time.h>
#include <stdio.h>

typedef struct _frame *_frame_t;
struct _frame
{
    long data[4];
};

#define _avg_time(n, t) ((double)(t)/((double)(n)*CLOCKS_PER_SEC))

static void
_benchmark(int N, int I)
{
    int n, i, j;
    struct cucon_stack stack;
    clock_t t_grow = 0, t_access = 0;
    _frame_t frame;
    int sum = 0;

    for (i = 0; i < I; ++i) {
	t_grow -= clock();
	cucon_stack_init(&stack);
	for (n = 0; n < N; ++n) {
	    frame = cucon_stack_alloc(&stack, sizeof(struct _frame));
	    for (j = 0; j < 4; ++j)
		frame->data[j] = n;
	}
	t_grow += clock();

	t_access -= clock();
	for (n = 0; n < N; ++n) {
	    frame = cucon_stack_at(&stack, n*sizeof(struct _frame));
	    for (j = 0; j < 4; ++j)
		sum += frame->data[j];
	}
	t_access += clock();
    }
    printf("%6d %12lg %12lg %12d\n", n,
	   _avg_time(i*n, t_grow), _avg_time(i*n, t_access), sum);
}

int
main()
{
    int N;
    cu_init();
    printf("   dim %12s %12s\n", "grow", "access");
    for (N = 8; N < 100000; N *= 2)
	_benchmark(N, 1000000/N);
    return 2*!!cu_test_bug_count();
}
