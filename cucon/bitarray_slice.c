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
#include <cu/wordarr.h>
#include <cu/word.h>
#include <cu/debug.h>
#include <inttypes.h>

#define WORD_MASK(c) ((CU_WORD_C(1) << (c)) - CU_WORD_C(1))

/* Helper to read up to CU_WORD_WIDTH bits from and array at a given bit
 * offset, and update the pointer and offset.  This is used when computing the
 * partial words at the beginning and and of slices. */
static cu_word_t
_read_bits(int width, int *offset_io, cu_word_t **arr_io)
{
    int offset = *offset_io;
    cu_word_t w = **arr_io >> offset;
    if (offset + width > CU_WORD_WIDTH) {
	++*arr_io;
	w |= **arr_io << (CU_WORD_WIDTH - offset);
	*offset_io = offset + width - CU_WORD_WIDTH;
    }
    else if (offset + width == CU_WORD_WIDTH) {
	++*arr_io;
	*offset_io = 0;
    }
    else
	*offset_io += width;
    return w;
}

void
cucon_bitarray_slice_init(cucon_bitarray_slice_t sl,
			  cucon_bitarray_t ba, size_t i, size_t j)
{
    cu_check_arg(2, i, i <= j);
    cu_check_arg(3, j, j <= ba->size);
    sl->arr = ba->arr + i / CU_WORD_WIDTH;
    sl->offset = i % CU_WORD_WIDTH;
    sl->size = j - i;
}

void
cucon_bitarray_slice_init_copy(cucon_bitarray_slice_t sl,
			       cucon_bitarray_slice_t sl_src)
{
    sl->arr = sl_src->arr;
    sl->offset = sl_src->offset;
    sl->size = sl_src->size;
}

void
cucon_bitarray_slice_init_sub(cucon_bitarray_slice_t sl,
			      cucon_bitarray_slice_t sl_src,
			      size_t i, size_t j)
{
    size_t k = sl_src->offset + i;
    cu_check_arg(2, i, i <= j);
    cu_check_arg(3, j, j <= sl_src->size);
    sl->arr = sl_src->arr + k / CU_WORD_WIDTH;
    sl->offset = k % CU_WORD_WIDTH;
    sl->size = j - i;
}

void
cucon_bitarray_slice_fill(cucon_bitarray_slice_t dst, cu_bool_t val)
{
    int first_width, last_width;
    size_t size = dst->size;
    cu_word_t *dst_arr = dst->arr;
    int dst_offset = dst->offset;
    size_t sizew;

    /* Partial first word. */
    if (dst_offset) {
	cu_word_t mask = ~WORD_MASK(dst_offset);
	first_width = CU_WORD_WIDTH - dst_offset;
	if (first_width > size) {
	    mask &= WORD_MASK(dst_offset + size);
	    if (val)  *dst_arr |= mask;
	    else      *dst_arr &= ~mask;
	    return;
	}
	if (val)  *dst_arr |= mask;
	else      *dst_arr &= ~mask;
	size -= first_width;
	++dst_arr;
    }

    /* Full words. */
    sizew = size / CU_WORD_WIDTH;
    if (val) while (sizew--) *dst_arr++ = ~CU_WORD_C(0);
    else     while (sizew--) *dst_arr++ = CU_WORD_C(0);

    /* Partial last word. */
    last_width = size % CU_WORD_WIDTH;
    if (last_width) {
	cu_word_t mask = WORD_MASK(last_width);
	if (val)  *dst_arr |= mask;
	else      *dst_arr &= ~mask;
    }
}

void
cucon_bitarray_slice_update_not(cucon_bitarray_slice_t dst)
{
    int first_width, last_width;
    size_t size = dst->size;
    cu_word_t *dst_arr = dst->arr;
    int dst_offset = dst->offset;
    size_t sizew;

    /* Partial first word. */
    if (dst_offset) {
	cu_word_t mask = ~WORD_MASK(dst_offset);
	first_width = CU_WORD_WIDTH - dst_offset;
	if (first_width > size) {
	    mask &= WORD_MASK(dst_offset + size);
	    *dst_arr = (*dst_arr & ~mask) | (~*dst_arr & mask);
	    return;
	}
	*dst_arr = (*dst_arr & ~mask) | (~*dst_arr & mask);
	++dst_arr;
	size -= first_width;
    }

    /* Full words. */
    sizew = size / CU_WORD_WIDTH;
    while (sizew--) {
	*dst_arr = ~*dst_arr;
	++dst_arr;
    }

    /* Partial last word. */
    last_width = size % CU_WORD_WIDTH;
    if (last_width) {
	cu_word_t mask = WORD_MASK(last_width);
	*dst_arr = (*dst_arr & ~mask) | (~*dst_arr & mask);
    }
}

