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
/** \defgroup cuos_dirpile_h cuos/dirpile.h: Prioritised Collection of Search Directories
 ** @{\ingroup cuos_mod
 **
 ** This class holds a list of directory paths and provides functions to locate
 ** files with given relative paths under these.  The semantics is suitable for
 ** implementing search paths similar to the compiler's -I flags or environment
 ** variables like $PATH, $PYTHONPATH, etc.
 **/

/** A "pile" of directories for prioritized searches of relative paths. */
struct cuos_dirpile
{
    struct cucon_list top_dirs;
};

/** Initialise an empty pile of directories. */
void cuos_dirpile_init(cuos_dirpile_t pile);

/** Returns an empty pile of directories. */
cuos_dirpile_t cuos_dirpile_new(void);

/** Initialise \a new_pile with the existing subdirectories \a src_subdir of \a
 ** src_pile in the order of occurence in \a src_pile. */
void cuos_dirpile_init_sub(cuos_dirpile_t new_pile,
			   cuos_dirpile_t src_pile, cu_str_t src_subdir);

/** Return a dirpile of the existing subdirectories \a src_subdir of \a
 ** src_pile in the order of occurrence in \a src_pile. */
cuos_dirpile_t cuos_dirpile_new_sub(cuos_dirpile_t src_pile,
				    cu_str_t src_subdir);

/** If \a dir exists, add it to \a pile and return true, else return false.  If
 ** \a on_top, then \a dir will be tried before the current list, otherwise
 ** after. */
cu_bool_t cuos_dirpile_insert(cuos_dirpile_t pile, cu_str_t dir,
			      cu_bool_t on_top);

/** \copydoc cuos_dirpile_insert */
cu_bool_t cuos_dirpile_insert_cstr(cuos_dirpile_t pile, char const *dir,
				   cu_bool_t on_top);

/** Add the existing subdirectories \a subdir of directories from the
 ** colon-separated directories in \a var to \a pile.  If \a subdir is \c NULL,
 ** "." is used.  If \a on_top, then the added directories will be tried before
 ** the current list, otherwise after.  Returns -1 if envvar is unset,
 ** otherwise the number of inserted directories. */
int cuos_dirpile_insert_env(cuos_dirpile_t pile, cu_bool_t on_top,
			    char const *var, cu_str_t subdir);

/** \deprecated Use cuos_dirpile_insert_env appsind \c NULL for \a subdir. */
CU_SINLINE cu_bool_t
cuos_dirpile_insert_envvar(cuos_dirpile_t pile, char const *var,
			   cu_bool_t on_top)
{ return cuos_dirpile_insert_env(pile, on_top, var, NULL); }

/** If \a dir exists, add it to the bottom of \a pile and return true, else
 ** return false.  */
CU_SINLINE cu_bool_t
cuos_dirpile_append(cuos_dirpile_t pile, cu_str_t dir)
{ return cuos_dirpile_insert(pile, dir, cu_false); }

/** \copydoc cuos_dirpile_append */
CU_SINLINE cu_bool_t
cuos_dirpile_append_cstr(cuos_dirpile_t pile, char const *dir)
{ return cuos_dirpile_insert_cstr(pile, dir, cu_false); }

/** Calls \a f with each top-level directory of \a pile in order.  Stops the
 ** iteration and returns false if \a f returns false, else returns true. */
cu_bool_t cuos_dirpile_iterA_top(cuos_dirpile_t pile,
				 cu_clop(f, cu_bool_t, cu_str_t));

/** Returns the full path of the first file or directory with a path \a
 ** rel_path relative to a top-level path of \a pile, or \c NULL if no such
 ** file or directory exists. */
cu_str_t cuos_dirpile_first_match(cuos_dirpile_t pile, cu_str_t rel_path);

/** Calls \a f with the full path of each existing file or directory with a
 ** path \a rel_path relative to a top-level path of \a pile in order.  Stops
 ** and returns false as soon as \a f returns false, else returns true. */
cu_bool_t cuos_dirpile_iterA_matches(cuos_dirpile_t pile,
				     cu_clop(f, cu_bool_t, cu_str_t),
				     cu_str_t rel_path);

void cuos_dirpile_append_matches(cuos_dirpile_t dst_pile,
				 cuos_dirpile_t src_pile, cu_str_t src_subdir);

/** @} */
CU_END_DECLARATIONS

#endif
