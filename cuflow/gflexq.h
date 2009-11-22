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

#ifndef CUFLOW_GFLEXQ_H
#define CUFLOW_GFLEXQ_H

#include <cuflow/fwd.h>
#include <cuflow/gworkq.h>
#include <cucon/list.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuflow_gflexq_h cuflow/gflexq.h Global Work Queue with Flexible Priority
 * @{\ingroup cuflow_mod
 *
 * This module provides a data structure \ref cuflow_gflexq, a work
 * queue linked to the global work queue at a modifiable priority.
 * These queues are hierachical, each queue can have several subqueues,
 * and several queues can share the same subqueue.  The hierachical
 * structure is used to bump up the priority of all dependent calculations
 * when a high priority job requires the calculation represented by a
 * queue (and its subqueues).
 *
 * The main purpose of all this is to implement \ref cuflow_promise_h
 * "promises", which present a high-level interface for work-sharing
 * between threads and guard the result so that it is not accessed
 * before it is finished. */

/*!A work queue linked to the global work queue at a flexible priority. */
struct cuflow_gflexq
{
    cu_inherit (cuflow_workq);
    AO_t priority;
    cuflow_gflexq_t next, prev;  /* gworkq links */
    struct cucon_list subqueue_list;
};

typedef struct cuflow_gflexq_entry_s *cuflow_gflexq_entry_t;
/*!The state saved by \ref cuflow_gflexq_enter. */
struct cuflow_gflexq_entry_s
{
    cuflow_gflexq_t gflexq;
    cucon_listnode_t subqueue_it;
};

/*!Construct \a flexq with initial priority \a initpri. */
void cuflow_gflexq_init(cuflow_gflexq_t flexq, cuflow_priority_t initpri);

/*!Return an empty queue with initial priority \a initpri. */
cuflow_gflexq_t cuflow_gflexq_new(cuflow_priority_t initpri);

/*!The priority at which work on \a flexq will be run. */
CU_SINLINE cuflow_priority_t
cuflow_gflexq_priority(cuflow_gflexq_t flexq)
{ return AO_load_acquire_read(&flexq->priority); }

/* Set the priority for work queued on \a flexq.  This can be called at
 * any time, and will cause already scheduled work on \a flexq to be run
 * at the new priority.  Consider \ref cuflow_gflexq_raise_priority
 * if you are using hierachical queues. */
/*void cuflow_gflexq_set_priority(cuflow_gflexq_t flexq,
				cuflow_priority_t new_priority);*/

/*!Raise the priority of \a flexq and all subqueues to at least \a
 * min_priority.  This function will never lower the priority of any
 * queues.  This semantics is acertains that when the same work is
 * requested by functions running at different priorities, it will be
 * scheduled at the highest priority amoung the requestors. */
void cuflow_gflexq_raise_priority(cuflow_gflexq_t flexq,
				  cuflow_priority_t min_priority);

void cuflow_gflexq_raise_priority_glck(cuflow_gflexq_t flexq,
				       cuflow_priority_t min_priority);

/*!Schedule \a fn on \a flexq.  If the queue is empty, this will cause
 * \a flexq to be activated by putting it on the global queue at the
 * predefined priority. */
void cuflow_gflexq_sched(cuflow_gflexq_t flexq, cuflow_workq_fn_t fn);

void cuflow_gflexq_sched_glck(cuflow_gflexq_t flexq, cuflow_workq_fn_t fn);

/*!Set \a flexq as the work queue for subsequent \ref cuflow_gworkq_sched
 * operations, and save the current state to \a entry for restoration
 * with \ref cuflow_gflexq_leave.  Several context may enter the same
 * queue, which implies that one queue can have several parent queues.
 * When the priority of a parent queue is raised, it affects all subqueues
 * recursively. */
void cuflow_gflexq_enter(cuflow_gflexq_t flexq, cuflow_gflexq_entry_t entry);

/*!Same as \ref cuflow_gflexq_enter, but assuming gworkq-lock is held. */
void cuflow_gflexq_enter_glck(cuflow_gflexq_t flexq,
			      cuflow_gflexq_entry_t entry);

/*!Drop the current work queue and restore the state saved in \a entry,
 * as obtain from the matching \ref cuflow_gflexq_enter.  This operation
 * also unlinks the subqueue, so that it will no longer be affected by
 * changes to the priority of its no-longer parent. */
void cuflow_gflexq_leave(cuflow_gflexq_entry_t entry);

/*!Same as \ref cuflow_gflexq_leave, but assuming gworkq-lock is held. */
void cuflow_gflexq_leave_glck(cuflow_gflexq_entry_t entry);

/*!@}*/
CU_END_DECLARATIONS

#endif
