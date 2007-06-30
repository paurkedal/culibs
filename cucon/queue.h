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

#ifndef CUCON_QUEUE_H
#define CUCON_QUEUE_H

#include <cucon/fwd.h>
#include <cu/diag.h>

CU_BEGIN_DECLARATIONS
/*!\file
 * \deprecated Not sure if this will be kept. */

struct cucon_queue_node_s
{
    struct cucon_queue_node_s *next;
    char *begin;
    char *end;
};

typedef struct cucon_queue_s *cucon_queue_t;
struct cucon_queue_s
{
    char *front_cur;
    struct cucon_queue_node_s *front;
    struct cucon_queue_node_s *back;
    char *back_cap_end;
};

void cucon_queue_cct(cucon_queue_t q);
cucon_queue_t cucon_queue_new(void);

CU_SINLINE cu_bool_fast_t
cucon_queue_is_empty(cucon_queue_t q)
{
    return q->front_cur == q->back->end;
}

void cuconP_queue_pop_mem(cucon_queue_t q);
void *cuconP_queue_push_mem(cucon_queue_t q, size_t size);

/* Push 'size' bytes onto the queue and return a pointer to that
 * region. */
CU_SINLINE void *
cucon_queue_push_mem(cucon_queue_t q, size_t size)
{
    char *tmp = q->back->end;
    if ((q->back->end += size) > q->back_cap_end)
	return cuconP_queue_push_mem(q, size);
    else
	return tmp;
}

/* Return a pointer to the front element of the queue. */
#define cucon_queue_front_mem(q) ((void*)CU_MARG(cucon_queue_t, q)->front_cur)

/* Pop 'size' bytes off the queue. */
CU_SINLINE void
cucon_queue_pop_mem(cucon_queue_t q, size_t size)
{
#ifndef CU_NDEBUG
    if (q->front_cur == NULL && size != 0)
	cu_bugf("cucon_queue_pop_mem called on empty queue.");
#endif
    q->front_cur += size;
    if (q->front_cur >= q->front->end)
	cuconP_queue_pop_mem(q);
}

#define cucon_queue_front_ptr(q) (*(void **)cucon_queue_front_mem(q))
#define cucon_queue_pop_ptr(q) cucon_queue_pop_mem(q, sizeof(void *))
#define cucon_queue_push_ptr(q, ptr) \
	((void)(*(void **)cucon_queue_push_mem(q, sizeof(void *)) = (ptr)))

CU_END_DECLARATIONS

#endif
