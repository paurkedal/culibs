/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2009  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUEX_ITERATION_H
#define CUEX_ITERATION_H

#include <cuex/fwd.h>
#include <cu/clos.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_iteration_h cuex/iteration.h: Iteration over Subterms
 *@{\ingroup cuex_mod */

#if defined(CUCONF_PARALLELIZE) || defined(CU_IN_DOXYGEN)

/*!True iff \a f takes every immediate subexpression of \a e to true.  This is
 * the same as \ref cuex_iterA_operands, but with unordered invocation, and the
 * additional requirement that \a f is safe for parallel invocation.  */
cu_bool_t cuex_A_operands(cu_clop(f, cu_bool_t, cuex_t), cuex_t e);

/*!True iff \a f takes some immediate subexpression of \a e to true.  This is
 * the same as \ref cuex_iterE_operand, but with unordered invocation, and the
 * additional requirement that \a f is safe for parallel invocation. */
cu_bool_t cuex_E_operand(cu_clop(f, cu_bool_t, cuex_t), cuex_t e);

#else
#  define cuex_A_operands	cuex_iterA_operands
#  define cuex_E_operand	cuex_iterE_operand
#endif

/*!A variant of \ref cuex_A_operands specialised for context-free callbacks. */
cu_bool_t cuex_bareA_operands(cu_bool_t (*f)(cuex_t), cuex_t e);

/*!A variant of \ref cuex_E_operand specialised for context-free callbacks. */
cu_bool_t cuex_bareE_operand(cu_bool_t (*f)(cuex_t), cuex_t e);

/*!Calls \a f on each immediate subexpression of \a e in operand order. */
void cuex_iter_operands(cu_clop(f, void, cuex_t), cuex_t e);

/*!Calls \a f on each subexpression of \a e in operand order and builds the
 * conjunction of the results, exiting as soon as \a f returns false. */
cu_bool_t cuex_iterA_operands(cu_clop(f, cu_bool_t, cuex_t), cuex_t e);

/*!Calls \a f on each subexpression of \a e in operand order and builds the
 * disjunction of the results, exiting as soon as \a f returns true. */
cu_bool_t cuex_iterE_operand(cu_clop(f, cu_bool_t, cuex_t), cuex_t e);

#if defined(CUCONF_PARALLELIZE) || defined(CU_IN_DOXYGEN)

/*!Returns the result of replacing each immediate subexpression of \a e with
 * its mapping under \a f.  This is the same as \ref cuex_iterimg_operands, but
 * with unordered invocation, and the additional requirement that \a f is safe
 * for parallel invocation. */
cuex_t cuex_img_operands(cu_clop(f, cuex_t, cuex_t), cuex_t e);

#else
#  define cuex_img_operands cuex_iterimg_operands
#endif

/*!A variant of \ref cuex_img_operands specialised for context-free callbacks. */
cuex_t cuex_bareimg_operands(cuex_t (*f)(cuex_t), cuex_t e);

/*!Returns the result of replacing each immediate subexpression of \a e with
 * its mapping under \a f, where \a f is called in operand order.  If \a f
 * returns \c NULL, this function immediately returns \c NULL, as well. */
cuex_t cuex_iterimg_operands(cu_clop(f, cuex_t, cuex_t), cuex_t e);

/*!@}*/
CU_END_DECLARATIONS

#endif
