/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2005--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUDYN_CTOR_H
#define CUDYN_CTOR_H

#include <cudyn/fwd.h>
#include <cuoo/type.h>
#include <cu/ptr.h>

CU_BEGIN_DECLARATIONS

struct cudyn_ctortype
{
    cu_inherit (cuoo_type);
    cudyn_duntype_t fulltype;
    cudyn_tuptype_t parttype;
};

extern cuoo_type_t cudynP_ctortype_type;

/*!The type of all constructor types. */
CU_SINLINE cuoo_type_t cudyn_ctortype_type()
{ return cudynP_ctortype_type; }

/*!True iff \a meta is a constructor type. */
CU_SINLINE cu_bool_t cudyn_meta_is_ctortype(cuex_meta_t meta)
{ return cuoo_type_to_meta(cudyn_ctortype_type()) == meta; }

/*!True iff \a t is a constructor type. */
CU_SINLINE cu_bool_t cuoo_type_is_ctortype(cuoo_type_t t)
{ return cuoo_type_shape(t) == CUOO_SHAPE_CTORTYPE; }

#define cudyn_ctortype_to_type(t) cu_to(cuoo_type, t)
#define cudyn_ctortype_from_type(t) cu_from(cudyn_ctortype, cuoo_type, t)

/*!Return the type of a constructor for a partition of type \a parttype
 * of \a restype.
 * \pre \a restype is a union type with a partition \a parttype. */
cudyn_ctortype_t
cudyn_ctortype(cudyn_tuptype_t parttype, cudyn_duntype_t restype);

/*!The full type of the union of which \a t constructs partitions. */
CU_SINLINE cudyn_duntype_t
cudyn_ctortype_union_type(cudyn_ctortype_t t) { return t->fulltype; }

/*!The type of a partition constructed by an instance of \a t. */
CU_SINLINE cudyn_tuptype_t
cudyn_ctortype_partition_type(cudyn_ctortype_t t) { return t->parttype; }

/*!Return a constructor of type \a type with constructor number \a ctor_num.
 * The lowest bits of \a ctor_num must match partition number of \a type. */
cuex_t cudyn_ctor(cudyn_ctortype_t type, unsigned int ctor_num);

/*!True iff \a ex is a constructor. */
CU_SINLINE cu_bool_t cudyn_is_ctor(cuex_t ex)
{
    cuex_meta_t meta = cuex_meta(ex);
    return cuex_meta_is_type(meta)
	&& cuoo_type_is_ctortype(cuoo_type_from_meta(meta));
}

/*!The enumerator of \a ctor. */
CU_SINLINE unsigned int cudyn_ctor_num(cuex_t ctor)
{ return *((unsigned int *)(cu_ptr_add(ctor, CUOO_HCOBJ_SHIFT))); }

CU_END_DECLARATIONS

#endif
