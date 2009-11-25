/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2006--2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cuflow/workers.h>
#include <cuflow/sched.h>
#include <cuflow/timespec.h>
#include <cucon/priq.h>
#include <cucon/list.h>
#include <cu/thread.h>
#include <cu/memory.h>
#include <errno.h>
#include <atomic_ops.h>

cu_dlog_def(_file, "dtag=cuflow.workers");

typedef struct _atjob  *_atjob_t;
typedef struct _worker *_worker_t;

struct _atjob
{
    struct timespec t_call;
    cu_clop(callback, void, struct timespec *current_time);
};

struct _worker
{
    pthread_t thread;
    cu_bool_t do_shutdown;
};

static pthread_mutex_t		_work_mutex = CU_MUTEX_INITIALISER;
static pthread_cond_t		_work_cond = PTHREAD_COND_INITIALIZER;
static struct cucon_priq	_work_atq;
static struct cucon_list	_work_nowlist;
static struct cucon_list	_work_scheduler_list;

static pthread_mutex_t		_workers_mutex = CU_MUTEX_INITIALISER;
static struct cucon_list	_workers_list; /* of struct _worker */
static AO_t			_workers_count = 0;

/* Other parts doing "on signal" work can increment this when there is work to
 * be done, so to prevent threads from sleeping on _work_cond.  It can then
 * broadcast to wake up those threads which are already sleeping on
 * _work_cond. */
AO_t cuflowP_pending_work = 0;

AO_t cuflowP_workers_waiting_count = 0;


cu_clop_def(_atjob_before_clop, cu_bool_t, void *atjob0, void *atjob1)
{
    return cuflow_timespec_lt(&((_atjob_t)atjob0)->t_call,
			      &((_atjob_t)atjob1)->t_call);
}

void
cuflow_workers_register_scheduler(cu_clop(sched, void, cu_bool_t))
{
    cu_mutex_lock(&_work_mutex);
    cucon_list_append_ptr(&_work_scheduler_list, (void *)sched);
    cu_mutex_unlock(&_work_mutex);
}

static void
_worker_atrun(_worker_t worker)
{
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    while (!cucon_priq_is_empty(&_work_atq)) {
	_atjob_t atjob = cucon_priq_front(&_work_atq);
	_atjob_t atjob_next;
	if (cuflow_timespec_lt(&now, &atjob->t_call))
	    break;
	cucon_priq_pop_front(&_work_atq);

	/* Signal more worker threads in case there is more to do right
	 * away. */
	atjob_next = cucon_priq_front(&_work_atq);
	if (cuflow_timespec_lt(&now, &atjob_next->t_call))
	    pthread_cond_signal(&_work_cond);

	cu_mutex_unlock(&_work_mutex);
	cu_call(atjob->callback, &now);
	cu_mutex_lock(&_work_mutex);
    }
}

void
cuflowP_workers_broadcast(void)
{
    cu_mutex_lock(&_work_mutex);
    pthread_cond_broadcast(&_work_cond);
    cu_mutex_unlock(&_work_mutex);
}

static void
cuflowP_yield_lck(cu_bool_t is_global)
{
    cucon_listnode_t itr;
    for (itr = cucon_list_begin(&_work_scheduler_list);
	 itr != cucon_list_end(&_work_scheduler_list);
	 itr = cucon_listnode_next(itr)) {
	cu_clop(callback, void, cu_bool_t);
	cu_debug_assert(itr);
	callback = cucon_listnode_ptr(itr);
	cu_mutex_unlock(&_work_mutex);
	cu_call(callback, is_global);
	cu_mutex_lock(&_work_mutex);
    }
}

void
cuflowP_yield(void)
{
    cu_mutex_lock(&_work_mutex);
    cuflowP_yield_lck(cu_false);
    cu_mutex_unlock(&_work_mutex);
}

