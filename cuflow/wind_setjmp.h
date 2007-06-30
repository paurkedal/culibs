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
#ifndef CUFLOW_WIND_SETJMP_H
#define CUFLOW_WIND_SETJMP_H

#include <cuflow/fwd.h>
#include <cu/tstate.h>
#include <setjmp.h>
#include <assert.h>

CU_BEGIN_DECLARATIONS

typedef struct cuflowP_windstate_s *cuflowP_windstate_t;

struct cuflowP_windstate_s
{
    cuflowP_windstate_t prev;
    sigjmp_buf door;
    cuflowP_windargs_t windargs;
};

#define cuflowP_wind_gotos(on_unw, on_rew, on_xc)		\
    struct cuflowP_windstate_s cuflowP_windstate;		\
    cuflowP_push_windstate(&cuflowP_windstate);		\
    if (sigsetjmp(cuflowP_windstate.door, 1))		\
	switch (cuflowP_windstate.windargs->direction) {	\
	case cuflow_wind_direction_unwind:		\
	    on_unw;					\
	case cuflow_wind_direction_rewind:		\
	    on_rew;					\
	case cuflow_wind_direction_except:		\
	    on_xc;					\
	default:					\
	    assert(!"Not reached.");			\
	    break;					\
	}

CU_ATTR_NORETURN void cuflowP_unwind(cuflowP_windstate_t);
CU_ATTR_NORETURN void cuflowP_rewind(cuflowP_windstate_t);
#define cuflow_rewind_continue cuflowP_rewind(&cuflowP_windstate)
#define cuflow_unwind_continue cuflowP_unwind(&cuflowP_windstate)
#define cuflow_except_continue cuflowP_unwind(&cuflowP_windstate)
#define cuflow_wind_return(val)					\
    do {							\
	cuP_tstate()->windstate = cuflowP_windstate.prev;		\
	return val;						\
    } while (0)
CU_ATTR_NORETURN void cuflowP_throw(cuflowP_windargs_t windargs);

void cuflowP_push_windstate(cuflowP_windstate_t wst);

CU_END_DECLARATIONS

#endif
