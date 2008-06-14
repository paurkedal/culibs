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

#ifndef CUFLOW_GWORKQ_H
#define CUFLOW_GWORKQ_H

#include <cuflow/fwd.h>
#include <cuflow/workq.h>
#include <cu/tstate.h>
#include <cu/inherit.h>
#include <cu/thread.h>
#include <stdio.h>
#include <atomic_ops.h>


CU_BEGIN_DECLARATIONS
/*!\defgroup cuflow_gworkq_h cuflow/gworkq.h Global Work Queue
 * @{\ingroup cuflow_mod
 * The global work queue is a way for threads to co-operate to performed
 * scheduled function calls in order of priority and in FIFO order within
 * a given priority.
 *
 * On SMP systems, this can be used to parallelise work by letting threads
 * performing low-priority tasks help out the high-priority threads when
 * work is available.
 * The global work queue can also be useful on non-SMP systems to run
 * tasks while waiting for resources to become available.  */

typedef int cuflow_priority_t;

/*!One above the maximum priority. */
#define cuflow_priority_postmax		16

/*!The priority of realtime tasks like audio and video. */
#define cuflow_priority_realtime	13

/*!The priority of interacive tasks like GUI or curses dialogs. */
#define cuflow_priority_interactive	10

/*!The priority of common tasks with no specific need. */
#define cuflow_priority_normal		 7
	/* NB! Update cu/tstate.c:cuP_tstate_init_glck if changed. */

/*!The priority of tasks which are to be run only when there is nothing
 * else to do. */
#define cuflow_priority_background	 3


/*!The current priority of the running thread. */
cuflow_priority_t cuflow_gworkq_current_priority(void);

/*!Set the current static priority of the running thread and return its
 * previous priority.
 * \pre The current thread must be running on a static-priority queue. */
cuflow_priority_t cuflow_gworkq_set_static_priority(cuflow_priority_t pri);

/*!Yield the processor to tasks at or above \a priority in the global
 * work queue and return true iff some work was done.  The selected
 * task is the first task of the highest priority non-empty queue. */
cu_bool_t cuflow_gworkq_yield_at(int priority);

cu_bool_t cuflow_gworkq_yield_at_glck(int priority);

/*!Yield the processor to work above the current priority until there is
 * no more such work.  Returns true iff some work was done.
 * This can be scattered at strategic points in low-priority code to
 * give more CPUs to high priority tasks. */
cu_bool_t cuflow_gworkq_yieldall_prior(void);

/*!Yield the processor to work at or above the current priority until
 * there is no more such work.  Returns true iff some work was done. */
cu_bool_t cuflow_gworkq_yieldall_prioreq(void);

void cuflow_gworkq_yield_gcond_glck(void);

/*!Same as \ref cuflow_gworkq_yieldall_prior, except that if no work was done,
 * try also to run a single task at the current priority.  Returns true iff
 * some work was done.  This is useful while waiting for resources. */
cu_bool_t cuflow_gworkq_yield_idle(void);

/*!Schedule \a fn to be called at fixed \a priority. */
void cuflow_gworkq_sched_at(cuflow_workq_fn_t fn, cuflow_priority_t priority);

/*!Schedule \a fn on the current thread's work queue.  If a
 * \ref cuflow_gflexq_h "flexible-priority queue" is active for the thread,
 * it is used, otherwise a the static-priority queue at the same priority
 * as the thread is used. */
void cuflow_gworkq_sched(cuflow_workq_fn_t fn);

/*!Debug dump of global work queues. */
void cuflow_gworkq_dump(FILE *out);

extern pthread_mutex_t cuflowP_gworkq_mutex;
extern pthread_cond_t cuflowP_gworkq_cond;

CU_SINLINE void
cuflow_gworkq_lock()
{ cu_mutex_lock(&cuflowP_gworkq_mutex); }

CU_SINLINE void
cuflow_gworkq_unlock()
{ cu_mutex_unlock(&cuflowP_gworkq_mutex); }

CU_SINLINE void
cuflow_gworkq_broadcast()
{ pthread_cond_broadcast(&cuflowP_gworkq_cond); }

/*!@}*/
CU_END_DECLARATIONS

#endif
