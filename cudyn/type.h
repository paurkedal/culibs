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

#ifndef CUDYN_TYPE_H
#define CUDYN_TYPE_H

#include <cuoo/type.h>
#include <cuoo/layout.h>
#include <cucon/pmap.h>
#include <cucon/ucmap.h>
#include <cudyn/fwd.h>
#ifdef CUCONF_HAVE_LIBFFI_FFI_H
#  include <libffi/ffi.h>
#else
#  include <ffi.h>
#endif

CU_BEGIN_DECLARATIONS
/*!\defgroup cudyn_type_h cudyn/type.h: Dynamic Types
 * @{ \ingroup cudyn_mod */

struct cudyn_inltype_s
{
    cu_inherit (cuoo_type_s);
    AO_t layout;	/* actually cuoo_layout_t */
    AO_t ffitype;
};

extern cuoo_stdtype_t cudynP_cuex_type;
extern cuoo_stdtype_t cudynP_ptrtype_type;
extern cuoo_stdtype_t cudynP_elmtype_type;
extern cuoo_stdtype_t cudynP_arrtype_type;
extern cuoo_stdtype_t cudynP_tuptype_type;
extern cuoo_stdtype_t cudynP_sigtype_type;
extern cuoo_stdtype_t cudynP_duntype_type;
extern cuoo_stdtype_t cuooP_type_type;

CU_SINLINE cuoo_type_t cuoo_type_type()
{ return cuoo_stdtype_to_type(cuooP_type_type); }


/* Dynamic Object Type
 * =================== */

/*!Returns the type of dynamic objects and expressions. */
CU_SINLINE cuoo_type_t cudyn_cuex_type()
{ return cuoo_stdtype_to_type(cudynP_cuex_type); }

/*!True iff \a t is the type of dynamic objects and expressions. */
CU_SINLINE cu_bool_t cudyn_is_cuex_type(cuoo_type_t t)
{ return cuoo_stdtype_to_type(cudynP_cuex_type) == t; }

/*!\defgroup cudyn_ptrtype_mod cudyn_ptrtype_t: Pointer Types
 * @{ */

/* Pointer Types
 * ============= */

struct cudyn_ptrtype_s
{
    cu_inherit (cudyn_inltype_s);
};

/*!The type of pointer types. */
CU_SINLINE cuoo_type_t cudyn_ptrtype_type()
{ return cuoo_stdtype_to_type(cudynP_ptrtype_type); }

/*!True iff \a t is a \c cudyn_ptrtype_t. */
CU_SINLINE cu_bool_t cuoo_type_is_ptrtype(cuoo_type_t t)
{ return cuoo_type_typekind(t) == cuoo_typekind_ptrtype; }

#define cudyn_ptrtype_to_type(t) \
    cu_to2(cuoo_type, cudyn_inltype, t)
#define cudyn_ptrtype_from_type(t) \
    cu_from2(cudyn_ptrtype, cudyn_inltype, cuoo_type, t)

/*!Type of pointers to objects of \a deref. */
cudyn_ptrtype_t cudyn_ptrtype(cuex_t deref);

/*!A \c cuoo_type_t representation of \a e, which must have a toplevel
 * \ref CUEX_O1_PTR_TO. */
cudyn_ptrtype_t cudyn_ptrtype_from_ex(cuex_t e);

/*!@}
 * \defgroup cudyn_elmtype_mod cudyn_elmtype_t: Elementary Types
 * @{\ingroup cuoo_type_h_mod */

/* Elementary Types
 * ================ */

struct cudyn_elmtype_s
{
    cu_inherit (cudyn_inltype_s);
};

/*!The type of elementary types. */
CU_SINLINE cuoo_type_t cudyn_elmtype_type()
{ return cuoo_stdtype_to_type(cudynP_elmtype_type); }

/*!True iff \a t is a \c cudyn_elmtype_t. */
CU_SINLINE cu_bool_t cuoo_type_is_elmtype(cuoo_type_t t)
{ return cuoo_type_typekind(t) >= cuoo_typekind_elmtype_MIN; }

#define cudyn_elmtype_to_type(t) \
    cu_to2(cuoo_type, cudyn_inltype, t)
#define cudyn_elmtype_from_type(t) \
    cu_from2(cudyn_elmtype, cudyn_inltype, cuoo_type, t)

/*!Create a unique elementary type, of which objects are \a size bytes,
 * require \a alignment bytes alignement and FFI type \a ffitype. */
