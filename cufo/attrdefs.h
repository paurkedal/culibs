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

#ifndef CUFO_ATTRDEFS_H
#define CUFO_ATTRDEFS_H

#include <cufo/attr.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cufo_attrdefs_h cufo/attrdefs.h: Various Attribute Definitions
 *@{\ingroup cufo_mod */

extern struct cufo_attr_s cufoP_attr_id;
extern struct cufo_attr_s cufoP_attr_class;

#define cufoA_id(s)	CUFO_ATTR_CSTR(&cufoP_attr_id, s)
#define cufoA_class(s)	CUFO_ATTR_CSTR(&cufoP_attr_class, s)

/*!@}*/
CU_END_DECLARATIONS

#endif
