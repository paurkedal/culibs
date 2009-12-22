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

#include <cucon/bitarray.h>
#include <cu/test.h>
#include <cu/int.h>
#include <cu/size.h>

#define SIZE 300

static void
_randomise(cucon_bitarray_t bv)
{
    size_t size = cucon_bitarray_size(bv);
    size_t i;
    for (i = 0; i < size; ++i)
	cucon_bitarray_set_at(bv, i, !!(lrand48() % 2));
}

static cucon_bitarray_t
_new_random(size_t size)
{
    cucon_bitarray_t ba = cucon_bitarray_new(size);
    _randomise(ba);
    return ba;
}

static void
_test_cmp(size_t size)
{
    int i, cmp;
    size_t size1, min_size;
    cucon_bitarray_t ba0 = _new_random(size);
    cucon_bitarray_t ba1 = cucon_bitarray_new_copy(ba0);
    cu_test_assert(cucon_bitarray_eq(ba0, ba1));
    cu_test_assert(cucon_bitarray_cmp(ba0, ba1) == 0);
    for (i = 0; i < size; ++i) {
	cu_bool_t v = cucon_bitarray_at(ba0, i);
	cucon_bitarray_set_at(ba1, i, !v);
	cu_test_assert(cucon_bitarray_cmp(ba0, ba1) == (v? 1 : -1));
	cucon_bitarray_set_at(ba1, i, v);
    }

    size1 = size/2 + lrand48() % size;
    ba1 = _new_random(size1);
    min_size = cu_size_min(size, size1);
    cmp = cucon_bitarray_cmp(ba0, ba1);
    for (i = 0; i < min_size; ++i) {
	cu_bool_t v0 = cucon_bitarray_at(ba0, i);
	cu_bool_t v1 = cucon_bitarray_at(ba1, i);
	if (v0 < v1) {
	    cu_test_assert(cmp == -1);
	    break;
	}
	if (v0 > v1) {
	    cu_test_assert(cmp == 1);
	    break;
	}
    }
    if (i == min_size) {
	if (size < size1)
	    cu_test_assert(cmp == -1);
	else if (size > size1)
	    cu_test_assert(cmp == 1);
	else
	    cu_test_assert(cmp == 0);
    }
}

static void
_test_get_set_find(size_t size)
{
    cucon_bitarray_t bv = cucon_bitarray_new_fill(size, cu_false);
    size_t i;
    for (i = 0; i < size - 1; ++i) {
	cu_test_assert(cucon_bitarray_at(bv, i) == cu_false);
	cucon_bitarray_set_at(bv, i, cu_true);
	cu_test_assert(cucon_bitarray_at(bv, i) == cu_true);
	cu_test_assert(cucon_bitarray_at(bv, i+1) == cu_false);
    }
    cucon_bitarray_set_at(bv, i, cu_true);

    for (i = 0; i < size; ++i) {
	_randomise(bv);
	cucon_bitarray_fill(bv, 0, i, cu_false);
	cucon_bitarray_set_at(bv, i, cu_true);
	cu_test_assert(cucon_bitarray_find(bv, 0, cu_true) == i);
    }
    cucon_bitarray_fill(bv, 0, size, cu_true);
    for (i = 0; i < size; ++i) {
	size_t j = cucon_bitarray_find(bv, 0, cu_true);
	cu_test_assert(j == i);
	cucon_bitarray_set_at(bv, j, cu_false);
    }
}

static void
_test_resize(size_t size)
{
    size_t i;
    struct cucon_bitarray ba;
    cucon_bitarray_init(&ba, 0);

    for (i = 0; i < size; ++i) {
	switch (lrand48() % 4) {
	    case 0: cucon_bitarray_resize_gp(&ba, i + 1); break;
	    case 1: cucon_bitarray_resize_gpmax(&ba, i + 1); break;
	    case 2: cucon_bitarray_resize_exact(&ba, i + 1); break;
	    case 3: cucon_bitarray_resize_exactmax(&ba, i + 1); break;
	}
	cucon_bitarray_set_at(&ba, i, cu_uint_bit_count(i) % 2);
    }
    for (i = 0; i < size; ++i)
	cu_test_assert(cucon_bitarray_at(&ba, i) == cu_uint_bit_count(i)%2);

    while (size) {
	switch (lrand48() % 4) {
	    case 0: cucon_bitarray_resize_gp(&ba, size); break;
	    case 1: cucon_bitarray_resize_gpmax(&ba, size); break;
	    case 2: cucon_bitarray_resize_exact(&ba, size); break;
	    case 3: cucon_bitarray_resize_exactmax(&ba, size); break;
	}
	for (i = 0; i < size; ++i)
	    cu_test_assert(cucon_bitarray_at(&ba, i) == cu_uint_bit_count(i)%2);
	size = (int)(size*drand48());
    }
}

void
_test(size_t size)
{
    _test_get_set_find(size);
    _test_resize(size);
    _test_cmp(size);
}

int
main()
{
    int i;
    cucon_init();
    for (i = 1; i < 400; ++i)
	_test(i);
    return 2*!!cu_test_bug_count();
}
