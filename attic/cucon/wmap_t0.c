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

#include <cucon/wmap.h>
#include <cu/memory.h>
#include <assert.h>
#include <cu/clos.h>
#include <stdio.h>


cu_clos_def(count,
	    cu_prot(void, void const *key, void *data),
	    (int cnt;))
{
    cu_clos_self(count);
    ++self->cnt;
}

int
wmap_count(cucon_wmap_t wmap)
{
    count_t cl;
    cl.cnt = 0;
    cucon_wmap_for(wmap, count_prep(&cl));
    return cl.cnt;
}

void
test_weak()
{
    int const N = 10000;
    int i, n;
    void *v[N];
    cucon_wmap_t wmap = cucon_wmap_new();
    for (i = 0; i < N; ++i) {
	int* data;
	int new_p;
	v[i] = cu_galloc(sizeof(int));
	new_p = cucon_wmap_insert_mem(wmap, v[i], sizeof(int), &data);
	assert(new_p);
	*data = i;
    }

    n = wmap_count(wmap);
    printf("wmap has %d elements (should be %d) before gc.\n", n, N);
    assert(n == N);
    cucon_wmap_show_stats(wmap);

    cucon_wmap_erase(wmap, v[0]);
    assert(wmap_count(wmap) == N - 1);
    for (i = 10; i < N; i += 2)
	v[i] = 0;
    GC_gcollect();
    for (i = 11; i < N; i += 2)
	v[i] = 0;
    GC_gcollect();

    n = wmap_count(wmap);
    printf("wmap has %d elements (should be 9 er slightly more) "
	   "after gc.\n", n);

    /* n == N is _possible_ in a correct program, since the GC is
     * conservative.  Ignore this needle in the haystack, and let's
     * signal n == N as a bug. */
    assert(n >= 9 && n < N - 1);

    cucon_wmap_show_stats(wmap);
    wmap = NULL;
}

void
test_weak_wmap_collect_before_keys()
{
    void *p0 = cu_galloc(1);
    void *p1 = cu_galloc(1);
    cucon_wmap_t wmap = cucon_wmap_new();
    printf("test_weak_wmap_collect_before_keys: "
	   "wmap = %p, p0 = %p, p1 = %p\n", wmap, p0, p1);
    cucon_wmap_insert_void(wmap, p0);
    cucon_wmap_insert_void(wmap, p1);
    wmap = NULL;
    GC_gcollect();
#if 0
    wmap = cucon_wmap_new();
    cucon_wmap_insert_void(wmap, p0);
    cucon_wmap_insert_void(wmap, p1);
    cucon_wmap_erase(wmap, p0);
    cucon_wmap_erase(wmap, p1);
    wmap = NULL;
    GC_gcollect();
#endif
}

int
main()
{
    cu_init();
    test_weak();
    test_weak_wmap_collect_before_keys();
    GC_gcollect();
    return 0;
}
