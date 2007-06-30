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

#ifndef CUDYN_PROPERTIES_H
#define CUDYN_PROPERTIES_H

#include <cudyn/fwd.h>
#include <cudyn/prop.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cudyn_properties_h cudyn/properties.h: Predefined Properties
 * @{ \ingroup cudyn_mod */

extern struct cudyn_prop_s cudynP_raw_c_name_prop;
extern struct cudyn_prop_s cudynP_sref_prop;
extern struct cudyn_prop_s cudynP_raw_print_fn_prop;

/* The C name of a type or function, if defined.  Contains only stored
 * names, so it will not synthesise names for constructs such as
 * pointers and prototypes. */
CU_SINLINE cudyn_prop_t cudyn_raw_c_name_prop()
{ return &cudynP_raw_c_name_prop; }

CU_SINLINE cu_idr_t cudyn_raw_c_name(cuex_t ex)
{ return cudyn_prop_get_ptr(cudyn_raw_c_name_prop(), ex); }

/* Source references. */
CU_SINLINE cudyn_prop_t cudyn_sref_prop()
{ return &cudynP_sref_prop; }

CU_SINLINE cu_sref_t cudyn_sref(cuex_t ex)
{ return cudyn_prop_get_ptr(cudyn_sref_prop(), ex); }

void cudyn_sref_push(cuex_t ex, cu_sref_t head);

/* Preliminary Printing Support */
CU_SINLINE cudyn_prop_t cudyn_raw_print_fn_prop()
{ return &cudynP_raw_print_fn_prop; }

cu_bool_t cudyn_raw_print(cuex_t ex, FILE *out);

/*!@}*/
CU_END_DECLARATIONS

#endif
