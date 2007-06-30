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

#ifndef CUEX_MISC_H
#define CUEX_MISC_H

#include <cuex/fwd.h>

CU_BEGIN_DECLARATIONS

extern cudyn_stdtype_t cuexP_long_type;
#define cuex_long_type() cuexP_long_type

cuex_t cuex_of_long(long);

CU_SINLINE cu_bool_t cuex_is_long(cuex_t ex)
{ return cuex_meta(ex) == cudyn_stdtype_to_meta(cuexP_long_type); }

CU_SINLINE long cuex_to_long(cuex_t ex)
{ return *(long *)((void *)(ex) + CU_HCOBJ_SHIFT); }

CU_END_DECLARATIONS

#endif
