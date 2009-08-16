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

#ifndef CUOS_TOPDIRS_H
#define CUOS_TOPDIRS_H

#include <cuos/fwd.h>
#include <cucon/list.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuos_dirpile_h cuos/dirpile.h: Prioritised Collection of Search Directories
 *@{\ingroup cuos_mod
 *
 * This class holds a list of directory paths and provides functions to locate
 * files with given relative paths under these.  The semantics is suitable for
 * implementing search paths similar to the compiler's -I flags or environment
 * variables like $PATH, $PYTHONPATH, etc.
 */

struct cuos_dirpile_s
{
    struct cucon_list_s top_dirs;
};

/*!Initialise an empty pile of directories. */
CU_SINLINE void
cuos_dirpile_init(cuos_dirpile_t pile)
{ cucon_list_init(&pile->top_dirs); }

/*!Returns an empty pile of directories. */
cuos_dirpile_t cuos_dirpile_new(void);

/*!If \a dir exists, add it to \a pile and return true, else return false.  If
 * \a on_top, then \a dir will be tried before the current list, otherwise
 * after. */
cu_bool_t cuos_dirpile_insert(cuos_dirpile_t pile, cu_str_t dir,
			      cu_bool_t on_top);

/*!Add the existing directories from the colon-separated directories in \a
 * envvar to \a pile.  if \a on_top, then the added directories will be tried
 * before the current list, otherwise after.  Returns -1 if envvar is unset,
 * otherwise the number of inserted directories. */
int cuos_dirpile_insert_envvar(cuos_dirpile_t pile, char const *envvar,
			       cu_bool_t on_top);

/*!Calls \a f with each top-level directory of \a pile in order.  Stops the
 * iteration and returns false if \a f returns false, else returns true. */
cu_bool_t cuos_dirpile_iterA_top(cuos_dirpile_t pile,
				 cu_clop(f, cu_bool_t, cu_str_t));

/*!Returns the full path of the first file or directory with a path \a rel_path
 * relative to a top-level path of \a pile, or \c NULL if no such file or
 * directory exists. */
cu_str_t cuos_dirpile_first_match(cuos_dirpile_t pile, cu_str_t rel_path);

/*!Calls \a f with the full path of each existing file or directory with a path
 * \a rel_path relative to a top-level path of \a pile in order.  Stops and
 * returns false as soon as \a f returns false, else returns true. */
cu_bool_t cuos_dirpile_iterA_matches(cuos_dirpile_t pile,
				     cu_clop(f, cu_bool_t, cu_str_t),
				     cu_str_t rel_path);

/*!@}*/
CU_END_DECLARATIONS

#endif
