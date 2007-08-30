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

#ifndef CUOO_SUPPORT_H
#define CUOO_SUPPORT_H

#include <cuoo/fwd.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuoo_support_h cuoo/support.h:
 *@{\ingroup cuoo_mod */

cu_hash_t cuoo_obj_hash_1w(void *);
cu_hash_t cuoo_obj_hash_2w(void *);
cu_hash_t (*cuoo_obj_hash_fn(size_t count))(void *);

/*!@}*/
CU_END_DECLARATIONS

#endif
