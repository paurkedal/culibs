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
#include <cu/word.h>
#include <cu/wordarr.h>
#include <string.h>

#define WORD_BIT(n) (CU_WORD_C(1) << (n))
#define WORD_MASK(n) (WORD_BIT(n) - CU_WORD_C(1))

CU_SINLINE size_t
_capacity_for(size_t n)
{
    return sizeof(cu_word_t)*cu_size_ceil_div(n, 8*sizeof(cu_word_t));
}

void
cucon_bitarray_init(cucon_bitarray_t ba, size_t size)
{
    size_t cap = _capacity_for(size);
    ba->cap = cap;
    ba->size = size;
    ba->arr = cu_galloc(cap);
}

cucon_bitarray_t
cucon_bitarray_new(size_t size)
{
    cucon_bitarray_t ba = cu_gnew(struct cucon_bitarray);
    cucon_bitarray_init(ba, size);
    return ba;
}

void
cucon_bitarray_init_fill(cucon_bitarray_t ba, size_t size, cu_bool_t val)
{
    size_t cap = _capacity_for(size);
    ba->cap = cap;
    ba->size = size;
    ba->arr = cu_galloc(cap);
    memset(ba->arr, (val? 0xff : 0), cap);
}

cucon_bitarray_t
cucon_bitarray_new_fill(size_t size, cu_bool_t val)
{
    cucon_bitarray_t ba = cu_gnew(struct cucon_bitarray);
    cucon_bitarray_init_fill(ba, size, val);
    return ba;
}

void
cucon_bitarray_init_copy(cucon_bitarray_t ba, cucon_bitarray_t ba_src)
{
    size_t size = ba_src->size;
    size_t cap = _capacity_for(size);
    ba->cap = cap;
    ba->size = size;
    ba->arr = cu_galloc(cap);
    memcpy(ba->arr, ba_src->arr, cap);
}

cucon_bitarray_t
cucon_bitarray_new_copy(cucon_bitarray_t ba_src)
{
    cucon_bitarray_t ba = cu_gnew(struct cucon_bitarray);
    cucon_bitarray_init_copy(ba, ba_src);
    return ba;
}

void
cucon_bitarray_swap(cucon_bitarray_t ba0, cucon_bitarray_t ba1)
{
    CU_SWAP(size_t, ba0->cap, ba1->cap);
    CU_SWAP(size_t, ba0->size, ba1->size);
    CU_SWAP(cu_word_t *, ba0->arr, ba1->arr);
}

void
cucon_bitarray_fill(cucon_bitarray_t ba, size_t i, size_t j, cu_bool_t val)
{
    size_t iword = i/CU_WORD_WIDTH;
    size_t ibit = i%CU_WORD_WIDTH;
    size_t jword = j/CU_WORD_WIDTH;
    size_t jbit = j%CU_WORD_WIDTH;
    if (val) {
	if (iword == jword) {
	    if (ibit == jbit)
		return; /* in case array is empty */
	    ba->arr[iword] |= WORD_BIT(jbit) - WORD_BIT(ibit);
	}
	else {
	    ba->arr[iword] |= ~(WORD_BIT(ibit) - CU_WORD_C(1));
	    while (++iword < jword)
		ba->arr[iword] = ~CU_WORD_C(0);
	    if (jbit == 0)
		return; /* in case it's equal to the size */
	    ba->arr[iword] |= WORD_BIT(jbit) - CU_WORD_C(1);
	}
    }
    else {
	if (iword == jword) {
	    if (ibit == jbit)
		return;
	    ba->arr[iword] &= ~(WORD_BIT(jbit) - WORD_BIT(ibit));
	}
	else {
	    ba->arr[iword] &= WORD_BIT(ibit) - CU_WORD_C(1);
	    while (++iword < jword)
		ba->arr[iword] = 0;
	    if (jbit == 0)
		return;
	    ba->arr[iword] &= ~(WORD_BIT(jbit) - CU_WORD_C(1));
	}
    }
}

