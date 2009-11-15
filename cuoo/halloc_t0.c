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

#include <cuoo/halloc.h>
#include <cu/test.h>
#include <cu/ptr.h>
#include <stdlib.h>

#define N_ALLOC 40000
#define MAX_SIZEW 5

static cuoo_type_t _testobj_type_arr[MAX_SIZEW];

static void
_testobj_init(cu_word_t *tpl, int J, int i)
{
    int j;
    for (j = 0; j < J; ++j)
	tpl[j] = i;
}

/* Test that equal object data gives the same object pointer. */
void
test(int n_alloc)
{
    int i, J, k;
    cu_word_t **obj_arr = cu_snewarr(cu_word_t *, n_alloc);
    for (J = 1; J < MAX_SIZEW; ++J)
	_testobj_type_arr[J - 1]
	    = cuoo_type_new_opaque_hcs(NULL, J*sizeof(cu_word_t));
    for (k = 0; k < 100; ++k) {
	for (i = 0; i < n_alloc; ++i) {
	    cu_word_t tpl[MAX_SIZEW];
	    int J = i % (MAX_SIZEW - 1) + 1;
	    _testobj_init(tpl, J, i);
	    obj_arr[i] =
		cuoo_halloc(_testobj_type_arr[J - 1], J*sizeof(cu_word_t), tpl);
	}
	for (i = 0; i < n_alloc; ++i) {
	    cu_word_t *obj;
	    cu_word_t tpl[4];
	    int J = i % (MAX_SIZEW - 1) + 1;
	    _testobj_init(tpl, J, i);
	    obj = cuoo_halloc(_testobj_type_arr[J - 1], J*sizeof(cu_word_t), tpl);
	    cu_test_assert_ptr_eq(obj, obj_arr[i]);
	    cu_test_assert(memcmp(cu_ptr_add(obj, CUOO_HCOBJ_SHIFT),
				  cu_ptr_add(obj_arr[i], CUOO_HCOBJ_SHIFT),
				  J*sizeof(cu_word_t)) == 0);
	}
	memset(obj_arr, 0, sizeof(cu_word_t *)*n_alloc);
	GC_gcollect();
    }
}

int
main(int argc, char **argv)
{
    cu_init();
    test(argc >= 2? atoi(argv[1]) : N_ALLOC);
    return 2*!!cu_test_bug_count();
}
