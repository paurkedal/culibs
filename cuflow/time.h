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

#ifndef CUFLOW_TIME_H
#define CUFLOW_TIME_H

#include <cuflow/fwd.h>
#include <time.h>

CU_BEGIN_DECLARATIONS

#ifdef CUCONF_HAVE_LIBRT

typedef uint64_t cuflow_cputime_t;
typedef uint64_t cuflow_walltime_t;

#define CUFLOW_CPUTIME_SECOND UINT64_C(1000000000)
#define CUFLOW_WALLTIME_SECOND UINT64_C(1000000000)

CU_SINLINE cuflow_cputime_t cuflow_threadtime(void)
{
    struct timespec tv;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tv);
    return (uint64_t)tv.tv_nsec + UINT64_C(1000000000)*(uint64_t)tv.tv_sec;
}

CU_SINLINE cuflow_walltime_t cuflow_walltime(void)
{
    struct timespec tv;
    clock_gettime(CLOCK_MONOTONIC, &tv);
    return (uint64_t)tv.tv_nsec + UINT64_C(1000000000)*(uint64_t)tv.tv_sec;
}

#else /* !CUCONF_HAVE_LIBRT */

typedef clock_t cuflow_cputime_t;
typedef uint64_t cuflow_walltime_t;

#define cuflow_threadtime clock

CU_SINLINE cuflow_walltime_t cuflow_walltime(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_usec + UINT64_C(1000000)*(uint64_t)tv.tv_sec;
}

#define CUCONF_CPUTIME_SECOND CLOCKS_PER_SEC
#define CUCONF_WALLTIME_SECOND UINT64_C(1000000)

#endif

cuflow_cputime_t cuflowP_threadtime_granularity;

CU_SINLINE cuflow_cputime_t cuflow_threadtime_granularity(void)
{ return cuflowP_threadtime_granularity; }

CU_END_DECLARATIONS

#endif
