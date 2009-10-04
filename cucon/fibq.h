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

#ifndef CUCON_FIBHEAPC_H
#define CUCON_FIBHEAPC_H

#include <cucon/fwd.h>
#include <cu/clos.h>
#include <cu/conf.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cucon_fibq_h cucon/fibq.h: Queue-Optimised Fibonacci Heap
 ** @{ \ingroup cucon_misc_mod
 **
 ** This is a variant of \ref cucon_fibheap_h "cucon/fibhap.h" which is
 ** optimised for queue only use.  That is, it does not provide functions to
 ** change priority of nodes or to remove nodes other that that of highest
 ** priority.
 **
 ** As opposed to \ref cucon_fibheap_h "cucon/fibheap.h", this Fibonacci heap
 ** implementation uses single links for the sibling node chain, and it has no
 ** parent links.  In total it has 3 word overhead per node, as compared to 5
 ** words for the doubly linked variant.  It also runs slightly faster. */

/** The node ordering function.  The function shall return true if the first
 ** argument is considered of equal or higher priority than the secord
 ** argument. */
CU_DOXY_FAKED
typedef cu_clop(cucon_fibq_prioreq_t, cu_bool_t,
		cucon_fibqnode_t, cucon_fibqnode_t)
CU_DOXY_ENDFAKED(typedef __see_below__ cucon_fibq_prioreq_t);

/** The base struct for node of \a cucon_fibq. */
struct cucon_fibqnode
{
    cucon_fibqnode_t siblings, children;
#if 2*CUCONF_SIZEOF_INT > CUCONF_SIZEOF_VOID_P
    cu_bool_least_t mark;
    short degree;
#else
    cu_bool_t mark;
    int degree;
#endif
};

/** The queue-optimised Fibonacci heap struct. */
struct cucon_fibq
{
    cucon_fibq_prioreq_t prioreq;
    size_t card;
    cucon_fibqnode_t roots;
    cucon_fibqnode_t *root_tail;
};

/** Initialise \a Q as an empty Fibonacci heap which orders its element by \a
 ** prioreq. */
void cucon_fibq_init(cucon_fibq_t Q, cucon_fibq_prioreq_t prioreq);

/** Return an empty Fibonacci heap which orders its elements by \a prioreq. */
cucon_fibq_t cucon_fibq_new(cucon_fibq_prioreq_t prioreq);

/** Validate the internal integrity of \a Q. */
void cucon_fibq_validate(cucon_fibq_t Q);

/** The cardinality of \a Q. */
CU_SINLINE size_t cucon_fibq_card(cucon_fibq_t Q) { return Q->card; }

/** Insert a node into \a Q, initialising the \ref cucon_fibqnode base struct
 ** in the process. */
void cucon_fibq_insert(cucon_fibq_t Q, cucon_fibqnode_t node);

/** Pop the highest priority node off \a Q. */
cucon_fibqnode_t cucon_fibq_pop_front(cucon_fibq_t Q);

/** Update \a Q0 to the union of \a Q0 and \a Q1_d, destructing \a Q1_d in the
 ** process. */
void cucon_fibq_union_d(cucon_fibq_t Q0, cucon_fibq_t Q1_d);

void cucon_fibq_filter_d(cucon_fibq_t Q,
			 cu_clop(f, cu_bool_t, cucon_fibqnode_t));

/** @} */
CU_END_DECLARATIONS

#endif
