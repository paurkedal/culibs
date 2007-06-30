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
#include <cuflow/sched.h>
//#include <cuflow/rotary.h>
#include <cuflow/wheel.h>
#include <cuflow/workers.h>
#include <cuflow/time.h>
#include <cuflow/tstate.h>
#include <cuflow/cached.h>

int
jobF(int n)
{
    if (n == 0)
	return 1;
    else
	return jobF(n/2) + jobF((n - 1)/2) + 1;
}

cu_clos_def(jobC, cu_prot0(void), (int n; int r;))
{
    cu_clos_self(jobC);
    jobC_t job[2];

    if (self->n == 0) {
	self->r = 1;
	return;
    }
    job[0].n = self->n / 2;
    job[1].n = (self->n - 1) / 2;
    cu_call0(jobC_prep(&job[0]));
    cu_call0(jobC_prep(&job[1]));
    cuflow_yield();
    self->r = job[0].r + job[1].r + 1;
}

cu_clos_def(jobS, cu_prot0(void), (int n; int r;))
{
    cu_clos_self(jobS);
    jobS_t job[2];
    AO_t cdisj = 0;

    if (self->n == 0) {
	self->r = 1;
	return;
    }
    job[0].n = self->n / 2;
    job[1].n = (self->n - 1) / 2;
    cuflow_sched_call(jobS_prep(&job[0]), &cdisj);
    //cuflow_sched_call(jobS_prep(&job[1]), &cdisj);
    cu_call0(jobS_prep(&job[1]));
    cuflow_cdisj_wait_while(&cdisj);
    self->r = job[0].r + job[1].r + 1;
    //printf("** %d + %d = %d\n", job[0].r, job[1].r, self->r);
}

#if 0
cu_clos_def(jobR, cu_prot0(void), (int n; int r;))
{
    cu_clos_self(jobR);
    jobR_t job[2];
    AO_t cdisj = 0;

    if (self->n == 0) {
	self->r = 1;
	return;
    }
    job[0].n = self->n / 2;
    job[1].n = (self->n - 1) / 2;
    cuflow_rotary_call(jobR_prep(&job[0]), &cdisj);
    cu_call0(jobR_prep(&job[1]));
    cuflow_cdisj_wait_while(&cdisj);
    self->r = job[0].r + job[1].r + 1;
}
#endif

cu_clos_def(jobW, cu_prot0(void), (int n; int r;))
{
    cu_clos_self(jobW);
    jobW_t job[2];
    AO_t cdisj = 0;

    if (self->n == 0) {
	self->r = 1;
	return;
    }
    job[0].n = self->n / 2;
    job[1].n = (self->n - 1) / 2;
    cuflow_wheel_call(jobW_prep(&job[0]), &cdisj);
    //cuflow_wheel_call(jobW_prep(&job[1]), &cdisj);
    cu_call0(jobW_prep(&job[1]));
    cuflow_cdisj_wait_while(&cdisj);
    self->r = job[0].r + job[1].r + 1;
}

int
main()
{
    jobC_t jobC;
    jobS_t jobS;
//    jobR_t jobR;
    jobW_t jobW;
    time_t tF, tC, tS, tW;
    cuflow_walltime_t wtF, wtC, wtS, wtW;
    int r;
    int n = 0x1000000;

    cuflow_init();

    tF = -clock();
    wtF = -cuflow_walltime();
    r = jobF(n);
    tF += clock();
    wtF += cuflow_walltime();

    tC = -clock();
    wtC = -cuflow_walltime();
    jobC.n = n;
    cu_call0(jobC_prep(&jobC));
    tC += clock();
    wtC += cuflow_walltime();
    cu_test_assert(jobC.r == r);

    cuflow_workers_spawn_at_least(7);

    tS = -clock();
    wtS = -cuflow_walltime();
    jobS.n = n;
    cu_call0(jobS_prep(&jobS));
    tS += clock();
    wtS += cuflow_walltime();
    cu_test_assert(jobS.r == r);

#if 0
    tR = -clock();
    jobR.n = n;
    cu_call0(jobR_prep(&jobR));
    tR += clock();
    cu_test_assert(jobR.r == r);
#endif

    tW = -clock();
    wtW = -cuflow_walltime();
    jobW.n = n;
    cu_call0(jobW_prep(&jobW));
    tW += clock();
    wtW += cuflow_walltime();
    cu_test_assert(jobW.r == r);

    cuflow_workers_spawn(0);

    printf("Result: %d\n", r);
    printf("              %10s %10s\n"
	   "    plain fn: %10.3lg %10.3lg | %10.3lg %10.3lg\n"
	   "  plain clos: %10.3lg %10.3lg | %10.3lg %10.3lg\n"
	   "  sched clos: %10.3lg %10.3lg | %10.3lg %10.3lg\n"
	   "  wheel clos: %10.3lg %10.3lg | %10.3lg %10.3lg\n",
	   "total", "per call",
	   tF/(double)CLOCKS_PER_SEC,
	   tF/(r*(double)CLOCKS_PER_SEC),
	   wtF/(double)CUFLOW_WALLTIME_SECOND,
	   wtF/(r*(double)CUFLOW_WALLTIME_SECOND),
	   tC/(double)CLOCKS_PER_SEC,
	   tC/(r*(double)CLOCKS_PER_SEC),
	   wtC/(double)CUFLOW_WALLTIME_SECOND,
	   wtC/(r*(double)CUFLOW_WALLTIME_SECOND),
	   tS/(double)CLOCKS_PER_SEC,
	   tS/(r*(double)CLOCKS_PER_SEC),
	   wtS/(double)CUFLOW_WALLTIME_SECOND,
	   wtS/(r*(double)CUFLOW_WALLTIME_SECOND),
	   tW/(double)CLOCKS_PER_SEC,
	   tW/(r*(double)CLOCKS_PER_SEC),
	   wtW/(double)CUFLOW_WALLTIME_SECOND,
	   wtW/(r*(double)CUFLOW_WALLTIME_SECOND));
    return 0;
}