#if 0
void
cuflowP_cdisj_wait_while(AO_t *cdisj, cu_bool_t cond_val)
{
    /* Here the cdisj has failed... */
    do {
	/* First make sure it's scheduled, */
	if (AO_load(&cuflowP_pending_work)) {
	    cuflow_yield();
	    if (!!AO_load(cdisj) == cond_val)
		return;
	}

	/* but then it's possible that other threads are working on the last
	 * prerequisites, so prepare to wait. */
	cu_mutex_lock(&_work_mutex);

	/* Notify that we are going to wait.  This forces other threads to
	 * sync on _work_mutex and signal us if they enqueue any work. */
	AO_fetch_and_add1_acquire(&cuflowP_workers_waiting_count);

	/* Then a last check if condition is fulfilled, */
	if (!!AO_load_acquire(cdisj) == cond_val) {
	    AO_fetch_and_sub1(&cuflowP_workers_waiting_count);
	    cu_mutex_unlock(&_work_mutex);
	    return;
	}
	/* or if more work is present before we sleep */
	if (!AO_load_acquire(&cuflowP_pending_work))
	    pthread_cond_wait(&_work_cond, &_work_mutex);
	AO_fetch_and_sub1(&cuflowP_workers_waiting_count);
	cu_mutex_unlock(&_work_mutex);
    } while (!AO_load_acquire(cdisj) == cond_val);
}
#endif

static void *
_worker_main(void *worker)
{
#define worker ((_worker_t)worker)
    cu_mutex_lock(&_work_mutex);
    while (!worker->do_shutdown) {
	cu_clop0(callback, void);
	while (!cucon_list_is_empty(&_work_nowlist)) {
	    callback = cucon_list_front_ptr(&_work_nowlist);
	    cucon_list_pop_front(&_work_nowlist);
	    cu_mutex_unlock(&_work_mutex);
	    cu_call0(callback);
	    cu_mutex_lock(&_work_mutex);
	    if (worker->do_shutdown)
		goto break_0;
	}
	if (AO_load(&cuflowP_pending_work))
	    cuflowP_yield_lck(cu_true);
	if (worker->do_shutdown)
	    break;
	AO_fetch_and_add1(&cuflowP_workers_waiting_count);
	if (!AO_load_acquire(&cuflowP_pending_work)) {
	    if (cucon_priq_is_empty(&_work_atq))
		pthread_cond_wait(&_work_cond, &_work_mutex);
	    else {
		_atjob_t atjob = cucon_priq_front(&_work_atq);
		int err;
		cu_debug_assert(atjob->t_call.tv_nsec >= 0);
		cu_debug_assert(atjob->t_call.tv_nsec < 1000000000);
		err = pthread_cond_timedwait(&_work_cond, &_work_mutex,
					     &atjob->t_call);
		cu_debug_assert(atjob->t_call.tv_nsec >= 0);
		cu_debug_assert(atjob->t_call.tv_nsec < 1000000000);
		switch (err) {
		    case 0:
			break;
		    case ETIMEDOUT:
			if (worker->do_shutdown)
			    break;
			_worker_atrun(worker);
			break;
		    default:
			cu_bugf("Unexpected failure of "
				"pthread_cond_timedwait: %s", strerror(err));
		}
	    }
	}
	AO_fetch_and_sub1(&cuflowP_workers_waiting_count);
    }
break_0:
    pthread_cond_signal(&_work_cond);
    cu_mutex_unlock(&_work_mutex);
    return NULL;
#undef worker
}

int
cuflow_workers_count()
{
    return AO_load_acquire_read(&_workers_count);
}

static void
_workers_drop_lck(int target_count)
{
    do {
	int last_workers_count;
	_worker_t worker;

	cu_debug_assert(!cucon_list_is_empty(&_workers_list));
	worker = cucon_list_front_mem(&_workers_list);
	cucon_list_pop_front(&_workers_list);
	--_workers_count;
	last_workers_count = _workers_count;

	cu_mutex_unlock(&_workers_mutex);
	cu_mutex_lock(&_work_mutex);
	worker->do_shutdown = cu_true;
	cu_mutex_unlock(&_work_mutex);
	cu_dlogf(_file,
		 "Shutting down %p, _workers_count=%d, workers_waiting=%d",
		 worker, _workers_count, cuflowP_workers_waiting_count);
	cuflow_workers_broadcast();
	pthread_join(worker->thread, NULL);
	cu_mutex_lock(&_workers_mutex);

	/* If strict monononicity of _workers_count is broken, it means that
	 * another thread increased the number of workers while _workers_mutex
	 * was unlocked.  If so, give up. */
	if (_workers_count > last_workers_count)
	    break; /* Another thread is trying to spawn new workers. */
    } while (target_count < _workers_count);
}

