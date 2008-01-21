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

#include <cucon/rbmap.h>
#include <cu/str.h>
#include <cu/test.h>


static cu_bool_t
check_insert(cucon_rbmap_t rbmap, int i, cu_bool_t expect)
{
    int *slot;
    cu_str_t key = cu_str_new_fmt("%d", i);
    if (cucon_rbmap_insert_mem(rbmap, key, sizeof(int), &slot)) {
	*slot = i;
	cu_test_assert(expect);
	return cu_true;
    }
    else {
	cu_test_assert(!expect);
	return cu_false;
    }
}

static cu_bool_t
check_find(cucon_rbmap_t rbmap, int i, cu_bool_t expect)
{
    cu_str_t key = cu_str_new_fmt("%d", i);
    int *slot = cucon_rbmap_find_mem(rbmap, key);
    if (slot)
	cu_test_assert(*slot == i);
    return !!slot;
}

static cu_bool_t
check_erase(cucon_rbmap_t rbmap, int i, cu_bool_t expect)
{
     cu_str_t key = cu_str_new_fmt("%d", i);
     cu_bool_t st = cucon_rbmap_erase(rbmap, key);
     cu_test_assert(st == expect);
     return st;
}

#define N 1000

int
main()
{
    int i;
    cucon_rbmap_t map;
    cu_init();

    map = cucon_rbmap_new((cu_clop(, int, void *, void *))cu_str_cmp_clop);
    for (i = 0; i < N; ++i)
	check_insert(map, i, cu_true);
    for (i = 0; i < N; ++i)
	check_find(map, i, cu_true);
    for (i = N; i < 2*N; ++i)
	check_find(map, i, cu_false);
    for (i = 0; i < N/2; ++i)
	check_erase(map, i, cu_true);
    for (i = 0; i < N; ++i)
	check_find(map, i, i >= N/2);
    return 2*!!cu_test_bug_count();
}
