/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2008--2009  Petter Urkedal <urkedal@nbi.dk>
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

#ifndef CUEX_OCCURTREE_H
#define CUEX_OCCURTREE_H

#include <cuex/iteration.h>
#include <cucon/ucset.h>
#include <stdio.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cuex_occurtree_h cuex/occurtree.h: Caching of Variable Occurences
 ** @{ \ingroup cuex_mod */

struct cuex_occurtree_s
{
    cuex_t e;
    cu_bool_least_t has_ref;
    int mu_height;
    cucon_ucset_t free_vars;
    cuex_occurtree_t sub[1];
};

/** Returns an occurtree treating μ-variables like regular variables.  If \a
 ** force_comm, then use commutative iteration for all compounds. */
cuex_occurtree_t cuex_folded_occurtree(cuex_t e, cu_bool_t force_comm);

/** Returns an occurtree where a free μ-variable also implies all variables
 ** which are free in the corresponding μ-bind.  If \a force_comm, then use the
 ** commutative iteration for all compounds. */
cuex_occurtree_t cuex_unfolded_occurtree(cuex_t e, cu_bool_t force_comm);

/** The expression from which \a tree was created. */
CU_SINLINE cuex_t
cuex_occurtree_expr(cuex_occurtree_t tree)
{ return tree->e; }

/** Returns occurtree for subexpression number \a i under \a tree. */
CU_SINLINE cuex_occurtree_t
cuex_occurtree_at(cuex_occurtree_t tree, cu_rank_t i)
{ return tree->sub[i]; }

/** True iff \a tree has no free variables. */
CU_SINLINE cu_bool_t
cuex_occurtree_is_closed(cuex_occurtree_t tree)
{ return cucon_ucset_is_empty(tree->free_vars); }

/** True iff \a tree has no free μ-variables. */
CU_SINLINE cu_bool_t
cuex_occurtree_is_muclosed(cuex_occurtree_t tree)
{ return tree->mu_height < 0; }

/** Returns the set of free variables in \a tree. */
CU_SINLINE cucon_ucset_t
cuex_occurtree_free_vars(cuex_occurtree_t tree)
{ return tree->free_vars; }

/** Remove unused μ-binding sites from \a tree and return the new root.  This
 ** destructs \a tree in the process. */
cuex_occurtree_t cuex_occurtree_prune_mu(cuex_occurtree_t tree,
					 cuex_opview_t view);

/** Debug printout of \a tree. */
void cuex_occurtree_dump(cuex_occurtree_t tree, FILE *out);

/** @} */
CU_END_DECLARATIONS

#endif
