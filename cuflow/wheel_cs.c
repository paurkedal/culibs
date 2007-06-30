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

#include <cuflow/wheel.h>
#include <cuflow/cdisj.h>
#include <cuflow/workers.h>
#include <stdio.h>

#define WHEEL_CIRC 32
#define WHEEL_MASK (WHEEL_CIRC - 1)
#define WHEEL_SATURATION 1

#define USE_TS 1

typedef struct sector_s *sector_t;
struct sector_s
{
    AO_t fn;    /* cu_clop0(, void) */
    AO_t cdisj; /* AO_t * */
};

AO_t cuflowP_wheel_nfree = WHEEL_SATURATION;
static AO_t wheel_nready = 0;
static AO_t wheel_ihead = 0;
static AO_t wheel_itail = 0;
static struct sector_s wheel_arr[WHEEL_CIRC];

#if CUFLOW_PROF_WHEEL
AO_t cuflowP_wheel_immcall_count = 0;
static AO_t wheel_schedcall_count = 0;
#endif

AO_t cuflowP_wheel_ratecounter = 0;


void
cuflowP_wheel_call(cu_clop0(fn, void), AO_t *cdisj)
{
    AO_fetch_and_add1(cdisj);
    AO_fetch_and_sub1(&cuflowP_wheel_nfree);
    for (;;) {
	int i = AO_fetch_and_add1(&wheel_ihead) & WHEEL_MASK;
	sector_t sector = &wheel_arr[i];
	if (AO_compare_and_swap(&sector->cdisj, 0, (AO_t)cdisj)) {
	    AO_store_release_write(&sector->fn, (AO_t)fn);
	    break;
	}
    }
    if (AO_fetch_and_add1(&wheel_nready) == 0)
	cuflow_workers_incr_pending();
#if CUFLOW_PROF_WHEEL
    AO_fetch_and_add1(&wheel_schedcall_count);
#endif
}

void
cuflowP_wheel_call_sub1(cu_clop0(fn, void), AO_t *cdisj)
{
    AO_fetch_and_sub1(&cuflowP_wheel_nfree);
    for (;;) {
	int i = AO_fetch_and_add1(&wheel_ihead) & WHEEL_MASK;
	sector_t sector = &wheel_arr[i];
	if (AO_compare_and_swap(&sector->cdisj, 0, (AO_t)cdisj)) {
	    AO_store_release_write(&sector->fn, (AO_t)fn);
	    break;
	}
    }
    if (AO_fetch_and_add1(&wheel_nready) == 0)
	cuflow_workers_incr_pending();
#if CUFLOW_PROF_WHEEL
    AO_fetch_and_add1(&wheel_schedcall_count);
#endif
}

#if 1
cu_clop_def(cuflowP_wheel_run, void, cu_bool_t is_global)
{
    if (cu_expect_false(AO_fetch_and_sub1(&wheel_nready) == 1))
	cuflow_workers_decr_pending();
    for (;;) {
	int i = AO_fetch_and_add1(&wheel_itail) & WHEEL_MASK;
	sector_t sector = &wheel_arr[i];
	cu_clop0(fn, void) = (cu_clop0(, void))AO_load(&sector->fn);
	if (fn && cu_expect_true(AO_compare_and_swap(&sector->fn,
						     (AO_t)fn, 0))) {
	    AO_t *cdisj = (AO_t *)AO_load(&sector->cdisj);
	    AO_store_release(&sector->cdisj, 0);
	    AO_fetch_and_add1_release(&cuflowP_wheel_nfree);
	    cu_call0(fn);
	    cuflow_cdisj_sub1_release_write(cdisj);
	    if (AO_load(&wheel_nready) == 0)
		return;
	    if (cu_expect_false(AO_fetch_and_sub1(&wheel_nready) == 1))
		cuflow_workers_decr_pending();
	} else if ((int)AO_load(&wheel_nready) < 0) {
#if 0
	    if (cu_expect_false(AO_fetch_and_add1(&wheel_nready) == 0))
		cuflow_workers_incr_pending();
	    /* The cuflow_yield function must guarantee that all jobs
	     * scheduled by the caller must either be run by the caller or be
	     * in progress by other threads.  Therefore this only works
	     * if cuflow_yield re-checks if there is pending work. */
	    return;
#else
	    int nready = AO_fetch_and_add1(&wheel_nready);
	    if (nready >= 0) {
		int nreadyp = AO_fetch_and_sub1(&wheel_nready);
		if (nready == 0) {
		    if (nreadyp != 1)
			cuflow_workers_incr_pending();
		} else {
		    if (nreadyp == 1)
			cuflow_workers_decr_pending();
		}
	    } else
		return;
#endif
	}
    }
}
#else
cu_clop_def(cuflowP_wheel_run, void, cu_bool_t is_global)
{
    while ((int)AO_load(&wheel_nready) > 0) {
	int i = AO_fetch_and_add1(&wheel_itail) & WHEEL_MASK;
	sector_t sector = &wheel_arr[i];
	cu_clop0(fn, void) = (cu_clop0(, void))AO_load(&sector->fn);
	if (fn && cu_expect_true(AO_compare_and_swap(&sector->fn,
						     (AO_t)fn, 0))) {
	    AO_t *cdisj;
	    if (cu_expect_false(AO_fetch_and_sub1(&wheel_nready) == 1))
		cuflow_workers_decr_pending();
	    cdisj = (AO_t *)AO_load(&sector->cdisj);
	    AO_store_release(&sector->cdisj, 0);
	    AO_fetch_and_add1_release(&cuflowP_wheel_nfree);
	    cu_call0(fn);
	    cuflow_cdisj_sub1_release_write(cdisj);
	}
    }
}
#endif

#if CUFLOW_PROF_WHEEL
static void
wheel_dump_stats()
{
    printf("wheel immediate calls: %ld\t\nwheel scheduled calls: %ld\n",
	   (long)AO_load(&cuflowP_wheel_immcall_count),
	   (long)AO_load(&wheel_schedcall_count));
}
#endif

void
cuflowP_wheel_init(void)
{
    int i;
    for (i = 0; i < WHEEL_CIRC; ++i) {
	sector_t sector = &wheel_arr[i];

	sector->fn = 0;
	sector->cdisj = 0;
    }
    cuflow_workers_register_scheduler(cuflowP_wheel_run);
#if CUFLOW_PROF_WHEEL
    atexit(wheel_dump_stats);
#endif
}
