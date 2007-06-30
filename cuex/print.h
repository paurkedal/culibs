/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUEX_PRINT_H
#define CUEX_PRINT_H

#include <cuex/fwd.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuex_print_h cuex/print.h: Printing Expressions
 *@{\ingroup cuex_mod
 * For general printing of expressions, \ref cuex_init registers the "%!"
 * format specifier which can be used in \ref cu_fprintf, \ref cu_errf, etc.
 * The following more esoteric.
 */

/*!Write out a set of vertex and edge declarations in Graphviz dot-format,
 * suitable as the body of a <tt>digraph</tt> definition.  */
cu_bool_t cuex_write_dot_decls(cuex_t e, FILE *out);

/*!Save \a e as a digraph \a graph_name in the file \a path.  If \a graph_name
 * is \c NULL, a default name is used.  If \a path is \c NULL, this saves to a
 * unique file under <tt>/tmp</tt>. */
void cuex_save_dot(cuex_t e, char const *graph_name, char const *path);

/*!@}*/
CU_END_DECLARATIONS

#endif
