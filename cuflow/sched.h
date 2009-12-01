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

void cuflowP_sched_call(cuflow_exeq_t exeq, cu_clop0(f, void), AO_t *cdisj);
void cuflowP_sched_call_sub1(cuflow_exeq_t exeq, cu_clop0(f, void),
			     AO_t *cdisj);

#if CUFLOW_PROFILE_SCHED
extern AO_t cuflowP_profile_sched_count;
extern AO_t cuflowP_profile_nonsched_count;
#endif

/** \defgroup cuflow_sched_h cuflow/sched.h: SMP Parallelization
 ** @{ \ingroup cuflow_smp_mod
 **
 ** This provides an efficient mechanism for sharing work across multiple CPUs
 ** or cores on an SMP system.  Before use, start some worker threads with \ref
 ** cuflow_workers_spawn_at_least or \ref cuflow_workers_spawn.  To parallelize
 ** a task, initialise a local \ref cuflow_cdisj_t \e guard to zero and request
 ** the subtasks with \ref cuflow_sched_call passing a reference to \e guard.
 ** Then use \ref cuflow_cdisj_wait_while to wait for the subtasks to finish.
 **/

/** The priority at which to run work scheduled by the current thread. */
CU_SINLINE cuflow_exeqpri_t
cuflow_sched_priority(void)
{
    return cuflow_tstate()->exeqpri;
}

/** Set the priority at which to run work scheduled by the current thread, and
 ** return the old priority. */
CU_SINLINE cuflow_exeqpri_t
cuflow_sched_change_priority(cuflow_exeqpri_t priority)
{
    cuflow_tstate_t tstate = cuflow_tstate();
    cuflow_exeqpri_t old_priority = tstate->exeqpri;
    tstate->exeqpri = priority;
    return old_priority;
}

/** The current current execution queue on which work will be scheduled. */
CU_SINLINE cuflow_exeq_t
cuflow_sched_exeq(void)
{
    cuflow_tstate_t tstate = cuflow_tstate();
    return &tstate->exeq[tstate->exeqpri];
}

/** If \a exeq queue is full, calls \a f and exits, else increments <code>\a
 ** cdisj</code> and schedules \a f for later execution, possibly by another
 ** thread.  In the latter case, <code>*\a cdisj</code> is decremented after \a
 ** f has been called.
 **
 ** This function does not alter the priority of the current thread.  It's main
 ** purpose is as an optimisation of successive calls to \ref cuflow_sched_call
 ** by letting the client fetch the thread-local \a exeq with \ref
 ** cuflow_sched_exeq once and pass it. */
CU_SINLINE void
cuflow_sched_call_on(cu_clop0(f, void), AO_t *cdisj, cuflow_exeq_t exeq)
{
#if CUFLOW_CALLS_BETWEEN_SCHED > 1
    if (cu_expect(!--exeq->calls_till_sched, 0)) {
	exeq->calls_till_sched = CUFLOW_CALLS_BETWEEN_SCHED;
#endif
	if (cu_expect_false(exeq->head != AO_load(&exeq->tail))) {
	    cuflowP_sched_call(exeq, f, cdisj);
	    return;
	}
#if CUFLOW_CALLS_BETWEEN_SCHED > 1
    }
#endif
#if CUFLOW_PROFILE_SCHED
    AO_fetch_and_add1(&cuflowP_profile_nonsched_count);
#endif
    cu_call0(f);
}

/** Same as \ref cuflow_sched_call with the current priority temporarily set to
 ** \a pri. */
void cuflow_sched_call_at(cu_clop0(f, void), AO_t *cdisj,
			  cuflow_exeqpri_t pri);

/** Same as \ref cuflow_sched_call_on with the current thread-local execution
 ** queue passed as the last argument. */
CU_SINLINE void
cuflow_sched_call(cu_clop0(f, void), AO_t *cdisj)
{
    cuflow_sched_call_on(f, cdisj, cuflow_tstate_exeq(cuflow_tstate()));
}

/** Same as \ref cuflow_sched_call, except that after \a f has been called, \a
 ** cdisj will be decremented by one.  Use this variant if you initialise \a
 ** cdisj in advance to the number of pending calls. */
CU_SINLINE void
cuflow_sched_call_sub1_on(cu_clop0(f, void), AO_t *cdisj, cuflow_exeq_t exeq)
{
#if CUFLOW_CALLS_BETWEEN_SCHED > 1
    if (cu_expect(!--exeq->calls_till_sched, 0)) {
	exeq->calls_till_sched = CUFLOW_CALLS_BETWEEN_SCHED;
#endif
	if (cu_expect_false(exeq->head != AO_load(&exeq->tail))) {
	    cuflowP_sched_call_sub1(exeq, f, cdisj);
	    return;
	}
#if CUFLOW_CALLS_BETWEEN_SCHED > 1
    }
#endif
#if CUFLOW_PROFILE_SCHED
    AO_fetch_and_add1(&cuflowP_profile_nonsched_count);
#endif
    cu_call0(f);
    cuflow_cdisj_sub1_release_write(cdisj);
}

/** Same as \ref cuflow_sched_call_sub1 with the current priority temporarily
 ** set to \a pri. */
void cuflow_sched_call_sub1_at(cu_clop0(f, void), AO_t *cdisj,
			       cuflow_exeqpri_t pri);

/** Same as \ref cuflow_sched_call_sub1_on with the current thread-local
 ** execution queue passed as the last argument. */
CU_SINLINE void
cuflow_sched_call_sub1(cu_clop0(f, void), AO_t *cdisj)
{
    cuflow_sched_call_sub1_on(f, cdisj, cuflow_tstate_exeq(cuflow_tstate()));
}

/** @} */
CU_END_DECLARATIONS

#endif
