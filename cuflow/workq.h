/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2005--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUFLOW_WORKQ_H
#define CUFLOW_WORKQ_H

#include <cuflow/fwd.h>
#include <cu/clos.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cuflow_workq_h cuflow/workq.h: FIFO for Queueing Pending Tasks
 * @{\ingroup cuflow_mod */

/*!The type of function which can be queued on work queues. */
typedef cu_clop0(cuflow_workq_fn_t, void);

typedef struct cuflowP_workqseg_s *cuflowP_workqseg_t;

struct cuflow_workq_s
{
    cuflowP_workqseg_t head_seg;
    cuflow_workq_fn_t *head_cur;
    cuflow_workq_fn_t *head_end;
    cuflowP_workqseg_t tail_seg;
    cuflow_workq_fn_t *tail_cur;
    cuflow_workq_fn_t *tail_end;
    cuflowP_workqseg_t last_seg;
    size_t free_seg_cnt;
};

cuflow_workq_fn_t cuflowP_workq_pop_front(cuflow_workq_t wq);
void cuflowP_workq_advance_tail(cuflow_workq_t wq);

/*!Construct a work queue. */
void cuflow_workq_cct(cuflow_workq_t wq);

/*!Append a task to the end of \a wq. */
CU_SINLINE void
cuflow_workq_append(cuflow_workq_t wq, cuflow_workq_fn_t fn)
{
    if (wq->tail_cur == wq->tail_end)
	cuflowP_workq_advance_tail(wq);
    *wq->tail_cur++ = fn;
}

/*!Pop off the first task on \a wq. */
CU_SINLINE cuflow_workq_fn_t
cuflow_workq_pop_front(cuflow_workq_t wq)
{
    if (wq->head_cur == wq->head_end)
	return cuflowP_workq_pop_front(wq);
    else
	return *wq->head_cur++;
}

/*!The number of functions on \a wq.  Note that it takes time linear in
 * the number of scheduled functions. */
size_t cuflow_workq_size(cuflow_workq_t wq);

/*!@}*/
CU_END_DECLARATIONS

#endif
