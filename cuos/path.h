/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2003--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUOS_PATH_H
#define CUOS_PATH_H

#include <cucon/fwd.h>
#include <cu/str.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuos_path cuos/path.h: File path functions
 * @{ \ingroup cuos_mod
 *
 * Definition (normalised file name).  A normalised file name is a string
 * subject to the following restrictions.  The maximum substrings not
 * containing "/" are called components.  Components of the form "",
 * "." and ".." are called special components.  A file name is
 * either absolute or relative.  An normalised absolute file name is
 * either "/", or starts with an empty component followed by one or
 * more non-special components.  A normalised relative file name is either
 * ".", or it starts with zero or more "..", followed by zero or more
 * non-special components, but adding up to at least one component.
 *
 * <pre>
 * <i>NFN</i>		::= <i>relative_NFN</i>
 * 		  | <i>absolute_NFN</i>
 * <i>relative_NFN</i>	::= "."
 * 		  | "../"* ".."
 *		  | "../"* (<i>non_special_comp</i> "/")* <i>non_special_comp</i>
 * <i>absolute_NFN</i>	::= "/"
 * 		  | ("/" <i>non_special_comp</i>)+
 * </pre>
 */

/*!True iff the string from \a s to \a s + \a len is a normalised file name. */
cu_bool_t cuos_is_path_charr(char const *s, size_t len);

/*!True iff \a s is a normalised file name. */
cu_bool_t cuos_is_path_cstr(char const *s);

/*!True iff \a str is a normalised file name. */
cu_bool_t cuos_is_path_str(cu_str_t str);

/*!Create normalised file names from the string from \a s to \a s + \a len
 * or \c NULL if it is not a valid file name. */
cu_str_t cuos_path_from_charr(char const *s, size_t len);

/*!Returns a normalised file name from \a s, or \c NULL if \a s does
 * not hold a valid file name. */
cu_str_t cuos_path_from_cstr(char const *s);

/*!Returns a normalised file name from \a str, or \c NULL if \a str does
 * not hold a valid file name. */
cu_str_t cuos_path_from_str(cu_str_t str);

/*!True iff the normalised file name \a path is absolute. */
cu_bool_t cuos_path_is_abs(cu_str_t path);

/*!True iff \a str is valid as a non-special path component.
 * \note It also returns false if the string contains ASCII non-printable
 * characters.  It does not check non-ASCII (UTF-8 encoded) non-printables,
 * but this may change.  Special path components are "." and "..".  Beware
 * that some file systems may have others as well, which are not checked for
 * currently. */
cu_bool_t cuos_is_pathcomp_str(cu_str_t str);

/*!True iff the string from \a s to \a s + \a len is valid as a non-special
 * path component.  See note under \ref cuos_is_pathcomp_str. */
cu_bool_t cuos_is_pathcomp_charr(char const *s, size_t len);

/*!True iff \a s is valid as a non-special path component.  See note under
 * \ref cuos_is_pathcomp_str. */
cu_bool_t cuos_is_pathcomp_cstr(char const *s);

/*!Return the number of components of \a path, where "." counts as zero and
 * other components, including a leading "/" counts as one. */
int cuos_path_component_count(cu_str_t path);

/*!Returns the depth of \a path, where each non-special component and a
 * leading "/" counts as one, "." counts as zero, and ".." counts as -1. */
int cuos_path_depth(cu_str_t path);

/*!True iff \a path has no components, i.e. it is ".".  This is faster
 * than using \ref cuos_path_component_count. */
CU_SINLINE cu_bool_t cuos_path_is_empty(cu_str_t path)
{ return cu_str_size(path) == 1 && cu_str_at(path, 0) == '.'; }

/*!Given that \a path0 and \a path1 are normalised file names, if \a path1
 * is absolute, return \a path1, else return the normalised file name of
 * \a path1 relative to \a path0, i.e. the normalisation of the
 * concatenation of (\a path0, "/", \a path1).  \a path1 must be non-empty. */
cu_str_t cuos_path_join(cu_str_t path0, cu_str_t path1);

/*!As \ref cuos_path_join but with C string as second argument. */
cu_str_t cuos_path_join_str_cstr(cu_str_t path0, char const *path1);

/*!As \ref cuos_path_join but with C string as first argument. */
cu_str_t cuos_path_join_cstr_str(char const *path0, cu_str_t path1);

/*!As \ref cuos_path_join but with 2 C string arguments. */
cu_str_t cuos_path_join_2cstr(char const *path0, char const *path1);

/*!\pre \a path is a normalised file name.
 * 
 * If \a pos ≥ 0, then split \a path
 * before component \a pos and store the first and last parts as a
 * normalised file names in \c *path0 and \c *path1, respectively.  If
 * \a pos < 0, the call is equivalent to one where \a pos is replaced by
 * \a n + \a pos, where \a n is the number of components.  */
cu_bool_t cuos_path_split(cu_str_t path, int pos,
			  cu_str_t *path0, cu_str_t *path1);

/*!Same as \a ret, where
 * \code cuos_path_split(path, -1, &ret, NULL)\endcode. */
cu_str_t cuos_path_dir(cu_str_t path);

/*!Same as \a ret, where
 * \code cuos_path_split(path, -1, NULL, &ret)\endcode. */
cu_str_t cuos_path_base(cu_str_t path);

/*!Returns extension of \a path, including the ".", or an empty string if
 * \a path has no extension. */
cu_str_t cuos_path_ext(cu_str_t path);

/*!Returns \a path without its extension or \a path itself if it has no
 * extension. */
cu_str_t cuos_path_sans_ext(cu_str_t path);

/*!True iff \a path ends with \a ext. */
cu_bool_t cuos_path_ext_eq(cu_str_t path, cu_str_t ext);

/*!True iff \a path ends with the substring from \a s to \a s + \a n. */
cu_bool_t cuos_path_ext_eq_charr(cu_str_t path, char const *s, size_t n);

/*!True iff \a path ends with \a ext. */
cu_bool_t cuos_path_ext_eq_cstr(cu_str_t path, char const *ext);

CU_END_DECLARATIONS

/* @} */
#endif
