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

#include <cu/dyn.h>
#include <cucon/layout.h>
#include <cucon/pmap.h>
#include <cucon/ucmap.h>
#include <cudyn/fwd.h>
#include <cuex/aci.h>
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
    cu_inherit (cudyn_hctype_s);
    AO_t layout;	/* actually cucon_layout_t */
    AO_t ffitype;
};

extern cudyn_stdtype_t cudynP_cuex_type;
extern cudyn_stdtype_t cudynP_ptrtype_type;
extern cudyn_stdtype_t cudynP_elmtype_type;
extern cudyn_stdtype_t cudynP_arrtype_type;
extern cudyn_stdtype_t cudynP_tuptype_type;
extern cudyn_stdtype_t cudynP_sigtype_type;
extern cudyn_stdtype_t cudynP_duntype_type;
extern cudyn_stdtype_t cudynP_type_type;

CU_SINLINE cudyn_type_t cudyn_type_type()
{ return cudyn_stdtype_to_type(cudynP_type_type); }


/* Dynamic Object Type
 * =================== */

/*!Returns the type of dynamic objects and expressions. */
CU_SINLINE cudyn_type_t cudyn_cuex_type()
{ return cudyn_stdtype_to_type(cudynP_cuex_type); }

/*!True iff \a t is the type of dynamic objects and expressions. */
CU_SINLINE cu_bool_t cudyn_is_cuex_type(cudyn_type_t t)
{ return cudyn_stdtype_to_type(cudynP_cuex_type) == t; }

/*!\defgroup cudyn_ptrtype_mod cudyn_ptrtype_t: Pointer Types
 * @{ */

/* Pointer Types
 * ============= */

struct cudyn_ptrtype_s
{
    cu_inherit (cudyn_inltype_s);
};

/*!The type of pointer types. */
CU_SINLINE cudyn_type_t cudyn_ptrtype_type()
{ return cudyn_stdtype_to_type(cudynP_ptrtype_type); }

/*!True iff \a t is a \c cudyn_ptrtype_t. */
CU_SINLINE cu_bool_t cudyn_type_is_ptrtype(cudyn_type_t t)
{ return cudyn_type_typekind(t) == cudyn_typekind_ptrtype; }

#define cudyn_ptrtype_to_type(t) \
    cu_to3(cudyn_type, cudyn_hctype, cudyn_inltype, t)
#define cudyn_ptrtype_from_type(t) \
    cu_from3(cudyn_ptrtype, cudyn_inltype, cudyn_hctype, cudyn_type, t)

/*!Type of pointers to objects of \a deref. */
cudyn_ptrtype_t cudyn_ptrtype(cuex_t deref);

/*!A \c cudyn_type_t representation of \a e, which must have a toplevel
 * \ref CUEX_O1_PTR_TO. */
cudyn_ptrtype_t cudyn_ptrtype_from_ex(cuex_t e);

/*!@}
 * \defgroup cudyn_elmtype_mod cudyn_elmtype_t: Elementary Types
 * @{\ingroup cudyn_type_h_mod */

/* Elementary Types
 * ================ */

struct cudyn_elmtype_s
{
    cu_inherit (cudyn_inltype_s);
};

/*!The type of elementary types. */
CU_SINLINE cudyn_type_t cudyn_elmtype_type()
{ return cudyn_stdtype_to_type(cudynP_elmtype_type); }

/*!True iff \a t is a \c cudyn_elmtype_t. */
CU_SINLINE cu_bool_t cudyn_type_is_elmtype(cudyn_type_t t)
{ return cudyn_type_typekind(t) >= cudyn_typekind_elmtype_MIN; }

#define cudyn_elmtype_to_type(t) \
    cu_to3(cudyn_type, cudyn_hctype, cudyn_inltype, t)
#define cudyn_elmtype_from_type(t) \
    cu_from3(cudyn_elmtype, cudyn_inltype, cudyn_hctype, cudyn_type, t)

/*!Create a unique elementary type, of which objects are \a size bytes,
 * require \a alignment bytes alignement and FFI type \a ffitype. */
