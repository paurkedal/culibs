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

#include <cuflow/gworkq.h>
#include <cuflow/gflexq.h>
#include <cu/debug.h>
#include <cu/memory.h>
#include <cu/clos.h>
#include <cu/test.h>
#include <stdio.h>

#define SLICE_CNT 10000
#define N_MAX 1000000
#define STEP (N_MAX/SLICE_CNT)
#define THR_CNT 16
#define FLEXQ_CNT 10000

cu_clos_def(message_cb,
	    cu_prot0(void),
    ( char *msg;
      cu_bool_t verbose; ))
{
    cu_clos_self(message_cb);
    if (self->verbose)
	fputs(self->msg, stdout);
}

cu_clop0(make_message(char *str, int verbose), void)
{
    message_cb_t *cb = cu_gnew(message_cb_t);
    cb->msg = str;
    cb->verbose = verbose;
    return message_cb_prep(cb);
}


cu_clos_def(comp,
	    cu_prot0(void),
    ( int cnt;
      double i0;
      double res; ))
{
    cu_clos_self(comp);
    int cnt = self->cnt;
    double i = self->i0;
    double res = 0.0;
    //printf("running comp(%lf, %d)\n", i, cnt);
    while (--cnt) {
	if (cnt % 2)
	    res += 1.0/i;
	else
	    res -= 1.0/i;
	i += 1.0;
    }
    self->res = res;
}

void
sched_rt()
{
    comp_t *c = cu_gnew(comp_t);
    c->cnt = 1;
    c->i0 = 1.0;
    cuflow_gworkq_sched_at(comp_prep(c), cuflow_priority_realtime);
}

void
test_gworkq(cu_bool_t verbose)
{
    int n;
    double i0 = 1.0;
    double res;
    comp_t *comp_arr[SLICE_CNT];
    for (n = 0; n < SLICE_CNT; ++n) {
	comp_arr[n] = cu_gnew(comp_t);
	comp_arr[n]->cnt = STEP;
	comp_arr[n]->i0 = i0;
	//printf("scheduling comp(%lf, %d)\n", i0, STEP);
	cuflow_gworkq_sched(comp_prep(comp_arr[n]));
	i0 += STEP;
	if (n % 1000 == 0)
	    sched_rt();
	if (((n - 11) & (n - 10)) == 0) {
	    if (verbose)
		cuflow_gworkq_dump(stdout);
	    cuflow_gworkq_yieldall_prioreq();
	}
    }
    cuflow_gworkq_yieldall_prioreq();
    res = 0.0;
    for (n = 0; n < SLICE_CNT; ++n)
	res += comp_arr[n]->res;
    if (verbose) {
	printf("Result: %lf\n", res);
	cuflow_gworkq_dump(stdout);
    }
    if (THR_CNT == 1) /* Need promises to ascertain result for MT case! */
	cu_test_assert(res > 0.79102 && res < 0.79103);
}

void
test_gflexq(cu_bool_t verbose)
{
    int n;
    for (n = 0; n < FLEXQ_CNT; ++n) {
	cuflow_gflexq_t fq0 = cuflow_gflexq_new(cuflow_priority_normal);
	if (verbose) {
	    printf("created gflexq, but no jobs (gworkq should be empty)\n");
	    cuflow_gworkq_dump(stdout);
	}
	cuflow_gflexq_sched(fq0, make_message("first gflexq job\n", verbose));
	cuflow_gflexq_sched(fq0, make_message("second gflexq job\n", verbose));
	if (verbose) {
	    printf("scheduled jobs\n");
	    cuflow_gworkq_dump(stdout);
	}
	cuflow_gflexq_raise_priority(fq0, cuflow_priority_interactive);
	if (verbose)
	    cuflow_gworkq_dump(stdout);
	cuflow_gworkq_yieldall_prioreq();
	if (verbose)
	    cuflow_gworkq_dump(stdout);
	verbose = 0;
    }
}

void *thread_main(void *_i_th)
{
    int i_th = *(int *)_i_th;
    test_gworkq(i_th == -1);
    test_gflexq(i_th == -1);
    return NULL;
}

int main()
{
    int i_th;
    pthread_t th[THR_CNT];
    cuflow_init();
    for (i_th = 0; i_th < THR_CNT; ++i_th) {
	if (cu_pthread_create(&th[i_th], NULL, thread_main, &i_th) != 0) {
	    fprintf(stderr, "Cound not create thread.\n");
	    return 1;
	}
    }
    for (i_th = 0; i_th < THR_CNT; ++i_th)
	cu_pthread_join(th[i_th], NULL);

    return 2*!!cu_test_bug_count();
}
