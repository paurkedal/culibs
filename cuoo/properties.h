/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2005--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUOO_PROPERTIES_H
#define CUOO_PROPERTIES_H

#include <cuoo/fwd.h>
#include <cuoo/prop.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuoo_properties_h cuoo/properties.h: Predefined Properties
 * @{ \ingroup cuoo_mod */

extern struct cuoo_prop_s cuooP_raw_c_name_prop;
extern struct cuoo_prop_s cuooP_sref_prop;

/* The C name of a type or function, if defined.  Contains only stored
 * names, so it will not synthesise names for constructs such as
 * pointers and prototypes. */
CU_SINLINE cuoo_prop_t cuoo_raw_c_name_prop()
{ return &cuooP_raw_c_name_prop; }

CU_SINLINE cu_idr_t cuoo_raw_c_name(cuex_t ex)
{ return (cu_idr_t)cuoo_prop_get_ptr(cuoo_raw_c_name_prop(), ex); }

/* Source references. */
CU_SINLINE cuoo_prop_t cuoo_sref_prop()
{ return &cuooP_sref_prop; }

CU_SINLINE cu_sref_t cuoo_sref(cuex_t ex)
{ return (cu_sref_t)cuoo_prop_get_ptr(cuoo_sref_prop(), ex); }

void cuoo_sref_push(cuex_t ex, cu_sref_t head);

cu_bool_t cuoo_raw_print(cuex_t ex, FILE *out) CU_ATTR_DEPRECATED;

/*!@}*/
CU_END_DECLARATIONS

#endif
