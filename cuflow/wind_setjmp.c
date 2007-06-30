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

#include <cuflow/wind.h>
#include <string.h>
#include <pthread.h>
#include <cu/conf.h>
#ifdef CUCONF_HAVE_GC_GC_H
#  include <gc/gc.h>
#else
#  include <gc.h>
#endif
#include <stdio.h>


void
cuflowP_unwind(cuflowP_windstate_t wst)
{
    cuflowP_windargs_t wargs = wst->windargs;
    wst = wst->prev;
    wst->windargs = wargs;
    cuP_tstate()->windstate = wst;
    siglongjmp(wst->door, 1);
}

void
cuflowP_rewind(cuflowP_windstate_t wst)
{
    assert(!"Not implemented.");
    abort();
}

void
cuflowP_throw(cuflowP_windargs_t wargs)
{
    cuflowP_windstate_t wst = cuP_tstate()->windstate;
    if (!wst) {
	fprintf(stderr,
		"error: Uncaught exception (no frame created).\n");
	abort();
    }
    wargs->direction = cuflow_wind_direction_except;
    wst->windargs = wargs;
    cuP_tstate()->windstate = wst;
    siglongjmp(wst->door, 1);
}

void
cuflowP_push_windstate(cuflowP_windstate_t wst)
{
    cuP_tstate_t st = cuP_tstate();
    wst->prev = st->windstate;
    st->windstate = wst;
}

