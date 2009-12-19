/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2004--2009  Petter Urkedal <urkedal@nbi.dk>
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
#include <cu/memory.h>
#include <cu/size.h>
#include <string.h>

#define WORD_BIT(n) (CU_WORD_C(1) << (n))

CU_SINLINE size_t
_capacity_for(size_t n)
{
    return sizeof(cu_word_t)*cu_size_ceil_div(n, 8*sizeof(cu_word_t));
}

void
cucon_bitarray_init(cucon_bitarray_t bv, size_t size)
{
    bv->size = size;
    bv->arr = cu_galloc(_capacity_for(size));
}

cucon_bitarray_t
cucon_bitarray_new(size_t size)
{
    cucon_bitarray_t bv = cu_gnew(struct cucon_bitarray);
    cucon_bitarray_init(bv, size);
    return bv;
}

void
cucon_bitarray_init_fill(cucon_bitarray_t bv, size_t size, cu_bool_t val)
{
    size_t bsize = _capacity_for(size);
    bv->size = size;
    bv->arr = cu_galloc(bsize);
    memset(bv->arr, (val? 0xff : 0), bsize);
}

cucon_bitarray_t
cucon_bitarray_new_fill(size_t size, cu_bool_t val)
{
    cucon_bitarray_t bv = cu_gnew(struct cucon_bitarray);
    cucon_bitarray_init_fill(bv, size, val);
    return bv;
}

void
cucon_bitarray_init_copy(cucon_bitarray_t bv, cucon_bitarray_t bv_src)
{
    size_t size = bv_src->size;
    size_t bsize = _capacity_for(size);
    bv->size = size;
    bv->arr = cu_galloc(bsize);
    memcpy(bv->arr, bv_src->arr, bsize);
}

cucon_bitarray_t
cucon_bitarray_new_copy(cucon_bitarray_t bv_src)
{
    cucon_bitarray_t bv = cu_gnew(struct cucon_bitarray);
    cucon_bitarray_init_copy(bv, bv_src);
    return bv;
}

void
cucon_bitarray_fill(cucon_bitarray_t bv, size_t i, size_t j, cu_bool_t val)
{
    size_t iword = i/CU_WORD_WIDTH;
    size_t ibit = i%CU_WORD_WIDTH;
    size_t jword = j/CU_WORD_WIDTH;
    size_t jbit = j%CU_WORD_WIDTH;
    if (val) {
	if (iword == jword) {
	    if (ibit == jbit)
		return; /* in case array is empty */
	    bv->arr[iword] |= WORD_BIT(jbit) - WORD_BIT(ibit);
	}
	else {
	    bv->arr[iword] |= ~(WORD_BIT(ibit) - CU_WORD_C(1));
	    while (++iword < jword)
		bv->arr[iword] = ~CU_WORD_C(0);
	    if (jbit == 0)
		return; /* in case it's equal to the size */
	    bv->arr[iword] |= WORD_BIT(jbit) - CU_WORD_C(1);
	}
    }
    else {
	if (iword == jword) {
	    if (ibit == jbit)
		return;
	    bv->arr[iword] &= ~(WORD_BIT(jbit) - WORD_BIT(ibit));
	}
	else {
	    bv->arr[iword] &= WORD_BIT(ibit) - CU_WORD_C(1);
	    while (++iword < jword)
		bv->arr[iword] = 0;
	    if (jbit == 0)
		return;
	    bv->arr[iword] &= ~(WORD_BIT(jbit) - CU_WORD_C(1));
	}
    }
}

size_t
cucon_bitarray_find(cucon_bitarray_t bv, size_t start, cu_bool_t val)
{
    size_t size = bv->size;
    size_t i, l, r;
    size_t n = cu_size_ceil_div(bv->size, CU_WORD_WIDTH);
    cu_word_t *arr = bv->arr;
    i = start / CU_WORD_WIDTH;
    l = start % CU_WORD_WIDTH;
    if (l) {
	if (arr[i]) {
	    r = i*CU_WORD_WIDTH + cu_size_log2_lowbit(arr[i] >> l) + l;
	    return r >= size? (size_t)-1 : r;
	}
	++i;
    }
    if (val) {
	for (; i < n; ++i)
	    if (arr[i] != 0) {
		r = i*CU_WORD_WIDTH + cu_size_log2_lowbit(arr[i]);
		return r >= size? (size_t)-1 : r;
	    }
    } else {
	for (; i < n; ++i)
	    if (~arr[i] != 0) {
		r = i*CU_WORD_WIDTH + cu_size_log2_lowbit(~arr[i]);
		return r >= size? (size_t)-1 : r;
	    }
    }
    return (size_t)-1;
}

size_t
cucon_bitarray_find2(cucon_bitarray_t bv0, cucon_bitarray_t bv1, size_t start,
		     cu_bool_t val0, cu_bool_t val1)
{
    size_t size = cu_size_min(bv0->size, bv1->size);
    size_t i, l, r;
    size_t n = cu_size_ceil_div(size, CU_WORD_WIDTH);
    cu_word_t *arr0 = bv0->arr;
    cu_word_t *arr1 = bv1->arr;
    cu_word_t x01;
    i = start / CU_WORD_WIDTH;
    l = start % CU_WORD_WIDTH;
    if (l) {
	x01 = (val0? arr0[i] : ~arr0[i]) & (val1? arr1[i] : ~arr1[i]);
	if (x01) {
	    r = i*CU_WORD_WIDTH + (cu_size_log2_lowbit(x01 >> l) + l);
	    return r >= size? (size_t)-1 : r;
	}
	++i;
    }
    for (; i < n; ++i) {
	x01 = (val0? arr0[i] : ~arr0[i]) & (val1? arr1[i] : ~arr1[i]);
	if (x01) {
	    r = i*CU_WORD_WIDTH + cu_size_log2_lowbit(x01);
	    return r >= size? (size_t)-1 : r;
	}
    }
    return (size_t)-1;
}
