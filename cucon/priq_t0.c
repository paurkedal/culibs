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

#include <cucon/priq.h>
#include <cu/memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

cu_clop_def(int_less, cu_bool_t, void *i0, void *i1)
{
    return *(int*)i0 < *(int*)i1;
}

cu_clop_def(int_print, void, void *i0, FILE* out)
{
    fprintf(out, "%d", *(int*)i0);
}

int
main()
{
    int i;
    cucon_priq_t q;

    cu_init();

    q = cucon_priq_new(int_less);

    printf("Inserting:");
    for (i = 0; i < 40; ++i) {
	int *k = cu_gnew(int);
	*k = lrand48() % 50;
	printf(" %d", *k);
	cucon_priq_insert(q, k);
    }
    printf("\n");
    cucon_priq_dump(q, int_print, stdout);
    printf("\nPopping:");
    int l = 0;
    for (i = 0; i < 40; ++i) {
	int *k = cucon_priq_pop_front(q);
	assert(*k >= l);
	printf(" %d", *k);
	l = *k;
    }
    printf("\n");
    for (i = 0; i < 4000; ++i) {
	int j;
	int n = lrand48() % 200;
	for (j = 0; j < n; ++j) {
	    int *k = cu_gnew(int);
	    *k = lrand48() % 100;
	    cucon_priq_insert(q, k);
	}
	n = lrand48() % 200;
	int l = 0;
	for (j = 0; j < n; ++j) {
	    int *k = cucon_priq_pop_front(q);
	    if (k == NULL)
		break;
	    assert(*k >= l);
	    l = *k;
	}
    }

    return 0;
}
