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

#include <cu/test.h>
#include <cuflow/workers.h>
#include <cuflow/time.h>
#include <cuflow/cached.h>

static AO_t discr = 0;
#define DISCR AO_fetch_and_add1(&discr)

cuflow_cached_def(jobCC, (int n; int discr;), (int r;))
{
    cuflow_cached_arg_res(jobCC);
    int n = arg->n;
    jobCC_result_t res0, res1;
    if (arg->n == 0) {
	res->r = 1;
	goto leave;
    }
    cuflow_cached_call(jobCC,
		       (arg->n = n / 2; arg->discr = DISCR;), &res0);
    cuflow_cached_call(jobCC,
		       (arg->n = (n - 1) / 2; arg->discr = DISCR;), &res1);
    res->r = res0->r + res1->r + 1;
leave:
    cuflow_cached_set_gain(jobCC, 4);
}

cuflow_cached_def(jobCS, (int n; int discr;), (int r;))
{
    cuflow_cached_arg_res(jobCS);
    int n = arg->n;
    jobCS_promise_t prom0, prom1;
    if (arg->n == 0) {
	res->r = 1;
	goto leave;
    }
    cuflow_cached_sched_call(jobCS,
			     (arg->n = n / 2; arg->discr = DISCR;), &prom0);
    cuflow_cached_sched_call(jobCS,
			     (arg->n = (n - 1)/2; arg->discr=DISCR;), &prom1);
    res->r = jobCS_fulfill(prom0)->r + jobCS_fulfill(prom1)->r;
leave:
    cuflow_cached_set_gain(jobCS, 4);
}

void cuflowP_cached_init(void);

int
main()
{
    jobCC_result_t jobCC_res;
    jobCS_result_t jobCS_res;
    time_t tCC, tCS;
    cuflow_walltime_t wtCC, wtCS;
    int n = 0x80000;
    int r;

    cuflow_init();
    cuflow_workers_spawn(3);

    tCS = -clock();
    wtCS = -cuflow_walltime();
    cuflow_cached_call(jobCS, (arg->n = n; arg->discr = DISCR;), &jobCS_res);
    tCS += clock();
    wtCS += cuflow_walltime();
    cu_test_assert(r = jobCS_res->r);

    cuflowP_cached_init();

    tCC = -clock();
    wtCC = -cuflow_walltime();
    cuflow_cached_call(jobCC, (arg->n = n; arg->discr = DISCR;), &jobCC_res);
    tCC += clock();
    wtCC += cuflow_walltime();
    r = jobCC_res->r;

    printf("Result: %d\n", r);
    printf("              %10s %10s | %10s %10s\n"
	   " cached clos: %10.3lg %10.3lg | %10.3lg %10.3lg\n"
	   "  C & S clos: %10.3lg %10.3lg | %10.3lg %10.3lg\n",
	   "CPU time", "per call", "wall time", "per call",
	   tCC/(double)CLOCKS_PER_SEC,
	   tCC/(r*(double)CLOCKS_PER_SEC),
	   wtCC/(double)CUFLOW_WALLTIME_SECOND,
	   wtCC/(r*(double)CUFLOW_WALLTIME_SECOND),
	   tCS/(double)CLOCKS_PER_SEC,
	   tCS/(r*(double)CLOCKS_PER_SEC),
	   wtCS/(double)CUFLOW_WALLTIME_SECOND,
	   wtCS/(r*(double)CUFLOW_WALLTIME_SECOND));
    return 0;
}
