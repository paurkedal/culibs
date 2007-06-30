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

#include <cu/memory.h>
#include <cucon/logchain.h>
#include <stdio.h>

#define N_subsk 1000
#define N_tests 1000000

int
real_main()
{
    int n_err = 0;
    int i;
    void *logchain[N_subsk];
    void *logchain_prev = NULL;
    logchain[0] = cu_galloc(1);
    for (i = 1; i < N_subsk; ++i)
	logchain[i] = logchain_prev
	    = cucon_logchain_galloc(0, i - 1, logchain_prev);
    for (i = 0; i < N_tests; ++i) {
	size_t i_cur = lrand48() % N_subsk;
	size_t i_dst = i_cur % N_subsk;
	void *logchain_dst;
/* 	printf("0x%04x 0x%04x\n", i_cur, i_dst); */
	logchain_dst = cucon_logchain_find(logchain[i_cur], i_cur, i_dst);
	if (logchain_dst != logchain[i_dst]) {
	    fprintf(stderr, "Failed to obtain stack 0x%lx from 0x%lx.\n",
		    (long)i_dst, (long)i_cur);
	    ++n_err;
	}
    }
    GC_gcollect(); /* see if we've obscured GC internals */
    return n_err != 0;
}

int
main()
{
    cu_init();
    return real_main();
}
