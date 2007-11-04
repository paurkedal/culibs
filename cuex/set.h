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

#ifndef CUEX_SET_H
#define CUEX_SET_H

#include <cuex/fwd.h>
#include <cuoo/type.h>
#include <cu/algo.h>

CU_BEGIN_DECLARATIONS
extern cuoo_stdtype_t cuexP_set_type;
extern cuex_t cuexP_set_empty;

/*!\defgroup cuex_set_h cuex/set.h: Set-Like Terms
 *@{\ingroup cuex_mod
 * \see cuex_semilattice_h
 * \see cuex_labelling_h
 * \see cuex_atree_h
 */

/*!The type of set terms. */
CU_SINLINE cuoo_type_t
cuex_set_type()
{ return cuoo_stdtype_to_type(cuexP_set_type); }

/*!The empty set term. */
CU_SINLINE cuex_t
cuex_empty_set(void)
{ return cuexP_set_empty; }

/*!True iff \a S is the empty set term. */
CU_SINLINE cu_bool_t
cuex_is_empty_set(cuex_t S)
{ return S == cuexP_set_empty; }

/*!Returns the singleton set {\a e}. */
cuex_t cuex_singleton_set(cuex_t e);

/*!Returns \a S ∪ \a e. */
cuex_t cuex_set_insert(cuex_t S, cuex_t e);

/*!Returns \a S ∖ \a e. */
cuex_t cuex_set_erase(cuex_t S, cuex_t e);

/*!Returns \a S0 ∪ \a S1. */
cuex_t cuex_set_union(cuex_t S0, cuex_t S1);

/*!Returns \a S0 ∩ \a S1. */
cuex_t cuex_set_isecn(cuex_t S0, cuex_t S1);

/*!True iff \a e ∈ \a S. */
cu_bool_t cuex_set_contains(cuex_t S, cuex_t e);

/*!True iff \a S0 ⊆ \a S1. */
cu_bool_t cuex_set_subeq(cuex_t S0, cuex_t S1);

/*!Return the most strict ordering predicate \e R such that \a S0 \e R \a S1. */
cu_order_t cuex_set_order(cuex_t S0, cuex_t S1);

/*!@}*/
CU_END_DECLARATIONS

#endif
