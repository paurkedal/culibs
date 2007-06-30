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

#include <cu/clos.h>
#include <stdio.h>
#include <stdlib.h>
#include <cu/conf.h>
#ifdef CUCONF_HAVE_GC_GC_H
#  include <gc/gc.h>
#else
#  include <gc.h>
#endif
#include <time.h>


#define N_ITERATE 1000
#define N_REPEAT_OUTER 100
#define N_REPEAT_INNER 1000

int f(int x)
{
    return 2*x + 1;
}

cu_clop_def(f_clop, int, int x)
{
    return 2*x + 1;
}

int g_plain(int *context, int x)
{
    *context += x;
    return *context;
}

cu_clos_def(g_clos, cu_prot(int, int x), (int context;))
{
    cu_clos_self(g_clos);
    self->context += x;
    return self->context;
}

int
test_simple_plain(int (*fn)(int), int *context)
{
    int i;
    int r = 0;
    for (i = 0; i < N_ITERATE; ++i)
	r = fn(r);
    return r;
}

int
test_clos(cu_clop(clptr, int, int))
{
    int i;
    int r = 0;
    for (i = 0; i < N_ITERATE; ++i)
	r = cu_call(clptr, r);
    return r;
}

int
test_context_plain(int (*fn)(int *context, int),
		   int *context)
{
    int i;
    int r = 0;
    for (i = 0; i < N_ITERATE; ++i)
	r = fn(context, r);
    return r;
}


int
submain()
{
    int i, j;
    int *g_context = GC_malloc(sizeof(int));
    clock_t t_c_plain = 0, t_s_plain = 0;
    clock_t t_c_clos = 0, t_s_clos = 0;
    g_clos_t g_clos;
    cu_clop(g_clop, int, int) = g_clos_prep(&g_clos);
    *g_context = 0;

    for (j = 0; j < N_REPEAT_OUTER; ++j) {
	t_s_plain -= clock();
	for (i = 0; i < N_REPEAT_INNER; ++i)
	    test_simple_plain(f, NULL);
	t_s_plain += clock();

	t_c_plain -= clock();
	for (i = 0; i < N_REPEAT_INNER; ++i)
	    test_context_plain(g_plain, g_context);
	t_c_plain += clock();

	t_s_clos -= clock();
	for (i = 0; i < N_REPEAT_INNER; ++i)
	    test_clos(f_clop);
	t_s_clos += clock();

	t_c_clos -= clock();
	for (i = 0; i < N_REPEAT_INNER; ++i)
	    test_clos(g_clop);
	t_c_clos += clock();
    }

    printf("\n         %10s     %10s\n"
	   "     plain %10lf s %10lf s\n"
	   "      clos %10lf s %10lf s\n"
	   "clos/plain %10lf   %10lf\n\n",
	   "simple",
	   "with cxt",
	   t_s_plain/(double)CLOCKS_PER_SEC,
	   t_c_plain/(double)CLOCKS_PER_SEC,
	   t_s_clos/(double)CLOCKS_PER_SEC,
	   t_c_clos/(double)CLOCKS_PER_SEC,
	   t_s_clos/(double)t_s_plain,
	   t_c_clos/(double)t_c_plain);
    return 0;
}

int
main()
{
    cu_init();
    submain();
    return 0;
}
