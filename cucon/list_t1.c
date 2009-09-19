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

#include <cucon/list.h>
#include <cu/test.h>
#include <stdio.h>
#include <string.h>

static void
_simple_test()
{
    cucon_list_t L0, L1;
    cucon_listnode_t n0, n1;

    L0 = cucon_list_new();
    cu_test_assert(cucon_list_is_empty(L0));
    cu_test_assert(cucon_list_is_empty_or_singleton(L0));
    cucon_list_append_ptr(L0, "beta");
    cu_test_assert(cucon_list_is_singleton(L0));
    cu_test_assert(cucon_list_is_empty_or_singleton(L0));
    cucon_list_prepend_ptr(L0, "alpha");
    n0 = cucon_list_append_mem(L0, sizeof(void*));
    *(void **)cucon_listnode_mem(n0) = "gamma";

    L1 = cucon_list_new();
    cucon_list_append_ptr(L1, "delta");
    cucon_list_append_live(L0, cucon_list_begin(L1));
    cu_test_assert(cucon_list_is_empty(L1));
    cu_test_assert(cucon_list_count(L0) == 4);

    L1 = cucon_list_new_copy_ptr(L0);

    for (n0 = cucon_list_begin(L0),    n1 = cucon_list_begin(L1);
	 n0 != cucon_list_end(L0) &&   n1 != cucon_list_end(L1);
	 n0 = cucon_listnode_next(n0), n1 = cucon_listnode_next(n1)) {
	char *p0 = cucon_listnode_ptr(n0);
	char *p1 = cucon_listnode_ptr(n1);
	printf("%s\n", p0);
	cu_test_assert_ptr_eq(p0, p1);
    }
    cu_test_assert(n0 == cucon_list_end(L0));
    cu_test_assert(n1 == cucon_list_end(L1));

    cucon_list_validate(L0);
    cucon_list_validate(L1);

    cu_test_assert(cucon_list_cmp_ptr(L0, L1) == 0);
    cucon_list_rotate(L0, cucon_list_begin(L0));
    cu_test_assert(cucon_list_cmp_ptr(L0, L1) == 0);
    cucon_list_rotate(L0, cucon_list_rbegin(L0));
    cu_test_assert(cucon_list_cmp_ptr(L0, L1) != 0);
    cu_test_assert(!strcmp(cucon_listnode_ptr(cucon_list_begin(L0)), "delta"));
    cucon_list_rotate_backwards(L0);
    cu_test_assert(cucon_list_cmp_ptr(L0, L1) == 0);

    cucon_list_validate(L0);
    cucon_list_validate(L1);
}

int
main()
{
    cucon_init();
    _simple_test();
    return 0;
}
