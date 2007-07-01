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

#ifndef CUFLOW_TIMER_H
#define CUFLOW_TIMER_H

#include <cuflow/fwd.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuflow_timer_h cuflow/timer.h: (unfinished)
 *@{\ingroup cuflow_mod */

struct cuflow_timer_s
{
    cuflow_time_t expiry;
    cu_clop(notifier, cuflow_time_t, cuflow_time_t correction);
};

typedef enum {
    cuflow_walltime_domain,
    cuflow_proctime_domain,
    cuflow_procandsystime_domain,
    cuflow_threadtime_domain,	/* not valid for timer */
} cuflow_temporaldomain_t;

void
cuflow_timer_cct(cuflow_timer_t timer,
		 cuflow_temporaldomain_t td, cuflow_time_t expiry,
		 cu_clop(notifier, cuflow_time_t, cuflow_time_t correction));

cuflow_timer_t
cuflow_timer_new(cuflow_temporaldomain_t td, cuflow_time_t expiry,
		 cu_clop(notifier, cuflow_time_t, cuflow_time_t correction));

void cuflow_timer_set(cuflow_timer_t timer, cuflow_time_t expiry);

/*!@}*/
CU_END_DECLARATIONS

#endif
