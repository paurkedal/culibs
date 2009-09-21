/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2009  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUCON_FIBHEAP_H
#define CUCON_FIBHEAP_H

#include <cucon/fwd.h>
#include <cu/inherit.h>
#include <cu/clos.h>
#include <cu/dlink.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cucon_fibheap_h cucon/fibheap.h: Fibonacci Heap
 ** @{ \ingroup cucon_misc_mod */

/** The type of ordering predicates for nodes of a Fibonacci heaps.  For
 ** efficiency it is recommended that it returns true for nodes of the same
 ** priority. */
    CU_DOXY_FAKED
typedef cu_clop(cucon_fibnode_prioreq_t, cu_bool_t,
		cucon_fibnode_t, cucon_fibnode_t)
    CU_DOXY_ENDFAKED(typedef __see_below__ cucon_fibnode_prioreq_t);

/** Base struct for Fibonacci heap nodes. */
struct cucon_fibnode
{
    cu_inherit (cu_dlink_s);
    cucon_fibnode_t children;
    cu_bool_t mark;
    int degree; /* The number of children. */
};

/** The Fibonacci heap struct. */
struct cucon_fibheap
{
    cucon_fibnode_prioreq_t prioreq;
    size_t card;
    cucon_fibnode_t min_root;
};

/** Initialise \a H as an empty Fibonacci heap ordered by \a prioreq. */
void cucon_fibheap_init(cucon_fibheap_t H, cucon_fibnode_prioreq_t prioreq);

/** Return an empty Fibonacci heap ordered by \a prioreq. */
cucon_fibheap_t cucon_fibheap_new(cucon_fibnode_prioreq_t prioreq);

/** Validate the internal consistency of \a H. */
void cucon_fibheap_validate(cucon_fibheap_t H);

/** True iff \a H is empty. */
CU_SINLINE cu_bool_t cucon_fibheap_is_empty(cucon_fibheap_t H)
{ return H->card == 0; }

/** The number of element in \a H. */
CU_SINLINE size_t cucon_fibheap_card(cucon_fibheap_t H) { return H->card; }

/** Swap the contents of two heaps. */
void cucon_fibheap_swap(cucon_fibheap_t H0, cucon_fibheap_t H1);

/** Insert \a node into \a H.  The \ref cucon_fibnode base struct will be
 ** initialised in the process. */
void cucon_fibheap_insert(cucon_fibheap_t H, cucon_fibnode_t node);

/** Merge elements of \a H1_d into \a H0, destructing \a H1_d in the process. */
void cucon_fibheap_union_d(cucon_fibheap_t H0, cucon_fibheap_t H1_d);

/** Return the node of the highest priority element of \a H. */
CU_SINLINE cucon_fibnode_t cucon_fibheap_front(cucon_fibheap_t H)
{ return H->min_root; }

/** Removes and returns the highest priority node of \a H, or return \c NULL if
 ** \a H is empty. */
cucon_fibnode_t cucon_fibheap_pop_front(cucon_fibheap_t H);

/** @} */
CU_END_DECLARATIONS

#endif