cudyn_elmtype_t cudyn_elmtype_new(cudyn_typekind_t kind,
				  cu_offset_t size, cu_offset_t alignment,
				  ffi_type *ffitype);


/*!@}
 * \defgroup cudyn_arrtype_mod cudyn_arrtype_t: Array Types
 * @{\ingroup cudyn_type_h_mod */

/* Array Types
 * =========== */

struct cudyn_arrtype_s
{
    cu_inherit (cudyn_inltype_s);
    size_t elt_cnt;
    cudyn_type_t elt_type;
};

/*!The type of array types. */
CU_SINLINE cudyn_type_t cudyn_arrtype_type()
{ return cudyn_stdtype_to_type(cudynP_arrtype_type); }

/*!True iff \a t is a \c cudyn_arrtype_t. */
CU_SINLINE cu_bool_t cudyn_type_is_arrtype(cudyn_type_t t)
{ return cudyn_type_typekind(t) == cudyn_typekind_arrtype; }

/*!True iff \a meta is a \c cudyn_arrtype_t. */
CU_SINLINE cu_bool_t cudyn_meta_is_arrtype(cuex_meta_t meta)
{ return cuex_meta_is_type(meta)
      && cudyn_type_is_arrtype(cudyn_type_from_meta(meta)); }

#define cudyn_arrtype_to_type(t) \
    cu_to3(cudyn_type, cudyn_hctype, cudyn_inltype, t)
#define cudyn_arrtype_from_type(t) \
    cu_from3(cudyn_arrtype, cudyn_inltype, cudyn_hctype, cudyn_type, t)
#define cudyn_arrtype_from_meta(meta) \
    cudyn_arrtype_from_type(cudyn_type_from_meta(meta))

/*!The dynamic representation of \a elt_type ↑ \a elt_cnt. */
cudyn_arrtype_t cudyn_arrtype(cudyn_type_t elt_type, size_t elt_cnt);

/*!The type of the elements of arrays of type \a t. */
CU_SINLINE cudyn_type_t cudyn_arrtype_elt_type(cudyn_arrtype_t t)
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
    cudyn_type_t type;
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
CU_SINLINE cudyn_type_t cudyn_tuptype_type()
{ return cudyn_stdtype_to_type(cudynP_tuptype_type); }

/*!True iff \a t is a cudyn_tuptype_t. */
CU_SINLINE cu_bool_t cudyn_type_is_tuptype(cudyn_type_t t)
{ return cudyn_type_typekind(t) == cudyn_typekind_tuptype; }

#define cudyn_tuptype_to_type(t) \
    cu_to3(cudyn_type, cudyn_hctype, cudyn_inltype, t)
#define cudyn_tuptype_from_type(t) \
    cu_from3(cudyn_tuptype, cudyn_inltype, cudyn_hctype, cudyn_type, t)

cudyn_tuptype_t cudyn_tuptype_glck(cuex_t typeex);

cudyn_tuptype_t cudyn_tuptype(cuex_t typeex);

cudyn_tuptype_t cudyn_tuptype_by_valist(cu_offset_t cnt, va_list vl);

CU_SINLINE cu_offset_t cudyn_tuptype_tcomp_cnt(cudyn_tuptype_t t)
{ return t->tcomp_cnt; }

CU_SINLINE cudyn_type_t cudyn_tuptype_at(cudyn_tuptype_t t, cu_offset_t i)
{ return t->tcomp_arr[i].type; }

cu_bool_t
cudyn_tuptype_conj(cudyn_tuptype_t t,
		   cu_clop(cb, cu_bool_t, cu_idr_t label, cu_offset_t bitoff,
					  cudyn_type_t type));

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


/* Singular Types
 * ============== */

struct cudyn_sngtype_s
{
    cu_inherit (cudyn_inltype_s);
};

extern cudyn_stdtype_t cudynP_sngtype_type;
extern cudyn_sngtype_t cudynP_default_sngtype;

CU_SINLINE cudyn_type_t cudyn_sngtype_type()
{ return cudyn_stdtype_to_type(cudynP_sngtype_type); }

