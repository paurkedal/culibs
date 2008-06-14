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

#ifndef CUCON_PARR_H
#define CUCON_PARR_H

#include <cucon/fwd.h>
#include <cucon/arr.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cucon_parr_h cucon/parr.h: Array of Pointers
 *@{\ingroup cucon_linear_mod
 * This is mostly a light wrapper around \ref cucon_arr_h "cucon/arr.h", with
 * sizes and indices scaled for pointer-typed elements.
 *
 * \see cucon_arr_h
 */

struct cucon_parr_s
{
    struct cucon_arr_s arr;
};

/*!\copydoc cucon_arr_init_empty */
CU_SINLINE void
cucon_parr_init_empty(cucon_parr_t arr)
{ cucon_arr_init_empty(&arr->arr); }

/*!\copydoc cucon_arr_new_empty */
CU_SINLINE cucon_parr_t
cucon_parr_new_empty(void)
{ return (cucon_parr_t)cucon_arr_new_empty(); }

/*!\copydoc cucon_arr_init_size */
CU_SINLINE void
cucon_parr_init_size(cucon_parr_t arr, size_t size)
{ cucon_arr_init_size(&arr->arr, size*sizeof(void *)); }

/*!\copydoc cucon_arr_new_size */
CU_SINLINE cucon_parr_t
cucon_parr_new_size(size_t size)
{ return (cucon_parr_t)cucon_arr_new_size(size*sizeof(void *)); }

/*!Construct a \a arr as an array of \a size elements all equal to \a ptr. */
void cucon_parr_init_fill(cucon_parr_t arr, size_t size, void *ptr);

/*!Returns an array of \a size elements all equal to \a ptr. */
cucon_parr_t cucon_parr_new_fill(size_t size, void *ptr);

/*!\copydoc cucon_arr_swap */
CU_SINLINE void
cucon_parr_swap(cucon_parr_t arr0, cucon_parr_t arr1)
{ cucon_arr_swap(&arr0->arr, &arr1->arr); }

/*!\copydoc cucon_arr_size */
CU_SINLINE size_t
cucon_parr_size(cucon_parr_t arr)
{ return cucon_arr_size(&arr->arr)/sizeof(void *); }

/*!\copydoc cucon_arr_ref_at */
CU_SINLINE cu_ptr_ptr_t
cucon_parr_ref_at(cucon_parr_t arr, size_t index)
{ return cucon_arr_ref_at(&arr->arr, index*sizeof(void *)); }

/*!Get component \a index of \a arr. */
CU_SINLINE void *
cucon_parr_at(cucon_parr_t arr, size_t index)
{ return *(void **)cucon_arr_ref_at(&arr->arr, index*sizeof(void *)); }

/*!Set component \a index of \a arr to \a ptr. */
CU_SINLINE void
cucon_parr_set_at(cucon_parr_t arr, size_t index, void *ptr)
{ *(void **)cucon_arr_ref_at(&arr->arr, index*sizeof(void *)) = ptr; }

/*!\copydoc cucon_arr_resize_gp */
CU_SINLINE void
cucon_parr_resize_gp(cucon_parr_t arr, size_t size)
{ cucon_arr_resize_gp(&arr->arr, size*sizeof(void *)); }

/*!\copydoc cucon_arr_resize_gpmax */
CU_SINLINE void
cucon_parr_resize_gpmax(cucon_parr_t arr, size_t size)
{ cucon_arr_resize_gpmax(&arr->arr, size*sizeof(void *)); }

/*!\copydoc cucon_arr_resize_exact */
CU_SINLINE void
cucon_parr_resize_exact(cucon_parr_t arr, size_t size)
{ cucon_arr_resize_exact(&arr->arr, size*sizeof(void *)); }

/*!\copydoc cucon_arr_resize_exactmax */
CU_SINLINE void
cucon_parr_resize_exactmax(cucon_parr_t arr, size_t size)
{ cucon_arr_resize_exactmax(&arr->arr, size*sizeof(void *)); }

/*!Perform \ref cucon_parr_resize_gp and set any new elements to \a ptr. */
void cucon_parr_resize_gp_fill(cucon_parr_t arr, size_t size, void *ptr);

/*!Perform \ref cucon_parr_resize_gpmax and set any new elements to \a ptr. */
void cucon_parr_resize_gpmax_fill(cucon_parr_t arr, size_t size, void *ptr);

/*!Perform \ref cucon_parr_resize_exact and set any new elements to \a ptr. */
void cucon_parr_resize_exact_fill(cucon_parr_t arr, size_t size, void *ptr);

/*!Perform \ref cucon_parr_resize_exactmax and set any new elements to \a ptr. */
void cucon_parr_resize_exactmax_fill(cucon_parr_t arr, size_t size, void *ptr);

/*!Append \a elt to \a arr, extending the capacity in geometric
 * progression. */
CU_SINLINE void
cucon_parr_append_gp(cucon_parr_t arr, void *elt)
{
    void **p = cucon_arr_extend_gp(&arr->arr, sizeof(void *));
    *p = elt;
}

/*!Set all elements of \a arr to \a ptr. */
void cucon_parr_fill_all(cucon_parr_t arr, void *ptr);

/*!Set elements of \a arr from \a start up to but not including \a end to \a
 * ptr. */
void cucon_parr_fill_range(cucon_parr_t arr,
			   size_t start, size_t end, void *ptr);

/*!A pointer to the start of the internal array.
 * \see cucon_parr_end */
CU_SINLINE void **
cucon_parr_begin(cucon_parr_t arr)
{ return cucon_arr_begin(&arr->arr); }

/*!A pointer past the end of the internal array.
 * \see cucon_parr_begin */
CU_SINLINE void **
cucon_parr_end(cucon_parr_t arr)
{ return cucon_arr_end(&arr->arr); }

/*!@}*/
#define cucon_parr_cct_empty	cucon_parr_init_empty
#define cucon_parr_cct_size	cucon_parr_init_size
#define cucon_parr_cct_fill	cucon_parr_init_fill
CU_END_DECLARATIONS

#endif