int
cucon_bitarray_cmp(cucon_bitarray_t ba0, cucon_bitarray_t ba1)
{
    int rest;
    size_t size0 = ba0->size;
    size_t size1 = ba1->size;
    size_t size = cu_size_min(size0, size1);
    size_t sizew = size / CU_WORD_WIDTH;
    cu_word_t *arr0 = ba0->arr;
    cu_word_t *arr1 = ba1->arr;
    while (sizew--) {
	int cmp = cu_word_rcmp(*arr0++, *arr1++);
	if (cmp)
	    return cmp;
    }
    rest = size % CU_WORD_WIDTH;
    if (rest) {
	int cmp = cu_word_rcmp(*arr0, *arr1);
	if (cmp)
	    return cmp;
    }
    if (size0 < size1)
	return -1;
    if (size0 > size1)
	return 1;
    return 0;
}

cu_bool_t
cucon_bitarray_eq(cucon_bitarray_t ba0, cucon_bitarray_t ba1)
{
    cu_word_t *arr0, *arr1;
    size_t size0 = ba0->size;
    size_t size1 = ba1->size;
    size_t sizew;
    int rest;
    if (size0 != size1)
	return cu_false;
    arr0 = ba0->arr;
    arr1 = ba1->arr;
    sizew = size0 / CU_WORD_WIDTH;
    while (sizew--)
	if (*arr0++ != *arr1++)
	    return cu_false;
    rest = size0 % CU_WORD_WIDTH;
    if (rest) {
	cu_word_t mask = WORD_MASK(rest);
	if ((*arr0 & mask) != (*arr1 & mask))
	    return cu_false;
    }
    return cu_true;
}