#define cudyn_sngtype_to_type(t) \
    cu_to3(cudyn_type, cudyn_hctype, cudyn_inltype, t)
#define cudyn_sngtype_from_type(t) \
    cu_from3(cudyn_sngtype, cudyn_inltype, cudyn_hctype, cudyn_type, t)

/*!True iff \a ex is the singular type. */
CU_SINLINE cu_bool_t cudyn_type_is_sngtype(cudyn_type_t t)
{ return cudyn_type_typekind(t) == cudyn_typekind_sngtype; }

cudyn_sngtype_t cudyn_sngtype_of_elt(cuex_t obj);

CU_SINLINE cudyn_sngtype_t cudyn_sngtype_default()
{ return cudynP_default_sngtype; }

CU_SINLINE cuex_t cudyn_sngtype_elt(cudyn_sngtype_t t)
{ return cuex_aci_at(cudyn_type_as_expr(cudyn_sngtype_to_type(t)), 0); }

/*!\defgroup cudyn_duntype_mod cudyn_duntype_t: Discriminated Unions
 * @{ */

/* Union Types
 * =========== */

typedef uintptr_t cudyn_cnum_t;

struct cudyn_dunpart_s
{
    cudyn_cnum_t cnum;
    cudyn_type_t type;
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
CU_SINLINE cudyn_type_t cudyn_duntype_type()
{ return cudyn_stdtype_to_type(cudynP_duntype_type); }

/*!True iff \a t is a cudyn_duntype_t. */
CU_SINLINE cu_bool_t cudyn_type_is_duntype(cudyn_type_t t)
{ return cudyn_type_typekind(t) == cudyn_typekind_duntype; }

#define cudyn_duntype_to_type(t) \
    cu_to3(cudyn_type, cudyn_hctype, cudyn_inltype, t)
#define cudyn_duntype_from_type(t) \
    cu_from3(cudyn_duntype, cudyn_inltype_s, cudyn_hctype, cudyn_type, t)

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
cudyn_type_t cudyn_duntype_at(cudyn_duntype_t u, cudyn_cnum_t i);

/*!Sequential conjunction of \a cb over the partitions of \a u. */
cu_bool_t
cudyn_duntype_conj(cudyn_duntype_t u,
		   cu_clop(cb, cu_bool_t, cudyn_cnum_t, cudyn_type_t));

/*!@}*/


cudyn_type_t cudyn_type_glck(cuex_t ex);

/*!Returns \a ex interpreted as a type, or \c NULL if not syntactically
 * correct. */
cudyn_type_t cudyn_type(cuex_t ex);

CU_SINLINE cu_bool_t
cudyn_type_is_singular(cudyn_type_t t)
{
    return cudyn_type_is_sngtype(t);
}

CU_SINLINE cudyn_type_t
cudyn_singular_type()
{
    return cudyn_sngtype_to_type(cudynP_default_sngtype);
}

/*!The memory layout of \a type. */
CU_SINLINE cucon_layout_t
cudyn_type_layout(cudyn_type_t type)
{
    if (cudyn_type_is_singular(type))
	return cucon_layout_void();
    else if (cudyn_type_is_nonptr_inltype(type))
	return (cucon_layout_t)cu_from2(cudyn_inltype, cudyn_hctype,
					cudyn_type, type)->layout;
    else
	return cucon_layout_ptr();
}

/*!The size in bits of objects of type \a t. */
CU_SINLINE size_t
cudyn_type_bitsize(cudyn_type_t t)
{
    return cucon_layout_bitsize(cudyn_type_layout(t));
}

/*!The size of object of type \a t. */
CU_SINLINE size_t
cudyn_type_size(cudyn_type_t t)
{
    return cucon_layout_size(cudyn_type_layout(t));
}

CU_SINLINE size_t
cudyn_type_bitalign(cudyn_type_t t)
{
    return cucon_layout_bitalign(cudyn_type_layout(t));
}

CU_SINLINE size_t
cudyn_type_align(cudyn_type_t t)
{
    return cucon_layout_align(cudyn_type_layout(t));
}

/*!@}*/
CU_END_DECLARATIONS

#endif
