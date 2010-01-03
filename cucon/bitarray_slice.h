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

#ifndef CUCON_BITARRAY_SLICE_H
#define CUCON_BITARRAY_SLICE_H

#include <cucon/fwd.h>
#include <cu/bool.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cucon_bitarray_slice_h cucon/bitarray_slice.h: Bitarray Slices
 ** @{ \ingroup cucon_mod */

/** A continuous range of bits of a \ref cucon_bitarray_h "bit-array". */
struct cucon_bitarray_slice
{
    int offset;
    size_t size;
    cu_word_t *arr;
};

/** The number of bits covered by \a sl. */
CU_SINLINE size_t
cucon_bitarray_slice_size(cucon_bitarray_slice_t sl)
{
    return sl->size;
}

/** Read bit number \a i of \a sl. */
CU_SINLINE cu_bool_t
cucon_bitarray_slice_at(cucon_bitarray_slice_t sl, size_t i)
{
    i += sl->offset;
    return (sl->arr[i / CU_WORD_WIDTH] >> (i % CU_WORD_WIDTH)) & 1;
}

CU_SINLINE cu_word_t
cucon_bitarray_slice_word_at(cucon_bitarray_slice_t sl, size_t i)
{
    size_t j, k;
    i += sl->offset;
    j = i / CU_WORD_WIDTH;
    k = i % CU_WORD_WIDTH;
    return k == 0
	 ? sl->arr[j]
	 : (sl->arr[j] >> k) | (sl->arr[j + 1] << (CU_WORD_WIDTH - k));
}

/** Assign \a v to bit \a i of \a sl. */
CU_SINLINE void
cucon_bitarray_slice_set_at(cucon_bitarray_slice_t sl, size_t i, cu_bool_t v)
{
    cu_word_t bit;
    i += sl->offset;
    bit = CU_WORD_C(1) << (i % CU_WORD_WIDTH);
    if (v)
	sl->arr[i / CU_WORD_WIDTH] |= bit;
    else
	sl->arr[i / CU_WORD_WIDTH] &= ~bit;
}

/** Initialise \a sl as a slice of \a ba covering the range from \a start_index
 ** up to \a end_index. */
void cucon_bitarray_slice_init(cucon_bitarray_slice_t sl, cucon_bitarray_t ba,
			       size_t start_index, size_t end_index);

/** Initialise \a sl to the same slice as \a sl_src. */
void cucon_bitarray_slice_init_copy(cucon_bitarray_slice_t sl,
				    cucon_bitarray_slice_t sl_src);

/** Initialise \a sl as a slice of the same array from which \a sl_src was
 ** constructed and covering \a start_index up to \a end_index relative to the
 ** start of \a sl_src. */
void cucon_bitarray_slice_init_sub(cucon_bitarray_slice_t sl,
				   cucon_bitarray_slice_t sl_src,
				   size_t start_index, size_t end_index);

/** Fill \a sl with the constant value \a v. */
void cucon_bitarray_slice_fill(cucon_bitarray_slice_t sl, cu_bool_t v);

/** Replace each bit \e x of \a dst with <code>cu_bool1f_apply(\a f, \e
 ** x)</code>. */
void cucon_bitarray_slice_update_bool1f(cu_bool1f_t f,
				       	cucon_bitarray_slice_t dst);

void cucon_bitarray_slice_update_bool2f(cu_bool2f_t f,
				       	cucon_bitarray_slice_t dst,
					cucon_bitarray_slice_t src1);

/** Copy the image of the slice \a src0 under \a f to \a dst.
 **
 ** \pre \a src0 must be at least as long as \a dst. */
void cucon_bitarray_slice_copy_bool1f(cu_bool1f_t f,
				      cucon_bitarray_slice_t dst,
				      cucon_bitarray_slice_t src0);

/** Copy the image under \a f of zipped pairs from \a src0 and \a src1 to \a
 ** dst.
 **
 ** \pre \a src0 and \a src1 must be at least as long as \a dst. */
void cucon_bitarray_slice_copy_bool2f(cu_bool2f_t f,
				      cucon_bitarray_slice_t dst,
				      cucon_bitarray_slice_t src0,
				      cucon_bitarray_slice_t src1);

/** Copy \a src to \a dst.
 **
 ** \pre \a src must be at least as long as \a dst. */
CU_SINLINE void
cucon_bitarray_slice_copy(cucon_bitarray_slice_t dst,
			  cucon_bitarray_slice_t src)
{
    cucon_bitarray_slice_copy_bool1f(CU_BOOL1F_IDENT, dst, src);
}

/** @} */
CU_END_DECLARATIONS

#endif
