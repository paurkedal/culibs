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

#define CUOS_TIME_C
#include <cuos/time.h>
#include <cu/debug.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include <time.h>


double
cuos_time_utc()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec*1e-6;
}

double
cuos_time_proc()
{
    struct rusage ru;
    if (getrusage(RUSAGE_SELF, &ru) == -1) {
	cu_debug_unreachable(); /* Only usage errors documented. */
	return 0.0;
    }
    return ru.ru_utime.tv_sec + ru.ru_stime.tv_sec
	+ (ru.ru_utime.tv_usec + ru.ru_stime.tv_usec)*1e-6;
}

double
cuos_sleep(double t)
{
    struct timespec spec, spec_rem;
    double t_int, t_frac;
    t_frac = modf(t, &t_int);
    spec.tv_sec = (time_t)t_int;
    spec.tv_nsec = (long)(t_frac*1e9);
    if (nanosleep(&spec, &spec_rem) < 0) {
	switch (errno) {
	case EINTR:
	    return spec_rem.tv_sec + spec_rem.tv_nsec*1e-9;
	default:
	    cu_debug_error("Internal error in cuos_sleep, "
			    "calling nanosleep.");
	    return 0.0;
	}
    }
    else
	return 0.0;
}
