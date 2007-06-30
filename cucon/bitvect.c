/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2004--2007  Petter Urkedal <urkedal@nbi.dk>
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

#define CUCON_BITVECT_C
#include <cucon/bitvect.h>
#include <cu/memory.h>
#include <cu/int.h>
#include <string.h>

#define BITVECT_ARRAY_SIZE(size)			\
	((size + sizeof(cuconP_bitvect_word_t) - 1)	\
	 /cuconP_bitvect_word_width)
#define WORD_C(c) ((cuconP_bitvect_word_t)(c))

void
cucon_bitvect_cct_uninit(cucon_bitvect_t bv, size_t size)
{
    bv->size = size;
    bv->arr = cu_galloc(cu_ulong_ceil_div(size, 8));
}

cucon_bitvect_t
cucon_bitvect_new_uninit(size_t size)
{
    cucon_bitvect_t bv = cu_gnew(struct cucon_bitvect_s);
    cucon_bitvect_cct_uninit(bv, size);
    return bv;
}

void
cucon_bitvect_cct_fill(cucon_bitvect_t bv, size_t size, cu_bool_t val)
{
    size_t bsize = cu_ulong_ceil_div(size, 8);
    bv->size = size;
    bv->arr = cu_galloc(bsize);
    memset(bv->arr, (val? 0xff : 0), bsize);
}

cucon_bitvect_t
cucon_bitvect_new_fill(size_t size, cu_bool_t val)
{
    cucon_bitvect_t bv = cu_gnew(struct cucon_bitvect_s);
    cucon_bitvect_cct_fill(bv, size, val);
    return bv;
}

void
cucon_bitvect_cct_copy(cucon_bitvect_t bv, cucon_bitvect_t bv_src)
{
    size_t size = bv_src->size;
    size_t bsize = cu_ulong_ceil_div(size, 8);
    bv->size = size;
    bv->arr = cu_galloc(bsize);
    memcpy(bv->arr, bv_src->arr, bsize);
}

cucon_bitvect_t
cucon_bitvect_new_copy(cucon_bitvect_t bv_src)
{
    cucon_bitvect_t bv = cu_gnew(struct cucon_bitvect_s);
    cucon_bitvect_cct_copy(bv, bv_src);
    return bv;
}

void
cucon_bitvect_fill(cucon_bitvect_t bv, size_t i, size_t j, cu_bool_t val)
{
    size_t iword = i/cuconP_bitvect_word_width;
    size_t ibit = i%cuconP_bitvect_word_width;
    size_t jword = j/cuconP_bitvect_word_width;
    size_t jbit = j%cuconP_bitvect_word_width;
    if (val) {
	if (iword == jword) {
	    if (ibit == jbit)
		return; /* in case array is empty */
	    bv->arr[iword] |= (WORD_C(1) << jbit) - (WORD_C(1) << ibit);
	}
	else {
	    bv->arr[iword] |= ~((WORD_C(1) << ibit) - WORD_C(1));
	    while (++iword < jword)
		bv->arr[iword] = ~WORD_C(0);
	    if (jbit == 0)
		return; /* in case it's equal to the size */
	    bv->arr[iword] |= (WORD_C(1) << jbit) - WORD_C(1);
	}
    }
    else {
	if (iword == jword) {
	    if (ibit == jbit)
		return;
	    bv->arr[iword] &= ~((WORD_C(1) << jbit) - (WORD_C(1) << ibit));
	}
	else {
	    bv->arr[iword] &= (WORD_C(1) << ibit) - WORD_C(1);
	    while (++iword < jword)
		bv->arr[iword] = 0;
	    if (jbit == 0)
		return;
	    bv->arr[iword] &= ~((WORD_C(1) << jbit) - WORD_C(1));
	}
    }
}

size_t
cucon_bitvect_find(cucon_bitvect_t bv, size_t start, cu_bool_t val)
{
    size_t size = bv->size;
    size_t i, l, r;
    size_t n = cu_ulong_ceil_div(bv->size, cuconP_bitvect_word_width);
    cuconP_bitvect_word_t *arr = bv->arr;
    i = start / cuconP_bitvect_word_width;
    l = start % cuconP_bitvect_word_width;
    if (l) {
	if (arr[i]) {
	    r = i*cuconP_bitvect_word_width
		+ (cu_ulong_log2_lowbit(arr[i] >> l) + l);
	    return r >= size? (size_t)-1 : r;
	}
	++i;
    }
    if (val) {
	for (; i < n; ++i)
	    if (arr[i] != 0) {
		r = i*cuconP_bitvect_word_width
		    + cu_ulong_log2_lowbit(arr[i]);
		return r >= size? (size_t)-1 : r;
	    }
    } else {
	for (; i < n; ++i)
	    if (~arr[i] != 0) {
		r = i*cuconP_bitvect_word_width
		    + cu_ulong_log2_lowbit(~arr[i]);
		return r >= size? (size_t)-1 : r;
	    }
    }
    return (size_t)-1;
}

size_t
cucon_bitvect_find2(cucon_bitvect_t bv0, cucon_bitvect_t bv1, size_t start,
		    cu_bool_t val0, cu_bool_t val1)
{
    size_t size = cu_ulong_min(bv0->size, bv1->size);
    size_t i, l, r;
    size_t n = cu_ulong_ceil_div(size, cuconP_bitvect_word_width);
    cuconP_bitvect_word_t *arr0 = bv0->arr;
    cuconP_bitvect_word_t *arr1 = bv1->arr;
    cuconP_bitvect_word_t x01;
    i = start / cuconP_bitvect_word_width;
    l = start % cuconP_bitvect_word_width;
    if (l) {
	x01 = (val0? arr0[i] : ~arr0[i]) & (val1? arr1[i] : ~arr1[i]);
	if (x01) {
	    r = i*cuconP_bitvect_word_width
		+ (cu_ulong_log2_lowbit(x01 >> l) + l);
	    return r >= size? (size_t)-1 : r;
	}
	++i;
    }
    for (; i < n; ++i) {
	x01 = (val0? arr0[i] : ~arr0[i]) & (val1? arr1[i] : ~arr1[i]);
	if (x01) {
	    r = i*cuconP_bitvect_word_width
		+ cu_ulong_log2_lowbit(x01);
	    return r >= size? (size_t)-1 : r;
	}
    }
    return (size_t)-1;
}