cudyn_elmtype_t cudyn_elmtype_new(cuoo_typekind_t kind, cuoo_impl_t impl,
				  cu_offset_t size, cu_offset_t alignment,
				  ffi_type *ffitype);


/*!@}
 * \defgroup cudyn_arrtype_mod cudyn_arrtype_t: Array Types
 * @{\ingroup cuoo_type_h_mod */

/* Array Types
 * =========== */

struct cudyn_arrtype_s
{
    cu_inherit (cudyn_inltype_s);
    size_t elt_cnt;
    cuoo_type_t elt_type;
};

/*!The type of array types. */
CU_SINLINE cuoo_type_t cudyn_arrtype_type()
{ return cuoo_stdtype_to_type(cudynP_arrtype_type); }

/*!True iff \a t is a \c cudyn_arrtype_t. */
CU_SINLINE cu_bool_t cuoo_type_is_arrtype(cuoo_type_t t)
{ return cuoo_type_typekind(t) == cuoo_typekind_arrtype; }

/*!True iff \a meta is a \c cudyn_arrtype_t. */
CU_SINLINE cu_bool_t cudyn_meta_is_arrtype(cuex_meta_t meta)
{ return cuex_meta_is_type(meta)
      && cuoo_type_is_arrtype(cuoo_type_from_meta(meta)); }

#define cudyn_arrtype_to_type(t) \
    cu_to2(cuoo_type, cudyn_inltype, t)
#define cudyn_arrtype_from_type(t) \
    cu_from2(cudyn_arrtype, cudyn_inltype, cuoo_type, t)
#define cudyn_arrtype_from_meta(meta) \
    cudyn_arrtype_from_type(cuoo_type_from_meta(meta))

/*!The dynamic representation of \a elt_type ↑ \a elt_cnt. */
cudyn_arrtype_t cudyn_arrtype(cuoo_type_t elt_type, size_t elt_cnt);

/*!The type of the elements of arrays of type \a t. */
CU_SINLINE cuoo_type_t cudyn_arrtype_elt_type(cudyn_arrtype_t t)
{ return t->elt_type; }

/*!The number of elements in arrays of type \a t. */
CU_SINLINE size_t cudyn_arrtype_elt_cnt(cudyn_arrtype_t t)
{ return t->elt_cnt; }

/*!@}
 * \defgroup cudyn_tuptype_mod cudyn_tuptype_t: Tuple Types
 * @{ */

/* Tuple Types
 * =========== */

struct cudyn_tupcomp_s
{
    cuoo_type_t type;
    cu_offset_t bitoffset;
};
struct cudyn_tuptype_s
{
    cu_inherit (cudyn_inltype_s);
    size_t tcomp_cnt;
    struct cudyn_tupcomp_s *tcomp_arr;
    struct cucon_pmap_s scomp_map;
};

typedef int cudyn_tupindex_t;
#define cudyn_tupindex_none cucon_pcmap_int_none

/*!The type of tuple types. */
CU_SINLINE cuoo_type_t cudyn_tuptype_type()
{ return cuoo_stdtype_to_type(cudynP_tuptype_type); }

/*!True iff \a t is a cudyn_tuptype_t. */
CU_SINLINE cu_bool_t cuoo_type_is_tuptype(cuoo_type_t t)
{ return cuoo_type_typekind(t) == cuoo_typekind_tuptype; }

#define cudyn_tuptype_to_type(t) \
    cu_to2(cuoo_type, cudyn_inltype, t)
#define cudyn_tuptype_from_type(t) \
    cu_from2(cudyn_tuptype, cudyn_inltype, cuoo_type, t)

cudyn_tuptype_t cudyn_tuptype_glck(cuex_t typeex);

cudyn_tuptype_t cudyn_tuptype(cuex_t typeex);

cudyn_tuptype_t cudyn_tuptype_by_valist(cu_offset_t cnt, va_list vl);

CU_SINLINE cu_offset_t cudyn_tuptype_tcomp_cnt(cudyn_tuptype_t t)
{ return t->tcomp_cnt; }

CU_SINLINE cuoo_type_t cudyn_tuptype_at(cudyn_tuptype_t t, cu_offset_t i)
{ return t->tcomp_arr[i].type; }

cu_bool_t
cudyn_tuptype_conj(cudyn_tuptype_t t,
		   cu_clop(cb, cu_bool_t, cu_idr_t label, cu_offset_t bitoff,
					  cuoo_type_t type));

/*!The offset in bits of the last component of \a t. */
CU_SINLINE size_t
cudyn_tuptype_bitoffset(cudyn_tuptype_t t, cudyn_tupindex_t i)
{
    return t->tcomp_arr[i].bitoffset;
}

