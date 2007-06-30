/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2002--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUCON_PRIQ_H
#define CUCON_PRIQ_H
/*!\defgroup cucon_priq cucon/priq.h: Priority Queues (Array-Based Implementation)
 * @{\ingroup cucon_misc_mod
 * This is an implementation of priority queues based on a pyramid-like
 * array.  It guarantees that the front element \a x fulfils
 * <tt>prior(x, y)</tt> for all other \c y in the queue.  */

#include <cucon/fwd.h>
#include <cu/clos.h>
#ifndef CU_NDEBUG
#include <stdio.h>
#endif

CU_BEGIN_DECLARATIONS

struct cucon_priq_s
{
    cu_clop(prior, cu_bool_t, void *, void *);
    size_t count;
    size_t capacity;
    void **arr;
};

typedef struct cucon_priq_s *cucon_priq_t;

/*!Construct \a q as a priority queue of elements with priority relation
 * \a prior. */
void cucon_priq_cct(cucon_priq_t q, cu_clop(prior, cu_bool_t, void *, void *));

/*!Return a priority queue with the priority relation \a prior. */
cucon_priq_t cucon_priq_new(cu_clop(prior, cu_bool_t, void *, void *));

/*!Construct \a q as a copy of \a src. */
void cucon_priq_cct_copy(cucon_priq_t q, cucon_priq_t src);

/*!Return the prior-relation, as passed to \c cucon_priq_cct. */
#define cucon_priq_prior(q) (CU_MARG(cucon_priq_t, q)->prior)

/*!Enqueue \a key. */
void cucon_priq_insert(cucon_priq_t q, void *key);

/*!Pop off and return the front element of the queue. */
void *cucon_priq_pop_front(cucon_priq_t q);

/*!The front element of the queue. */
CU_SINLINE void *cucon_priq_front(cucon_priq_t q) { return q->arr[0]; }

/*!True iff the queue is empty. */
CU_SINLINE cu_bool_t cucon_priq_is_empty(cucon_priq_t q)
{ return q->count == 0; }

/*!Return the number of elements in the queue. */
CU_SINLINE size_t cucon_priq_count(cucon_priq_t q) { return q->count; }

#ifndef CU_NDEBUG
/*!Debug dump. */
void cucon_priq_dump(cucon_priq_t q,
		     cu_clop(print_key_fn, void, void *key, FILE *out),
		     FILE *out);
#endif

/*!@}*/
CU_END_DECLARATIONS

#endif
