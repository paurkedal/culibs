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

#ifndef CUCON_PARRAY_H
#define CUCON_PARRAY_H

#include <cucon/fwd.h>
#include <cucon/array.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cucon_parray_h cucon/parray.h: Array of Pointers
 ** @{ \ingroup cucon_linear_mod
 **
 ** This is mostly a light wrapper around \ref cucon_array_h "cucon/array.h",
 ** with sizes and indices scaled for pointer-typed elements.
 **
 ** \see cucon_array_h
 ** \see cucon_bitarray_h
 **/

struct cucon_parray
{
    struct cucon_array arr;
};

/** Initialises \a arr to as empty array. */
CU_SINLINE void
cucon_parray_init_empty(cucon_parray_t arr)
{ cucon_array_init(&arr->arr, cu_false, 0); }

/** Returns an empty array. */
CU_SINLINE cucon_parray_t
cucon_parray_new_empty(void)
{ return (cucon_parray_t)cucon_array_new(cu_false, 0); }

/** Initialises \a arr to an array of \a size uninitialised pointers. */
CU_SINLINE void
cucon_parray_init_size(cucon_parray_t arr, size_t size)
{ cucon_array_init(&arr->arr, cu_false, size*sizeof(void *)); }

/** Returns an array of \a size uninitialised pointers. */
CU_SINLINE cucon_parray_t
cucon_parray_new_size(size_t size)
{ return (cucon_parray_t)cucon_array_new(cu_false, size*sizeof(void *)); }

/** Construct a \a arr as an array of \a size elements all equal to \a ptr. */
void cucon_parray_init_fill(cucon_parray_t arr, size_t size, void *ptr);

/** Returns an array of \a size elements all equal to \a ptr. */
cucon_parray_t cucon_parray_new_fill(size_t size, void *ptr);

/** \copydoc cucon_array_swap */
CU_SINLINE void
cucon_parray_swap(cucon_parray_t arr0, cucon_parray_t arr1)
{ cucon_array_swap(&arr0->arr, &arr1->arr); }

/** \copydoc cucon_array_size */
CU_SINLINE size_t
cucon_parray_size(cucon_parray_t arr)
{ return cucon_array_size(&arr->arr)/sizeof(void *); }

/** \copydoc cucon_array_ref_at */
CU_SINLINE cu_ptr_ptr_t
cucon_parray_ref_at(cucon_parray_t arr, size_t index)
{ return cucon_array_ref_at(&arr->arr, index*sizeof(void *)); }

/** Get component \a index of \a arr. */
CU_SINLINE void *
cucon_parray_at(cucon_parray_t arr, size_t index)
{ return *(void **)cucon_array_ref_at(&arr->arr, index*sizeof(void *)); }

/** Set component \a index of \a arr to \a ptr. */
CU_SINLINE void
cucon_parray_set_at(cucon_parray_t arr, size_t index, void *ptr)
{ *(void **)cucon_array_ref_at(&arr->arr, index*sizeof(void *)) = ptr; }

/** \copydoc cucon_array_resize_gp */
CU_SINLINE void
cucon_parray_resize_gp(cucon_parray_t arr, size_t size)
{ cucon_array_resize_gp(&arr->arr, size*sizeof(void *)); }

/** \copydoc cucon_array_resize_gpmax */
CU_SINLINE void
cucon_parray_resize_gpmax(cucon_parray_t arr, size_t size)
{ cucon_array_resize_gpmax(&arr->arr, size*sizeof(void *)); }

/** \copydoc cucon_array_resize_exact */
CU_SINLINE void
cucon_parray_resize_exact(cucon_parray_t arr, size_t size)
{ cucon_array_resize_exact(&arr->arr, size*sizeof(void *)); }

/** \copydoc cucon_array_resize_exactmax */
CU_SINLINE void
cucon_parray_resize_exactmax(cucon_parray_t arr, size_t size)
{ cucon_array_resize_exactmax(&arr->arr, size*sizeof(void *)); }

/** Perform \ref cucon_parray_resize_gp and set any new elements to \a ptr. */
void cucon_parray_resize_gp_fill(cucon_parray_t arr, size_t size, void *ptr);

/** Perform \ref cucon_parray_resize_gpmax and set any new elements to \a ptr.
 **/
void cucon_parray_resize_gpmax_fill(cucon_parray_t arr, size_t size, void *ptr);

/** Perform \ref cucon_parray_resize_exact and set any new elements to \a ptr.
 **/
void cucon_parray_resize_exact_fill(cucon_parray_t arr, size_t size, void *ptr);

/** Perform \ref cucon_parray_resize_exactmax and set any new elements to
 ** \a ptr. */
void cucon_parray_resize_exactmax_fill(cucon_parray_t arr, size_t size,
				       void *ptr);

/** Append \a elt to \a arr, extending the capacity in geometric
 ** progression. */
CU_SINLINE void
cucon_parray_append_gp(cucon_parray_t arr, void *elt)
{
    void **p = (void **)cucon_array_extend_gp(&arr->arr, sizeof(void *));
    *p = elt;
}

/** Set all elements of \a arr to \a ptr. */
void cucon_parray_fill_all(cucon_parray_t arr, void *ptr);

/** Set elements of \a arr from \a start up to but not including \a end to \a
 ** ptr. */
void cucon_parray_fill_range(cucon_parray_t arr,
			     size_t start, size_t end, void *ptr);

/** A pointer to the start of the internal array.
 ** \see cucon_parray_end */
CU_SINLINE void **
cucon_parray_begin(cucon_parray_t arr)
{ return (void **)cucon_array_begin(&arr->arr); }

/** A pointer past the end of the internal array.
 ** \see cucon_parray_begin */
CU_SINLINE void **
cucon_parray_end(cucon_parray_t arr)
{ return (void **)cucon_array_end(&arr->arr); }

/** @} */
CU_END_DECLARATIONS

#endif
