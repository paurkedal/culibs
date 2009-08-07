/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2004--2007  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUCON_PRITREE_H
#define CUCON_PRITREE_H

#include <cucon/fwd.h>
#include <cu/clos.h>
#include <stdio.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cucon_pritree cucon/pritree.h: Priority Queues (Tree-Based, Priority Stored Directly)
 * @{\ingroup cucon_misc_mod */

struct cucon_pritree_s
{
    cucon_prinode_t front;
    size_t size;
};

struct cucon_prinode_s
{
    double priority;
    cucon_prinode_t prior;
    cucon_prinode_t left;
    cucon_prinode_t right;
};

/*!Construct an empty tree. */
void cucon_pritree_init(cucon_pritree_t pritree);

/*!Return an empty tree. */
cucon_pritree_t cucon_pritree_new(void);

/*!The front node of \a pretree. */
#define cucon_pritree_front(pritree) \
	(CU_MARG(cucon_pritree_t, pritree)->front)

/*!True iff \a pritree is empty. */
#define cucon_pritree_is_empty(pritree) \
	(CU_MARG(cucon_pritree_t, pritree)->front==NULL)

/*!The number of nodes in \a pretree. */
#define cucon_pritree_size(pritree) (CU_MARG(cucon_pritree_t, pritree)->size)

/*!Pop off the front of \a pretree.
 * \pre \a pritree is non-empty. */
void cucon_pritree_pop(cucon_pritree_t pritree);

/*!Construct \a prinode with priority \a priority and insert it into
 * \a pritree. */
void cucon_pritree_insert_init(cucon_pritree_t pritree,
			       cucon_prinode_t prinode,
			       double priority);

/*!Erase \a prinode from \a pritree. */
void cucon_pritree_erase(cucon_pritree_t pritree, cucon_prinode_t prinode);

/*!Modify the priority of \a prinode to \a priority. */
void cucon_pritree_change_priority(cucon_pritree_t pritree,
				   cucon_prinode_t prinode,
				   double priority);

/*!Add a constant to the priorities of all nodes. */
void cucon_pritree_add_to_all_priorities(cucon_pritree_t pritree, double delta);

/*!The priority of \a prinode. */
#define cucon_prinode_priority(prinode) \
	(CU_MARG(cucon_prinode_t, prinode)->priority)

/*!Sequential conjunction over nodes in order from highest priority down to
 * \a min_priority. */
cu_bool_t cucon_prinode_conj_down_to(cucon_prinode_t prinode,
				     double min_priority,
				     cu_clop(cb, cu_bool_t, cucon_prinode_t));

/*!Debug dump. */
void cucon_pritree_dump_priorities(cucon_pritree_t pritree, FILE *out);

/*!@}*/
CU_END_DECLARATIONS

#endif
