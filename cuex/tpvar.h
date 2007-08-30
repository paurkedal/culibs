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

#ifndef CUEX_TPVAR_H
#define CUEX_TPVAR_H

#include <cuex/fwd.h>
#include <cuex/var.h>
#include <cu/int.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_tpvar_h cuex/tpvar.h: Parametric Type Variables
 * @{\ingroup cuex_mod
 * Type parameter variables meant for representing Hindley-Milner style type
 * parameters with size information and optional links to type classes. They
 * are hash-consed based on an index (and two more arguments), so as to
 * facilitate cannonical representations of parametric expressions.
 *
 * A typical use for this is to represent definitions of the form
 * <pre>
 * α list = nil | cons of α × α list   (a type)
 * map ∈ (α → β → β) → α list → β → β  (a function)
 * </pre>
 * where the greek letters denote universally quantised type parameters.
 *
 * These variables have an associated width mask, which stores all possible
 * widths of the parametric type for which the definition shall be
 * instantiated. Bit number \e n indicates whether the width 2↑\e n is
 * supported by the definition. It is implied here that the caller will cast
 * values of a given width to the closest bigger supported with, or if none is
 * available, pass the value as a pointer. Therefore, the bit corresponding to
 * the width of a pointer (bit 5 or 6 for 32 or 64 bit architectures,
 * respectively) should always be set. It is further implied that a parametric
 * function only deals with objects of a parametric type opaquely, which is
 * the case for ML-style parametric functions.
 *
 * A way to go beyond the opaque treatment of parametrically typed objects, is
 * to introduce type-classes. Therefore, a final type class argument is passed
 * to the hash-constructor. We make no assumption about how this is
 * represented except that it must be an expression to allow client code to
 * check its type at runtime. You can embed any structure in an expression,
 * just make an opaque type for it.
 *
 * A way to achieve a cannonical representation is to start indexing from the
 * innermost expressions, re-using indexes for parallel definitions, and using
 * higher indices for enclosing expressions. */

#define cuex_tpvarmeta(index)						\
    cuex_varmeta_kqis(cuex_varkind_tpvar, cuex_qcode_u, 0,		\
		      (sizeof(struct cuex_tpvar_s) - CUOO_HCOBJ_SHIFT)	\
		        / sizeof(cu_word_t))
#define cuex_is_tpvarmeta(meta)						\
    cuex_is_varmeta_k(meta, cuex_varkind_tpvar)

struct cuex_tpvar_s
{
    CUOO_HCOBJ
    cu_offset_t width_mask;
    cuex_t type_class;
};

/*!Hash-cons a variable based on the arguments. If \a type_class is unused,
 * set it to \c NULL. */
cuex_tpvar_t cuex_tpvar(int index, cu_offset_t width_mask, cuex_t type_class);

/*!The index of \a alpha. */
CU_SINLINE int cuex_tpvar_index(cuex_tpvar_t alpha)
{ return cuex_varmeta_index(cuex_meta(alpha)); }

/*!The width mask of \a alpha. For each bit set, the quantified definition
 * shall be specialised for the width 2↑\e n where \e n is the bit number. */
CU_SINLINE cu_offset_t cuex_tpvar_width_mask(cuex_tpvar_t alpha)
{ return alpha->width_mask; }

/*!The number of widths for which this type parameter shall be specialised. */
CU_SINLINE int cuex_tpvar_width_count(cuex_tpvar_t alpha)
{ return CU_OFFSET_NAME(cu_, bit_count)(alpha->width_mask); }

/*!The type class of \a alpha, or \c NULL if none. */
CU_SINLINE cuex_t cuex_tpvar_type_class(cuex_tpvar_t alpha)
{ return alpha->type_class; }

/*!@}*/
CU_END_DECLARATIONS

#endif
