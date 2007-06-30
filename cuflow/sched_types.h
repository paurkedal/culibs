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

#ifndef CUFLOW_SCHED_TYPES_H
#define CUFLOW_SCHED_TYPES_H

#include <cuflow/fwd.h>
#include <cu/clos.h>
#include <atomic_ops.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuflow_sched_types_h cuflow/sched_types.h:
 *@{\ingroup cuflow_mod */

/* The number of entries in each thread-local execution queue.  This must be
 * a power of 2. */
#define CUFLOW_EXEQ_SIZE 8
#define CUFLOW_EXEQ_MASK (CUFLOW_EXEQ_SIZE - 1)
#define CUFLOW_PROFILE_SCHED 0
#define CUFLOW_CALLS_BETWEEN_SCHED 1

typedef struct cuflow_exeq_s *cuflow_exeq_t;
typedef struct cuflow_exeq_entry_s *cuflow_exeq_entry_t;

typedef enum {
    cuflow_exeqpri_begin	= 0,
    cuflow_exeqpri_normal	= 0,
    cuflow_exeqpri_background	= 1,
    cuflow_exeqpri_end
} cuflow_exeqpri_t;

#define cuflow_exeqpri_succ(pri) ((cuflow_exeqpri_t)((pri) + 1))

CU_SINLINE cu_bool_t
cuflow_exeqpri_prioreq(cuflow_exeqpri_t pri0, cuflow_exeqpri_t pri1)
{ return pri0 <= pri1; }

CU_SINLINE cu_bool_t
cuflow_exeqpri_prior(cuflow_exeqpri_t pri0, cuflow_exeqpri_t pri1)
{ return pri0 < pri1; }

struct cuflow_exeq_entry_s
{
    cu_clop0(fn, void);
    AO_t *cdisj;
};

struct cuflow_exeq_s
{
    pthread_mutex_t pickup_mutex;
    cuflow_exeqpri_t priority;
    struct cuflow_exeq_entry_s call_arr[CUFLOW_EXEQ_SIZE];
    AO_t head, tail;
#if CUFLOW_CALLS_BETWEEN_SCHED > 1
    int calls_till_sched;
#endif
};

/*!@}*/
CU_END_DECLARATIONS

#endif
