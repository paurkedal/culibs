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

#ifndef CUFLOW_EXEQ_H
#define CUFLOW_EXEQ_H

#include <cuflow/fwd.h>
#include <cuflow/tstate.h>
#include <cuflow/cdisj.h>
#include <cu/clos.h>
#include <cu/thread.h>
#include <atomic_ops.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuflow_sched_h cuflow/sched.h: A Cross-CPU Work-Sharing Mechanism
 *@{\ingroup cuflow_mod */

void cuflowP_sched_call(cuflow_exeq_t exeq, cu_clop0(fn, void), AO_t *cdisj);
void cuflowP_sched_call_sub1(cuflow_exeq_t exeq, cu_clop0(fn, void),
			     AO_t *cdisj);

#if CUFLOW_PROFILE_SCHED
extern AO_t cuflowP_profile_sched_count;
extern AO_t cuflowP_profile_nonsched_count;
#endif

/*!If the execution queue is full, calls \a fn and exits, else increments
 * \c *\a cdisj and schedules \a fn for later execution, possibly by another
 * thread.  In the latter case, \c *\a cdisj is decremented after \a fn has
 * been called. */
CU_SINLINE void
cuflow_sched_call(cu_clop0(fn, void), AO_t *cdisj)
{
    cuflow_exeq_t exeq = cuflow_tstate_exeq(cuflow_tstate());
#if CUFLOW_CALLS_BETWEEN_SCHED > 1
    if (cu_expect(!--exeq->calls_till_sched, 0)) {
	exeq->calls_till_sched = CUFLOW_CALLS_BETWEEN_SCHED;
#endif
	if (cu_expect_false(exeq->head != AO_load(&exeq->tail))) {
	    cuflowP_sched_call(exeq, fn, cdisj);
#if CUFLOW_PROFILE_SCHED
	    AO_fetch_and_add1(&cuflowP_profile_sched_count);
#endif
	    return;
	}
#if CUFLOW_CALLS_BETWEEN_SCHED > 1
    }
#endif
#if CUFLOW_PROFILE_SCHED
    AO_fetch_and_add1(&cuflowP_profile_nonsched_count);
#endif
    cu_call0(fn);
}

CU_SINLINE void
cuflow_sched_call_sub1(cu_clop0(fn, void), AO_t *cdisj)
{
    cuflow_exeq_t exeq = cuflow_tstate_exeq(cuflow_tstate());
#if CUFLOW_CALLS_BETWEEN_SCHED > 1
    if (cu_expect(!--exeq->calls_till_sched, 0)) {
	exeq->calls_till_sched = CUFLOW_CALLS_BETWEEN_SCHED;
#endif
	if (cu_expect_false(exeq->head != AO_load(&exeq->tail))) {
	    cuflowP_sched_call_sub1(exeq, fn, cdisj);
#if CUFLOW_PROFILE_SCHED
	    AO_fetch_and_add1(&cuflowP_profile_sched_count);
#endif
	    return;
	}
#if CUFLOW_CALLS_BETWEEN_SCHED > 1
    }
#endif
#if CUFLOW_PROFILE_SCHED
    AO_fetch_and_add1(&cuflowP_profile_nonsched_count);
#endif
    cu_call0(fn);
    cuflow_cdisj_sub1_release_write(cdisj);
}

/*!@}*/
CU_END_DECLARATIONS

#endif
