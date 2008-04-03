/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CU_PTR_H
#define CU_PTR_H

#include <cu/fwd.h>
#include <stddef.h>
#include <stdint.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cu_ptr_h cu/ptr.h: Pointer Arithmetic
 *@{\ingroup cu_mod */

/*!Add \a diff bytes to \a ptr irrespective of the underlying type. */
CU_SINLINE void *cu_ptr_add(void *ptr, ptrdiff_t diff)
{ return (char *)ptr + diff; }

/*!Subtract \a diff bytes from \a ptr irrespective of the underlying type. */
CU_SINLINE void *cu_ptr_sub(void *ptr, ptrdiff_t diff)
{ return (char *)ptr - diff; }

/*!The difference between \a ptr0 and \a ptr1 in bytes, irrespective of the
 * underlying types. */
CU_SINLINE ptrdiff_t cu_ptr_diff(void const *ptr_lhs, void const *ptr_rhs)
{ return (char const *)ptr_lhs - (char const *)ptr_rhs; }

/*!The pointer \a ptr rounded down to the nearest multiple of \a m. */
CU_SINLINE void *cu_ptr_mulfloor(void *ptr, cu_offset_t m)
{ return (void *)((uintptr_t)ptr/m*m); }

/*!The pointer \a ptr rounded up to the nearest multiple of \a m. */
CU_SINLINE void *cu_ptr_mulceil(void *ptr, cu_offset_t m)
{ return (void *)(((uintptr_t)ptr + m - 1)/m*m); }

/*!The pointer \a ptr rounded down to the nearest multiple of \f$2^b\f$. */
CU_SINLINE void *cu_ptr_scal2floor(void *ptr, unsigned int b)
{ return (void *)((uintptr_t)ptr & ~(((uintptr_t)1 << b) - (uintptr_t)1)); }

/*!The pointer \a ptr rounded up to the nearest multiple of \f$2^b\f$. */
CU_SINLINE void *cu_ptr_scal2ceil(void *ptr, unsigned int b)
{
    uintptr_t m = ((uintptr_t)1 << b) - (uintptr_t)1;
    return (void *)(((uintptr_t)ptr + m) & ~m);
}

/*!The pointer \a ptr rounded down to the nearest multiple of the maximum
 * alignment. */
CU_SINLINE void *cu_ptr_alignfloor(void *ptr)
{ return cu_ptr_mulfloor(ptr, CUCONF_MAXALIGN); }

/*!The pointer \a ptr rounded up to the nearest multiple of the maximum
 * alignment. */
CU_SINLINE void *cu_ptr_alignceil(void *ptr)
{ return cu_ptr_mulceil(ptr, CUCONF_MAXALIGN); }

/*!@}*/
CU_END_DECLARATIONS

#endif
