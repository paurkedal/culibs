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

#ifndef CUEX_TYPEVAR_H
#define CUEX_TYPEVAR_H

#include <cuex/fwd.h>
#include <cuex/var.h>
#include <atomic_ops.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_tvar_h cuex/tvar.h: Typed and Type-Representing Variables
 * @{\ingroup cuex_mod
 * A type variable represents an unspecified type which may be part of
 * compound types, and it can itself have a type, which may be another
 * type variable.  It is meant for parametric typing and represents
 * equality constraints for type components holding the same variable,
 * as opposed to using a generic type like <tt>void *</tt>.
 *
 * It may be desireable to substitute the type variables with the correct
 * type before use, if they are known.  Otherwise, type variables are
 * assumed to be the size of a pointer for storage calculations.  So,
 * a construct containing type variables can be used directly if bigger
 * objects are stored by reference. */

#define cuex_tvarmeta(qc) cuex_varmeta_kqi(cuex_varkind_tvar, qc, 0)

CU_SINLINE cu_bool_t cuex_is_tvarmeta_q(cuex_meta_t meta, cuex_qcode_t qc)
{ return meta == cuex_tvarmeta(qc); }

CU_SINLINE cu_bool_t cuex_is_tvarmeta(cuex_meta_t meta)
{ return cuex_is_varmeta_k(meta, cuex_varkind_tvar); }


struct cuex_tvar_s
{
    cu_inherit (cudyn_type_s);
    AO_t type;
};

/*!True iff \a ex is a \c cuex_tvar_t. */
CU_SINLINE cu_bool_t cuex_is_tvar(cuex_t ex)
{ return cuex_is_tvarmeta(cuex_meta(ex)); }

/*!True iff \a tv represents a type. */
CU_SINLINE cu_bool_t cuex_tvar_is_type(cuex_tvar_t tv)
{ return cudyn_type_typekind(cu_to(cudyn_type, tv)) != cudyn_typekind_none; }

/*!Cast \a tv to a type. */
CU_SINLINE cudyn_type_t cuex_tvar_to_type(cuex_tvar_t tv)
{ return cu_to(cudyn_type, tv); }

/*!Unchecked cast from \a t to a type variable.  Use \ref cuex_is_tvar
 * first. */
CU_SINLINE cuex_tvar_t cuex_tvar_from_type(cudyn_type_t t)
{ return cu_from(cuex_tvar, cudyn_type, t); }

/*!Cast \a tv to a variable. */
CU_SINLINE cuex_var_t cuex_tvar_to_var(cuex_tvar_t tv)
{ return (cuex_var_t)tv; }

/*!Unchecked cast from \a v to a type variable.  Use \ref cuex_is_tvar
 * first. */
CU_SINLINE cuex_tvar_t cuex_tvar_from_var(cuex_var_t v)
{ return (cuex_tvar_t)v; }

/*!Create a type variable which represents a type of typeclass \a type. */
cuex_tvar_t cuex_tvar_new_type(cuex_qcode_t qc, cudyn_type_t type);

/*!Create a variable which represents an object of type \a type. */
cuex_tvar_t cuex_tvar_new_obj(cuex_qcode_t qc, cudyn_type_t type);

/*!Create a variable of the same kind, type and quantisation as \a v. */
cuex_tvar_t cuex_tvar_new_as(cuex_tvar_t v);

/*!Create an untyped universal type variable. */
CU_SINLINE cuex_tvar_t cuex_tvar_new_utype(void)
{ return cuex_tvar_new_type(cuex_qcode_u, NULL); }

/*!Create an untyped existential type variable. */
CU_SINLINE cuex_tvar_t cuex_tvar_new_etype(void)
{ return cuex_tvar_new_type(cuex_qcode_e, NULL); }

/*!Create an untyped universal variable. */
CU_SINLINE cuex_tvar_t cuex_tvar_new_uobj(void)
{ return cuex_tvar_new_obj(cuex_qcode_u, NULL); }

/*!Create an untyped existential variable. */
CU_SINLINE cuex_tvar_t cuex_tvar_new_eobj(void)
{ return cuex_tvar_new_obj(cuex_qcode_e, NULL); }

/*!Return the type of \a tv or \c NULL if untyped. */
CU_SINLINE cudyn_type_t
cuex_tvar_type(cuex_tvar_t tv) { return (cudyn_type_t)tv->type; }

CU_SINLINE void
cuex_tvar_set_type(cuex_tvar_t tv, cuex_t type)
{
    /* This may not be sufficient MT safety for most cases. */
    AO_store(&tv->type, (AO_t)type);
}

/*!Return the type of \a tv, creating a new universal type variable for it if
 * \a tv is untyped. */
CU_SINLINE cudyn_type_t
cuex_tvar_force_utype(cuex_tvar_t tv)
{
    if (!tv->type)
	AO_compare_and_swap(&tv->type, 0, (AO_t)cuex_tvar_new_utype());
    return (cudyn_type_t)tv->type;
}

/*!@}*/
CU_END_DECLARATIONS

#endif
