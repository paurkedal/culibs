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

#ifndef CUFLOWP_IN_WIND_H
#  error This shall be included through ccf/wind.h, not directly.
#endif
#ifndef CUFLOW_WIND_UNW_H
#define CUFLOW_WIND_UNW_H

#include <cuflow/fwd.h>
#include <libunwind.h>

CU_BEGIN_DECLARATIONS


typedef struct cuflowP_windstate_s *cuflowP_windstate_t;
struct cuflowP_windstate_s
{
    cuflowP_windargs_t windargs;
};

typedef struct cuflowP_fninfo_s *cuflowP_fninfo_t;
struct cuflowP_fninfo_s
{
    unw_word_t key;
    cuflowP_fninfo_t next;
    unw_word_t ip;
    size_t windargs_offset;
};

void cuflowP_wind_init_function(int *done_init, cuflowP_windstate_t wst,
			     cuflowP_fninfo_t static_storage);

#define cuflowP_wind_gotos(on_unw, on_rew, on_xc)			\
    static int cuflowL_done_init = 0;					\
    static struct cuflowP_fninfo_s cuflowL_fninfo;			\
    cuflowP_windargs_t cuflowL_windargs = NULL;				\
    if (!cuflowL_done_init) {						\
	cuflowP_wind_init_function(&cuflowL_done_init,			\
				   &cuflowL_windargs, &cuflowL_fninfo);	\
	switch (cuflowL_windargs->direction) {				\
	case cuflow_wind_direction_unwind:				\
	    on_unw;							\
	case cuflow_wind_direction_rewind:				\
	    on_rew;							\
	case cuflow_wind_direction_except:				\
	    on_xc;							\
	default:							\
	    break;							\
	}								\
    }

CU_ATTR_NORETURN void cuflowP_unwind(cuflowP_windargs_t windargs);
CU_ATTR_NORETURN void cuflowP_rewind(cuflowP_windargs_t windargs);
#define cuflow_rewind_continue cuflowP_rewind(cuflowP_windargs)
#define cuflow_unwind_continue cuflowP_unwind(cuflowP_windargs)
#define cuflow_except_continue cuflowP_unwind(cuflowP_windargs)
#define cuflow_wind_return(val) return val;
CU_ATTR_NORETURN void cuflowP_throw(cuflowP_windargs_t windargs);

CU_END_DECLARATIONS

#endif
