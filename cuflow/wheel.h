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

#ifndef CUFLOW_WHEEL_H
#define CUFLOW_WHEEL_H

#include <cuflow/fwd.h>
#include <cuflow/cdisj.h>
#include <cu/clos.h>
#include <atomic_ops.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuflow_wheel_h cuflow/wheel.h:
 *@{\ingroup cuflow_mod */

#define CUFLOW_PROF_WHEEL 0

extern AO_t cuflowP_wheel_nfree;
#if CUFLOW_PROF_WHEEL
extern AO_t cuflowP_wheel_immcall_count;
#endif

#if 0
extern AO_t cuflowP_wheel_ratecounter;
#define CUFLOW_WHEEL_RATELIM_STEP ((AO_t)1 << (sizeof(AO_t)*8 - 8))
#define CUFLOWP_WHEEL_RATELIM_PASS \
    (!(AO_fetch_and_add(&cuflowP_wheel_ratecounter, CUFLOW_WHEEL_RATELIM_STEP)))
#else
#define CUFLOWP_WHEEL_RATELIM_PASS cu_true
#endif

void cuflowP_wheel_call(cu_clop0(fn, void), AO_t *cdisj);
void cuflowP_wheel_call_sub1(cu_clop0(fn, void), AO_t *cdisj);

CU_SINLINE void
cuflow_wheel_call(cu_clop0(fn, void), AO_t *cdisj)
{
    if (cu_expect_false(CUFLOWP_WHEEL_RATELIM_PASS &&
			(int)AO_load_acquire_read(&cuflowP_wheel_nfree) > 0))
	cuflowP_wheel_call(fn, cdisj);
    else {
	cu_call0(fn);
#if CUFLOW_PROF_WHEEL
	AO_fetch_and_add1(&cuflowP_wheel_immcall_count);
#endif
    }
}

CU_SINLINE void
cuflow_wheel_call_sub1(cu_clop0(fn, void), AO_t *cdisj)
{
    if (cu_expect_false((int)AO_load_acquire_read(&cuflowP_wheel_nfree) > 0))
	cuflowP_wheel_call_sub1(fn, cdisj);
    else {
	cu_call0(fn);
	cuflow_cdisj_sub1_release_write(cdisj);
#if CUFLOW_PROF_WHEEL
	AO_fetch_and_add1(&cuflowP_wheel_immcall_count);
#endif
    }
}

/*!@}*/
CU_END_DECLARATIONS

#endif
