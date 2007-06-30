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

#ifndef CUFLOW_TIMESPEC_H
#define CUFLOW_TIMESPEC_H

#include <cuflow/fwd.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuflow_timespec_h cuflow/timespec.h: Arithetic on the timespec Struct
 *@{\ingroup cuflow_mod */

CU_SINLINE void
cuflow_timespec_assign(struct timespec *dst, struct timespec *src)
{
    dst->tv_sec = src->tv_sec;
    dst->tv_nsec = src->tv_nsec;
}

CU_SINLINE void
cuflow_timespec_assign_sum(struct timespec *dst,
			   struct timespec *src0, struct timespec *src1)
{
    dst->tv_nsec = src0->tv_nsec + src1->tv_nsec;
    if (dst->tv_nsec >= 1000000000) {
	dst->tv_nsec -= 1000000000;
	dst->tv_sec = src0->tv_sec + src1->tv_sec + 1;
    }
    else
	dst->tv_sec = src0->tv_sec + src1->tv_sec;
}

CU_SINLINE void
cuflow_timespec_assign_diff(struct timespec *dst,
			    struct timespec *src0, struct timespec *src1)
{
    dst->tv_nsec = src0->tv_nsec - src1->tv_nsec;
    if (dst->tv_nsec < 0) {
	dst->tv_nsec += 1000000000;
	dst->tv_sec = src0->tv_sec - src1->tv_sec - 1;
    }
    else
	dst->tv_sec = src0->tv_sec - src1->tv_sec;
}

CU_SINLINE void
cuflow_timespec_add(struct timespec *dst, struct timespec *src)
{
    dst->tv_nsec += src->tv_nsec;
    if (dst->tv_nsec >= 1000000000) {
	dst->tv_nsec -= 1000000000;
	dst->tv_sec += 1;
    }
    dst->tv_sec += src->tv_sec;
}

CU_SINLINE void
cuflow_timespec_sub(struct timespec *dst, struct timespec *src)
{
    dst->tv_nsec -= src->tv_nsec;
    if (dst->tv_nsec < 0) {
	dst->tv_nsec += 1000000000;
	dst->tv_sec -= 1;
    }
    dst->tv_sec -= src->tv_sec;
}

CU_SINLINE cu_bool_t
cuflow_timespec_eq(struct timespec *t0, struct timespec *t1)
{
    return t0->tv_nsec == t1->tv_nsec && t0->tv_sec == t1->tv_sec;
}

CU_SINLINE cu_bool_t
cuflow_timespec_leq(struct timespec *t0, struct timespec *t1)
{
    return (t0->tv_sec == t1->tv_sec && t0->tv_nsec <= t1->tv_nsec)
	||  t0->tv_sec <  t1->tv_sec;
}

CU_SINLINE cu_bool_t
cuflow_timespec_lt(struct timespec *t0, struct timespec *t1)
{
    return (t0->tv_sec == t1->tv_sec && t0->tv_nsec < t1->tv_nsec)
	||  t0->tv_sec <  t1->tv_sec;
}

/*!@}*/
CU_END_DECLARATIONS

#endif
