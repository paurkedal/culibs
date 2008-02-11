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

#include <cu/hash.h>
#include <cu/test.h>
#include <time.h>
#include <stdio.h>

#define clockavg(t, n) ((t)/((double)CLOCKS_PER_SEC*(n)))

int
main()
{
    int sizew;
    cu_word_t arr[8];

    cu_init();
    for (sizew = 1; sizew <= 8; ++sizew) {
	int N = 40000000 / (1 + (sizew - 1)/3);
	int i;
	clock_t t_arr, t_arr_noinit;

	t_arr = -clock();
	for (i = 0; i < N; ++i)
	    cu_wordarr_hash_bj(sizew, arr, 0);
	t_arr += clock();

	t_arr_noinit = -clock();
	for (i = 0; i < N; ++i)
	    cu_wordarr_hash_noinit_bj(sizew, arr);
	t_arr_noinit += clock();

	printf("%3d %12lg%12lg", sizew, clockavg(t_arr, N), clockavg(t_arr_noinit, N));

	if (sizew == 1) {
	    clock_t t_1w, t_1w_noinit;

	    t_1w = -clock();
	    for (i = 0; i < N; ++i)
		cu_1word_hash_bj(arr[0], 0);
	    t_1w += clock();

	    t_1w_noinit = -clock();
	    for (i = 0; i < N; ++i)
		cu_1word_hash_noinit_bj(arr[0]);
	    t_1w_noinit += clock();

	    printf("%12lg%12lg", clockavg(t_1w, N), clockavg(t_1w_noinit, N));
	} else if (sizew == 2) {
	    clock_t t_2w, t_2w_noinit;

	    t_2w = -clock();
	    for (i = 0; i < N; ++i)
		cu_2word_hash_bj(arr[0], arr[1], 0);
	    t_2w += clock();

	    t_2w_noinit = -clock();
	    for (i = 0; i < N; ++i)
		cu_2word_hash_noinit_bj(arr[0], arr[1]);
	    t_2w_noinit += clock();

	    printf("%12lg%12lg", clockavg(t_2w, N), clockavg(t_2w_noinit, N));
	}
	fputc('\n', stdout);
    }
    return 2*!!cu_test_bug_count();
}
