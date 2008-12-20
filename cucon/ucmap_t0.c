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

#include <cucon/ucmap.h>
#include <cucon/ucset.h>
#include <cu/test.h>

#define REPEAT 50
#define N_INS 0x800
#define MAX_KEY N_INS

cu_clos_def(_iter_cb, cu_prot(void, uintptr_t key, int val),
    ( cucon_ucset_t S;
      size_t count; ))
{
    cu_clos_self(_iter_cb);
    cu_test_assert(cucon_ucset_find(self->S, key));
    cu_test_assert_int_eq((int)key, val);
    ++self->count;
}

void
test()
{
    int i, j;
    for (i = 0; i < REPEAT; ++i) {
	cucon_ucmap_t M = NULL;
	cucon_ucset_t S = NULL;
	uintptr_t key;
	_iter_cb_t iter_cb;
	size_t count = 0;

	for (j = 0; j < N_INS; ++j) {
	    key = lrand48() % MAX_KEY;
	    if (!cucon_ucset_find(S, key))
		++count;
	    M = cucon_ucmap_insert_int(M, key, key);
	    S = cucon_ucset_insert(S, key);
	}
	for (key = 0; key < MAX_KEY; ++key) {
	    uintptr_t val = cucon_ucmap_find_int(M, key);
	    if (val == cucon_ucmap_int_none) {
		cu_test_assert(!cucon_ucset_find(S, key));
		cu_test_assert(cucon_ucmap_erase(M, key) == M);
	    }
	    else {
		cucon_ucmap_t Mp;
		cu_test_assert(cucon_ucset_find(S, key));
		cu_test_assert(val == key);
		Mp = cucon_ucmap_erase(M, key);
		cu_test_assert(cucon_ucmap_find_int(Mp, key) ==
			       cucon_ucmap_int_none);
	    }
	}

	for (j = 0; j < 4; ++j) {
	    cucon_ucmap_t Mp;
	    key = lrand48() % MAX_KEY;
	    Mp = cucon_ucmap_erase(M, key);
	    if (Mp != M)
		cu_test_assert(cucon_ucmap_card(Mp) == count - 1);
	}

	cu_test_assert_size_eq(count, cucon_ucmap_card(M));

	iter_cb.S = S;
	iter_cb.count = 0;
	cucon_ucmap_iter_int(M, _iter_cb_prep(&iter_cb));
	cu_test_assert(iter_cb.count == count);
    }
}

int
main()
{
    cu_init();
    test();
    return 2*!!cu_test_bug_count();
}
