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

#ifndef CUFLOW_TSTATE_H
#define CUFLOW_TSTATE_H

#include <cuflow/fwd.h>
#include <cu/conf.h>
#include <cu/inherit.h>
#include <cu/dlink.h>
#include <cuflow/sched_types.h>
#include <pthread.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuflow_tstate_h cuflow/tstate.h:
 *@{\ingroup cuflow_mod */

#define CUFLOW_EXEQ_PRI_COUNT 2

struct cuflow_tstate_s
{
    cu_inherit (cu_dlink_s);
    struct cuflow_exeq_s exeq[cuflow_exeqpri_end];
    cuflow_exeqpri_t exeqpri;
};

#ifdef CUCONF_HAVE_THREAD_KEYWORD

extern __thread cu_bool_t cuflowP_tstate_initialised;
extern __thread struct cuflow_tstate_s cuflowP_tstate;

void cuflowP_tstate_initialise(void);

CU_SINLINE cuflow_tstate_t
cuflow_tstate()
{
    if (!cuflowP_tstate_initialised)
	cuflowP_tstate_initialise();
    return &cuflowP_tstate;
}

#else /* !CUFLOW_HAVE_THREAD_KEYWORD */

extern pthread_key_t cuflowP_tstate_key;

cuflow_tstate_t cuflowP_tstate_initialise(void);

CU_SINLINE cuflow_tstate_t
cuflow_tstate()
{
    cuflow_tstate_t tstate = pthread_getspecific(cuflowP_tstate_key);
    if (!tstate)
	tstate = cuflowP_tstate_initialise();
    return tstate;
}

#endif /* !CUFLOW_HAVE_THREAD_KEYWORD */

CU_SINLINE cuflow_tstate_t
cuflow_tstate_next(cuflow_tstate_t ts)
{
    return cu_from(cuflow_tstate, cu_dlink, cu_to(cu_dlink, ts)->next);
}

CU_SINLINE cuflow_exeq_t
cuflow_tstate_exeq(cuflow_tstate_t tstate)
{
    return &tstate->exeq[tstate->exeqpri];
}

/*!@}*/
CU_END_DECLARATIONS

#endif
