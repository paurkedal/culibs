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

#ifndef CUCON_RBMAP_H
#define CUCON_RBMAP_H

/*!\defgroup cucon_rbmap_h cucon/rbmap.h: Maps, Red-Black Tree Implementation
 * @{\ingroup cucon_maps_and_sets_mod
 *
 * \see cucon_rbset_h
 * \see cucon_pmap_h
 */
#include <cucon/rbtree.h>
#include <cu/inherit.h>

CU_BEGIN_DECLARATIONS

struct cucon_rbmap_s
{
    cu_inherit (cucon_rbtree_s);
    cu_clop(cmp, int, void *, void *);
};

/*!Construct \a map where \a cmp is a total ordering over keys
 * as indicated by a return of -1 for ‘<’, 0 for ‘=’ and 1 for ‘>’. */
void cucon_rbmap_init(cucon_rbmap_t map, cu_clop(cmp, int, void *, void *));

/*!Return a map where keys are ordered according to \a cmp, which
 * returns -1 for ‘<’, 0 for ‘=’ and 1 for ‘>’. */
cucon_rbmap_t cucon_rbmap_new(cu_clop(cmp, int, void *, void *));

/*!The number of elements in \a map. */
CU_SINLINE size_t cucon_rbmap_size(cucon_rbmap_t map)
{ return cucon_rbtree_size(cu_to(cucon_rbtree, map)); }

/*!Return the slot for \a key in \a map, or \c NULL if not found. */
void *cucon_rbmap_find_mem(cucon_rbmap_t map, void *key);

/*!Return the pointer value of the slot of \a key in \a map, or \c NULL if
 * not found. */
void *cucon_rbmap_find_ptr(cucon_rbmap_t map, void *key);

/*!If \a key is not in \a map, then insert it with an associated slot
 * of \a slot_size bytes.  Returns true iff \a key was inserted.
 * In any case, return the slot in \c *\a slot. */
cu_bool_t cucon_rbmap_insert_mem(cucon_rbmap_t map, void *key,
			         size_t slot_size, cu_ptr_ptr_t slot);

/*!If \a key is not in \a map, then insert it with a pointer-valued
 * slot set to \a val.  Returns true iff \a key was inserted. */
cu_bool_t cucon_rbmap_insert_ptr(cucon_rbmap_t map, void *key, void *val);

/*!If \a key is in \a map, erase it and return true, else return false. */
cu_bool_t cucon_rbmap_erase(cucon_rbmap_t map, void *key);

/*!Sequentially conjunct \a cb over (key, slotptr)-pairs of \a map in order
 * as given by the ordering operator of \a map. */
cu_bool_t cucon_rbmap_conj_mem(cucon_rbmap_t map,
			       cu_clop(cb, cu_bool_t, void *key, void *val));

/*!Sequantially conjunct \a cb over pairs of keys and defererenced pointer
 * slots of \a map in order as given by the ordering operator of \a map. */
cu_bool_t cucon_rbmap_conj_ptr(cucon_rbmap_t map,
			       cu_clop(cb, cu_bool_t, void *key, void *val));

CU_END_DECLARATIONS
/*!@}*/
#endif
