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

#ifndef CUFLOW_PROMISE_H
#define CUFLOW_PROMISE_H

#include <cuflow/fwd.h>
#include <cuflow/gworkq.h>


CU_BEGIN_DECLARATIONS
/*!\defgroup cuflow_promise_h cuflow/promise.h: Delayed Fulfillment of Computations
 * @{\ingroup cuflow_oldsmp_mod
 *
 * \deprecated Similar functionality is provided by \ref cuflow_sched_h
 * "cuflow/sched.h" using \ref cuflow_cdisj_h "cuflow/cdisj.h" for guarding the
 * result of a computation.
 *
 * A promise is a handle which contains a task to be performed at a later
 * time, and a state of the computation.  The computation may schedule
 * further work on the \ref cuflow_gworkq_h "global work queue", in which
 * case other threads requesting the computation can take part in the
 * workload.
 *
 * The state is initially \ref cuflow_promise_state_latent.  When the
 * promise is requested, the state is updated to
 * \ref cuflow_promise_state_initiated before the main task is called.
 * Subsequent states may be written to the promise by the computation
 * to represent various degrees of fulfillment.  In the simplest case
 * there is one more states, meaning that the promise is fulfilled.  When
 * working on recursive structures, a four-state promise may be needed,
 * where first additional state means that a data-structure is allocated
 * so that the pointer is available for linking, and the last state means
 * that the data structure is constructed and thus fully usable. */

/*!The state of a promise. */
typedef int cuflow_promise_state_t;
typedef cu_clop(cuflow_promise_initiator_t, void, cuflow_promise_t);

/*!The promise has not yet been requested. */
#define cuflow_promise_state_latent 0

/*!The promise has been requested and the main task have been started. */
#define cuflow_promise_state_initiated 1

/*!When this state or a higher state is written to the promise by the client
 * implementation, which may be either the initiator or a job submitted by it,
 * the reference to the work queue will be unlinked from the promise, so that
 * it can be freed by the collector when it becomes empty (though it will
 * usually already be empty). */
#define cuflow_promise_state_fulfilled 0x8000

struct cuflow_promise
{
    AO_t state;
    union {
	cuflow_promise_initiator_t initiator;
	cuflow_gflexq_t gflexq;
	void *value;
    } u0;
};

/*!Construct \a promise which will be fulfilled by calling \a initiator
 * and performing any work which it passes onto the work queue of \a
 * promise. */
void cuflow_promise_init(cuflow_promise_t promise,
			 cu_clop(initiator, void, cuflow_promise_t));

void cuflowP_promise_process(cuflow_promise_t promise,
			     cuflow_promise_state_t state);

/*!Return the current state of the promise. */
CU_SINLINE cuflow_promise_state_t
cuflow_promise_state(cuflow_promise_t promise)
{
    return AO_load_acquire_read(&promise->state);
}

/*!Set the state of \a promise to \a state.
 * \pre \a state is greater than the current state. */
void cuflow_promise_set_state(cuflow_promise_t promise,
			      cuflow_promise_state_t state);

/*!Set the state of \a promise to \a cuflow_promise_state_fulfilled and store
 * \a ptr with its value.  Data stored before this state is reached must be
 * stored elsewhere, like after the promise struct, cf \ref cuflow_promise_mem,
 * or by embedding the promise struct in a larger struct.  In that can you
 * may pass \a ptr to point to it, or <tt>\a ptr = NULL</tt>. */
void cuflow_promise_set_fulfilled(cuflow_promise_t promise, void *ptr);

/*!Increment the state of \a promise and return the post value. */
cuflow_promise_state_t
cuflow_promise_increment_state(cuflow_promise_t promise);

/*!Fulfill \a promise upto \a state by calling the initiator if not already
 * done, then call the work queue and possibly wait for other threads which
 * process the promise, until the state is reached. */
CU_SINLINE void
cuflow_promise_process(cuflow_promise_t promise, cuflow_promise_state_t state)
{
    if (AO_load_acquire_read(&promise->state) < state)
	cuflowP_promise_process(promise, state);
}

/*!Fulfill \a promise upto \a cuflow_promise_state_fulfilled, and return
 * the value that it set with \a cuflow_promise_set_fulfilled.  Some
 * promises may wish to store the value elsewhere, like after the promise
 * struct. */
CU_SINLINE void *
cuflow_promise_fulfill(cuflow_promise_t promise)
{
    cuflow_promise_process(promise, cuflow_promise_state_fulfilled);
    return promise->u0.value;
}

/*!Returns a pointer to the end of the \a promise struct itself, which may
 * be used by client, if allocated, to store working state or final data. */
CU_SINLINE void *
cuflow_promise_mem(cuflow_promise_t promise)
{
    return promise + 1;
}

/*!@}*/
CU_END_DECLARATIONS

#endif
