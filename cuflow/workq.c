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

#include <cuflow/workq.h>
#include <cu/memory.h>

#define WORKQSEG_SIZE 32
#define WORKQ_MAX_FREE_SEG_CNT 2

struct cuflowP_workqseg_s
{
    cuflowP_workqseg_t next;
    cuflow_workq_fn_t fn_arr[WORKQSEG_SIZE];
};

CU_SINLINE cuflow_workq_fn_t *
workqseg_begin(cuflowP_workqseg_t wqs)
{
    return &wqs->fn_arr[0];
}

CU_SINLINE cuflow_workq_fn_t *
workqseg_end(cuflowP_workqseg_t wqs)
{
    return &wqs->fn_arr[WORKQSEG_SIZE];
}

void
cuflow_workq_cct(cuflow_workq_t wq)
{
#if 0
    cuflowP_workqseg_t seg = cu_gnew(struct cuflowP_workqseg_s);
    wq->head_seg = wq->tail_seg = wq->last_seg = seg;
    wq->head_cur = wq->head_end = wq->tail_cur = workqseg_begin(seg);
    wq->tail_end = workqseg_end(seg);
    wq->free_seg_cnt = 0;
#else
    wq->head_seg = wq->tail_seg = wq->last_seg = NULL;
    wq->head_cur = wq->head_end = wq->tail_cur = wq->tail_end = NULL;
    wq->free_seg_cnt = 0;
#endif
}

void
cuflowP_workq_advance_tail(cuflow_workq_t wq)
{
    if (wq->free_seg_cnt == 0) {
	cuflowP_workqseg_t seg = cu_gnew(struct cuflowP_workqseg_s);
	cu_debug_assert(wq->tail_seg == wq->last_seg);
	if (wq->last_seg == NULL) { /* if not used since construction */
	    wq->head_seg = seg;
	    wq->head_cur = wq->head_end = workqseg_begin(seg);
	}
	else
	    wq->last_seg->next = seg;
	wq->last_seg = seg;
	wq->tail_seg = seg;
    }
    else {
	wq->tail_seg = wq->tail_seg->next;
	--wq->free_seg_cnt;
    }
    wq->tail_cur = workqseg_begin(wq->tail_seg);
    wq->tail_end = workqseg_end(wq->tail_seg);
}

cuflow_workq_fn_t
cuflowP_workq_pop_front(cuflow_workq_t wq)
{
    cuflowP_workqseg_t head_seg = wq->head_seg;
    if (head_seg == wq->tail_seg) {
	if (wq->head_end != wq->tail_cur)
	    wq->head_end = wq->tail_cur;
	else
	    return NULL;
    }
    else if (wq->head_end != workqseg_end(head_seg))
	wq->head_end = workqseg_end(wq->head_seg);
    else {
#if 0
	if (wq->free_seg_cnt < WORKQ_MAX_FREE_SEG_CNT) {
	    wq->last_seg->next = head_seg;
	    wq->last_seg = head_seg;
	    ++wq->free_seg_cnt;
	}
#endif
	head_seg = head_seg->next;
	wq->head_seg = head_seg;
	wq->head_cur = workqseg_begin(head_seg);
	if (head_seg == wq->tail_seg) {
	    wq->head_end = wq->tail_cur;
	    cu_debug_assert(wq->head_cur != wq->tail_cur);
	}
	else
	    wq->head_end = workqseg_end(head_seg);
    }
    return *wq->head_cur++;
}

size_t
cuflow_workq_size(cuflow_workq_t wq)
{
    cuflowP_workqseg_t seg = wq->head_seg;
    if (seg == NULL)
	return 0;
    else if (seg == wq->tail_seg)
	return wq->tail_cur - wq->head_cur;
    else {
	size_t size = workqseg_end(seg) - wq->head_cur;
	seg = seg->next;
	while (seg != wq->tail_seg) {
	    size += WORKQSEG_SIZE;
	    seg = seg->next;
	}
	return size + (wq->tail_cur - workqseg_begin(seg));
    }
}
