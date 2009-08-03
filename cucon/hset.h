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

#ifndef CUCON_HSET_H
#define CUCON_HSET_H

#include <cucon/hmap.h>

CU_BEGIN_DECLARATIONS
/*!\defgroup cucon_hset_h cucon/hset.h: General Hash Set
 * @{\ingroup cucon_maps_and_sets_mod
 * \see cucon_hmap_h
 * \see cucon_pset_h
 */

struct cucon_hset_s
{
    struct cucon_hmap_s impl;
};

/*!Constructs a hash set with equality predicate \a eq and hash function \a
 * hash. */
CU_SINLINE void
cucon_hset_init(cucon_hset_t set,
		cu_clop(eq, cu_bool_t, void const *, void const *),
		cu_clop(hash, cu_hash_t, void const *))
{ cucon_hmap_init(&set->impl, eq, hash); }

/*!Returns a hash set with equality predicate \a eq and hash function
 * \a hash. */
CU_SINLINE cucon_hset_t
cucon_hset_new(cu_clop(eq, cu_bool_t, void const *, void const *),
	       cu_clop(hash, cu_hash_t, void const *))
{ return (cucon_hset_t)cucon_hmap_new(eq, hash); }

/*!True iff \a key is in \a set. */
CU_SINLINE cu_bool_t
cucon_hset_contains(cucon_hset_t set, void const *key)
{ return cucon_hmap_find_mem(&set->impl, key) != NULL; }

/*!If \a key is not in \a set, inserts it and returns true.  Otherwise, returns
 * false. */
CU_SINLINE cu_bool_t
cucon_hset_insert(cucon_hset_t set, void const *key)
{
    void *slot;
    return cucon_hmap_insert_mem(&set->impl, key, 0, &slot);
}

/*!If an key equal to \a key is in \a set, erase it and return true, else
 * return false. */
CU_SINLINE cu_bool_t
cucon_hset_erase(cucon_hset_t set, void const *key)
{ return cucon_hmap_erase(&set->impl, key) != NULL; }

CU_SINLINE cu_bool_t
cucon_hset_erase_keep_capacity(cucon_hset_t set, void const *key)
{ return cucon_hmap_erase_keep_capacity(&set->impl, key) != NULL; }

CU_SINLINE void
cucon_hset_set_capacity(cucon_hset_t set, int cap)
{ cucon_hmap_set_capacity(&set->impl, cap); }

CU_SINLINE size_t
cucon_hset_size(cucon_hset_t set)
{ return cucon_hmap_size(&set->impl); }

CU_SINLINE cu_bool_t
cucon_hset_is_empty(cucon_hset_t set)
{ return cucon_hmap_is_empty(&set->impl); }

CU_SINLINE cu_bool_t
cucon_hset_conj(cucon_hset_t set,
		cu_clop(f, cu_bool_t, void const *))
{ return cucon_hmap_conj_keys(&set->impl, f); }

/* Comparison of whole hash sets, assuming they have the same callbacks. */
cu_bool_t cucon_hset_eq(cucon_hset_t hs0, cucon_hset_t hs1);	/* = */
cu_bool_t cucon_hset_sub(cucon_hset_t hs0, cucon_hset_t hs1);	/* ⊂ */
cu_bool_t cucon_hset_subeq(cucon_hset_t hs0, cucon_hset_t hs1);	/* ⊆ */

/*!\deprecated Use cucon_hset_init */
#define cucon_hset_cct cucon_hset_init

/*!@}*/
CU_END_DECLARATIONS
#endif