void
cucon_bitarray_slice_update_bool1f(cu_bool1f_t f, cucon_bitarray_slice_t dst)
{
    switch (f) {
	case CU_BOOL1F_IDENT:
	    break;
	case CU_BOOL1F_FALSE:
	    cucon_bitarray_slice_fill(dst, cu_false);
	    break;
	case CU_BOOL1F_TRUE:
	    cucon_bitarray_slice_fill(dst, cu_true);
	    break;
	case CU_BOOL1F_NOT:
	    cucon_bitarray_slice_update_not(dst);
	    break;
    }
}

void
cucon_bitarray_slice_update_bool2f(cu_bool2f_t f,
				   cucon_bitarray_slice_t dst,
				   cucon_bitarray_slice_t src1)
{
    /* This'll do for now. */
    cucon_bitarray_slice_copy_bool2f(f, dst, dst, src1);
}

void
cucon_bitarray_slice_copy_bool1f(cu_bool1f_t f, cucon_bitarray_slice_t dst,
				 cucon_bitarray_slice_t src0)
{
    cu_word_t x0, y;
    int first_width, last_width;
    size_t sizew, size = dst->size;
    cu_word_t *dst_arr = dst->arr;
    int dst_offset = dst->offset;
    cu_word_t *src0_arr = src0->arr;
    int src0_offset = src0->offset;

    /* Check domain. */
#ifdef CUCONF_DEBUG_CLIENT
    if (src0->size < size)
	cu_bugf_domain(2, "src0",
		       "Source slice is shorter than destination slice.");
#endif

    /* Handle partial first destination word. */
    if (dst_offset) {
	cu_word_t mask = ~WORD_MASK(dst_offset);
	first_width = CU_WORD_WIDTH - dst_offset;
	x0 = _read_bits(first_width, &src0_offset, &src0_arr);
	y = cu_word_bitimg(f, x0) << dst_offset;
	if (first_width > size) {
	    mask &= WORD_MASK(dst_offset + size);
	    *dst_arr = (*dst_arr & ~mask) | (y & mask);
	    return;
	}
	*dst_arr = (*dst_arr & ~mask) | y;
	++dst_arr;
	size -= first_width;
    }

    /* Handle all full-width words. */
    sizew = size / CU_WORD_WIDTH;
    cu_wordarr_skewcopy_bitimg(f, sizew, dst_arr, src0_offset, src0_arr);

    /* Handle partial last destination word. */
    last_width = size % CU_WORD_WIDTH;
    if (last_width) {
	cu_word_t mask = WORD_MASK(last_width);
	dst_arr += sizew;
	src0_arr += sizew;
	x0 = _read_bits(last_width, &src0_offset, &src0_arr);
	y = cu_word_bitimg(f, x0);
	*dst_arr = (*dst_arr & ~mask) | (y & mask);
    }
}

void
cucon_bitarray_slice_copy_bool2f(cu_bool2f_t f, cucon_bitarray_slice_t dst,
				 cucon_bitarray_slice_t src0,
				 cucon_bitarray_slice_t src1)
{
    cu_word_t x0, x1, y;
    int first_width, last_width;
    size_t sizew, size = dst->size;
    cu_word_t *dst_arr = dst->arr;
    int dst_offset = dst->offset;
    cu_word_t *src0_arr = src0->arr;
    int src0_offset = src0->offset;
    cu_word_t *src1_arr = src1->arr;
    int src1_offset = src1->offset;

    /* Check domain. */
#ifdef CUCONF_DEBUG_CLIENT
    if (src0->size < size)
	cu_bugf_domain(2, "src0", "First source slice is shorter than "
		       "destination slice.");
    if (src1->size < size)
	cu_bugf_domain(3, "src1", "Second source slice is shorter than "
		       "destination slice.");
#endif

    /* Handle partial first destination word. */
    if (dst_offset) {
	cu_word_t mask = ~WORD_MASK(dst_offset);
	first_width = CU_WORD_WIDTH - dst_offset;
	x0 = _read_bits(first_width, &src0_offset, &src0_arr);
	x1 = _read_bits(first_width, &src1_offset, &src1_arr);
	y = cu_word_bitimg2(f, x0, x1) << dst_offset;
	if (first_width > size) {
	    mask &= WORD_MASK(dst_offset + size);
	    *dst_arr = (*dst_arr & ~mask) | (y & mask);
	    return;
	}
	*dst_arr = (*dst_arr & ~mask) | y;
	++dst_arr;
	size -= first_width;
    }

    /* Handle all full-width words. */
    sizew = size / CU_WORD_WIDTH;
    cu_wordarr_skewcopy_bitimg2(f, sizew, dst_arr,
				src0_offset, src0_arr,
				src1_offset, src1_arr);

    /* Handle partial last destination word. */
    last_width = size % CU_WORD_WIDTH;
    if (last_width) {
	cu_word_t mask = WORD_MASK(last_width);
	dst_arr += sizew;
	src0_arr += sizew;
	src1_arr += sizew;
	x0 = _read_bits(last_width, &src0_offset, &src0_arr);
	x1 = _read_bits(last_width, &src1_offset, &src1_arr);
	y = cu_word_bitimg2(f, x0, x1);
	*dst_arr = (*dst_arr & ~mask) | (y & mask);
    }
}
