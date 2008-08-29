/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2008  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUEX_STR_ALGO_H
#define CUEX_STR_ALGO_H

#include <cuex/fwd.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_str_algo_h cuex/str_algo.h: String-Related Algorithms
 *@{\ingroup cuex_mod */

/*!If \a e is a left-associated \a opr combination of identifiers, puts each to
 * \a buf as a character string, delimited by \a delim and returns true,
 * othewise returns false. */
cu_bool_t cuex_left_idrjoin_charbuf(cu_buffer_t buf, cuex_meta_t opr, cuex_t e,
				    char const *delim);

/*!If \a e is a left-associated \a opr combination of identifiers, returns a
 * string formed by the same identifiers delimited by \a delim, otherwise
 * returns \c NULL. */
cu_str_t cuex_left_idrjoin_str(cuex_meta_t opr, cuex_t e, char const *delim);

/*!if \a e is a left-associated \a opr combination of identifiers, returns a C
 * string formed by the same identifiers delimited by \a delim, otherwise
 * returns \c NULL. */
char const *cuex_left_idrjoin_cstr(cuex_meta_t opr, cuex_t e,
				   char const *delim);

/*!@}*/
CU_END_DECLARATIONS

#endif