size_t
cucon_bitarray_find(cucon_bitarray_t ba, size_t start, cu_bool_t val)
{
    size_t size = ba->size;
    size_t i, l, r;
    size_t n = cu_size_ceil_div(ba->size, CU_WORD_WIDTH);
    cu_word_t *arr = ba->arr;
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
cucon_bitarray_find2(cucon_bitarray_t ba0, cucon_bitarray_t ba1, size_t start,
		     cu_bool_t val0, cu_bool_t val1)
{
    size_t size = cu_size_min(ba0->size, ba1->size);
    size_t i, l, r;
    size_t n = cu_size_ceil_div(size, CU_WORD_WIDTH);
    cu_word_t *arr0 = ba0->arr;
    cu_word_t *arr1 = ba1->arr;
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

static void
_bitarray_realloc(cucon_bitarray_t ba, size_t new_cap, size_t new_size)
{
    cu_word_t *old_arr = ba->arr;
    size_t copy_cap = _capacity_for(cu_size_min(new_size, ba->size));
    cu_debug_assert(new_cap >= copy_cap);
    ba->cap = new_cap;
    ba->arr = cu_galloc_atomic(new_cap);
    memcpy(ba->arr, old_arr, copy_cap);
    cu_gfree_atomic(old_arr);
}

void
cucon_bitarray_resize_gp(cucon_bitarray_t ba, size_t size)
{
    size_t min_cap = _capacity_for(size);
    size_t old_cap = ba->cap;
    if (min_cap > old_cap) {
	size_t new_cap = cu_size_max(min_cap, old_cap*2);
	_bitarray_realloc(ba, new_cap, size);
    }
    else if (min_cap*2 < old_cap)
	_bitarray_realloc(ba, min_cap, size);
    ba->size = size;
}

void
cucon_bitarray_resize_gpmax(cucon_bitarray_t ba, size_t size)
{
    size_t min_cap = _capacity_for(size);
    size_t old_cap = ba->cap;
    if (min_cap > old_cap) {
	size_t new_cap = cu_size_max(min_cap, old_cap*2);
	_bitarray_realloc(ba, new_cap, size);
    }
    ba->size = size;
}

void
cucon_bitarray_resize_exact(cucon_bitarray_t ba, size_t size)
{
    size_t new_cap = _capacity_for(size);
    if (new_cap != ba->cap)
	_bitarray_realloc(ba, new_cap, size);
    ba->size = size;
}

void
cucon_bitarray_resize_exactmax(cucon_bitarray_t ba, size_t size)
{
    size_t new_cap = _capacity_for(size);
    if (new_cap > ba->cap)
	_bitarray_realloc(ba, new_cap, size);
    ba->size = size;
}

void
cucon_bitarray_resize_fill_gp(cucon_bitarray_t ba, size_t new_size,
			      cu_bool_t fill_value)
{
    size_t old_size = ba->size;
    cucon_bitarray_resize_gp(ba, new_size);
    if (old_size < new_size)
	cucon_bitarray_fill(ba, old_size, new_size, fill_value);
}

void
cucon_bitarray_resize_fill_exact(cucon_bitarray_t ba, size_t new_size,
				 cu_bool_t fill_value)
{
    size_t old_size = ba->size;
    cucon_bitarray_resize_exact(ba, new_size);
    if (old_size < new_size)
	cucon_bitarray_fill(ba, old_size, new_size, fill_value);
}

void
cucon_bitarray_resize_set_at(cucon_bitarray_t ba, size_t i,
			     cu_bool_t fill_value, cu_bool_t set_value)
{
    size_t old_size = ba->size;
    cucon_bitarray_resize_gp(ba, i + 1);
    if (old_size <= i)
	cucon_bitarray_fill(ba, old_size, i + 1, fill_value);
    cucon_bitarray_set_at(ba, i, set_value);
}

void
cucon_bitarray_init_img_bool1f(cu_bool1f_t f, cucon_bitarray_t ba,
			       cucon_bitarray_t arg)
{
    cucon_bitarray_init(ba, cucon_bitarray_size(arg));
    cu_wordarr_copy_bitimg(f, cu_size_ceil_div(ba->size, CU_WORD_WIDTH),
			   ba->arr, arg->arr);
}

void
cucon_bitarray_init_img_bool2f(cu_bool2f_t f, cucon_bitarray_t ba,
			       cucon_bitarray_t arg0, cucon_bitarray_t arg1)
{
    size_t size = cu_size_min(cucon_bitarray_size(arg0),
			      cucon_bitarray_size(arg1));
    cucon_bitarray_init(ba, size);
    cu_wordarr_copy_bitimg2(f, cu_size_ceil_div(size, CU_WORD_WIDTH),
			    ba->arr, arg0->arr, arg1->arr);
}

cucon_bitarray_t
cucon_bitarray_img_bool1f(cu_bool1f_t f, cucon_bitarray_t arg)
{
    cucon_bitarray_t ba = cu_gnew(struct cucon_bitarray);
    cucon_bitarray_init_img_bool1f(f, ba, arg);
    return ba;
}

cucon_bitarray_t
cucon_bitarray_img_bool2f(cu_bool2f_t f,
			  cucon_bitarray_t arg0, cucon_bitarray_t arg1)
{
    cucon_bitarray_t ba = cu_gnew(struct cucon_bitarray);
    cucon_bitarray_init_img_bool2f(f, ba, arg0, arg1);
    return ba;
}

void
cucon_bitarray_update_img_bool1f(cu_bool1f_t f, cucon_bitarray_t ba)
{
    cu_wordarr_copy_bitimg(f, cu_size_ceil_div(ba->size, CU_WORD_WIDTH),
			   ba->arr, ba->arr);
}

void
cucon_bitarray_update_img_bool2f(cu_bool2f_t f, cucon_bitarray_t ba,
				 cucon_bitarray_t ba_src)
{
    cu_wordarr_copy_bitimg2(f, cu_size_ceil_div(ba->size, CU_WORD_WIDTH),
			    ba->arr, ba->arr, ba_src->arr);
}
