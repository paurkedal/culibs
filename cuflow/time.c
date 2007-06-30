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

#include <cuflow/time.h>

cuflow_cputime_t cuflowP_threadtime_granularity;

void
cuflowP_time_init(void)
{
    cuflow_cputime_t t0, t1;
#ifdef CUCONF_CPUUSE_BY_LIBRT
    struct timespec tt;
    if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tt) != 0) {
	cu_errf("culibs is configured to use clock_gettime but the required " 
		"CLOCK_THREAD_CPUTIME_ID clock is not available.");
	exit(2);
    }
    if (clock_gettime(CLOCK_MONOTONIC, &tt) != 0) {
	cu_errf("culibs is configured to use clock_gettime but the required " 
		"CLOCK_MONOTONIC clock is not available.");
	exit(2);
    }
#endif

    /* Measure the granularity of the clock we're using.  clock_getres is no
     * good here, as I've seen it returing 1 ns whereas the granularity is 1
     * ms. */
    t0 = cuflow_threadtime();
    do
	t1 = cuflow_threadtime();
    while (t0 == t1);
    cuflowP_threadtime_granularity = t1 - t0;
}
