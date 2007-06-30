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

#include <cucon/bitvect.h>
#include <cu/test.h>

#define SIZE 300

void
randomise(cucon_bitvect_t bv)
{
    size_t size = cucon_bitvect_size(bv);
    size_t i;
    for (i = 0; i < size; ++i)
	cucon_bitvect_set_at(bv, i, !!(lrand48() % 2));
}

void
test(size_t size)
{
    cucon_bitvect_t bv = cucon_bitvect_new_fill(size, cu_false);
    size_t i;
    for (i = 0; i < size - 1; ++i) {
	cu_test_assert(cucon_bitvect_at(bv, i) == cu_false);
	cucon_bitvect_set_at(bv, i, cu_true);
	cu_test_assert(cucon_bitvect_at(bv, i) == cu_true);
	cu_test_assert(cucon_bitvect_at(bv, i+1) == cu_false);
    }
    cucon_bitvect_set_at(bv, i, cu_true);

    for (i = 0; i < size; ++i) {
	randomise(bv);
	cucon_bitvect_fill(bv, 0, i, cu_false);
	cucon_bitvect_set_at(bv, i, cu_true);
	cu_test_assert(cucon_bitvect_find(bv, 0, cu_true) == i);
    }
    cucon_bitvect_fill(bv, 0, size, cu_true);
    for (i = 0; i < size; ++i) {
	size_t j = cucon_bitvect_find(bv, 0, cu_true);
	cu_test_assert(j == i);
	cucon_bitvect_set_at(bv, j, cu_false);
    }
}

int
main()
{
    cucon_init();
    test(300);
    test(16);
    test(31);
    test(32);
    test(33);
    test(63);
    test(64);
    test(65);
    test(1);
    return 2*!!cu_test_bug_count();
}
