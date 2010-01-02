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

#include <cu/wordarr.h>
#include <cu/word.h>
#include <cu/size.h>
#include <cu/test.h>
#include <cu/conf.h>

#define ARRAY_SIZE 32

static cu_word_t
_wrand48()
{
#if CU_WORD_WIDTH <= 32
    return mrand48();
#else
    int j;
    cu_word_t w = 0;
    for (j = 0; j < sizeof(cu_word_t)/4; ++j) {
	w <<= 32;
	w += mrand48();
    }
    return w;
#endif
}

static void
_test_copy(void)
{
    int n;
    size_t i, word_cnt;
    cu_word_t src0_arr[ARRAY_SIZE];
    cu_word_t src1_arr[ARRAY_SIZE];
    cu_word_t dst_arr[ARRAY_SIZE];

    for (i = 0; i < ARRAY_SIZE; ++i) {
	src0_arr[i] = _wrand48();
	src1_arr[i] = _wrand48();
    }
    word_cnt = lrand48() % (ARRAY_SIZE + 1);

    for (n = 0; n < 4; ++n) {
	cu_bool1f_t f = (cu_bool1f_t)n;
	cu_wordarr_copy_bitimg(f, word_cnt, dst_arr, src0_arr);
	for (i = 0; i < word_cnt; ++i)
	    cu_test_assert(dst_arr[i] == cu_word_bitimg(f, src0_arr[i]));
    }
    for (n = 0; n < 16; ++n) {
	cu_bool2f_t f = (cu_bool2f_t)n;
	cu_wordarr_copy_bitimg2(f, word_cnt, dst_arr, src0_arr, src1_arr);
	for (i = 0; i < word_cnt; ++i)
	    cu_test_assert(dst_arr[i]
			   == cu_word_bitimg2(f, src0_arr[i], src1_arr[i]));
    }
}

static void
_test_skewcopy(void)
{
    int n;
    size_t i, j, l, max_width, word_cnt;
    cu_word_t src0_arr[ARRAY_SIZE];
    cu_word_t src1_arr[ARRAY_SIZE];
    cu_word_t dst_arr[ARRAY_SIZE];

    for (i = 0; i < ARRAY_SIZE; ++i) {
	src0_arr[i] = _wrand48();
	src1_arr[i] = _wrand48();
	dst_arr[i] = i;
    }
    i = lrand48() % (ARRAY_SIZE * CU_WORD_WIDTH);
    j = lrand48() % (ARRAY_SIZE * CU_WORD_WIDTH);
    max_width = ARRAY_SIZE - cu_size_ceil_div(cu_size_max(i, j), CU_WORD_WIDTH);
    word_cnt = lrand48() % (max_width + 1);

    for (n = 0; n < 4; ++n) {
	cu_bool1f_t f = (cu_bool1f_t)n;
	cu_wordarr_skewcopy_bitimg(f, word_cnt, dst_arr,
		i % CU_WORD_WIDTH, src0_arr + (i / CU_WORD_WIDTH));
	for (l = 0; l < word_cnt * CU_WORD_WIDTH; ++l) {
	    cu_word_t a, r;
	    size_t ip = i + l;
	    a = (src0_arr[ip / CU_WORD_WIDTH] >> (ip % CU_WORD_WIDTH)) & 1;
	    r = (dst_arr[l / CU_WORD_WIDTH] >> (l % CU_WORD_WIDTH)) & 1;
	    cu_test_assert(r == cu_bool1f_apply(f, a));
	}
    }
    for (n = 0; n < 16; ++n) {
	cu_bool2f_t f = (cu_bool2f_t)n;
	cu_wordarr_skewcopy_bitimg2(f, word_cnt, dst_arr,
		i % CU_WORD_WIDTH, src0_arr + (i / CU_WORD_WIDTH),
		j % CU_WORD_WIDTH, src1_arr + (j / CU_WORD_WIDTH));
	for (l = 0; l < word_cnt * CU_WORD_WIDTH; ++l) {
	    cu_word_t a0, a1, r;
	    size_t ip = i + l;
	    size_t jp = j + l;
	    a0 = (src0_arr[ip / CU_WORD_WIDTH] >> (ip % CU_WORD_WIDTH)) & 1;
	    a1 = (src1_arr[jp / CU_WORD_WIDTH] >> (jp % CU_WORD_WIDTH)) & 1;
	    r = (dst_arr[l / CU_WORD_WIDTH] >> (l % CU_WORD_WIDTH)) & 1;
	    cu_test_assert(r == cu_bool2f_apply(f, a0, a1));
	}
    }
}

int
main()
{
    int i;
    cu_init();

    for (i = 0; i < 4096; ++i)
	_test_copy();
    for (i = 0; i < 4096; ++i)
	_test_skewcopy();

    return 2*!!cu_test_bug_count();
}
