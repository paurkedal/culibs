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

#define WHEEL_CIRC 8
#define WHEEL_MASK (WHEEL_CIRC - 1)
#define WHEEL_SATURATION 1

#define USE_TS 1

typedef struct sector_s *sector_t;
struct sector_s
{
#if USE_TS
    AO_TS_t read_guard;
    AO_TS_t write_guard;
#else
    AO_t state;
#   define STATE_FREE 0
#   define STATE_PREP 1
#   define STATE_READY 2
#endif
    cu_clop0(fn, void);
    AO_t *cdisj;
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


void
cuflowP_wheel_call(cu_clop0(fn, void), AO_t *cdisj)
{
    AO_fetch_and_add1(cdisj);
    AO_fetch_and_sub1(&cuflowP_wheel_nfree);
    for (;;) {
	int i = AO_fetch_and_add1(&wheel_ihead) & WHEEL_MASK;
	sector_t sector = &wheel_arr[i];
#if USE_TS
	if (AO_test_and_set_acquire(&sector->write_guard) == AO_TS_CLEAR) {
	    sector->fn = fn;
	    sector->cdisj = cdisj;
	    AO_CLEAR(&sector->read_guard);
		/* Despite the name AO_CLEAR includes a _release barrier,
		 * which is what we need. */
	    break;
	}
#else
	if (AO_compare_and_swap(&sector->state, STATE_FREE, STATE_PREP)) {
	    sector->fn = fn;
	    sector->cdisj = cdisj;
	    AO_store_release(&sector->state, STATE_READY);
	    break;
	}
#endif
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
#if USE_TS
	if (AO_test_and_set_acquire(&sector->write_guard) == AO_TS_CLEAR) {
	    sector->fn = fn;
	    sector->cdisj = cdisj;
	    AO_CLEAR(&sector->read_guard);
		/* Despite the name AO_CLEAR includes a _release barrier,
		 * which is what we need. */
	    break;
	}
#else
	if (AO_compare_and_swap(&sector->state, STATE_FREE, STATE_PREP)) {
	    sector->fn = fn;
	    sector->cdisj = cdisj;
	    AO_store_release(&sector->state, STATE_READY);
	    break;
	}
#endif
    }
    if (AO_fetch_and_add1(&wheel_nready) == 0)
	cuflow_workers_incr_pending();
#if CUFLOW_PROF_WHEEL
    AO_fetch_and_add1(&wheel_schedcall_count);
#endif
}

cu_clop_def0(cuflowP_wheel_run, void)
{
    if (cu_expect_false(AO_fetch_and_sub1(&wheel_nready) == 1))
	cuflow_workers_decr_pending();
    for (;;) {
	int i = AO_fetch_and_add1(&wheel_itail) & WHEEL_MASK;
	sector_t sector = &wheel_arr[i];
#if USE_TS
	if (AO_test_and_set_acquire(&sector->read_guard) == AO_TS_CLEAR) {
#else
	if (AO_compare_and_swap(&sector->state, STATE_READY, STATE_PREP)) {
#endif
	    cu_clop0(fn, void) = sector->fn;
	    AO_t *cdisj = sector->cdisj;
#if USE_TS
	    AO_CLEAR(&sector->write_guard);
#else
	    AO_store_release(&sector->state, STATE_FREE);
#endif
	    AO_fetch_and_add1(&cuflowP_wheel_nfree);
	    cu_call0(fn);
	    cuflow_cdisj_sub1_release(cdisj);
	    if (AO_load(&wheel_nready) == 0)
		return;
	    if (cu_expect_false(AO_fetch_and_sub1(&wheel_nready) == 1))
		cuflow_workers_decr_pending();
	} else if ((int)AO_load(&wheel_nready) < 0) {
#if 0
	    /* This only works if cuflow_yield re-checks for pending work
	     * before returning to caller. */
	    if (cu_expect_false(AO_fetch_and_add1(&wheel_nready) == 0))
		cuflow_workers_incr_pending();
	    return;
#endif
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
	    }
	    else
		return;
	}
    }
}

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

#if USE_TS
	sector->read_guard = AO_TS_INITIALIZER;
	AO_test_and_set(&sector->read_guard);
	sector->write_guard = AO_TS_INITIALIZER;
#else
	sector->state = STATE_FREE;
#endif
	sector->fn = NULL;
	sector->cdisj = NULL;
    }
    cuflow_workers_register_scheduler(cuflowP_wheel_run);
#if CUFLOW_PROF_WHEEL
    atexit(wheel_dump_stats);
#endif
}