static void
_workers_spawn_lck(int target_count)
{
    while (target_count > _workers_count) {
	cucon_listnode_t worker_it;
	_worker_t worker;

	worker_it = cucon_list_append_mem(&_workers_list,
					  sizeof(struct _worker));
	worker = cucon_listnode_mem(worker_it);
	worker->do_shutdown = cu_false;
	cu_pthread_create(&worker->thread, NULL, _worker_main, worker);
	++_workers_count;
    }
}

void
cuflow_workers_spawn(int target_count)
{
    cu_mutex_lock(&_workers_mutex);
    if (target_count < _workers_count)
	_workers_drop_lck(target_count);
    else if (target_count > _workers_count)
	_workers_spawn_lck(target_count);
    cu_mutex_unlock(&_workers_mutex);
}

void
cuflow_workers_spawn_at_most(int target_count)
{
    cu_mutex_lock(&_workers_mutex);
    if (target_count < _workers_count)
	_workers_drop_lck(target_count);
    cu_mutex_unlock(&_workers_mutex);
}

void
cuflow_workers_spawn_at_least(int target_count)
{
    cu_mutex_lock(&_workers_mutex);
    if (target_count > _workers_count)
	_workers_spawn_lck(target_count);
    cu_mutex_unlock(&_workers_mutex);
}

void
cuflow_workers_call_at(cu_clop(callback, void, struct timespec *t_now),
		       struct timespec *t_call)
{
    _atjob_t atjob = cu_gnew(struct _atjob);
    _atjob_t atjob_prev_head;

    cu_debug_assert(atjob->t_call.tv_nsec < 1000000000);
    cu_debug_assert(atjob->t_call.tv_nsec >= 0);
    memcpy(&atjob->t_call, t_call, sizeof(struct timespec));
    atjob->callback = callback;

    cu_mutex_lock(&_work_mutex);
    if (cucon_priq_is_empty(&_work_atq))
	atjob_prev_head = NULL;
    else
	atjob_prev_head = cucon_priq_front(&_work_atq);
    cucon_priq_insert(&_work_atq, atjob);

    /* Make sure we have at least one worker which will do the timed job.  This
     * worker will signal (from _worker_atrun) another worker if there is more
     * work to do right away. */
    if (!atjob_prev_head || cuflow_timespec_lt(t_call, &atjob_prev_head->t_call))
	pthread_cond_signal(&_work_cond);

    cu_mutex_unlock(&_work_mutex);
}

void
cuflow_workers_call(cu_clop0(callback, void))
{
    cu_mutex_lock(&_work_mutex);
    cucon_list_append_ptr(&_work_nowlist, (void *)callback);
    pthread_cond_signal(&_work_cond);
    cu_mutex_unlock(&_work_mutex);
}

void
cuflow_workers_call_on_signal(cu_clop0(callback, void))
{
    cu_mutex_lock(&_work_mutex);
    cucon_list_append_ptr(&_work_scheduler_list, (void *)callback);
    pthread_cond_signal(&_work_cond);
    cu_mutex_unlock(&_work_mutex);
}

static void
_workers_cleanup(void)
{
    cuflow_workers_spawn(0);
}

void
cuflowP_workers_init()
{
    cucon_priq_init(&_work_atq, _atjob_before_clop);
    cucon_list_init(&_work_nowlist);
    cucon_list_init(&_work_scheduler_list);
    cucon_list_init(&_workers_list);
    atexit(_workers_cleanup);
}
