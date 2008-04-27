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

#ifndef CUOS_SINK_H
#define CUOS_SINK_H

#include <cu/fwd.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuos_dsink_h cuos/dsink.h: Binding Data Sinks to Resources
 *@{\ingroup cuos_mod */

/*!Returns a \ref cu_dsink_h "cu_dsink_t" tied to the file descriptor \a fd.
 * Closing the sink preserves \a fd. */
cu_dsink_t cuos_dsink_fdopen(int fd);

/*!Returns a \ref cu_dsink_h "cu_dsink_t" tied to the file descriptor \a fd.
 * Closing the sink effects a \c close(fd). */
cu_dsink_t cuos_dsink_fdopen_close(int fd);

/*!Creates a regular file at \a ref path to be filled with any data written to
 * the sink. */
cu_dsink_t cuos_dsink_fopen(char const *path);

/*!@}*/
CU_END_DECLARATIONS

#endif
