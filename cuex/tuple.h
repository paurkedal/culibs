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

#ifndef CUEX_TUPLE_H
#define CUEX_TUPLE_H

#include <cuex/fwd.h>
#include <cuex/oprdefs.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_tuple_h cuex/tuple.h: Support Functions for Variable Length Tuples
 * @{\ingroup cuex_mod
 * These are helper functions for expressions of \ref CUEX_OR_TUPLE. It's
 * safe to process the operations directly, and things that are easy to do
 * with the generic expression functionality is intentionally left out.
 */

CU_SINLINE cu_bool_t cuex_opr_is_tuple(cuex_meta_t opr)
{ return cuex_opr_sans_arity(opr) == CUEX_OR_TUPLE(0); }

/*!True iff \a e is a variable-length tuple. */
CU_SINLINE cu_bool_t cuex_is_tuple(cuex_t e)
{ return cuex_opr_sans_arity(cuex_meta(e)) == CUEX_OR_TUPLE(0); }

/*!The arity of \a e. Same as the operator arity. */
CU_SINLINE cu_rank_t cuex_tuple_r(cuex_t e)
{ return cuex_opr_r(cuex_meta(e)); }

/*!Returns the sub-tuple from position \a begin inclusive to position \a end
 * exclusive. */
cuex_t cuex_tuple_sub(cuex_t tuple, cu_rank_t begin, cu_rank_t end);

/*!A variant of \ref cuex_tuple_sub which prepends \a carg to the result
 * tuple. The size of the tuple will be \a end - \a begin. */
cuex_t cuex_tuple_sub_carg(cuex_t carg, cuex_t tuple,
			   cu_rank_t begin, cu_rank_t end);

/*!Returns the tuple formed by appending \a tuple0 and \a tuple1. */
cuex_t cuex_tuple_append(cuex_t tuple0, cuex_t tuple1);

/*!@}*/
CU_END_DECLARATIONS

#endif
