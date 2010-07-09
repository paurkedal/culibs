/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2010  Petter Urkedal <paurkedal@eideticdew.org>
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

#include <cu/memory.h>
#include <cu/test.h>
#include <string.h>

static void
_test(size_t n)
{
    void *p;

    p = cu_galloc(n);
    cu_test_assert_ptr_eq(cu_gc_base(p), p);
    memset(p, 0xff, n);
    cu_gfree(p);

    p = cu_galloc_atomic(n);
    cu_test_assert_ptr_eq(cu_gc_base(p), p);
    memset(p, 0xff, n);
    cu_gfree_atomic(p);

    p = cu_ualloc(n);
    cu_test_assert_ptr_eq(cu_gc_base(p), p);
    memset(p, 0xff, n);
    cu_ufree(p);

    p = cu_ualloc_atomic(n);
    /* Base pointer may not be available for uncollectable atomic memory. */
    memset(p, 0xff, n);
    cu_ufree_atomic(p);
}

int
main()
{
    cu_init();

#ifdef CU_DEBUG_MEMORY
    printf("Testing allocation with CU_DEBUG_MEMORY.\n");
#else
    printf("Testing allocation without CU_DEBUG_MEMORY.\n");
#endif

    _test(1);
    _test(2);
    _test(3);
    _test(4);
    _test(10);
    _test(100);
    _test(1000);
    _test(10000);
    return 2*!!cu_test_bug_count();
}
