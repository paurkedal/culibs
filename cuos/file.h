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

#ifndef CUOS_FILE_H
#define CUOS_FILE_H

#include <cu/fwd.h>
#include <cu/algo.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuos_file_h cuos/file.h: Functions on Files
 *@{\ingroup cuos_mod */

/*!Returns the ordering relation of the content of the files at \a path0 and \a
 * path1, or \ref cu_order_none if an error occured.  The order is that of the
 * first differing byte, where EOF is taken as zero.  When en error occurs, \c
 * errno will be set by \c fopen. */
cu_order_t cuos_file_order(cu_str_t path0, cu_str_t path1);

/*!If the file at \a tmp_path differ or cannot be compared to that at \a
 * dst_path, renames \a tmp_path to \a dst_path (overwriting the latter) and
 * returns 1, otherwise removes \a tmp_path and returns 0.  In case of error -1
 * is returned, and \c errno is set by the system \c unlink or \c rename
 * call. */
int cuos_file_update(cu_str_t tmp_path, cu_str_t dst_path);

/*!@}*/
CU_END_DECLARATIONS

#endif