/*!The offset in bytes of the last component of \a t, assuming it is not
 * a bitfield. */
CU_SINLINE size_t
cudyn_tuptype_offset_at(cudyn_tuptype_t t, cudyn_tupindex_t i)
{
    return cudyn_tuptype_bitoffset(t, i)/8;
}

/*!@}*/

#if 0
/*!\defgroup cudyn_duntype_mod cudyn_duntype_t: Discriminated Unions
 * @{ */

/* Union Types
 * =========== */

typedef uintptr_t cudyn_cnum_t;

struct cudyn_dunpart_s
{
    cudyn_cnum_t cnum;
    cuoo_type_t type;
};

/*!A union of types.  To make discriminated unions, add an integer of
 * suitable size as the first component of each type.  It must be the
 * same size for all component types.  By convention, enumerate them left
 * to right, starting with 0.  Unions made this way can be extended by
 * adding to the right. */
struct cudyn_duntype_s
{
    cu_inherit (cudyn_inltype_s);
    struct cucon_pmap_s idr_to_part;
};

/*!Type type of discriminated union types. */
CU_SINLINE cuoo_type_t cudyn_duntype_type()
{ return cuoo_stdtype_to_type(cudynP_duntype_type); }

/*!True iff \a t is a cudyn_duntype_t. */
CU_SINLINE cu_bool_t cuoo_type_is_duntype(cuoo_type_t t)
{ return cuoo_type_typekind(t) == cuoo_typekind_duntype; }

#define cudyn_duntype_to_type(t) \
    cu_to2(cuoo_type, cudyn_inltype, t)
#define cudyn_duntype_from_type(t) \
    cu_from2(cudyn_duntype, cudyn_inltype_s, cuoo_type, t)

/*!Return the discriminated union of the partitions formed by joining
 * the keys of \a partmap as discriminators with the values of \a partmap.
 * The first component of each partition are used for the discriminator.
 * \pre \a partmap maps from unsigned integers to \c cudyn_tuptype_t.
 * \pre The first component of each partiton must be the same integral
 * type. */
cudyn_duntype_t cudyn_duntype_of_ucmap(cucon_ucmap_t partmap);

/*!Return the discriminated union \a u extended with a partition \a t with
 * discriminator \a cnum.  Use \a u = NULL to start a new union.
 * \pre \a cnum must not be among the discriminators of \a u.
 * \pre The first component of \a t must be an integral type equal to
 * the type of the first components of all other partitions of \a u. */
cudyn_duntype_t cudyn_duntype_insert(cudyn_duntype_t u, cudyn_cnum_t cnum,
				     cudyn_tuptype_t t);

/*!Return the component of \a u with constructor number \a i. */
cuoo_type_t cudyn_duntype_at(cudyn_duntype_t u, cudyn_cnum_t i);

/*!Sequential conjunction of \a cb over the partitions of \a u. */
cu_bool_t
cudyn_duntype_conj(cudyn_duntype_t u,
		   cu_clop(cb, cu_bool_t, cudyn_cnum_t, cuoo_type_t));

/*!@}*/
#endif

cuoo_type_t cuoo_type_glck(cuex_t ex);

/*!Returns \a ex interpreted as a type, or \c NULL if not syntactically
 * correct. */
cuoo_type_t cuoo_type(cuex_t ex);

/*!The memory layout of \a type. */
CU_SINLINE cuoo_layout_t
cuoo_type_layout(cuoo_type_t type)
{
    if (cuoo_type_is_nonptr_inltype(type))
	return (cuoo_layout_t)cu_from(cudyn_inltype,
				       cuoo_type, type)->layout;
    else
	return cuoo_layout_ptr();
}

/*!The size in bits of objects of type \a t. */
CU_SINLINE size_t
cuoo_type_bitsize(cuoo_type_t t)
{
    return cuoo_layout_bitsize(cuoo_type_layout(t));
}

/*!The size of object of type \a t. */
CU_SINLINE size_t
cuoo_type_size(cuoo_type_t t)
{
    return cuoo_layout_size(cuoo_type_layout(t));
}

CU_SINLINE size_t
cuoo_type_bitalign(cuoo_type_t t)
{
    return cuoo_layout_bitalign(cuoo_type_layout(t));
}

CU_SINLINE size_t
cuoo_type_align(cuoo_type_t t)
{
    return cuoo_layout_align(cuoo_type_layout(t));
}

/*!@}*/
CU_END_DECLARATIONS

#endif
