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

#define PLAIN_CONTEXT_FIRST CU_CLOS_CONTEXT_FIRST

#define N_ITERATE 1000
#define N_REPEAT_OUTER 400
#define N_REPEAT_INNER 1000

static int _f_func(int x)
{
    return 2*x + 1;
}

cu_clop_def(_f_clop, int, int x)
{
    return 2*x + 1;
}

static int 
#if PLAIN_CONTEXT_FIRST
_g_func(int *context, int x)
#else
_g_func(int x, int *context)
#endif
{
    *context += x;
    return *context;
}

cu_clos_def(_g_clos, cu_prot(int, int x), (int context;))
{
    cu_clos_self(_g_clos);
    self->context += x;
    return self->context;
}

int
_iter_func_noctx(int (*fn)(int), int *context)
{
    int i;
    int r = 0;
    for (i = 0; i < N_ITERATE; ++i)
	r = fn(r);
    return r;
}

int
#if PLAIN_CONTEXT_FIRST
_iter_func_ctx(int (*fn)(int *context, int), int *context)
#else
_iter_func_ctx(int (*fn)(int, int *context), int *context)
#endif
{
    int i;
    int r = 0;
    for (i = 0; i < N_ITERATE; ++i)
#if PLAIN_CONTEXT_FIRST
	r = fn(context, r);
#else
	r = fn(r, context);
#endif
    return r;
}

int
_iter_clos(cu_clop(clptr, int, int))
{
    int i;
    int r = 0;
    for (i = 0; i < N_ITERATE; ++i)
	r = cu_call(clptr, r);
    return r;
}


static int
_bench()
{
    static const double t_scale =
	1.0/((double)CLOCKS_PER_SEC*N_REPEAT_OUTER*N_REPEAT_INNER*N_ITERATE);
    int i, j;
    int *g_context = GC_malloc(sizeof(int));
    clock_t t_c_func = 0, t_s_func = 0;
    clock_t t_c_clos = 0, t_s_clos = 0;
    _g_clos_t _g_clos;
    cu_clop(g_clop, int, int) = _g_clos_prep(&_g_clos);
    *g_context = 0;

    for (j = 0; j < N_REPEAT_OUTER; ++j) {
	t_s_func -= clock();
	for (i = 0; i < N_REPEAT_INNER; ++i)
	    _iter_func_noctx(_f_func, NULL);
	t_s_func += clock();

	t_c_func -= clock();
	for (i = 0; i < N_REPEAT_INNER; ++i)
	    _iter_func_ctx(_g_func, g_context);
	t_c_func += clock();

	t_s_clos -= clock();
	for (i = 0; i < N_REPEAT_INNER; ++i)
	    _iter_clos(_f_clop);
	t_s_clos += clock();

	t_c_clos -= clock();
	for (i = 0; i < N_REPEAT_INNER; ++i)
	    _iter_clos(g_clop);
	t_c_clos += clock();
    }

    printf("\n         %10s     %10s\n"
	   "     plain %10.3lg s %10.3lg s\n"
	   "      clos %10.3lg s %10.3lg s\n"
	   "clos/plain %10.3lf   %10.3lf\n\n",
	   "simple",
	   "with cxt",
	   t_s_func*t_scale,
	   t_c_func*t_scale,
	   t_s_clos *t_scale,
	   t_c_clos *t_scale,
	   t_s_clos/(double)t_s_func,
	   t_c_clos/(double)t_c_func);
    return 0;
}

int
main()
{
    cu_init();
    _bench();
    return 0;
}
