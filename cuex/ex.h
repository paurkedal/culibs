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

#ifndef CUEX_EX_H
#define CUEX_EX_H

#include <cuex/fwd.h>
#include <cuoo/meta.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_ex cuex/ex.h: Expressions
 * @{ \ingroup cuex_mod */

/*!Create an operator for \c cuex_opn_t with arity \a arity and identified
 * by (\a index, \a arity). */
#define cuex_opr(index, arity)						\
    (((cuex_meta_t)(index) << CUEX_OPR_SELECT_SHIFT) |			\
     ((cuex_meta_t)(arity) << CUEX_OPR_ARITY_SHIFT) | 1)
#define cuex_opr_with_ctor(index, arity)				\
    (cuex_opr(index, arity) | CUEX_OPRFLAG_CTOR)
#define cuex_opr_index(opr)						\
    (((opr) & CUEX_OPR_SELECT_MASK) >> CUEX_OPR_SELECT_SHIFT)
#define cuex_opr_sans_arity(opr)	((opr) & ~CUEX_OPR_ARITY_MASK)
#define cuex_opr_flags(opr)		((opr) & CUEX_OPR_FLAGS_MASK)
#define cuex_opr_sans_flags(opr)	((opr) & ~CUEX_OPR_FLAGS_MASK)
#define cuex_opr_has_ctor(opr)		((opr) & CUEX_OPRFLAG_CTOR)

/*!Placement for unspecified operator, e.g. as return value.  The unique
 * \c cuex_opn_t expression of this operator can be used as a placement
 * expression.  As opposed to \c NULL, this null-expression can be handled
 * gracefully by expression-traversing functions without special attention,
 * and shall therefore be used instead of \c NULL in sub-expressions. */
#define cuex_opr_null	cuex_opr(0, 0)

/*!Placement for an unknown expression. */
#define CUEX_O0_UNKNOWN cuex_opr(1, 0)

extern cuex_t cuexP_null;
extern cuex_t cuexP_unknown;

#define cuex_opn_to_ex(opn) ((cuex_t)(opn))
#define cuex_opn_from_ex(ex) ((cuex_opn_t)(ex))

/*!True iff \a ex is an operation. */
CU_SINLINE cu_bool_t	cuex_is_opn(cuex_t ex)
{ return cuex_meta_kind(cuex_meta(ex)) == cuex_meta_kind_opr; }

/*!True iff \a ex is a dynamic object. */
CU_SINLINE cu_bool_t	cuex_is_obj(cuex_t ex)
{ return cuex_meta_kind(cuex_meta(ex)) == cuex_meta_kind_type; }

/*!The arity of \a opn. */
CU_SINLINE cu_rank_t	cuex_opn_r(cuex_opn_t opn)
{ return cuex_opr_r(cuex_meta(opn)); }

/*!Returns the designated null expression. */
CU_SINLINE cuex_t	cuex_null() { return cuexP_null; }

/*!True iff \a ex is the null expression. */
CU_SINLINE cu_bool_t	cuex_is_null(cuex_t ex)
{ return cuex_meta(ex) == cuex_opr_null; }

/*!Returns the designated placement for unknown expressions. */
CU_SINLINE cuex_t	cuex_unknown() { return cuexP_unknown; }

/*!True iff \a ex is unknown. */
CU_SINLINE cu_bool_t	cuex_is_unknown(cuex_t ex)
{ return cuex_meta(ex) == CUEX_O0_UNKNOWN; }

/*!True iff \a ex0 and \a ex1 are structurally equal.  This is trivial
 * for hash-consed expressions.  (Non-hash-consed expressions are not
 * implemented.) */
CU_SINLINE cu_bool_t	cuex_eq(cuex_t ex0, cuex_t ex1) { return ex0 == ex1; }

/*!An ordering classification for expressions used as keys e.g. in ACI
 * expressons, which returns negative for ‘<’, 0 for ‘=’ and positive for ‘>’.
 * Note that the range may not by constrained to {-1, 0, 1}.  The ordering is
 * not persistent accross processes.  In the future, persistent ordering may
 * be implemented for identifiers. */
CU_SINLINE int		cuex_cmp(cuex_t lhs, cuex_t rhs)
{ return lhs < rhs ? -1 : lhs == rhs ? 0 : 1; }

/*!True iff \a lhs ≤ \a rhs according to \ref cuex_cmp. */
CU_SINLINE cu_bool_t	cuex_leq(cuex_t lhs, cuex_t rhs)
{ return cuex_cmp(lhs, rhs) <= 0; }

/*!True iff \a lhs < \a rhs according to \ref cuex_cmp. */
CU_SINLINE cu_bool_t	cuex_lt(cuex_t lhs, cuex_t rhs)
{ return cuex_cmp(lhs, rhs) < 0; }

/*!True iff \a lhs ≥ \a rhs acconding to \ref cuex_cmp. */
CU_SINLINE cu_bool_t	cuex_geq(cuex_t lhs, cuex_t rhs)
{ return cuex_cmp(lhs, rhs) >= 0; }

/*!True iff \a lhs > \a rhs according to \ref cuex_cmp. */
CU_SINLINE cu_bool_t	cuex_gt(cuex_t lhs, cuex_t rhs)
{ return cuex_cmp(lhs, rhs) > 0; }

/*!The minimum of \a lhs and \a rhs according to \ref cuex_cmp. */
CU_SINLINE cuex_t	cuex_min(cuex_t lhs, cuex_t rhs)
{ return cuex_leq(lhs, rhs)? lhs : rhs; }

/*!The maximum of \a lhs and \a rhs according to \ref cuex_cmp. */
CU_SINLINE cuex_t	cuex_max(cuex_t lhs, cuex_t rhs)
{ return cuex_leq(lhs, rhs)? rhs : lhs; }

/*!Turns \a ex into a hash-consed representation.  This is trivial for
 * already hash-consed expressions.  (Non-hash-consed expressions are not
 * implemented.) */
CU_SINLINE cuex_t	cuex_uniq(cuex_t ex) { return ex; }

/*!@}*/
CU_END_DECLARATIONS

#endif
