/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2002--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CHI_CFN_H
#define CHI_CFN_H

#include <cudyn/type.h>
#ifdef CUCONF_HAVE_LIBFFI_FFI_H
#  include <libffi/ffi.h>
#else
#  include <ffi.h>
#endif
#include <atomic_ops.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cudyn_proto_h cudyn/proto.h: Dynamic Prototypes and FFI Calls
 * @{ \ingroup cudyn_mod */

/* The class of C functions
 * ------------------------ */
struct cudyn_proto_s
{
    cu_inherit (cuoo_type_s);

    cudyn_tuptype_t arg_type;
    cuoo_type_t res_type;

    /* These are not hash consed */
    ffi_cif cif;
    /* ffi_type args[arity] */
};
#define CUDYN_PROTO_KEY_SIZE \
	(offsetof(struct cudyn_proto_s, cif) - CUOO_HCOBJ_SHIFT)

extern cuoo_type_t cudynP_proto_type;

/*!The type of all prototypes. */
CU_SINLINE cuoo_type_t cudyn_proto_type()
{ return cudynP_proto_type; }

/*!Cast to base struct. */
CU_SINLINE cuoo_type_t cudyn_proto_to_type(cudyn_proto_t proto)
{ return cu_to(cuoo_type, proto); }

/*!Cast from base struct. */
CU_SINLINE cudyn_proto_t cudyn_proto_from_type(cuoo_type_t t)
{ return cu_from(cudyn_proto, cuoo_type, t); }

/*!True if \a ex points to a prototype. */
CU_SINLINE cu_bool_t cudyn_is_fn(cuex_t ex)
{ return cuex_meta(ex) == cuoo_type_to_meta(cudyn_proto_type()); }

/*!Return the prototype for \a arg_type → \a res_type */
cudyn_proto_t cudyn_proto_by_tuptype(cudyn_tuptype_t arg_type,
				     cuoo_type_t res_type);

/*!Constructor of prototypes of any arity.  The varargs are the
 * \c cuoo_type_t for the argument types, followed by that of the result
 * type. */
cudyn_proto_t cudyn_proto(cu_rank_t r, ...);

/*!Type-safe variant of \ref cudyn_proto. */
CU_SINLINE cudyn_proto_t
cudyn_proto_0(cuoo_type_t tR)
{ return cudyn_proto(0, tR); }

/*!\copydoc cudyn_proto_0 */
CU_SINLINE cudyn_proto_t
cudyn_proto_1(cuoo_type_t t0, cuoo_type_t tR)
{ return cudyn_proto(1, t0, tR); }

/*!\copydoc cudyn_proto_0 */
CU_SINLINE cudyn_proto_t
cudyn_proto_2(cuoo_type_t t0, cuoo_type_t t1, cuoo_type_t tR)
{ return cudyn_proto(2, t0, t1, tR); }

/*!\copydoc cudyn_proto_0 */
CU_SINLINE cudyn_proto_t
cudyn_proto_3(cuoo_type_t t0, cuoo_type_t t1, cuoo_type_t t2,
	      cuoo_type_t tR)
{ return cudyn_proto(3, t0, t1, t2, tR); }

/*!\copydoc cudyn_proto_0 */
CU_SINLINE cudyn_proto_t
cudyn_proto_4(cuoo_type_t t0, cuoo_type_t t1, cuoo_type_t t2,
	      cuoo_type_t t3, cuoo_type_t tR)
{ return cudyn_proto(4, t0, t1, t2, t3, tR); }

/*!Return the arity of \a proto. */
CU_SINLINE size_t
cudyn_proto_r(cudyn_proto_t proto)
{ return cudyn_tuptype_tcomp_cnt(proto->arg_type); }

/* True iff 'proto0 ⊆ proto1'.  That is, if 'proto0' is more special
 * than 'proto1'. */
//cu_bool_t cudyn_proto_subeq(cudyn_proto_t proto0, cudyn_proto_t proto1);

/*!Apply \a fn to arguments in \a arg_arr, assuming that \a fn has prototype
 * \a proto, and wrap the result in a dynamic type capsule.  */
cuex_t cudyn_proto_apply_fn(cudyn_proto_t proto, cu_fnptr_t fn,
			    cuex_t *arg_arr);
//cuex_t cudyn_proto_apply_fnc(cudyn_proto_t proto, cu_clptr_t fnc,
//			     cuex_t *arg_arr);

/*!The type of argument number \a i of \a proto. */
CU_SINLINE cuoo_type_t
cudyn_proto_arg_type_at(cudyn_proto_t proto, cu_rank_t i)
{ return cudyn_tuptype_at(proto->arg_type, i); }

/*!The result type part of \a proto. */
CU_SINLINE cuoo_type_t cudyn_proto_res_type(cudyn_proto_t proto)
{ return proto->res_type; }

/*!@}*/
CU_END_DECLARATIONS
#endif
