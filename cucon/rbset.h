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

#ifndef CUCON_RBSET_H
#define CUCON_RBSET_H

/*!\defgroup cucon_rbset_h cucon/rbset.h: Sets, Red-Black Tree Implementation
 * @{\ingroup cucon_maps_and_sets_mod
 * \see cucon_rbmap_h
 * \see cucon_hset_h
 * \see cucon_pset_h
 */

#include <cucon/fwd.h>
#include <cu/inherit.h>
#include <cucon/rbtree.h>

CU_BEGIN_DECLARATIONS

/* Red-black Tree Based Sets
 * ========================= */

struct cucon_rbset_s
{
    cu_inherit (cucon_rbtree_s);
    cu_clop(cmp, int, void *, void *);
};

/*!Construct \a rbset where \a cmp is a total ordering over elements
 * as indicated by a return of -1 for ‘<’, 0 for ‘=’, and 1 for ‘>’. */
void cucon_rbset_init(cucon_rbset_t rbset, cu_clop(cmp, int, void *, void *));

/*!Return set where elements are compared according to \a cmp (cf.
 * \ref cucon_rbset_init). */
cucon_rbset_t cucon_rbset_new(cu_clop(cmp, int, void *, void *));

/*!True iff \a set is empty. */
CU_SINLINE cu_bool_t cucon_rbset_is_empty(cucon_rbset_t set)
{ return cucon_rbtree_is_empty(cu_to(cucon_rbtree, set)); }

/*!Construct \a rbset as an empty set of strings, using \a cu_str_cmp. */
void cucon_rbset_init_str_cmp(cucon_rbset_t rbset);

/*!Return an empty set of string, using \a cu_str_cmp. */
cucon_rbset_t cucon_rbset_new_str_cmp(void);

/*!If \a key is not in \a rbset, insert it and return true, else return
 * false. */
CU_SINLINE cu_bool_t
cucon_rbset_insert(cucon_rbset_t rbset, void *key)
{
    return cucon_rbtree_insert2p_ptr(cu_upcast(cucon_rbtree_s, rbset),
				     rbset->cmp, &key);
}

/*!If \a key is in \a rbset, erase it and return true, else return false. */
CU_SINLINE cu_bool_t
cucon_rbset_erase(cucon_rbset_t rbset, void *key)
{
    return cucon_rbtree_erase2p(cu_upcast(cucon_rbtree_s, rbset),
				rbset->cmp, key) != NULL;
}

/*!Return stored element of \a rbset equal to \a key if found, else NULL. */
CU_SINLINE void *
cucon_rbset_find(cucon_rbset_t rbset, void *key)
{
    return cucon_rbtree_find2p_ptr(cu_upcast(cucon_rbtree_s, rbset),
				   rbset->cmp, key);
}

/*!Return the elements of \a set below, equal, and above \a key in \c *\a
 * below_out, \c *\a equal_out, and \c *\a above_out, respectively.  \c NULL
 * is returned where no applicable node exists if \a set. */
void cucon_rbset_nearest(cucon_rbset_t set, void *key,
			 cu_ptr_ptr_t below_out,
			 cu_ptr_ptr_t equal_out,
			 cu_ptr_ptr_t above_out);

/*!Applies \a cb to each element of \a set in order. */
CU_SINLINE void
cucon_rbset_iter(cucon_rbset_t set, cu_clop(cb, void, void *))
{ cucon_rbtree_iter_ptr(cu_to(cucon_rbtree, set), cb); }

/*!Applies \a cb to each element of \a set in reverse order. */
CU_SINLINE void
cucon_rbset_rev_iter(cucon_rbset_t set, cu_clop(cb, void, void *))
{ cucon_rbtree_rev_iter_ptr(cu_to(cucon_rbtree, set), cb); }

/*!Applies \a cb to each element of \a set in order, exiting with false as soon
 * as \a cb returns false, otherwise returns true. */
CU_SINLINE cu_bool_t
cucon_rbset_conj(cucon_rbset_t set, cu_clop(cb, cu_bool_t, void *key))
{ return cucon_rbtree_conj_ptr(cu_to(cucon_rbtree, set), cb); }

/*!Applies \a cb to each element of \a set in reverse order, exiting
 * immediately with false if \a cb returns false, otherwise returns true. */
CU_SINLINE cu_bool_t
cucon_rbset_rev_conj(cucon_rbset_t set, cu_clop(cb, cu_bool_t, void *))
{ return cucon_rbtree_rev_conj_ptr(cu_to(cucon_rbtree, set), cb); }

/*!@}*/

#define cucon_rbset_cct		cucon_rbset_init
#define cucon_rbset_cct_str_cmp	cucon_rbset_init_str_cmp

CU_END_DECLARATIONS

#endif
