/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2006--2009  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUFLOW_WORKERS_H
#define CUFLOW_WORKERS_H

#include <cuflow/fwd.h>
#include <cu/clos.h>
#include <time.h>
#include <unistd.h>
#include <atomic_ops.h>

CU_BEGIN_DECLARATIONS

extern AO_t cuflowP_pending_work;
extern AO_t cuflowP_workers_waiting_count;
void cuflowP_workers_broadcast(void);
void cuflowP_yield(void);

/** \defgroup cuflow_workers_h cuflow/workers.h: Auxiliary Worker Threads
 ** @{\ingroup cuflow_mod
 **
 ** This header provides an API to launch one or more worker threads and run
 ** functions within them, either as soon as one is idle or at a given time.
 ** The main purpose of this is to unify such work in a common framework, so
 ** that the same thread(s) can be used, rather than adding new threads for
 ** each task.  The functions defined here is aimed to be the bare minimum to
 ** achieve this.  For instance, if you need to know when a callback is
 ** finised, you may need to supply it's closure struct with an additional
 ** mutex and possibly a condition variable to communicate with consumer(s) of
 ** the computation.
 **
 ** This is implemenated purely with POSIX threads, mutices and condition
 ** variables, and in particular avoid timers or signals which may cause
 ** trouble in a multi-threaded environment.
 **/

/** The current number of worker threads. */
int cuflow_workers_count(void);

/** Spawn or drop worker threads until there are exactly \a target_count left.
 ** This is not safe to call within worker threads. */
void cuflow_workers_spawn(int target_count);

/** Spawns worker threads as needed so that there are at least \a target_count
 ** of them. */
void cuflow_workers_spawn_at_least(int target_count);

/** Drops worker threads as needed so that there are at most \a target_count
 ** left. */
void cuflow_workers_spawn_at_most(int target_count);

/** Call \a f in one of the worker threads, as soon as one is ready. */
void cuflow_workers_call(cu_clop0(f, void));

/** Call \a f at absolute \c CLOCK_REALTIME \a t_call. */
void cuflow_workers_call_at(cu_clop(f, void, struct timespec *t_now),
			    struct timespec *t_call);

/** Arranged for \a sched to be called round-robin along with other registered
 ** work.  This can happen either when a task is waiting for work to be
 ** completed or directly from a worker thread.  When called from a worker
 ** thread, true is passed for the first argument.  Otherwise, false is passed
 ** to indicate that \a sched is called from an arbitrary subroutine, in which
 ** case \a sched should only do a small amount of work and exit. */
void cuflow_workers_register_scheduler(cu_clop(sched, void, cu_bool_t));

/** A call to this function performs some work which have been registered by
 ** one of the other functions from this header.  This is typcially invoked
 ** when waiting for resources or results from other scheduled work. */
CU_SINLINE void
cuflow_yield(void)
{
    if (AO_load(&cuflowP_pending_work))
	cuflowP_yield();
}

/** Wake up any sleeping workers so that they can pick up any new work.  You
 ** usually don't need to call this, since it's automically called by functions
 ** which add new work or unlocks existing work. */
CU_SINLINE void
cuflow_workers_broadcast(void)
{
    if (AO_load(&cuflowP_workers_waiting_count))
	cuflowP_workers_broadcast();
}

/** Indicate that your scheduler have pending work.  Pair it with a call to
 ** \ref cuflow_workers_decr_pending when the work is done.  All schedulers
 ** will run round-robin as long as this function has been called more times
 ** than \ref cuflow_workers_decr_pending. */
CU_SINLINE void
cuflow_workers_incr_pending(void)
{
    AO_fetch_and_add1_acquire(&cuflowP_pending_work);
    cuflow_workers_broadcast();
}

/** Indicate that a unit of work signalled with \ref
 ** cuflow_workers_incr_pending has been picked up.  This is typically called
 ** from a custom scheduler. */
CU_SINLINE void
cuflow_workers_decr_pending(void)
{
    AO_fetch_and_sub1(&cuflowP_pending_work);
}

/** @} */
CU_END_DECLARATIONS

#endif
