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

#ifndef CUCON_HMAP_H
#define CUCON_HMAP_H

#include <cucon/fwd.h>
#include <cu/clos.h>

CU_BEGIN_DECLARATIONS
/** \defgroup cucon_hmap_h cucon/hmap.h: General-Purpose Hash Map
 ** @{ \ingroup cucon_maps_and_sets_mod
 **
 ** Defined here is a hash map where the keys are pointers with associated
 ** client-provided equality and hash functions.  The values are arbitrary
 ** sized slots, but shortcuts for pointer-valued maps are provided.
 **
 ** \see cucon_hset_h
 ** \see cucon_pmap_h
 **/

typedef struct cucon_hmap_node *cucon_hmap_node_t;
struct cucon_hmap_node
{
    cucon_hmap_node_t next;
    void const *key;
    /* value data */
};

struct cucon_hmap
{
    cu_clop(eq, cu_bool_t, void const *, void const *);
    cu_clop(hash, cu_hash_t, void const *);
    cucon_hmap_node_t *table;
    int size;
    cu_hash_t mask;
    struct cucon_hmap_node tail;
};

/** Initialise \a map as a hash set over objects with equality \a eq and hash
 ** function \a hash. */
void cucon_hmap_init(cucon_hmap_t map,
		     cu_clop(eq, cu_bool_t, void const *, void const *),
		     cu_clop(hash, cu_hash_t, void const *));

/** Return a hash set over objects with equality defined by \a eq and hash
 ** function \a hash. */
cucon_hmap_t cucon_hmap_new(cu_clop(eq, cu_bool_t, void const *, void const *),
			    cu_clop(hash, cu_hash_t, void const *));

/** Erase all entries in \a hash. */
void cucon_hmap_clear(cucon_hmap_t);

/** The number of elements of \a map. */
CU_SINLINE size_t
cucon_hmap_card(cucon_hmap_t map) { return map->size; }

/** True iff \a map is empty. */
CU_SINLINE cu_bool_t
cucon_hmap_is_empty(cucon_hmap_t map)
{ return map->size != 0; }

/** If an object equal to \a key is in \a map, return its value slot, else
 ** return \c NULL. */
void *cucon_hmap_find_mem(cucon_hmap_t map, void const *key);

/** If an object equal to \a key is in \a map, assume it maps to a slot
 ** containing a pointer, and return the pointer, otherwise return \c NULL. */
void *cucon_hmap_find_ptr(cucon_hmap_t map, void const *key);

/** If \a key is in \a map, return false and set \c *\a slot to its value slot,
 ** else return true and associate \a key with \a slot_size bytes of value slot
 ** assigned to \c *\a slot. */
cu_bool_t cucon_hmap_insert_mem(cucon_hmap_t map, void const *key,
				size_t slot_size, cu_ptr_ptr_t slot);

/** If \a key has a mapping in \a map, return the slot and erase it, otherwise
 ** return \c NULL. */
void *cucon_hmap_pop_mem(cucon_hmap_t map, void const *key);

/** If \a key has a mapping in \a map, return the single pointer which is
 ** assumed to be stored in the slot and delete the mapping, otherwise return
 ** \c NULL. */
void *cucon_hmap_pop_ptr(cucon_hmap_t map, void const *key);

/** Delete the mapping of \a key from \a map if it exists and return true,
 ** otherwise return false. */
CU_SINLINE cu_bool_t
cucon_hmap_erase(cucon_hmap_t map, void const *key)
{ return cucon_hmap_pop_mem(map, key) != NULL; }

/** Same as \ref cucon_hmap_pop_mem, except that the capacity is kept constant.
 ** Use in conjunction with \ref cucon_hmap_set_capacity as an optimisation. */
void *cucon_hmap_isocap_pop_mem(cucon_hmap_t map, void const *key);

/** Same as \ref cucon_hmap_pop_ptr, except that the capacity is kept constant.
 ** Use in conjunction with \ref cucon_hmap_set_capacity as an optimisation. */
void *cucon_hmap_isocap_pop_ptr(cucon_hmap_t map, void const *key);

/** Same as \ref cucon_hmap_erase, except that the capacity is kept constant.
 ** Use in conjunction with \ref cucon_hmap_set_capacity as an optimisation. */
CU_SINLINE cu_bool_t
cucon_hmap_isocap_erase(cucon_hmap_t map, void const *key)
{ return cucon_hmap_isocap_pop_mem(map, key) != NULL; }

/** Set the capacity of \a map to \a cap, which must be a power of 2.  This is
 ** used either as a preparation for subsequent insertions or as a final step
 ** after a series of calls to the "isocap" functions.  It is merely an
 ** optimisation to avoid unnecessary intermediate adjustments of the capacity
 ** when the final cardinality is known. */
void cucon_hmap_set_capacity(cucon_hmap_t map, size_t cap);

cu_bool_t cucon_hmap_conj_keys(cucon_hmap_t map,
			       cu_clop(f, cu_bool_t, void const *));

cu_bool_t cucon_hmap_conj_mem(cucon_hmap_t map,
			      cu_clop(f, cu_bool_t, void const *, void *));

#if 0
struct cucon_hmap_itr
{
    cucon_hmap_node_t *node_head;
    cucon_hmap_node_t node;
};
#endif

/** @} */

#if defined(CU_COMPAT) && CU_COMPAT < 20091116
#  define cucon_hmap_size cucon_hmap_card
#  define cucon_hmap_erase_keep_capacity cucon_hmap_isocap_pop_mem
#endif

CU_END_DECLARATIONS

#endif
