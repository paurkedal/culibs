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

#ifndef CUCON_BITARRAY_H
#define CUCON_BITARRAY_H

#include <cucon/fwd.h>
#include <cu/conf.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cucon_bitarray_h cucon/bitarray.h: Array of Bits
 ** @{ \ingroup cucon_linear_mod
 **
 ** The following implements a compact, mutable, and resizable array of
 ** booleans with an API similar to that of \ref cucon_array_h "cucon/array.h"
 ** and \ref cucon_parray_h "cucon/parray.h".
 **
 ** \see cucon_array_h
 ** \see cucon_parray_h
 **/

struct cucon_bitarray
{
    size_t cap;   /* Capacity in bytes. */
    size_t size;  /* Size in bits. */
    cu_word_t *arr;
};

/** Construct an uninitialised vector which can hold \a size bits. */
void cucon_bitarray_init(cucon_bitarray_t ba, size_t size);

/** Return an uninitialised vector which can hold \a size bits. */
cucon_bitarray_t cucon_bitarray_new(size_t size);

/** Construct an vector of \a size bits initialised to \a val. */
void cucon_bitarray_init_fill(cucon_bitarray_t ba, size_t size, cu_bool_t val);

/** Return a vector of \a size bits initialised to \a val. */
cucon_bitarray_t cucon_bitarray_new_fill(size_t size, cu_bool_t val);

/** Construct \a ba as a copy of \a ba_src. */
void cucon_bitarray_init_copy(cucon_bitarray_t ba, cucon_bitarray_t ba_src);

/** Return a copy of \a ba_src. */
cucon_bitarray_t cucon_bitarray_new_copy(cucon_bitarray_t ba_src);

/** Swap the contents of \a ba0 and \a ba1. */
void cucon_bitarray_swap(cucon_bitarray_t ba0, cucon_bitarray_t ba1);

/** Set bits [\a low, \a high) in \a ba to \a val. */
void cucon_bitarray_fill(cucon_bitarray_t ba, size_t low, size_t high,
			 cu_bool_t val);

/** The number of bits in \a ba. */
CU_SINLINE size_t
cucon_bitarray_size(cucon_bitarray_t ba) { return ba->size; }

/** Returns -1, 0, or 1 if \a ba0 is less, equal, or greater than \a ba1,
 ** respectively.  The lowest positions counts as most significant. */
int cucon_bitarray_cmp(cucon_bitarray_t ba0, cucon_bitarray_t ba1);

/** True iff \a ba0 and \a ba1 contain the same bits. */
cu_bool_t cucon_bitarray_eq(cucon_bitarray_t ba0, cucon_bitarray_t ba1);

/** Set element \a i of \a ba to \a value.
 ** \pre cucon_bitarray_size(\a ba) >= \a i */
CU_SINLINE void
cucon_bitarray_set_at(cucon_bitarray_t ba, size_t i, cu_bool_t value)
{
    cu_word_t mask;
    mask = CU_WORD_C(1) << (i % CU_WORD_WIDTH);
    if (value)
	ba->arr[i / CU_WORD_WIDTH] |= mask;
    else
	ba->arr[i / CU_WORD_WIDTH] &= ~mask;
}

/** Return element \a i of \a ba.
 ** \pre cucon_bitarray_size(\a ba) >= \a i */
CU_SINLINE cu_bool_t
cucon_bitarray_at(cucon_bitarray_t ba, size_t i)
{
    return !!(ba->arr[i / CU_WORD_WIDTH] &
	      (CU_WORD_C(1) << (i % CU_WORD_WIDTH)));
}

/** Return the lowest index greater or equal to \a start where \a ba holds \a
 ** value, or \c -1 if not found. */
size_t cucon_bitarray_find(cucon_bitarray_t ba, size_t start, cu_bool_t value);

size_t cucon_bitarray_find2(cucon_bitarray_t ba0, cucon_bitarray_t ba1,
			    size_t start, cu_bool_t val0, cu_bool_t val1);

/** Resize \a ba to \a size, adjusting the capacity in geometric progression as
 ** needed. */
void cucon_bitarray_resize_gp(cucon_bitarray_t ba, size_t size);

/** Resize \a ba to \a size, increasing the capacity in geometric progression
 ** as needed.  This function does not reduce the capacity. */
void cucon_bitarray_resize_gpmax(cucon_bitarray_t ba, size_t size);

/** Resize \a ba to \a size, adjusting the capacity to the minimum required for
 ** the new size. */
void cucon_bitarray_resize_exact(cucon_bitarray_t ba, size_t size);

/** Resize \a ba to \a size, increasing the capacity if required, but only to
 ** the minimum required.  This function does not reduce the capacity. */
void cucon_bitarray_resize_exactmax(cucon_bitarray_t ba, size_t size);

/** Call \ref cucon_bitarray_resize_gp and fill any newly allocated elements
 ** with \a fill_value. */
void cucon_bitarray_resize_fill_gp(cucon_bitarray_t ba, size_t size,
				   cu_bool_t fill_value);

/** Call \ref cucon_bitarray_resize_exact and fill any newly allocated elements
 ** with \a fill_value. */
void cucon_bitarray_resize_fill_exact(cucon_bitarray_t ba, size_t size,
				      cu_bool_t fill_value);

/** Set element \a i of \a ba to \a set_value, resizing if necessary and
 ** filling undefined elements with \a fill_value.  The resize is done in
 ** geometric progression. */
void cucon_bitarray_resize_set_at(cucon_bitarray_t ba, size_t i,
				  cu_bool_t fill_value, cu_bool_t set_value);

/** @} */
CU_END_DECLARATIONS

#endif
