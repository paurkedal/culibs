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

#ifndef CUEX_DUNION_H
#define CUEX_DUNION_H

#include <cuex/fwd.h>
#include <cuex/oprdefs.h>
#include <cuex/aci.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_dunion_h cuex/dunion.h: Discriminated Unions
 * @{\ingroup cuex_mod
 * This header provides supporting functions for discriminated unions
 * represented as \ref cuex_aci_h "ACI expressions" of the \ref
 * CUEX_O4ACI_DUNION operator.  The keys of the ACI trees are labels of
 * type \c cu_idr_t, and the next operand of the ACI nodes are expressions,
 * typically interpreted as types or sets.
 *
 * The \ref CUEX_O4ACI_DUNION operator has a constructor which keeps track
 * of the number of nodes in the ACI tree.  This is unilized in \ref
 * cuex_dunion_find to provied a unique integer for each term in the union.
 * In other words, these integers can be used to discriminate the type rather
 * than the identifiers.  */

extern cuex_opn_t cuexP_dunion_empty;

/*!Returns ∅. */
CU_SINLINE cuex_opn_t cuex_dunion_empty(void) { return cuexP_dunion_empty; }

/*!True iff \a U = ∅. */
CU_SINLINE cu_bool_t cuex_is_dunion_empty(cuex_opn_t U)
{ return U == cuexP_dunion_empty; }

/*!Returns {\a label} × \a type. */
CU_SINLINE cuex_opn_t cuex_dunion_generator(cu_idr_t label, cuex_t type)
{ return cuex_aci_generator(CUEX_O4ACI_DUNION, label, type); }

/*!Returns \a lhs ∪ \a rhs. */
CU_SINLINE cuex_opn_t
cuex_dunion(cuex_opn_t lhs, cuex_opn_t rhs)
{
    cu_debug_assert(cuex_is_aci(CUEX_O4ACI_DUNION, lhs));
    cu_debug_assert(cuex_is_aci(CUEX_O4ACI_DUNION, rhs));
    return cuex_aci_join(CUEX_O4ACI_DUNION, lhs, rhs);
}

/*!Returns \a lhs ∪ {\a rhs_label} × \a rhs_type. */
CU_SINLINE cuex_opn_t
cuex_dunion_insert(cuex_opn_t lhs, cu_idr_t rhs_label, cuex_t rhs_type)
{
    cu_debug_assert(cuex_is_aci(CUEX_O4ACI_DUNION, lhs));
    return cuex_aci_insert(CUEX_O4ACI_DUNION, lhs, rhs_label, rhs_type);
}

/*!If {\a label} × \e type ⊆ \a U for some \e type, then store \a type in
 * <tt>*</tt>\a type_out , store the enumerator of \a label in <tt>*</tt>\a
 * index_out and return true.  Otherwise return false. */
cu_bool_t cuex_dunion_find(cuex_opn_t U, cu_idr_t label,
			   cuex_t *type_out, int *index_out);

/*!@}*/
CU_END_DECLARATIONS

#endif
