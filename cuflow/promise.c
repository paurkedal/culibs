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

#include <cuflow/promise.h>
#include <cuflow/gflexq.h>

void
cuflow_promise_init(cuflow_promise_t promise,
		    cu_clop(initiator, void, cuflow_promise_t promise))
{
    promise->state = cuflow_promise_state_latent;
    promise->u0.initiator = initiator;
}

void
cuflowP_promise_process(cuflow_promise_t promise,
			cuflow_promise_state_t state)
{
    cuflow_gworkq_lock();
    if (promise->state < state) {
	cuflow_gflexq_t gflexq;
	struct cuflow_gflexq_entry entry;
	if (promise->state == cuflow_promise_state_latent) {
	    /* Replace initiator with gflexq. */
	    cuflow_promise_initiator_t initiator;
	    initiator = promise->u0.initiator;
	    gflexq = cuflow_gflexq_new(cuflow_gworkq_current_priority());
	    promise->u0.gflexq = gflexq;
	    promise->state = cuflow_promise_state_initiated;

	    /* Call with gflexq. */
	    cuflow_gflexq_enter_glck(gflexq, &entry);
	    cuflow_gworkq_unlock();
	    cu_call(initiator, promise);
	    cuflow_gworkq_lock();
	    while (promise->state < state)
		cuflow_gworkq_yield_gcond_glck();
	    cuflow_gflexq_leave_glck(&entry);
	    if (AO_load_acquire_read(&promise->state) >= state) {
		cuflow_gworkq_unlock();
		return;
	    }
	}
	else if (promise->state < cuflow_promise_state_fulfilled) {
	    gflexq = promise->u0.gflexq;
	    cuflow_gflexq_enter_glck(gflexq, &entry);
	    while (promise->state < state)
		cuflow_gworkq_yield_gcond_glck();
	    cuflow_gflexq_leave_glck(&entry);
	}
	else
	    while (promise->state < state)
		cuflow_gworkq_yield_gcond_glck();
    }
    cuflow_gworkq_unlock();
}

void
cuflow_promise_set_state(cuflow_promise_t promise,
			 cuflow_promise_state_t state)
{
    cuflow_promise_state_t old_state = AO_load_acquire_read(&promise->state);
    if (cuflow_promise_state_fulfilled <= state
	&& old_state < cuflow_promise_state_fulfilled) {
	cuflow_gworkq_lock();
	promise->u0.value = NULL;
	AO_store_release_write(&promise->state, state);
	cuflow_gworkq_unlock();
	cuflow_gworkq_broadcast();
    }
    else if (old_state < state) {
	/* Some other thread may race us, but it only means an extra
	 * broadcast, and the state assignment must be guaranteed monotonic
	 * by the client. */
	AO_store_release_write(&promise->state, state);
	cuflow_gworkq_broadcast();
    }
    else if (old_state > state)
	cu_bugf("cuflow_promise_set_state: client tried to decrease "
		"state.");
}

void
cuflow_promise_set_fulfilled(cuflow_promise_t promise, void *value)
{
    cuflow_promise_state_t old_state = AO_load_acquire_read(&promise->state);
    if (old_state < cuflow_promise_state_fulfilled) {
	cuflow_gworkq_lock();
	promise->u0.value = value;
	AO_store_release_write(&promise->state,
			       cuflow_promise_state_fulfilled);
	cuflow_gworkq_unlock();
	cuflow_gworkq_broadcast();
    }
}

cuflow_promise_state_t
cuflow_promise_increment_state(cuflow_promise_t promise)
{
    cuflow_promise_state_t st = AO_fetch_and_add1(&promise->state);
    cuflow_gworkq_broadcast();
    ++st;
    if (st == cuflow_promise_state_fulfilled) {
	cuflow_gworkq_lock();
	promise->u0.gflexq = NULL;
	cuflow_gworkq_unlock();
    }
    return st;
}
