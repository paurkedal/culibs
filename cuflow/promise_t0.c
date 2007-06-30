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

#include <cuflow/promise.h>
#include <cu/debug.h>
#include <cu/memory.h>
#include <unistd.h>
#include <time.h>

#define TH_CNT 10
#define JOB_CNT 20

static struct cuflow_promise_s promise;
static pthread_key_t thread_info_key;

struct thread_info_s
{
    int i_th;
    cuflow_priority_t priority;
};

struct job_result_s
{
    int i_th;
    cuflow_priority_t sched_pri;
    cuflow_priority_t run_pri;
    cuflow_promise_state_t pst;
} job_result[JOB_CNT];

cu_clos_def(job, cu_prot0(void),
    ( int i;
      cuflow_priority_t pri;
      cuflow_promise_t promise; ))
{
    cu_clos_self(job);
    struct thread_info_s *ti = pthread_getspecific(thread_info_key);
    cuflow_promise_state_t pst;
    struct job_result_s *jr = &job_result[self->i];
    cu_debug_assert(self->pri <= cuflow_gworkq_current_priority());
    jr->run_pri = cuflow_gworkq_current_priority();
    while (drand48() > 0.000001);
    pst = cuflow_promise_increment_state(self->promise);
    cu_debug_assert(pst - cuflow_promise_state_initiated <= JOB_CNT);
    if (pst - cuflow_promise_state_initiated == JOB_CNT) {
	printf("done\n");
	cuflow_promise_set_state(self->promise,
				 cuflow_promise_state_fulfilled);
    }
    jr->i_th = ti->i_th;
    jr->sched_pri = self->pri;
    jr->pst = pst;
}

cu_clop_def(initiator, void, cuflow_promise_t promise)
{
    int i;
    printf("scheduling\n");
    for (i = 0; i < JOB_CNT; ++i) {
	job_t *cb = cu_gnew(job_t);
	cb->i = i;
	cb->pri = cuflow_gworkq_current_priority();
	cb->promise = promise;
	cuflow_gworkq_sched(job_prep(cb));
    }
    printf("done scheduling\n");
    cuflow_gworkq_dump(stdout);
}

static void *
thread_main(void *thread_info)
{
#define thread_info ((struct thread_info_s *)thread_info)
    cuflow_priority_t pri = thread_info->priority;
    pthread_setspecific(thread_info_key, thread_info);
    cuflow_gworkq_set_static_priority(pri);
    cuflow_promise_fulfill(&promise);
    return NULL;
#undef thread_info
}

int
main()
{
    pthread_t th[TH_CNT];
    struct thread_info_s thread_info[TH_CNT];
    int i_th;
    int i;

    cuflow_init();

    pthread_key_create(&thread_info_key, NULL);
    srand48(time(NULL));
    cuflow_promise_cct(&promise, initiator);
    for (i_th = 0; i_th < TH_CNT; ++i_th) {
	struct thread_info_s *ti = &thread_info[i_th];
	ti->i_th = i_th;
	ti->priority = lrand48() % cuflow_priority_postmax;
	printf("Thread %d, priority = %d\n", i_th, ti->priority);
	if (cu_pthread_create(&th[i_th], NULL, thread_main,
			      &thread_info[i_th]) != 0) {
	    fprintf(stderr, "Could not create thread.\n");
	    exit(2);
	}
    }
    for (i_th = 0; i_th < TH_CNT; ++i_th)
	cu_pthread_join(th[i_th], NULL);
    for (i = 0; i < JOB_CNT; ++i) {
	struct job_result_s *jr = &job_result[i];
	printf("job %d: i_th %d, sched_pri = %d, run_pri = %d, pst = %d\n",
	       i, jr->i_th, jr->sched_pri, jr->run_pri, jr->pst);
    }
    return 0;
}
