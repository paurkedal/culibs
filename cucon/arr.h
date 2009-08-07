/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2006--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUCON_ARR_H
#define CUCON_ARR_H

#include <cucon/fwd.h>
#include <stdint.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cucon_arr_h cucon/arr.h: Generic Array
 ** @{ \ingroup cucon_linear_mod
 ** This header provides generic arrays of inlined elements.  All sizes and
 ** indices are measured in bytes, and must be scaled with the element size.
 **
 ** \see \ref cucon_parr_h
 **/

struct cucon_arr_s
{
    size_t size;
    size_t cap;
    char *carr;
};

/** Construct an empty array. */
void cucon_arr_init_empty(cucon_arr_t arr);

/** Return a new empty array. */
cucon_arr_t cucon_arr_new_empty(void);

/** Construct an array with \a size initial elements. */
void cucon_arr_init_size(cucon_arr_t arr, size_t size);

/** Return a new array with \a size initial elements. */
cucon_arr_t cucon_arr_new_size(size_t size);

/** Swap the values of \a arr0 and \a arr1. */
void cucon_arr_swap(cucon_arr_t arr0, cucon_arr_t arr1);

/** The size of \a arr. */
CU_SINLINE size_t
cucon_arr_size(cucon_arr_t arr) { return arr->size; }

/** Pointer to entry at \a index in \a arr.
 ** \pre \a arr is least \a index + 1 long. */
CU_SINLINE void *
cucon_arr_ref_at(cucon_arr_t arr, size_t index) { return arr->carr + index; }

/** Returns a plain memory fragment with the data stored in the array and
 ** invalidates \a arr.  The memory fragment may be reallocated and copied if
 ** the capacity of \a arr is larger than its size. */
void *cucon_arr_detach(cucon_arr_t arr);

/** Resize \a arr to \a size, changing the capacity in geometric progression if
 ** necessary. */
void cucon_arr_resize_gp(cucon_arr_t arr, size_t size);

/** Resize \a arr to \a size, increasing capacity geometrically if necessary.
 ** This call never decrease the capacity. */
void cucon_arr_resize_gpmax(cucon_arr_t arr, size_t size);

/** Resize \a arr to \a size, changing the capacity to match the size.  If you
 ** are growing an array, \ref cucon_arr_resize_gp gives better amortised
 ** time-complexity. */
void cucon_arr_resize_exact(cucon_arr_t arr, size_t size);

/** Resize \a arr to \a size, increasing capacity to match the size if
 ** necesary.  This call never decrease the capacity.  If you are growing an
 ** array, \ref cucon_arr_resize_gpmax gives better amortised time-complexity.
 **/
void cucon_arr_resize_exactmax(cucon_arr_t arr, size_t size);

/** Extend \a arr with \a size entries, and return a pointer to the first one.
 ** The array capacity is increased geometrically. */
CU_SINLINE void *
cucon_arr_extend_gp(cucon_arr_t arr, size_t size)
{
    size_t old_size = arr->size;
    cucon_arr_resize_gpmax(arr, old_size + size);
    return arr->carr + old_size;
}

/** Extend \a arr with \a size entries, and return a pointer to the first one.
 ** The array capacity is increased to exactly match \a size if needed.  If you
 ** are calling this many times, use \ref cucon_arr_extend_gp for better time
 ** complexity. */
CU_SINLINE void *
cucon_arr_extend_exact(cucon_arr_t arr, size_t size)
{
    size_t old_size = arr->size;
    cucon_arr_resize_exactmax(arr, old_size + size);
    return arr->carr + old_size;
}

/** A pointer to the start of the internal array.  If converted to an
 ** appropriate pointer type, it can be used as an iterator.
 ** \see cucon_arr_end */
CU_SINLINE void *cucon_arr_begin(cucon_arr_t arr)
{ return arr->carr; }

/** A pointer past the end of the internal array.
 ** \see cucon_arr_begin */
CU_SINLINE void *cucon_arr_end(cucon_arr_t arr)
{ return arr->carr + arr->size; }

/** @} */
#define cucon_arr_cct_empty	cucon_arr_init_empty
#define cucon_arr_cct_size	cucon_arr_init_size
CU_END_DECLARATIONS

#endif
