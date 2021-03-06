/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2006--2009  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUCON_ARRAY_H
#define CUCON_ARRAY_H

#include <cucon/fwd.h>
#include <stdint.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cucon_array_h cucon/array.h: Generic Array
 ** @{ \ingroup cucon_linear_mod
 **
 ** This header provides generic arrays of inlined elements.  All sizes and
 ** indices are measured in bytes, and must be scaled with the element size.
 **
 ** \see \ref cucon_bitarray_h
 ** \see \ref cucon_parray_h
 **/

struct cucon_array
{
    size_t size;
    size_t cap;
    char *carr;
};

/** Construct an array of initially \a size bytes.  If true is passed for \a
 ** is_atomic, then atomic allocation will be used for the array data.  */
void cucon_array_init(cucon_array_t arr, cu_bool_t is_atomic, size_t size);

/** Returns an array of initially \a size bytes.  If true is passed for \a
 ** is_atomic, then atomic allocation will be used for the array data. */
cucon_array_t cucon_array_new(cu_bool_t is_atomic, size_t size);

/** Swap the values of \a arr0 and \a arr1. */
void cucon_array_swap(cucon_array_t arr0, cucon_array_t arr1);

/** The size of \a arr. */
CU_SINLINE size_t
cucon_array_size(cucon_array_t arr) { return arr->size; }

/** Pointer to entry at \a index in \a arr.
 ** \pre \a arr is least \a index + 1 long. */
CU_SINLINE void *
cucon_array_ref_at(cucon_array_t arr, size_t index)
{ return arr->carr + index; }

/** Returns a plain memory fragment with the data stored in the array and
 ** invalidates \a arr.  The memory fragment may be reallocated and copied if
 ** the capacity of \a arr is larger than its size. */
void *cucon_array_detach(cucon_array_t arr);

/** Resize \a arr to \a size, changing the capacity in geometric progression if
 ** necessary. */
void cucon_array_resize_gp(cucon_array_t arr, size_t size);

/** Resize \a arr to \a size, increasing capacity geometrically if necessary.
 ** This call never decrease the capacity. */
void cucon_array_resize_gpmax(cucon_array_t arr, size_t size);

/** Resize \a arr to \a size, changing the capacity to match the size.  If you
 ** are growing an array, \ref cucon_array_resize_gp gives better amortised
 ** time-complexity. */
void cucon_array_resize_exact(cucon_array_t arr, size_t size);

/** Resize \a arr to \a size, increasing capacity to match the size if
 ** necesary.  This call never decrease the capacity.  If you are growing an
 ** array, \ref cucon_array_resize_gpmax gives better amortised time-complexity.
 **/
void cucon_array_resize_exactmax(cucon_array_t arr, size_t size);

/** Extend \a arr with \a size entries, and return a pointer to the first one.
 ** The array capacity is increased geometrically. */
CU_SINLINE void *
cucon_array_extend_gp(cucon_array_t arr, size_t size)
{
    size_t old_size = arr->size;
    cucon_array_resize_gpmax(arr, old_size + size);
    return arr->carr + old_size;
}

/** Extend \a arr with \a size entries, and return a pointer to the first one.
 ** The array capacity is increased to exactly match \a size if needed.  If you
 ** are calling this many times, use \ref cucon_array_extend_gp for better time
 ** complexity. */
CU_SINLINE void *
cucon_array_extend_exact(cucon_array_t arr, size_t size)
{
    size_t old_size = arr->size;
    cucon_array_resize_exactmax(arr, old_size + size);
    return arr->carr + old_size;
}

/** A pointer to the start of the internal array.  If converted to an
 ** appropriate pointer type, it can be used as an iterator.
 ** \see cucon_array_end */
CU_SINLINE void *cucon_array_begin(cucon_array_t arr)
{ return arr->carr; }

/** A pointer past the end of the internal array.
 ** \see cucon_array_begin */
CU_SINLINE void *cucon_array_end(cucon_array_t arr)
{ return arr->carr + arr->size; }

/** @} */
CU_END_DECLARATIONS

#endif
