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

#ifndef CU_OALLOC_H
#define CU_OALLOC_H

#include <cuoo/type.h>
#include <cu/memory.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuoo_oalloc_h cuoo/oalloc.h: Object Allocation Functions
 *@{\ingroup cuoo_mod */

#define CUP_OBJ_EXTRA_BYTES 0

#define CUOO_OBJ_ALLOC_SIZEG(struct_size)				\
    (((struct_size) + sizeof(cuex_meta_t)				\
      + CU_GRAN_SIZE - 1 + CUP_OBJ_EXTRA_BYTES)/CU_GRAN_SIZE)

void *cuexP_oalloc_ord_fin_raw(cuex_meta_t meta, size_t sizeg);
void *cuexP_oalloc_unord_fin_raw(cuex_meta_t meta, size_t sizeg);

CU_SINLINE void *
cuexP_oalloc(cuex_meta_t meta, size_t size)
{
    cuex_meta_t *p = cu_galloc(size + sizeof(cuex_meta_t));
    *p = meta + 1;
    cu_debug_assert(((uintptr_t)*p & 3) != 0);
    return p + 1;
}

/*!Allocate a dynamically typed object.  The result points to a memory region
 * of at least \a size bytes, where the actual object data is stored.  The type
 * information is stored before this pointer. */
CU_SINLINE void *
cuoo_oalloc(cuoo_type_t type, size_t size)
{
    cuex_meta_t *p = cu_galloc(size + sizeof(cuex_meta_t));
    *p = cuoo_type_to_meta(type) + 1;
    cu_debug_assert(((uintptr_t)*p & 3) != 0);
    return p + 1;
}

/*!A variant of \ref cuoo_oalloc which sets the type of the returned object to
 * the object itself.  The return object must be initialised as a type, which
 * then becomes it's own type. */
CU_SINLINE void *
cuoo_oalloc_self_instance(size_t size)
{
    cuex_meta_t *p = cu_galloc(size + sizeof(cuex_meta_t));
    *p = cuoo_type_to_meta((cuoo_type_t)(p + 1)) + 1;
    return p + 1;
}

/*!As \a cuoo_oalloc, except that the finaliser in \a type is run before the
 * object is reclaimed by the garbage collector.  Objects reachable from the
 * finaliser are kept at least until the next GC cycle. */
CU_SINLINE void *
cuoo_oalloc_ord_fin(cuoo_type_t type, size_t size)
{
    return cuexP_oalloc_ord_fin_raw(cuoo_type_to_meta(type),
				    CUOO_OBJ_ALLOC_SIZEG(size));
}

/*!As \a cuoo_oalloc, except that the finaliser in \a type is run before the
 * object is reclaimed by the garbage collector.  Objects reachable from the
 * finaliser may already have been reclaimed when the finaliser is run, and
 * shall therefore not be dereferenced.  If this is not desirable, see \ref
 * cuoo_oalloc_ord_fin. */
CU_SINLINE void *
cuoo_oalloc_unord_fin(cuoo_type_t type, size_t size)
{
    return cuexP_oalloc_unord_fin_raw(cuoo_type_to_meta(type),
				      CUOO_OBJ_ALLOC_SIZEG(size));
}

#define cuoo_onew(prefix)						\
    ((struct prefix##_s *)						\
     cuoo_oalloc(prefix##_type(), sizeof(struct prefix##_s)))

#define cuoo_onew_ord_fin(prefix)					\
    ((struct prefix##_s *)						\
     cuexP_oalloc_ord_fin_raw(cuoo_type_to_meta(prefix##_type()),	\
			      CUOO_OBJ_ALLOC_SIZEG(sizeof(struct prefix##_s))))

/*!@}*/
CU_END_DECLARATIONS

#endif
