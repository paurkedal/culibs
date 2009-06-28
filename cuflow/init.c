/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2004--2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cuflow/fwd.h>
#include <gc/gc.h>

void cu_init(void);
void cuflowP_wind_init(void);
void cuflowP_signal_init(void);
void cuflowP_gworkq_init(void);
#ifdef CUCONF_ENABLE_EXPERIMENTAL
void cuflowP_tstate_init(void);
void cuflowP_time_init(void);
void cuflowP_cntn_common_init(void);
void cuflowP_workers_init(void);
void cuflowP_cdisj_init(void);
void cuflowP_sched_init(void);
void cuflowP_wheel_init(void);
void cuflowP_cached_init(void);
#endif

void
cuflow_init()
{
    static int done_init = 0;
    if (done_init)
	return;
    done_init = 1;

    cu_init();
    cuflowP_signal_init();
    cuflowP_gworkq_init();
    cuflowP_tstate_init();
    cuflowP_cdisj_init();
    cuflowP_workers_init();
    cuflowP_sched_init();	/* after: cuflowP_workers_init */
#ifdef CUCONF_ENABLE_EXPERIMENTAL
    cuflowP_time_init();
    cuflowP_cntn_common_init();
    cuflowP_wheel_init();	/* after: cuflowP_workers_init */
    cuflowP_cached_init();
#endif
}
