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

#ifndef CUEX_RECURSION_H
#define CUEX_RECURSION_H

#include <cuex/fwd.h>
#include <cuex/var.h>
#include <cuex/binding.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_recursion_h cuex/recursion.h: Functions on the Recursive Structure of Expressions
 *@{\ingroup cuex_mod */

/*!Unfold the μ-expression at the top-level of \a e.  That is, assuming \a e
 * = \e μx \e e′, returns \e e′[x ↦ \e μx \e e′].
 * \pre \e must have a top-level μ-bind. */
cuex_t cuex_mu_unfold(cuex_t e);

/*!Minimise \a e by μ-folding it as much as possible.  This is done using a
 * modified version of Hopcroft's algorithm for minimising a finite
 * automaton. */
cuex_t cuex_mu_minimise(cuex_t e);

cuex_t cuex_optimal_fold(cuex_t e);

/*!@}*/
CU_END_DECLARATIONS

#endif
