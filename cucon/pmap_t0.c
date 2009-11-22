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

#include <cucon/pmap.h>
#include <cu/memory.h>
#include <cu/debug.h>
#include <cu/test.h>

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
pmap_count(cucon_pmap_t pmap)
{
    count_t cl;
    cl.cnt = 0;
    cucon_pmap_iter_mem(pmap, count_prep(&cl));
    return cl.cnt;
}

void
test_strong()
{
    int i;
    cucon_pmap_t pmap = cucon_pmap_new();
    void *somekey;
    cucon_pmap_insert_ptr(pmap, somekey = cu_galloc(1), NULL);
    for (i = 0; i < 10; ++i)
	cucon_pmap_insert_ptr(pmap, cu_galloc(1), NULL);
    GC_gcollect();
    assert(cucon_pmap_size(pmap) == 11);
    cucon_pmap_erase(pmap, somekey);
    assert(cucon_pmap_size(pmap) == 10);
}

cu_clos_def(test_isecn_union_cb,
	    cu_prot(void, uintptr_t key),
	    ( uintptr_t sum;
	      cu_bool_t is_union; ))
{
    cu_clos_self(test_isecn_union_cb);
    self->sum += key;
    if (!self->is_union)
	cu_test_assert((key & 3) == 3);
}

void
test_isecn_union()
{
    size_t N = 10000;
    size_t n;
    struct cucon_umap S, T;
    size_t S_cnt = 0;
    size_t T_cnt = 0;
    uintptr_t sum = 0;
    test_isecn_union_cb_t cb;
    cu_clop(cb_clop, void, uintptr_t) = test_isecn_union_cb_prep(&cb);
    cucon_umap_init(&S);
    cucon_umap_init(&T);
    for (n = 0; n < N; ++n) {
	uintptr_t key = lrand48() % N;
	if (key & 1) {
	    if (cucon_umap_insert_void(&S, key)) {
		sum += key;
		++S_cnt;
	    }
	}
	if (key & 2) {
	    if (cucon_umap_insert_void(&T, key)) {
		sum += key;
		++T_cnt;
	    }
	}
    }
    cucon_umap_assign_isecn_union(&S, &T);
    cu_test_assert(cucon_umap_size(&S) + cucon_umap_size(&T) == S_cnt + T_cnt);
    cb.sum = 0;
    cb.is_union = cu_false;
    cucon_umap_iter_keys(&S, cb_clop);
    cb.is_union = cu_true;
    cucon_umap_iter_keys(&T, cb_clop);
    cu_test_assert(cb.sum == sum);
}

int
main()
{
    int i;
    cu_init();
    test_strong();
    for (i = 0; i < 100; ++i)
	test_isecn_union();
    GC_gcollect();
    return 2*!!cu_test_bug_count();
}
