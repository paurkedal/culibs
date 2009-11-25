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

#include <cuflow/sched.h>
#include <cuflow/tstate.h>
#include <cuflow/workers.h>
#include <cuflow/cdisj.h>

cu_dlog_def(_file, "dtag=cuflow.sched");

void
cuflowP_sched_call(cuflow_exeq_t exeq, cu_clop0(fn, void),
		   AO_t *cdisj)
{
    cuflow_exeq_entry_t ent = &exeq->call_arr[exeq->head];
    cu_dlogf(_file,
	     "ENQUEUE %p %ld %ld: fn=%p qsize=%ld; thread=0x%lx; tstate=%p",
	     exeq, exeq->head, exeq->tail, fn,
	     ((exeq->head - exeq->tail - 1) & CUFLOW_EXEQ_MASK),
	     (long)pthread_self(), cuflow_tstate());
    ent->fn = fn;
    ent->cdisj = cdisj;
    AO_fetch_and_add1(cdisj);
    AO_store_release_write(&exeq->head, (exeq->head + 1) & CUFLOW_EXEQ_MASK);

    cuflow_workers_incr_pending();
#if CUFLOW_PROFILE_SCHED
    AO_fetch_and_add1(&cuflowP_profile_sched_count);
#endif
}

void
cuflowP_sched_call_sub1(cuflow_exeq_t exeq, cu_clop0(fn, void),
			AO_t *cdisj)
{
    cuflow_exeq_entry_t ent = &exeq->call_arr[exeq->head];
    cu_dlogf(_file,
	     "ENQUEUE %p %ld %ld: fn=%p qsize=%ld; thread=0x%lx; tstate=%p",
	     exeq, exeq->head, exeq->tail, fn,
	     ((exeq->head - exeq->tail - 1) & CUFLOW_EXEQ_MASK),
	     (long)pthread_self(), cuflow_tstate());
    ent->fn = fn;
    ent->cdisj = cdisj;
    AO_store_release_write(&exeq->head, (exeq->head + 1) & CUFLOW_EXEQ_MASK);

    cuflow_workers_incr_pending();
#if CUFLOW_PROFILE_SCHED
    AO_fetch_and_add1(&cuflowP_profile_sched_count);
#endif
}

void
cuflow_sched_call_at(cu_clop0(f, void), AO_t *cdisj, cuflow_exeqpri_t pri)
{
    cuflow_tstate_t tstate = cuflow_tstate();
    cuflow_exeqpri_t old_pri = tstate->exeqpri;
    tstate->exeqpri = pri;
    cuflow_sched_call_on(f, cdisj, &tstate->exeq[pri]);
    tstate->exeqpri = old_pri;
}

void
cuflow_sched_call_sub1_at(cu_clop0(f, void), AO_t *cdisj, cuflow_exeqpri_t pri)
{
    cuflow_tstate_t tstate = cuflow_tstate();
    cuflow_exeqpri_t old_pri = tstate->exeqpri;
    tstate->exeqpri = pri;
    cuflow_sched_call_sub1_on(f, cdisj, &tstate->exeq[pri]);
    tstate->exeqpri = old_pri;
}

cu_clop_edef(cuflowP_schedule, void, cu_bool_t is_global)
{
    cuflow_tstate_t ts0 = cuflow_tstate();
    cuflow_exeqpri_t pri;
    cuflow_exeqpri_t caller_pri = ts0->exeqpri;
    for (pri = cuflow_exeqpri_begin; cuflow_exeqpri_prioreq(pri, caller_pri);
	 pri = cuflow_exeqpri_succ(pri)) {
	cuflow_tstate_t ts = ts0;
	do {
	    cuflow_exeq_t exeq = &ts->exeq[pri];
	    /* The pickup_mutex atomise picking up the entry at exeq->tail and
	     * incrementing exeq->tail. */
	    cu_mutex_lock(&exeq->pickup_mutex);
	    cu_dlogf(_file, "CHECKQUEUE %p %ld %ld; cnt=%ld",
		     exeq, exeq->head, exeq->tail,
		     ((exeq->head - exeq->tail - 1) & CUFLOW_EXEQ_MASK));
	    while (((AO_load_acquire_read(&exeq->head) - exeq->tail - 1)
		    & CUFLOW_EXEQ_MASK) > 0) {
		cu_clop0(fn, void);
		AO_t *cdisj;
		cuflow_exeq_entry_t ent;
		AO_t new_tail = (exeq->tail + 1) & CUFLOW_EXEQ_MASK;
		ent = &exeq->call_arr[new_tail];
		fn = ent->fn;
		cdisj = ent->cdisj;
		cu_dlogf(_file, "DEQUEUE %p %ld %ld: fn=%p",
			 exeq, exeq->head, exeq->tail, fn);
		/* Atomically update tail to the benefit of
		 * cuflow_try_sched_call. Release fence to make sure we have
		 * read the entry before it gets overwritten by the same
		 * function. */
		AO_store_release(&exeq->tail, new_tail);
		cuflow_workers_decr_pending();
		cu_mutex_unlock(&exeq->pickup_mutex);
		ts0->exeqpri = pri;
		cu_call0(fn);
		cu_dlogf(_file, "Done job %p, decrementing %p.", fn, cdisj);
		cuflow_cdisj_sub1_release_write(cdisj);
		cu_mutex_lock(&exeq->pickup_mutex);
	    }
	    cu_mutex_unlock(&exeq->pickup_mutex);
	    if (!is_global)
		break;
	    ts = cuflow_tstate_next(ts);
	} while (ts != ts0);
    }
    ts0->exeqpri = caller_pri;
}

#if CUFLOW_PROFILE_SCHED
AO_t cuflowP_profile_sched_count = 0;
AO_t cuflowP_profile_nonsched_count = 0;

static void
cuflowP_profile_dump()
{
    printf("    scheduled calls: %ld\n"
	   "non-scheduled calls: %ld\n",
	   (long)cuflowP_profile_sched_count,
	   (long)cuflowP_profile_nonsched_count);
}
#endif

void
cuflowP_exeq_init_tstate(cuflow_tstate_t ts)
{
    cuflow_exeqpri_t pri;
    ts->exeqpri = cuflow_exeqpri_normal;
    for (pri = cuflow_exeqpri_begin;
	 pri != cuflow_exeqpri_end;
	 pri = cuflow_exeqpri_succ(pri)) {
	cuflow_exeq_t exeq = &ts->exeq[pri];
	cu_mutex_init(&exeq->pickup_mutex);
	exeq->priority = pri;
	exeq->head = 0;
	exeq->tail = CUFLOW_EXEQ_MASK;
#if CUFLOW_CALLS_BETWEEN_SCHED > 1
	exeq->calls_till_sched = CUFLOW_CALLS_BETWEEN_SCHED;
#endif
    }
}

void cuflowP_sched_init()
{
    cuflow_workers_register_scheduler(cuflowP_schedule);
#if CUFLOW_PROFILE_SCHED
    atexit(cuflowP_profile_dump);
#endif
}
