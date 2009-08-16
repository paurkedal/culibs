/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2004--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUFLOW_WIND_H
#define CUFLOW_WIND_H

#include <cuflow/fwd.h>
#include <cu/conf.h>

CU_BEGIN_DECLARATIONS

typedef enum {
    cuflow_wind_direction_normal,
    cuflow_wind_direction_unwind,
    cuflow_wind_direction_rewind,
    cuflow_wind_direction_except,
} cuflow_wind_direction_t;

typedef struct cuflowP_windargs_s *cuflowP_windargs_t;
struct cuflowP_windargs_s
{
    cuflow_wind_direction_t direction;
    cu_fnptr_t xc_key;
};

#define CUFLOW_WIND_VARIANT_SETJMP 1
#define CUFLOW_WIND_VARIANT_UNW 2
#define CUFLOWP_IN_WIND_H
CU_END_DECLARATIONS
#if CUCONF_WIND_VARIANT == CUFLOW_WIND_VARIANT_UNW
#  include <cuflow/wind_unw.h>
#elif CUCONF_WIND_VARIANT == CUFLOW_WIND_VARIANT_SETJMP
#  include <cuflow/wind_setjmp.h>
#else
#  error Bad value or undefined CUCONF_WIND_VARIANT.
#endif
CU_BEGIN_DECLARATIONS
#undef CUFLOWP_IN_WIND_H

#define cuflow_wind_gotos() \
	cuflowP_wind_gotos(goto unwind, goto rewind, goto except)
#define cuflow_except_goto() \
	cuflowP_wind_gotos(cuflow_unwind_return, cuflow_rewind_return, goto except)

CU_END_DECLARATIONS

#endif
