/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2009  Petter Urkedal <urkedal@nbi.dk>
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

#include <cucon/bitarray_slice.h>
#include <cucon/bitarray.h>
#include <cu/util.h>
#include <cu/size.h>
#include <cu/test.h>

#define MAX_SIZE 1024
#define REPEAT 8000

static cucon_bitarray_t
_new_random(size_t size)
{
    size_t i;
    cucon_bitarray_t ba = cucon_bitarray_new(size);
    for (i = 0; i < size; ++i)
	cucon_bitarray_set_at(ba, i, lrand48() % 2);
    return ba;
}

void
_test_update()
{
    size_t i, j, iS, iE;
    size_t size = lrand48() % MAX_SIZE;
    cucon_bitarray_t ba_orig = _new_random(size);
    cucon_bitarray_t ba = cucon_bitarray_new_copy(ba_orig);
    struct cucon_bitarray_slice sl;
    iS = lrand48() % (size + 1);
    iE = lrand48() % (size + 1);
    if (iS > iE)
	CU_SWAP(size_t, iS, iE);
    cucon_bitarray_slice_init(&sl, ba, iS, iE);
    cu_test_assert(cucon_bitarray_slice_size(&sl) == iE - iS);

    /* Test cucon_bitarray_slice_at and cucon_bitarray_slice_set_at. */
    for (i = 0; i < iE - iS; ++i) {
	cu_bool_t v = cucon_bitarray_slice_at(&sl, i);
	cu_test_assert(v == cucon_bitarray_at(ba, iS + i));
	cucon_bitarray_slice_set_at(&sl, i, !v);
	cu_test_assert((!v) == cucon_bitarray_at(ba, iS + i));
    }
    if (iS != iE)
	cu_test_assert(!cucon_bitarray_eq(ba, ba_orig));

    /* Test of cucon_bitarray_slice_update_bool1f, which also covers
     * cucon_bitarray_slice_update_not and cucon_bitarray_slice_fill. */
    cucon_bitarray_slice_update_bool1f(CU_BOOL1F_NOT, &sl);
    cu_test_assert(cucon_bitarray_eq(ba, ba_orig));
    cucon_bitarray_slice_update_bool1f(CU_BOOL1F_IDENT, &sl);
    cu_test_assert(cucon_bitarray_eq(ba, ba_orig));
    for (j = 0; j < 2; ++j) {
	cu_bool_t c = j;
	cucon_bitarray_slice_update_bool1f(cu_bool1f_konst(c), &sl);
	for (i = 0; i < size; ++i) {
	    if (iS <= i && i < iE)
		cu_test_assert(cucon_bitarray_at(ba, i) == c);
	    else
		cu_test_assert(cucon_bitarray_at(ba, i)
			       == cucon_bitarray_at(ba_orig, i));
	}
    }
}

static void
_test_copy_boolxf()
{
    cu_bool1f_t f1;
    cu_bool2f_t f2;
    size_t size = lrand48() % MAX_SIZE;
    size_t iS0, iE0, iS1, iE1, iSdst, iEdst, max_size;
    cucon_bitarray_t ba_src0 = _new_random(size);
    cucon_bitarray_t ba_src1 = _new_random(size);
    cucon_bitarray_t ba_dst = _new_random(size);
    cucon_bitarray_t ba_dst_orig = cucon_bitarray_new_copy(ba_dst);
    cucon_bitarray_t ba_dst_up;
    struct cucon_bitarray_slice src0, src1, dst;
    size_t i, i0, i1;

    iS0 = lrand48() % (size + 1);
    iE0 = lrand48() % (size + 1);
    if (iS0 > iE0) CU_SWAP(size_t, iS0, iE0);
    iS1 = lrand48() % (size + 1);
    iE1 = lrand48() % (size + 1);
    if (iS1 > iE1) CU_SWAP(size_t, iS1, iE1);
    iSdst = lrand48() % (size + 1);
    iEdst = lrand48() % (size + 1);
    if (iSdst > iEdst) CU_SWAP(size_t, iSdst, iEdst);
    max_size = cu_size_min(iE0 - iS0, iE1 - iS1);
    if (iEdst - iSdst > max_size)
	iEdst = iSdst + max_size;

    cucon_bitarray_slice_init(&src0, ba_src0, iS0, iE0);
    cucon_bitarray_slice_init(&src1, ba_src1, iS1, iE1);
    cucon_bitarray_slice_init(&dst, ba_dst, iSdst, iEdst);

    /* Test cucon_bitarray_slice_copy_bool1f. */
    f1 = (cu_bool2f_t)(lrand48() % 4);
    cucon_bitarray_slice_copy_bool1f(f1, &dst, &src0);

    for (i = iSdst, i0 = iS0; i < iEdst; ++i, ++i0) {
	cu_bool_t r = cucon_bitarray_at(ba_dst, i);
	cu_bool_t x0 = cucon_bitarray_at(ba_src0, i0);
	cu_test_assert(r == cu_bool1f_apply(f1, x0));
    }

    /* Test cucon_bitarray_slice_copy_bool2f. */
    f2 = (cu_bool2f_t)(lrand48() % 16);
    cucon_bitarray_slice_copy_bool2f(f2, &dst, &src0, &src1);

    for (i = iSdst, i0 = iS0, i1 = iS1; i < iEdst; ++i, ++i0, ++i1) {
	cu_bool_t r = cucon_bitarray_at(ba_dst, i);
	cu_bool_t x0 = cucon_bitarray_at(ba_src0, i0);
	cu_bool_t x1 = cucon_bitarray_at(ba_src1, i1);
	cu_test_assert(r == cu_bool2f_apply(f2, x0, x1));
    }

    /* Test cucon_bitarray_slice_update2f. */
    ba_dst_up = cucon_bitarray_new_copy(ba_dst_orig);
    cucon_bitarray_slice_init(&dst, ba_dst_up, iSdst, iEdst);
    cucon_bitarray_slice_copy(&dst, &src0);
    cucon_bitarray_slice_update_bool2f(f2, &dst, &src1);
    cu_test_assert(cucon_bitarray_eq(ba_dst, ba_dst_up));

    /* Check that bits outside the dst slice are untouched. */
    for (i = 0; i < iSdst; ++i)
	cu_test_assert(cucon_bitarray_at(ba_dst, i) ==
		       cucon_bitarray_at(ba_dst_orig, i));
    for (i = iEdst; i < size; ++i)
	cu_test_assert(cucon_bitarray_at(ba_dst, i) ==
		       cucon_bitarray_at(ba_dst_orig, i));

}

int
main()
{
    int i;
    cu_init();
    for (i = 0; i < REPEAT; ++i)
	_test_update();
    for (i = 0; i < REPEAT; ++i)
	_test_copy_boolxf();
    return 2*!!cu_test_bug_count();
}
