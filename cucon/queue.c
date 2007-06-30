/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2003--2007  Petter Urkedal <urkedal@nbi.dk>
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

#include <cucon/queue.h>
#include <cu/memory.h>
#include <assert.h>

#define CHUNK_SIZE 4096

void
cucon_queue_cct(cucon_queue_t q)
{
    struct cucon_queue_node_s *new_node;
    q->front = q->back
	= new_node = cu_galloc(sizeof(struct cucon_queue_node_s) + CHUNK_SIZE);
    q->front_cur = new_node->begin = new_node->end = (char *)(new_node + 1);
    q->back_cap_end = (char *)(new_node + 1) + CHUNK_SIZE;
}

cucon_queue_t
cucon_queue_new()
{
    cucon_queue_t q = cu_gnew(struct cucon_queue_s);
    cucon_queue_cct(q);
    return q;
}

void *
cuconP_queue_push_mem(cucon_queue_t q, size_t size)
{
    struct cucon_queue_node_s *new_node;
    size_t alloc_size = size > CHUNK_SIZE? size : CHUNK_SIZE;
    q->back->end -= size;
    q->back = q->back->next = new_node
	= cu_galloc(sizeof(struct cucon_queue_node_s) + alloc_size);
    new_node->begin = (char *)(new_node + 1);
    new_node->end = (char *)(new_node + 1) + size;
    q->back_cap_end = (char *)(new_node + 1) + alloc_size;
    return new_node + 1;
}

void
cuconP_queue_pop_mem(cucon_queue_t q)
{
    struct cucon_queue_node_s *next;
    do {
	next = q->front->next;
	if (next == NULL) {
#ifndef CU_NDEBUG
	    if (q->front_cur != q->front->end)
		cu_bugf("cucon_queue_pop_mem asked to advance off "
			 "the queue.");
	    assert(q->front == q->back);
#endif
	    q->front_cur = q->front->begin = q->front->end
		= (void*)(q->front + 1);
	    break;
	}
	q->front_cur += next->begin - q->front->end;
	q->front = next;
    } while (q->front_cur >= next->end);
}
