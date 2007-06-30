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

#ifndef CUEX_FALGO_H
#define CUEX_FALGO_H

#include <cuex/fwd.h>

CU_BEGIN_DECLARATIONS

cuex_t cuex_ftype_apply_n(cuex_t ftype, cu_rank_t n);

cuex_t cuex_ftype_apply_argtuple(cuex_t fn_type, cuex_t arg_tuple);

/*!Makes a tuple suitable as an argument vector using indexed \ref
 * cuex_fpvar_t variables in the range from \a index_begin inclusive to \a
 * index_end exclusive. The types are taken from \a ftype, which must be a
 * chain of \ref CUEX_O2_FARROW of sufficient length, or else this function
 * will return \c NULL to indicate the error. If \a result_type_out is non-\c
 * NULL, then the result type after \a index_end - \a index_begin application
 * of \a fn_type is stored at \c *\a result_type_out. */
cuex_t cuex_ftype_argtuple(cuex_t ftype,
			   cu_rank_t index_begin, cu_rank_t index_end,
			   cuex_t *result_type_out);

/*!A variant of \ref cuex_ftype_argtuple, suitable if a closure argument \a
 * carg_type not mentioned in \a ftype is the first argument. The total number
 * of components in the tuple is still \a index_end - \a index_begin. */
cuex_t cuex_ftype_argtuple_carg(cuex_t carg_type, cuex_t ftype,
				cu_rank_t index_begin, cu_rank_t index_end,
				cuex_t *result_type_out);


/*!Remove universal quantification nodes (\ref CUEX_O2_FORALL) in \a ftype
 * and replace the corresponding variables with universally quantified indexed
 * (\ref cuex_ivar) variables.  Indices range from 0 to exclusive \c *\a
 * tparam_cnt_out. */
cuex_t cuex_ftype_normalise_no_quant(cuex_t ftype, cu_rank_t *tparam_cnt_out);

/*!Compute \ref cuex_ftype_normalise_no_quant and wrap quantification nodes
 * around the result. */
cuex_t cuex_ftype_normalise_outmost_quant(cuex_t ftype,
					  cu_rank_t *tparam_cnt_out);

CU_END_DECLARATIONS

#endif
