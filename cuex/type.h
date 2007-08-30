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

#ifndef CUEX_TYPE_H
#define CUEX_TYPE_H

#include <cuex/fwd.h>
#include <cuex/oprdefs.h>
#include <cudyn/type.h>
#include <cuoo/halloc.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_type_h cuex/type.h: Dynamic Type Support
 * @{\ingroup cuex_mod */

size_t cuex_type_bitsize(cuex_t type);

CU_SINLINE size_t cuex_type_size(cuex_t type)
{
    size_t s = cuex_type_bitsize(type);
    return (s + 7)/8;
}

cu_offset_t cuex_type_bitalign(cuex_t type);

CU_SINLINE cu_offset_t cuex_type_align(cuex_t type)
{
    cu_offset_t a = cuex_type_bitalign(type);
    return (a + 7)/8;
}

cu_rank_t cuex_farrow_arity(cuex_t type);

cuex_t cuex_typeof(cuex_t e);


/* Pointers
 * -------- */

extern cuex_t cuexP_generic_ptr_type;

/*!A generic pointer type. This is a \ref CUEX_O1_PTR_TO. */
CU_SINLINE cuex_t cuex_generic_ptr_type(void)
{ return cuexP_generic_ptr_type; }

/*!True iff \a e is \ref cuex_generic_ptr_type(). */
CU_SINLINE cu_bool_t cuex_is_generic_ptr_type(cuex_t e)
{ return e == cuexP_generic_ptr_type; }

/*!Returns \a fn as an expression of type \a t, which should be the type of \a
 * fn, expressed as a \ref CUEX_O2_FARROW_NATIVE with the first operand
 * being the argument types as a left-associated tree of \ref
 * CUEX_O2_GPROD, and the second operand being the result type. E.g.
 * \code
 * cuex_t t = cuex_o2_farrow_native(cuex_o2_gprod(cudyn_double_type(), cudyn_double_type()), cudyn_double_type());
 * cuex_t e = cuex_of_fnptr(t, (cu_fnptr_t)atan2);
 * \endcode
 */
CU_SINLINE cuex_t
cuex_of_fnptr(cuex_t t, cu_fnptr_t fn)
{
    cu_debug_assert(cuex_meta(t) == CUEX_O2_FARROW_NATIVE
		    || cuex_meta(t) == CUEX_O2_FORALL);
    return cuoo_halloc(cuoo_type(t), sizeof(void *), &fn);
}

CU_SINLINE cu_bool_t
cuex_is_fnptr(cuex_t t)
{
    cuex_meta_t m = cuex_meta(t);
    return cuex_meta_is_type(m)
	&& cuex_meta(cuoo_type_as_expr(cuoo_type_from_meta(m)))
	   == CUEX_O2_FARROW_NATIVE;
}

CU_SINLINE cu_fnptr_t
cuex_to_fnptr(cuex_t e)
{
    return *(cu_fnptr_t *)e;
}

/*!@}*/
CU_END_DECLARATIONS

#endif
