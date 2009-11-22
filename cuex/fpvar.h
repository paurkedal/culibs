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

#ifndef CUEX_FPVAR_H
#define CUEX_FPVAR_H

#include <cuex/var.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_fpvar_h cuex/fpvar.h: Formal Function Parameter Variables
 * @{\ingroup cuex_mod
 * This is a variable type suited for formal function parameters if hash-cons
 * equivalence is required.  These variables are identified by argument number
 * and type.  Note that local functions, if supported by the application, may
 * have clashing variables, and this must be dealt with when traversing
 * expression trees.  It this is not feasible, it is suggested to give up
 * hash-cons equivalence and use \ref cuex_tvar "cuex_tvar_t" instead. */

#define cuex_fpvarmeta(index)						\
    cuex_varmeta_kqis(cuex_varkind_fpvar, cuex_qcode_u, index,		\
	sizeof(cuex_t)/sizeof(cu_word_t))
#define cuex_is_fpvarmeta(meta) cuex_is_varmeta_k(meta, cuex_varkind_fpvar)

/*!A formal parameter variable. */
struct cuex_fpvar
{
    CUOO_HCOBJ
    cuex_t type;
};

/*!Hash-cons a function parameter variable based on the argument number and
 * its type. */
cuex_fpvar_t cuex_fpvar(int index, cuex_t type);

/*!True iff \a e is a formal function parameter variable. */
CU_SINLINE cu_bool_t cuex_is_fpvar(cuex_t e)
{ return cuex_is_fpvarmeta(cuex_meta(e)); }

/*!The index of \a v. */
CU_SINLINE int cuex_fpvar_index(cuex_fpvar_t v)
{ return cuex_varmeta_index(cuex_meta(v)); }

/*!The type of \a v. */
CU_SINLINE cuex_t cuex_fpvar_type(cuex_fpvar_t v) { return v->type; }

CU_END_DECLARATIONS

/*!@}*/
#endif
