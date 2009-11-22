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
#include <cu/threadlocal.h>
#include <cuflow/sched_types.h>
#include <pthread.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuflow_tstate_h cuflow/tstate.h: Thread-Local State
 *@{\ingroup cuflow_mod */

struct cuflow_tstate
{
    cu_inherit (cu_dlink);
    struct cuflow_exeq_s exeq[cuflow_exeqpri_end];
    cuflow_exeqpri_t exeqpri;
};

CU_THREADLOCAL_DECL(cuflow_tstate, cuflowP_tstate);

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
