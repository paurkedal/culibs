/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2004--2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cucon/po.h>
#include <time.h>

typedef unsigned long eltval_t;
#define eltval(elt) (*(eltval_t *)cucon_poelt_get_mem(elt))

static cu_bool_t
elt_prec(cucon_poelt_t e0, cucon_poelt_t e1)
{
    eltval_t i0 = eltval(e0);
    eltval_t i1 = eltval(e1);
    return (i0 | i1) == i1;
}

static clock_t
insert_some(cucon_po_t po, eltval_t maxval, size_t cnt, cucon_poelt_t *arr)
{
    size_t n, m;
    clock_t t = -clock();
    for (n = 0; n < cnt; ++n) {
	cucon_poelt_t elt = cucon_po_insert_mem(po, sizeof(eltval_t));
	eltval_t val = lrand48() % maxval;
	*(eltval_t *)cucon_poelt_get_mem(elt) = val;
	arr[n] = elt;
	for (m = 0; m < n; ++m)
	    if (elt_prec(arr[m], arr[n]))
		cucon_po_constrain_prec(po, arr[m], arr[n]);
	    else if (elt_prec(arr[n], arr[m]))
		cucon_po_constrain_prec(po, arr[n], arr[m]);
    }
    t += clock();
    return t;
}

static clock_t
check_some(size_t cnt, cucon_poelt_t *arr)
{
    size_t n;
    clock_t t = -clock();
    for (n = 0; n < cnt; ++n) {
	size_t m = lrand48() % cnt;
	cucon_po_prec(arr[n], arr[m]);
    }
    t += clock();
    return t;
}

static void
report(char const *what, size_t cnt, clock_t t)
{
    printf("%30s: %10lg\n", what, cnt*(double)CLOCKS_PER_SEC/t);
}

#define N 1000
int
main()
{
    cucon_poelt_t arr[N];
    cucon_po_t po;
    clock_t t0, t1;
    cu_init();
    po = cucon_po_new();
    t0 = insert_some(po, N/2, N, arr);
    t1 = check_some(N, arr);
    report("insert", N, t0);
    report("check", N, t1);
    printf("connection count: %zd\n", cucon_po_debug_count_connections(po));
    return 0;
}
